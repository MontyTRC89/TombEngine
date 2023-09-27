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
	constexpr auto CAM_SIZE = 32;

	bool TestVolumeContainment(const TriggerVolume& volume, const BoundingOrientedBox& box, short roomNumber)
	{
		float color = !volume.StateQueue.empty() ? 1.0f : 0.4f;

		switch (volume.Type)
		{
		case VolumeType::Box:
			if (roomNumber == Camera.pos.RoomNumber)
			{
				g_Renderer.AddDebugBox(volume.Box, 
					Vector4(color, 0.0f, color, 1.0f), RendererDebugPage::CollisionStats);
			}
			return volume.Box.Intersects(box);

		case VolumeType::Sphere:
			if (roomNumber == Camera.pos.RoomNumber)
			{
				g_Renderer.AddDebugSphere(volume.Sphere.Center, volume.Sphere.Radius, 
					Vector4(color, 0.0f, color, 1.0f), RendererDebugPage::CollisionStats);
			}
			return volume.Sphere.Intersects(box);

		default:
			TENLog("Unsupported volume type encountered in room " + std::to_string(roomNumber), LogLevel::Error);
			return false;
		}
	}

	BoundingOrientedBox ConstructRoughBox(ItemInfo& item, const CollisionSetup& coll)
	{
		auto pBounds = GameBoundingBox(&item).ToBoundingOrientedBox(item.Pose);
		auto pos = Vector3(item.Pose.Position.x, pBounds.Center.y, item.Pose.Position.z);
		auto rot = item.Pose.Orientation.ToQuaternion();
		return BoundingOrientedBox(pos, Vector3(coll.Radius, pBounds.Extents.y, coll.Radius), rot);
	}

	void HandleEvent(VolumeEvent& event, VolumeActivator& activator)
	{
		if (event.Function.empty() || event.CallCounter == 0)
			return;

		g_GameScript->ExecuteFunction(event.Function, activator, event.Data);
		if (event.CallCounter != NO_CALL_COUNTER)
			event.CallCounter--;
	}

	bool HandleEvent(const std::string& name, VolumeEventType eventType, VolumeActivator activator)
	{
		// Cache last used event sets so that whole list is not searched every time user calls this.
		static VolumeEventSet* lastEventSetPtr = nullptr;

		if (lastEventSetPtr != nullptr && lastEventSetPtr->Name != name)
			lastEventSetPtr = nullptr;

		if (lastEventSetPtr == nullptr)
		{
			for (auto& eventSet : g_Level.EventSets)
			{
				if (eventSet.Name == name)
				{
					lastEventSetPtr = &eventSet;
					break;
				}
			}
		}

		if (lastEventSetPtr != nullptr)
		{
			HandleEvent(lastEventSetPtr->Events[(int)eventType], activator);
			return true;
		}
	
		return false;
	}

	void TestVolumes(short roomNumber, const BoundingOrientedBox& box, VolumeActivatorFlags activatorFlag, VolumeActivator activator)
	{
		if (roomNumber == NO_ROOM)
			return;

		auto& room = g_Level.Rooms[roomNumber];

		for (auto& volume : room.triggerVolumes)
		{
			if (!volume.Enabled)
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

				volume.StateQueue.erase(std::remove_if(volume.StateQueue.begin(), volume.StateQueue.end(), 
					[](const VolumeState& obj) { return obj.Status == VolumeStateStatus::Outside; }),
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

					HandleEvent(set.Events[(int)VolumeEventType::Enter], activator);
				}
				else
				{
					entryPtr->Status = VolumeStateStatus::Inside;
					entryPtr->Timestamp = GameTimer;

					HandleEvent(set.Events[(int)VolumeEventType::Inside], activator);
				}
			}
			else if (entryPtr != nullptr)
			{
				// Only fire leave event when a certain timeout has passed.
				// This helps to filter out borderline cases when moving around volumes.

				if ((GameTimer - entryPtr->Timestamp) > VOLUME_LEAVE_TIMEOUT)
				{
					entryPtr->Status = VolumeStateStatus::Leaving;
					entryPtr->Timestamp = GameTimer;

					HandleEvent(set.Events[(int)VolumeEventType::Leave], activator);
				}
			}
		}
	}
	
	void TestVolumes(CAMERA_INFO* camera)
	{
		auto pose = Pose(camera->pos.ToVector3i(), EulerAngles::Zero);
		auto bounds = GameBoundingBox::Zero;
		bounds.X1 = bounds.Y1 = bounds.Z1 =  CAM_SIZE;
		bounds.X2 = bounds.Y2 = bounds.Z2 = -CAM_SIZE;

		auto box = bounds.ToBoundingOrientedBox(pose);

		TestVolumes(camera->pos.RoomNumber, box, VolumeActivatorFlags::Flyby, camera);
	}

	void TestVolumes(short roomNumber, MESH_INFO* mesh)
	{
		auto box = GetBoundsAccurate(*mesh, false).ToBoundingOrientedBox(mesh->pos);
		
		TestVolumes(roomNumber, box, VolumeActivatorFlags::Static, mesh);
	}

	void TestVolumes(short itemNumber, const CollisionSetup* coll)
	{
		auto& item = g_Level.Items[itemNumber];
		auto box = (coll != nullptr) ?
			ConstructRoughBox(item, *coll) : GameBoundingBox(&item).ToBoundingOrientedBox(item.Pose);

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

	void InitializeNodeScripts()
	{
		std::string nodeScriptPath = g_GameFlow->GetGameDir() + "Scripts/Engine/NodeCatalogs/";

		if (!std::filesystem::is_directory(nodeScriptPath))
			return;
		
		std::vector<std::string> nodeCatalogs;
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

            TENLog(std::to_string(nodeCatalogs.size()) + " node catalogs were found and loaded.", LogLevel::Info);
        }
        else
        {
            TENLog("No node catalogs were found.", LogLevel::Warning);
        }

		unsigned int nodeCount = 0;
		for (const auto& set : g_Level.EventSets)
		{
			if ((set.Events[(int)VolumeEventType::Enter].Mode == VolumeEventMode::Nodes) && 
				!set.Events[(int)VolumeEventType::Enter].Data.empty())
			{
				g_GameScript->ExecuteString(set.Events[(int)VolumeEventType::Enter].Data);
				nodeCount++;
			}

			if ((set.Events[(int)VolumeEventType::Inside].Mode == VolumeEventMode::Nodes) &&
				!set.Events[(int)VolumeEventType::Inside].Data.empty())
			{
				g_GameScript->ExecuteString(set.Events[(int)VolumeEventType::Inside].Data);
				nodeCount++;
			}				

			if ((set.Events[(int)VolumeEventType::Leave].Mode == VolumeEventMode::Nodes) && 
				!set.Events[(int)VolumeEventType::Leave].Data.empty())
			{
				g_GameScript->ExecuteString(set.Events[(int)VolumeEventType::Leave].Data);
				nodeCount++;
			}
		}

		if (nodeCount != 0)
			TENLog(std::to_string(nodeCount) + " node scripts were found and loaded.", LogLevel::Info);
	}
}
