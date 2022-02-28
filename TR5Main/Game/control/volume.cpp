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


using TEN::Renderer::g_Renderer;

namespace TEN::Control::Volumes
{
	constexpr auto CAM_SIZE = 32;

	int CurrentCollidedVolume;

	void TestVolumes(short roomNumber, BoundingOrientedBox bbox, TriggerVolumeActivators activatorType)
	{
		CurrentCollidedVolume = 0;

		auto* room = &g_Level.Rooms[roomNumber];

		for (size_t i = 0; i < room->triggerVolumes.size(); i++)
		{
			auto volume = &room->triggerVolumes[i];

			if ((volume->activators & activatorType) != activatorType)
				continue;

			bool contains = false;

			switch (volume->type)
			{
			case VOLUME_BOX:
				if (roomNumber == Camera.pos.roomNumber)
					g_Renderer.addDebugBox(volume->box, Vector4(1.0f, 0.0f, 1.0f, 1.0f), RENDERER_DEBUG_PAGE::LOGIC_STATS);
				
				contains = volume->box.Intersects(bbox);
				break;

			case VOLUME_SPHERE:
				if (roomNumber == Camera.pos.roomNumber)
					g_Renderer.addDebugSphere(volume->sphere.Center, volume->sphere.Radius, Vector4(1.0f, 0.0f, 1.0f, 1.0f), RENDERER_DEBUG_PAGE::LOGIC_STATS);
				
				contains = volume->sphere.Intersects(bbox);
				break;
			}

			// TODO: Implement checks on which item is entering/inside/leaving volume
			// and pass item name or ID as argument for lua function, so it knows its caller.

			if (contains)
			{
				CurrentCollidedVolume = i + 1;

				if (volume->status == TriggerStatus::TS_OUTSIDE)
				{
					volume->status = TriggerStatus::TS_ENTERING;
					
					if (!volume->onEnter.empty())
						g_GameScript->ExecuteFunction(volume->onEnter);
				}
				else
				{
					volume->status = TriggerStatus::TS_INSIDE;
					
					if (!volume->onInside.empty())
						g_GameScript->ExecuteFunction(volume->onInside);
				}
			}
			else
			{
				if (volume->status == TriggerStatus::TS_INSIDE)
				{
					volume->status = TriggerStatus::TS_LEAVING;
					
					if (!volume->onLeave.empty())
						g_GameScript->ExecuteFunction(volume->onLeave);
				}
				else
					volume->status = TriggerStatus::TS_OUTSIDE;
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
		TestVolumes(camera->pos.roomNumber, bbox, TriggerVolumeActivators::FLYBYS);
	}

	void TestVolumes(short roomNumber, MESH_INFO* mesh)
	{
		auto* staticInfo = &StaticObjects[mesh->staticNumber];
		auto bbox = TO_DX_BBOX(mesh->pos, &staticInfo->collisionBox);

		TestVolumes(roomNumber, bbox, TriggerVolumeActivators::STATICS);
	}

	void TestVolumes(ITEM_INFO* item)
	{
		auto bbox = TO_DX_BBOX(item->Position, GetBoundsAccurate(item));

#ifdef _DEBUG
		g_Renderer.addDebugBox(bbox, Vector4(1.0f, 1.0f, 0.0f, 1.0f), RENDERER_DEBUG_PAGE::LOGIC_STATS);
#endif

		if (item->ObjectNumber == ID_LARA)
			TestVolumes(item->RoomNumber, bbox, TriggerVolumeActivators::PLAYER);
		else if (Objects[item->ObjectNumber].intelligent)
			TestVolumes(item->RoomNumber, bbox, TriggerVolumeActivators::NPC);
		else
			TestVolumes(item->RoomNumber, bbox, TriggerVolumeActivators::MOVEABLES);
	}
}
