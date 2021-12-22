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
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	if (TriggerActive(item))
	{
		SoundEffect(SFX_TR4_HELICOPTER_LOOP, &item->pos, 0);

		GAME_VECTOR pos;
		pos.x = ((GetRandomControl() & 0x1FF) - 255);
		pos.y = (GetRandomControl() & 0x1FF) - 255;
		pos.z = (GetRandomControl() & 0x1FF) - 255;
		GetLaraJointPosition((PHD_VECTOR*)&pos, LM_TORSO);

		GAME_VECTOR end = pos;

		if (!item->itemFlags[0] && !item->itemFlags[1] && !item->itemFlags[2])
		{
			item->itemFlags[0] = pos.x / 16;
			item->itemFlags[1] = pos.y / 16;
			item->itemFlags[2] = pos.z / 16;
		}

		pos.x = (pos.x + 80 * item->itemFlags[0]) / 6;
		pos.y = (pos.y + 80 * item->itemFlags[1]) / 6;
		pos.z = (pos.z + 80 * item->itemFlags[2]) / 6;

		item->itemFlags[0] = pos.x / 16;
		item->itemFlags[1] = pos.y / 16;
		item->itemFlags[2] = pos.z / 16;

		if (item->triggerFlags == 1)
			item->pos.zPos += (pos.z - item->pos.zPos) / 32;
		else
			item->pos.xPos += (pos.x - item->pos.xPos) / 32;
		item->pos.yPos += (pos.y - item->pos.yPos - 256) / 32;

		GAME_VECTOR start;
		start.x = GetRandomControl() + item->pos.xPos - 128;
		start.y = GetRandomControl() + item->pos.yPos - 128;
		start.z = GetRandomControl() + item->pos.zPos - 128;
		start.roomNumber = item->roomNumber;
		int los = LOS(&start, &end);

		end.x = 3 * pos.x - 2 * start.x;
		end.y = 3 * pos.y - 2 * start.y;
		end.z = 3 * pos.z - 2 * start.z;
		int los2 = LOS(&start, &end);

		if (los)
			GunShipCounter = 1;
		else
			GunShipCounter++;

		if (GunShipCounter <= 15)
			item->meshBits |= 0x100;
		else
			item->meshBits &= 0xFEFF;

		if (GunShipCounter < 15)
			SoundEffect(SFX_TR5_HK_FIRE, &item->pos, 0xC00004);

		if (!(GlobalCounter & 1))
			return AnimateItem(item);

		PHD_VECTOR hitPos;
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
					TestTriggers(hitMesh->pos.xPos, hitMesh->pos.yPos, hitMesh->pos.zPos, end.roomNumber, true);
					SoundEffect(GetShatterSound(hitMesh->staticNumber), &hitMesh->pos, 0);
				}

				TriggerRicochetSpark((GAME_VECTOR*)&hitPos, 2 * GetRandomControl(), 3, 0);
				TriggerRicochetSpark((GAME_VECTOR*)&hitPos, 2 * GetRandomControl(), 3, 0);
			}
		}
		else
		{
			ITEM_INFO* hitItem = &g_Level.Items[objOnLos];

			if (hitItem->objectNumber != ID_LARA)
			{
				if (hitItem->objectNumber >= ID_SMASH_OBJECT1
					&& hitItem->objectNumber <= ID_SMASH_OBJECT8)
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
					LaraItem->roomNumber);

				LaraItem->hitPoints -= 20;
			}
		}

		if (GunShipCounter < 15)
		{
			SPARKS* spark = &Sparks[GetFreeSpark()];
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
			spark->transType = TransTypeEnum::COLADD;
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