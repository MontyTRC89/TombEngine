#include "framework.h"
#include "tr5_gunship.h"
#include "Specific/level.h"
#include "Specific/setup.h"
#include "Game/control/los.h"
#include "Sound/sound.h"
#include "Game/animation.h"
#include "Game/camera.h"
#include "Game/effects/effects.h"
#include "Game/effects/debris.h"
#include "Objects/Generic/Object/objects.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"

int GunShipCounter = 0;

void ControlGunShip(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	if (TriggerActive(item))
	{
		SoundEffect(SFX_TR4_HELICOPTER_LOOP, &item->Pose);

		GameVector pos;
		pos.x = ((GetRandomControl() & 0x1FF) - 255);
		pos.y = (GetRandomControl() & 0x1FF) - 255;
		pos.z = (GetRandomControl() & 0x1FF) - 255;
		GetLaraJointPosition((Vector3Int*)&pos, LM_TORSO);

		GameVector end = pos;

		if (!item->ItemFlags[0] && !item->ItemFlags[1] && !item->ItemFlags[2])
		{
			item->ItemFlags[0] = pos.x / 16;
			item->ItemFlags[1] = pos.y / 16;
			item->ItemFlags[2] = pos.z / 16;
		}

		pos.x = (pos.x + 80 * item->ItemFlags[0]) / 6;
		pos.y = (pos.y + 80 * item->ItemFlags[1]) / 6;
		pos.z = (pos.z + 80 * item->ItemFlags[2]) / 6;

		item->ItemFlags[0] = pos.x / 16;
		item->ItemFlags[1] = pos.y / 16;
		item->ItemFlags[2] = pos.z / 16;

		if (item->TriggerFlags == 1)
			item->Pose.Position.z += (pos.z - item->Pose.Position.z) / 32;
		else
			item->Pose.Position.x += (pos.x - item->Pose.Position.x) / 32;
		item->Pose.Position.y += (pos.y - item->Pose.Position.y - 256) / 32;

		GameVector start;
		start.x = GetRandomControl() + item->Pose.Position.x - 128;
		start.y = GetRandomControl() + item->Pose.Position.y - 128;
		start.z = GetRandomControl() + item->Pose.Position.z - 128;
		start.roomNumber = item->RoomNumber;
		bool los = LOS(&start, &end);

		end.x = 3 * pos.x - 2 * start.x;
		end.y = 3 * pos.y - 2 * start.y;
		end.z = 3 * pos.z - 2 * start.z;
		bool los2 = LOS(&start, &end);

		if (los)
			GunShipCounter = 1;
		else
			GunShipCounter++;

		if (GunShipCounter <= 15)
			item->MeshBits |= 0x100;
		else
			item->MeshBits &= 0xFEFF;

		if (GunShipCounter < 15)
			SoundEffect(SFX_TR4_HK_FIRE, &item->Pose, SoundEnvironment::Land, 0.8f);

		if (!(GlobalCounter & 1))
			return AnimateItem(item);

		Vector3Int hitPos;
		MESH_INFO* hitMesh = NULL;
		int objOnLos = ObjectOnLOS2(&start, &end, &hitPos, &hitMesh, GAME_OBJECT_ID::ID_LARA);

		if (objOnLos == NO_LOS_ITEM || objOnLos < 0)
		{
			if (GunShipCounter >= 15)
				return AnimateItem(item);

			TriggerDynamicLight(
				start.x, start.y, start.z, 16,
				(GetRandomControl() & 0x3F) + 96,
				(GetRandomControl() & 0x1F) + 64,
				0);

			if (!los2)
			{
				TriggerRicochetSpark(&end, 2 * GetRandomControl(), 3, 0);
				TriggerRicochetSpark(&end, 2 * GetRandomControl(), 3, 0);
			}

			if (objOnLos < 0 && GetRandomControl() & 1)
			{
				if (StaticObjects[hitMesh->staticNumber].shatterType != SHT_NONE)
				{
					ShatterObject(0, hitMesh, 64, end.roomNumber, 0);
					hitMesh->flags &= ~StaticMeshFlags::SM_VISIBLE;
					TestTriggers(hitMesh->pos.Position.x, hitMesh->pos.Position.y, hitMesh->pos.Position.z, end.roomNumber, true);
					SoundEffect(GetShatterSound(hitMesh->staticNumber), &hitMesh->pos);
				}

				TriggerRicochetSpark((GameVector*)&hitPos, 2 * GetRandomControl(), 3, 0);
				TriggerRicochetSpark((GameVector*)&hitPos, 2 * GetRandomControl(), 3, 0);
			}
		}
		else
		{
			auto* hitItem = &g_Level.Items[objOnLos];

			if (hitItem->ObjectNumber != ID_LARA)
			{
				if (hitItem->ObjectNumber >= ID_SMASH_OBJECT1 &&
					hitItem->ObjectNumber <= ID_SMASH_OBJECT8)
				{
					ExplodeItemNode(hitItem, 0, 0, 128);
					SmashObject(objOnLos);
					KillItem(objOnLos);
				}
			}
			else
			{
				TriggerDynamicLight(
					start.x, start.y, start.z,
					16,
					(GetRandomControl() & 0x3F) + 96,
					(GetRandomControl() & 0x1F) + 64,
					0);

				DoBloodSplat(
					start.x, start.y, start.z,
					(GetRandomControl() & 1) + 2,
					2 * GetRandomControl(),
					LaraItem->RoomNumber);

				DoDamage(LaraItem, 20);
			}
		}

		if (GunShipCounter < 15)
		{
			auto* spark = GetFreeParticle();

			spark->on = 1;
			spark->sR = spark->dR = (GetRandomControl() & 0x7F) + -128;
			spark->sG = (spark->dR / 2) + (GetRandomControl() & 0x7F);

			if (spark->sG > spark->sR)
				spark->sG = spark->sR;

			spark->sB = 0;
			spark->dB = 0;
			spark->dR = 0;
			spark->dG = 0;
			spark->colFadeSpeed = 12;
			spark->blendMode = BLEND_MODES::BLENDMODE_ADDITIVE;
			spark->fadeToBlack = 0;
			spark->life = 12;
			spark->sLife = 12;
			spark->x = start.x;
			spark->y = start.y;
			spark->z = start.z;
			spark->xVel = 4 * (end.x - start.x);
			spark->yVel = 4 * (end.y - start.y);
			spark->zVel = 4 * (end.z - start.z);
			spark->friction = 0;
			spark->maxYvel = 0;
			spark->flags = SP_NONE;
		}

		AnimateItem(item);
	}
}
