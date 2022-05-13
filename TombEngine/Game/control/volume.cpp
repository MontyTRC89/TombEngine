#include "framework.h"
#include "Game/control/volume.h"

#include "Game/animation.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/room.h"
#include "Game/savegame.h"
#include "Specific/setup.h"
#include "Renderer/Renderer11Enums.h"
#include "Renderer/Renderer11.h"
#include "ScriptInterfaceGame.h"

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

			if ((volume->Activators & activatorType) != activatorType)
				continue;

			bool contains = false;

			switch (volume->Type)
			{
			case TriggerVolumeType::Box:
				if (roomNumber == Camera.pos.roomNumber)
					g_Renderer.AddDebugBox(volume->Box, Vector4(1.0f, 0.0f, 1.0f, 1.0f), RENDERER_DEBUG_PAGE::LOGIC_STATS);
				contains = volume->Box.Intersects(bbox);
				break;

			case TriggerVolumeType::Sphere:
				if (roomNumber == Camera.pos.roomNumber)
					g_Renderer.AddDebugSphere(volume->Sphere.Center, volume->Sphere.Radius, Vector4(1.0f, 0.0f, 1.0f, 1.0f), RENDERER_DEBUG_PAGE::LOGIC_STATS);
				contains = volume->Sphere.Intersects(bbox);
				break;
			}

			if (contains)
			{
				CurrentCollidedVolume = i + 1;

				if (volume->Status == TriggerStatus::Outside)
				{
					volume->Status = TriggerStatus::Entering;
					if (!volume->OnEnter.empty())
						g_GameScript->ExecuteFunction(volume->OnEnter, triggerer);
				}
				else
				{
					volume->Status = TriggerStatus::Inside;
					if (!volume->OnInside.empty())
						g_GameScript->ExecuteFunction(volume->OnInside, triggerer);
				}
			}
			else
			{
				if (volume->Status == TriggerStatus::Inside)
				{
					volume->Status = TriggerStatus::Leaving;
					if (!volume->OnLeave.empty())
						g_GameScript->ExecuteFunction(volume->OnLeave, triggerer);
				}
				else
					volume->Status = TriggerStatus::Outside;
			}
		}
	}

	void TestVolumes(CAMERA_INFO* camera)
	{
		auto pos = PHD_3DPOS(camera->pos.x, camera->pos.y, camera->pos.z, 0, 0, 0);
		auto box = BOUNDING_BOX();
		box.X1 = box.Y1 = box.Z1 =  CAM_SIZE;
		box.X2 = box.Y2 = box.Z2 = -CAM_SIZE;

		auto bbox = TO_DX_BBOX(pos, &box);

		TestVolumes(camera->pos.roomNumber, bbox, TriggerVolumeActivators::Flyby, camera);
	}

	void TestVolumes(short roomNumber, MESH_INFO* mesh)
	{
		auto* staticInfo = &StaticObjects[mesh->staticNumber];
		auto bbox = TO_DX_BBOX(mesh->pos, &staticInfo->collisionBox);

		TestVolumes(roomNumber, bbox, TriggerVolumeActivators::Static, mesh);
	}

	void TestVolumes(short itemNum)
	{
		auto item = &g_Level.Items[itemNum];
		auto bbox = TO_DX_BBOX(item->Pose, GetBoundsAccurate(item));

#ifdef _DEBUG
		g_Renderer.AddDebugBox(bbox, Vector4(1.0f, 1.0f, 0.0f, 1.0f), RENDERER_DEBUG_PAGE::LOGIC_STATS);
#endif

		if (item->ObjectNumber == ID_LARA)
			TestVolumes(item->RoomNumber, bbox, TriggerVolumeActivators::Player, itemNum);
		else if (Objects[item->ObjectNumber].intelligent)
			TestVolumes(item->RoomNumber, bbox, TriggerVolumeActivators::NPC, itemNum);
		else
			TestVolumes(item->RoomNumber, bbox, TriggerVolumeActivators::Movable, itemNum);
	}
}
