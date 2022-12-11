#include "framework.h"
#include "Game/control/volume.h"

#include <filesystem>
#include "Game/animation.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/room.h"
#include "Game/savegame.h"
#include "ScriptInterfaceGame.h"
#include "Specific/setup.h"
#include "Renderer/Renderer11.h"
#include "Renderer/Renderer11Enums.h"

using TEN::Renderer::g_Renderer;

namespace TEN::Control::Volumes
{
	constexpr auto CAM_SIZE = 32;

	bool TestVolumeContainment(TriggerVolume& volume, BoundingOrientedBox& bbox, short roomNumber)
	{
		switch (volume.Type)
		{
		case VolumeType::Box:
			if (roomNumber == Camera.pos.RoomNumber)
				g_Renderer.AddDebugBox(volume.Box, Vector4(1.0f, 0.0f, 1.0f, 1.0f), RENDERER_DEBUG_PAGE::LARA_STATS);
			return volume.Box.Intersects(bbox);

		case VolumeType::Sphere:
			if (roomNumber == Camera.pos.RoomNumber)
				g_Renderer.AddDebugSphere(volume.Sphere.Center, volume.Sphere.Radius, Vector4(1.0f, 0.0f, 1.0f, 1.0f), RENDERER_DEBUG_PAGE::LARA_STATS);
			return volume.Sphere.Intersects(bbox);

		default:
			TENLog("Unsupported volume type encountered in room " + std::to_string(roomNumber), LogLevel::Error);
			return false;
		}
	}

	void HandleEvent(VolumeEvent& evt, VolumeTriggerer& triggerer)
	{
		if (!evt.Function.empty() && evt.CallCounter != 0)
		{
			g_GameScript->ExecuteFunction(evt.Function, triggerer, evt.Data);
			if (evt.CallCounter != NO_CALL_COUNTER)
				evt.CallCounter--;
		}
	}

	void TestVolumes(short roomNumber, BoundingOrientedBox bbox, VolumeActivatorType activatorType, VolumeTriggerer triggerer)
	{
		auto* room = &g_Level.Rooms[roomNumber];

		for (size_t i = 0; i < room->triggerVolumes.size(); i++)
		{
			auto* volume = &room->triggerVolumes[i];

			if (!volume->Enabled)
				continue;

			if (volume->EventSetIndex == NO_EVENT_SET)
				continue;

			auto* set = &g_Level.EventSets[volume->EventSetIndex];

			if ((set->Activators & activatorType) != activatorType)
				continue;

			VolumeState* entry = nullptr;

			for (int j = volume->StateQueue.size() - 1; j >= 0; j--)
			{
				VolumeState* candidate = &volume->StateQueue[j];

				switch (candidate->Status)
				{
				case VolumeStateStatus::Leaving:
					if (GameTimer - candidate->Timestamp > VOLUME_BUSY_TIMEOUT)
					{
						candidate->Status = VolumeStateStatus::Outside;
					}
					break;

				case VolumeStateStatus::Outside:
					volume->StateQueue.erase(volume->StateQueue.begin() + j);
					break;

				default:
					if (candidate->Triggerer == triggerer)
					{
						entry = candidate;
					}
					break;
				}
			}

			if (TestVolumeContainment(*volume, bbox, roomNumber))
			{
				if (entry == nullptr)
				{
					volume->StateQueue.push_back(
						VolumeState
						{ 
							VolumeStateStatus::Entering,
							triggerer, 
							GameTimer 
						});

					HandleEvent(set->OnEnter, triggerer);
				}
				else
				{
					entry->Status = VolumeStateStatus::Inside;
					entry->Timestamp = GameTimer;

					HandleEvent(set->OnInside, triggerer);
				}
			}
			else if (entry != nullptr)
			{
				// Only fire leave event when a certain timeout has passed.
				// This helps to filter out borderline cases when moving around volumes.

				if (GameTimer - entry->Timestamp > VOLUME_LEAVE_TIMEOUT)
				{
					entry->Status = VolumeStateStatus::Leaving;
					entry->Timestamp = GameTimer;

					HandleEvent(set->OnLeave, triggerer);
				}
			}
		}
	}
	
	void TestVolumes(CAMERA_INFO* camera)
	{
		auto pos = Pose(camera->pos.ToVector3i(), EulerAngles::Zero);
		auto box = GameBoundingBox::Zero;
		box.X1 = box.Y1 = box.Z1 =  CAM_SIZE;
		box.X2 = box.Y2 = box.Z2 = -CAM_SIZE;

		auto bBox = box.ToBoundingOrientedBox(pos);

		TestVolumes(camera->pos.RoomNumber, bBox, VolumeActivatorType::Flyby, camera);
	}

	void TestVolumes(short roomNumber, MESH_INFO* mesh)
	{
		const auto& bBox = GetBoundsAccurate(*mesh, false).ToBoundingOrientedBox(mesh->pos);
		
		TestVolumes(roomNumber, bBox, VolumeActivatorType::Static, mesh);
	}

	void TestVolumes(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];
		auto bBox = GameBoundingBox(item).ToBoundingOrientedBox(item->Pose);

		g_Renderer.AddDebugBox(bBox, Vector4(1.0f, 1.0f, 0.0f, 1.0f), RENDERER_DEBUG_PAGE::LARA_STATS);

		if (item->ObjectNumber == ID_LARA || item->Index == Lara.Vehicle)
			TestVolumes(item->RoomNumber, bBox, VolumeActivatorType::Player, itemNumber);
		else if (Objects[item->ObjectNumber].intelligent)
			TestVolumes(item->RoomNumber, bBox, VolumeActivatorType::NPC, itemNumber);
		else
			TestVolumes(item->RoomNumber, bBox, VolumeActivatorType::Moveable, itemNumber);
	}

	void InitialiseNodeScripts()
	{
		static const std::string nodeScriptPath = "Scripts/Engine/NodeCatalogs/";

		if (!std::filesystem::exists(nodeScriptPath))
			return;
		
		std::vector<std::string> nodeCatalogs;
		for (auto& path : std::filesystem::recursive_directory_iterator(nodeScriptPath))
			if (path.path().extension() == ".lua")
				nodeCatalogs.push_back(path.path().filename().string());

		if (nodeCatalogs.size() == 0)
			return;

		TENLog("Loading node scripts...", LogLevel::Info);

		std::sort(nodeCatalogs.rbegin(), nodeCatalogs.rend());
		for (auto& file : nodeCatalogs)
			g_GameScript->ExecuteScriptFile(nodeScriptPath + file);

		TENLog(std::to_string(nodeCatalogs.size()) + " node catalogs were found and loaded.", LogLevel::Info);

		int nodeCount = 0;
		for (auto& set : g_Level.EventSets)
		{
			if ((set.OnEnter.Mode == VolumeEventMode::Nodes) && (set.OnEnter.Data.size() > 0))
			{
				g_GameScript->ExecuteString(set.OnEnter.Data);
				nodeCount++;
			}

			if ((set.OnInside.Mode == VolumeEventMode::Nodes) && (set.OnInside.Data.size() > 0))
			{
				g_GameScript->ExecuteString(set.OnInside.Data);
				nodeCount++;
			}				

			if ((set.OnLeave.Mode == VolumeEventMode::Nodes) && (set.OnLeave.Data.size() > 0))
			{
				g_GameScript->ExecuteString(set.OnLeave.Data);
				nodeCount++;
			}
		}

		if (nodeCount != 0)
			TENLog(std::to_string(nodeCount) + " node scripts were found and loaded.", LogLevel::Info);
	}
}
