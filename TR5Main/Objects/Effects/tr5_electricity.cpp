#include "framework.h"
#include "tr5_electricity.h"
#include "effects\effects.h"
#include "control/control.h"
#include "level.h"
#include "setup.h"
#include "animation.h"
#include "lara.h"
#include "lara_collide.h"
#include "Sound\sound.h"
#include "sphere.h"
#include "traps.h"
#include "Game/effects/lara_burn.h"
#include "items.h"

using namespace TEN::Effects::Fire;

void TriggerElectricityWiresSparks(int x, int z, byte objNum, byte node, int flags)
{
	SPARKS* spark = &Sparks[GetFreeSpark()];
	spark->on = true;
	spark->sR = 255;
	spark->sG = 255;
	spark->sB = 255;
	spark->dR = 0;
	spark->dG = (GetRandomControl() & 0x7F) + 64;
	spark->dB = 255;

	if (flags)
	{
		spark->colFadeSpeed = 1;
		spark->fadeToBlack = 0;
		spark->life = spark->sLife = 4;
	}
	else
	{
		spark->colFadeSpeed = 3;
		spark->fadeToBlack = 4;
		spark->life = spark->sLife = 16;
	}

	spark->fxObj = objNum;
	spark->transType = TransTypeEnum::COLADD;
	spark->flags = SP_ITEM | SP_NODEATTACH | SP_SCALE | SP_DEF;
	spark->nodeNumber = node;
	spark->x = x;
	spark->z = z;
	spark->y = 0;

	if (flags)
	{
		spark->xVel = 0;
		spark->yVel = 0;
		spark->zVel = 0;
	}
	else
	{
		spark->xVel = (GetRandomControl() & 0x1FF) - 256;
		spark->yVel = GetRandomControl() - 64;
		spark->zVel = (GetRandomControl() & 0x1FF) - 256;
	}
	spark->friction = 51;
	spark->maxYvel = 0;
	spark->gravity = 0;

	if (flags)
	{
		spark->scalar = 1;
		spark->def = Objects[ID_DEFAULT_SPRITES].meshIndex + 11;
		spark->size = spark->sSize = (GetRandomControl() & 0x1F) + 160;
	}
	else
	{
		spark->scalar = 0;
		spark->def = Objects[ID_DEFAULT_SPRITES].meshIndex + 14;
		spark->size = spark->sSize = (GetRandomControl() & 7) + 8;
	}

	spark->dSize = spark->size / 2;
}

void TriggerLaraElectricitySparks(int flame)
{
	PHD_VECTOR pos;
	pos.x = 0;
	pos.y = 0;
	pos.z = 0;
	GetLaraJointPosition(&pos, GetRandomControl() % 15);

	SPARKS* spark = &Sparks[GetFreeSpark()];

	spark->on = 1;
	spark->dR = 0;
	spark->colFadeSpeed = 8;
	byte color = (GetRandomControl() & 0x3F) - 64;
	spark->sR = color;
	spark->sB = color;
	spark->sG = color;
	spark->dB = color;
	spark->dG = color / 2;
	spark->transType = TransTypeEnum::COLADD;
	spark->fadeToBlack = 4;
	spark->life = 12;
	spark->sLife = 12;
	spark->x = pos.x;
	spark->y = pos.y;
	spark->z = pos.z;
	spark->xVel = 2 * (GetRandomControl() & 0x1FF) - 512;
	spark->yVel = 2 * (GetRandomControl() & 0x1FF) - 512;
	spark->zVel = 2 * (GetRandomControl() & 0x1FF) - 512;
	spark->friction = 51;
	spark->maxYvel = 0;
	spark->gravity = 0;
	spark->flags = SP_NONE;

	if (flame)
		TriggerFireFlame(pos.x, pos.y, pos.z, -1, 254);
}

static bool ElectricityWireCheckDeadlyBounds(PHD_VECTOR* pos, short delta)
{
	if (pos->x + delta >= DeadlyBounds[0] && pos->x - delta <= DeadlyBounds[1]
	&&  pos->y + delta >= DeadlyBounds[2] && pos->y - delta <= DeadlyBounds[3]
	&&  pos->z + delta >= DeadlyBounds[4] && pos->z - delta <= DeadlyBounds[5])
	{
		return true;
	}

	return false;
}

