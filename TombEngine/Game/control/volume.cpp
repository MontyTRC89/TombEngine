#include "framework.h"
#include "Game/control/volume.h"

#include <filesystem>

#include "Game/animation.h"
#include "Game/collision/collide_room.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/room.h"
#include "Game/savegame.h"
#include "Game/Setup.h"
#include "Renderer/Renderer11.h"
#include "Renderer/Renderer11Enums.h"
#include "Scripting/Include/ScriptInterfaceGame.h"

using TEN::Renderer::g_Renderer;

namespace TEN::Control::Volumes
{
	constexpr auto NO_EVENT_SET = -1;

	constexpr auto VOLUME_BUSY_TIMEOUT	= 10;
	constexpr auto VOLUME_LEAVE_TIMEOUT = 5;

	void InitializeNodeScripts()
	{
		auto nodeScriptPath = g_GameFlow->GetGameDir() + "Scripts/Engine/NodeCatalogs/";
		if (!std::filesystem::is_directory(nodeScriptPath))
			return;

		auto nodeCatalogs = std::vector<std::string>{};
		for (const auto& path : std::filesystem::recursive_directory_iterator(nodeScriptPath))
		{
			if (path.path().extension() == ".lua")
				nodeCatalogs.push_back(path.path().filename().string());
		}

		if (nodeCatalogs.empty())
			return;

		TENLog("Loading node scripts...", LogLevel::Info);

		std::sort(nodeCatalogs.rbegin(), nodeCatalogs.rend());

		if (!nodeCatalogs.empty())
		{
			for (const auto& file : nodeCatalogs)
				g_GameScript->ExecuteScriptFile(nodeScriptPath + file);

			TENLog(std::to_string(nodeCatalogs.size()) + " node catalogs found and loaded.", LogLevel::Info);
		}
		else
		{
			TENLog("Node catalogs not found.", LogLevel::Warning);
		}

		unsigned int nodeCount = 0;
		for (const auto& set : g_Level.EventSets)
		{
			if ((set.OnEnter.Mode == VolumeEventMode::Nodes) && !set.OnEnter.Data.empty())
			{
				g_GameScript->ExecuteString(set.OnEnter.Data);
				nodeCount++;
			}

			if ((set.OnInside.Mode == VolumeEventMode::Nodes) && !set.OnInside.Data.empty())
			{
				g_GameScript->ExecuteString(set.OnInside.Data);
				nodeCount++;
			}				

			if ((set.OnLeave.Mode == VolumeEventMode::Nodes) && !set.OnLeave.Data.empty())
			{
				g_GameScript->ExecuteString(set.OnLeave.Data);
				nodeCount++;
			}
		}

		if (nodeCount != 0)
			TENLog(std::to_string(nodeCount) + " node scripts found and loaded.", LogLevel::Info);
	}

	void HandleEvent(VolumeEvent& event, VolumeActivator& activator)
	{
		if (event.Function.empty() || event.CallCounter == 0)
			return;

		g_GameScript->ExecuteFunction(event.Function, activator, event.Data);
		if (event.CallCounter != NO_CALL_COUNTER)
			event.CallCounter--;
	}

	static bool TestVolumeContainment(const TriggerVolume& volume, const BoundingOrientedBox& box, int roomNumber)
	{
		float color = !volume.StateQueue.empty() ? 1.0f : 0.4f;

		switch (volume.Type)
		{
		case VolumeType::Box:
			if (roomNumber == Camera.pos.RoomNumber)
				g_Renderer.AddDebugBox(volume.Box, Vector4(color, 0.0f, color, 1.0f), RendererDebugPage::CollisionStats);

			return volume.Box.Intersects(box);

		case VolumeType::Sphere:
			if (roomNumber == Camera.pos.RoomNumber)
			{
				g_Renderer.AddDebugSphere(
					volume.Sphere.Center, volume.Sphere.Radius,
					Vector4(color, 0.0f, color, 1.0f), RendererDebugPage::CollisionStats);
			}

			return volume.Sphere.Intersects(box);

		default:
			TENLog("Unsupported volume type encountered in room " + std::to_string(roomNumber), LogLevel::Error);
			return false;
		}
	}

