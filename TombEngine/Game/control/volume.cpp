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

	int CurrentCollidedVolume;

	void TestVolumes(short roomNumber, BoundingOrientedBox bbox, TriggerVolumeActivators activatorType, VolumeTriggerer triggerer)
	{
		CurrentCollidedVolume = 0;

		auto* room = &g_Level.Rooms[roomNumber];

		for (size_t i = 0; i < room->triggerVolumes.size(); i++)
		{
			auto* volume = &room->triggerVolumes[i];

			if (volume->EventSetIndex == NO_EVENT_SET)
				continue;

			auto* set = &g_Level.EventSets[volume->EventSetIndex];

			if ((set->Activators & activatorType) != activatorType)
				continue;
			
			// Determine what to do if volume is busy with another triggerer.
			if (!std::holds_alternative<nullptr_t>(volume->Triggerer) && volume->Triggerer != triggerer)
			{
				if (GameTimer - volume->Timeout > VOLUME_BUSY_TIMEOUT)
				{
					// We are past the busy timeout, reset current triggerer and volume status.
					volume->Triggerer = nullptr;
					volume->Status = TriggerStatus::Outside;
				}
				else
					// We are in the same frame, triggerer is busy, leave it alone.
					continue;
			}

			bool contains = false;

			switch (volume->Type)
			{
			case TriggerVolumeType::Box:
				if (roomNumber == Camera.pos.RoomNumber)
					g_Renderer.AddDebugBox(volume->Box, Vector4(1.0f, 0.0f, 1.0f, 1.0f), RENDERER_DEBUG_PAGE::LOGIC_STATS);
				contains = volume->Box.Intersects(bbox);
				break;

			case TriggerVolumeType::Sphere:
				if (roomNumber == Camera.pos.RoomNumber)
					g_Renderer.AddDebugSphere(volume->Sphere.Center, volume->Sphere.Radius, Vector4(1.0f, 0.0f, 1.0f, 1.0f), RENDERER_DEBUG_PAGE::LOGIC_STATS);
				contains = volume->Sphere.Intersects(bbox);
				break;
			}

			if (contains)
			{
				CurrentCollidedVolume = i + 1;

				if (volume->Status == TriggerStatus::Outside)
				{
					volume->Triggerer = triggerer;
					volume->Timeout = GameTimer;
					volume->Status = TriggerStatus::Entering;
					if (!set->OnEnter.Function.empty() && set->OnEnter.CallCounter != 0)
					{
						g_GameScript->ExecuteFunction(set->OnEnter.Function, triggerer, set->OnEnter.Data);
						if (set->OnEnter.CallCounter != NO_CALL_COUNTER)
							set->OnEnter.CallCounter--;
					}
				}
				else
				{
					volume->Status = TriggerStatus::Inside;
					if (!set->OnInside.Function.empty() && set->OnInside.CallCounter != 0)
					{
						g_GameScript->ExecuteFunction(set->OnInside.Function, triggerer, set->OnInside.Data);
						if (set->OnInside.CallCounter != NO_CALL_COUNTER)
							set->OnInside.CallCounter--;
					}
				}
			}
			else
			{
				if (volume->Status == TriggerStatus::Inside)
				{
					// Only fire leave event when a certain timeout has passed.
					// This helps to filter out borderline cases when moving around volumes.

					if (GameTimer - volume->Timeout > VOLUME_LEAVE_TIMEOUT)
					{
						volume->Triggerer = nullptr;
						volume->Status = TriggerStatus::Leaving;
						if (!set->OnLeave.Function.empty() && set->OnLeave.CallCounter != 0)
						{
							g_GameScript->ExecuteFunction(set->OnLeave.Function, triggerer, set->OnLeave.Data);
							if (set->OnLeave.CallCounter != NO_CALL_COUNTER)
								set->OnLeave.CallCounter--;
						}
					}
				}
				else
					volume->Status = TriggerStatus::Outside;
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

		TestVolumes(camera->pos.RoomNumber, bBox, TriggerVolumeActivators::Flyby, camera);
	}

	void TestVolumes(short roomNumber, MESH_INFO* mesh)
	{
		const auto& bBox = GetBoundsAccurate(*mesh, false).ToBoundingOrientedBox(mesh->pos);
		
		TestVolumes(roomNumber, bBox, TriggerVolumeActivators::Static, mesh);
	}

	void TestVolumes(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];
		auto bBox = GameBoundingBox(item).ToBoundingOrientedBox(item->Pose);

#ifdef _DEBUG
		g_Renderer.AddDebugBox(bBox, Vector4(1.0f, 1.0f, 0.0f, 1.0f), RENDERER_DEBUG_PAGE::LOGIC_STATS);
#endif

		if (item->ObjectNumber == ID_LARA)
			TestVolumes(item->RoomNumber, bBox, TriggerVolumeActivators::Player, itemNumber);
		else if (Objects[item->ObjectNumber].intelligent)
			TestVolumes(item->RoomNumber, bBox, TriggerVolumeActivators::NPC, itemNumber);
		else
			TestVolumes(item->RoomNumber, bBox, TriggerVolumeActivators::Movable, itemNumber);
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