void ElectricityWiresControl(short itemNumber)
{
	bool flag = false;
	int counter = 3;

	ITEM_INFO* item = &g_Level.Items[itemNumber];

	if (item->itemFlags[0] > 2)
	{
		TriggerDynamicLight(
			LaraItem->pos.xPos,
			LaraItem->pos.yPos,
			LaraItem->pos.zPos,
			item->itemFlags[0],
			0,
			(GetRandomControl() & 0x1F) + 8 * item->itemFlags[0],
			(GetRandomControl() & 0x1F) + 8 * item->itemFlags[0]);

		item->itemFlags[0] -= 2;
	}

	if (TriggerActive(item))
	{
		SoundEffect(SFX_TR5_ELECTRIC_WIRES, &item->pos, 0);

		counter = (abs(LaraItem->pos.xPos - item->pos.xPos) > 2048)
			+ (abs(LaraItem->pos.zPos - item->pos.zPos) > 2048)
			+ (abs(LaraItem->pos.yPos - item->pos.yPos) > 4096);

		int x = (GetRandomControl() & 0x1F) - 16;
		int z = (GetRandomControl() & 0x1F) - 16;

		for (int i = 0; i < 3; i++)
		{
			if (GetRandomControl() & 1)
				TriggerElectricityWiresSparks(x, z, itemNumber, i + 2, 0);
		}

		if (!(GlobalCounter & 3))
		{
			TriggerElectricityWiresSparks(0, 0, itemNumber, 2, 1);
			TriggerElectricityWiresSparks(0, 0, itemNumber, 3, 1);
			TriggerElectricityWiresSparks(0, 0, itemNumber, 4, 1);
		}
	}
	else
	{
		flag = true;
	}

	AnimateItem(item);

	if (!Lara.burn && !flag && !counter)
	{
		GetLaraDeadlyBounds();

		for (int i = 2; i < 28; i += 3)
		{
			PHD_VECTOR pos;
			pos.x = 0;
			pos.y = 0;
			pos.z = 0;

			GetJointAbsPosition(item, &pos, i);

			if (ElectricityWireCheckDeadlyBounds(&pos, item->triggerFlags))
			{
				for (int i = 0; i < 48; i++)
					TriggerLaraElectricitySparks(0);

				item->itemFlags[0] = 28;
				LaraBurn();
				Lara.burnBlue = 1;
				Lara.burnCount = 48;
				LaraItem->hitPoints = 0;
			}
		}
	}

	int i = 8;
	int j = 0;
	counter = GlobalCounter % 3;
	short roomNumber = item->roomNumber;
	bool water = false;

	for (int i = 8; i < 27; i += 9, j++)
	{
		PHD_VECTOR pos;
		pos.x = 0;
		pos.y = 0;
		pos.z = 256;
		GetJointAbsPosition(item, &pos, i);

		if (GetRandomControl() & 1 && !flag)
		{
			TriggerDynamicLight(pos.x, pos.y, pos.z, 12, 0, ((GetRandomControl() & 0x3F) + 128) / 2, (GetRandomControl() & 0x3F) + 128);
		}

		roomNumber = item->roomNumber;
		GetFloor(pos.x, pos.y, pos.z, &roomNumber);
		ROOM_INFO* r = &g_Level.Rooms[roomNumber];

		if (r->flags & ENV_FLAG_WATER)
		{
			if (counter == j)
			{
				SetupRipple(pos.x, r->maxceiling, pos.z, (GetRandomControl() & 7) + 32, 16, Objects[ID_DEFAULT_SPRITES].meshIndex + SPR_RIPPLES);
			}

			water = true;
		}
	}

	if (!flag && !Lara.burn)
	{
		if (water)
		{
			int flipNumber = g_Level.Rooms[roomNumber].flipNumber;

			PHD_VECTOR pos1;
			pos1.x = 0;
			pos1.y = 0;
			pos1.z = 0;
			GetLaraJointPosition(&pos1, LM_LFOOT);

			short roomNumber1 = LaraItem->roomNumber;
			GetFloor(pos1.x, pos1.y, pos1.z, &roomNumber1);

			PHD_VECTOR pos2;
			pos2.x = 0;
			pos2.y = 0;
			pos2.z = 0;
			GetLaraJointPosition(&pos2, LM_RFOOT);

			short roomNumber2 = LaraItem->roomNumber;
			GetFloor(pos2.x, pos2.y, pos2.z, &roomNumber2);

			if (g_Level.Rooms[roomNumber1].flipNumber == flipNumber || g_Level.Rooms[roomNumber2].flipNumber == flipNumber)
			{
				if (LaraItem->hitPoints > 32)
				{
					SoundEffect(SFX_TR4_LARA_ELECTRIC_CRACKLES, &LaraItem->pos, 0);
					TriggerLaraElectricitySparks(0);
					TriggerLaraElectricitySparks(1);
					TriggerDynamicLight(pos1.x, pos1.y, pos1.z, 8, 0, GetRandomControl() & 0x7F, (GetRandomControl() & 0x3F) + 128);
					LaraItem->hitPoints -= 10;
				}
				else
				{
					item->itemFlags[0] = 28;
					LaraBurn();
					Lara.burnBlue = 1;
					Lara.burnCount = 48;
					LaraItem->hitPoints = 0;
				}
			}
		}
	}
}