	void TestVolumes(int roomNumber, const BoundingOrientedBox& box, VolumeActivatorFlags activatorFlag, VolumeActivator activator)
	{
		if (roomNumber == NO_ROOM)
			return;

		auto& room = g_Level.Rooms[roomNumber];

		for (auto& volume : room.triggerVolumes)
		{
			if (!volume.IsEnabled)
				continue;

			if (volume.EventSetIndex == NO_EVENT_SET)
				continue;

			auto& set = g_Level.EventSets[volume.EventSetIndex];

			if (((int)set.Activators & (int)activatorFlag) != (int)activatorFlag)
				continue;

			VolumeState* entryPtr = nullptr;

			for (int j = (int)volume.StateQueue.size() - 1; j >= 0; j--)
			{
				auto& candidate = volume.StateQueue[j];

				if (candidate.Status == VolumeStateStatus::Leaving)
				{
					if ((GameTimer - candidate.Timestamp) > VOLUME_BUSY_TIMEOUT)
						candidate.Status = VolumeStateStatus::Outside;
				}
				else if (candidate.Status != VolumeStateStatus::Outside)
				{
					if (candidate.Activator == activator)
						entryPtr = &candidate;
				}

				volume.StateQueue.erase(
					std::remove_if(
						volume.StateQueue.begin(), volume.StateQueue.end(), 
						[](const VolumeState& object) { return (object.Status == VolumeStateStatus::Outside); }),
					volume.StateQueue.end());
			}

			if (TestVolumeContainment(volume, box, roomNumber))
			{
				if (entryPtr == nullptr)
				{
					volume.StateQueue.push_back(
						VolumeState
						{ 
							VolumeStateStatus::Entering,
							activator,
							GameTimer 
						});

					HandleEvent(set.OnEnter, activator);
				}
				else
				{
					entryPtr->Status = VolumeStateStatus::Inside;
					entryPtr->Timestamp = GameTimer;

					HandleEvent(set.OnInside, activator);
				}
			}
			else if (entryPtr != nullptr)
			{
				// Only fire leave event when certain timeout has passed.
				// Helps filter out borderline cases when moving around volumes.
				if ((GameTimer - entryPtr->Timestamp) > VOLUME_LEAVE_TIMEOUT)
				{
					entryPtr->Status = VolumeStateStatus::Leaving;
					entryPtr->Timestamp = GameTimer;

					HandleEvent(set.OnLeave, activator);
				}
			}
		}
	}

	static BoundingOrientedBox ConstructRoughBox(const ItemInfo& item, const CollisionSetup& coll)
	{
		auto playerBox = GameBoundingBox(&item).ToBoundingOrientedBox(item.Pose);

		auto pos = Vector3(item.Pose.Position.x, playerBox.Center.y, item.Pose.Position.z);
		auto extents = Vector3(coll.Radius, playerBox.Extents.y, coll.Radius);
		auto orient = item.Pose.Orientation.ToQuaternion();

		return BoundingOrientedBox(pos, extents, orient);
	}

	void TestVolumes(int itemNumber, const CollisionSetup* collPtr)
	{
		const auto& item = g_Level.Items[itemNumber];

		auto box = (collPtr != nullptr) ?
			ConstructRoughBox(item, *collPtr) :
			GameBoundingBox(&item).ToBoundingOrientedBox(item.Pose);

		g_Renderer.AddDebugBox(box, Vector4(1.0f, 1.0f, 0.0f, 1.0f), RendererDebugPage::CollisionStats);

		if (item.IsLara() || item.Index == Lara.Context.Vehicle)
		{
			TestVolumes(item.RoomNumber, box, VolumeActivatorFlags::Player, itemNumber);
		}
		else if (Objects[item.ObjectNumber].intelligent)
		{
			TestVolumes(item.RoomNumber, box, VolumeActivatorFlags::NPC, itemNumber);
		}
		else
		{
			TestVolumes(item.RoomNumber, box, VolumeActivatorFlags::Moveable, itemNumber);
		}
	}

	void TestVolumes(int roomNumber, MESH_INFO* meshPtr)
	{
		auto box = GetBoundsAccurate(*meshPtr, false).ToBoundingOrientedBox(meshPtr->pos);
		
		TestVolumes(roomNumber, box, VolumeActivatorFlags::Static, meshPtr);
	}

	void TestVolumes(CAMERA_INFO* cameraPtr)
	{
		constexpr auto CAMERA_EXTENT = 32.0f;

		// TODO: Camera box is currently AABB. Should derive orientation from look-at? -- Sezz 2023.08.11
		auto box = BoundingOrientedBox(cameraPtr->pos.ToVector3(), Vector3(CAMERA_EXTENT), Quaternion::Identity);
		TestVolumes(cameraPtr->pos.RoomNumber, box, VolumeActivatorFlags::Flyby, cameraPtr);
	}
}
