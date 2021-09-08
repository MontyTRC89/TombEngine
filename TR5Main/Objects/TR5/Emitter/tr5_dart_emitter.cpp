#include "framework.h"
#include "tr5_dart_emitter.h"
#include "level.h"
#include "lara.h"
#include "effects\effects.h"
#include "items.h"
#include "Sound\sound.h"

void DartControl(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	if (item->touchBits)
	{
		LaraItem->hitPoints -= 25;
		LaraItem->hitStatus = true;
		Lara.poisoned += 160;
		DoBloodSplat(item->pos.xPos, item->pos.yPos, item->pos.zPos, (GetRandomControl() & 3) + 4, LaraItem->pos.yRot, LaraItem->roomNumber);
		KillItem(itemNumber);
	}
	else
	{
		item->pos.xPos += item->speed * phd_sin(item->pos.yRot);
		item->pos.yPos -= item->speed * phd_sin(item->pos.xRot);
		item->pos.xPos += item->speed * phd_cos(item->pos.yRot);

		short roomNumber = item->roomNumber;
		FLOOR_INFO* floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);

		if (item->roomNumber != roomNumber)
			ItemNewRoom(itemNumber, roomNumber);

		int height = GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);
		item->floor = height;

		if (item->pos.yPos >= height)
		{
			for (int i = 0; i < 4; i++)
			{
				TriggerDartSmoke(item->pos.xPos, item->pos.yPos, item->pos.zPos, 0, 0, 1);
			}

			KillItem(itemNumber);
		}
	}
}

void DartEmitterControl(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	if (item->active)
	{
		if (item->timer > 0)
		{
			item->timer--;
			return;
		}
		else
		{
			item->timer = 24;
		}
	}

	short dartItemNumber = CreateItem();

	if (dartItemNumber != NO_ITEM)
	{
		ITEM_INFO* dartItem = &g_Level.Items[dartItemNumber];

		dartItem->objectNumber = ID_DARTS;
		dartItem->roomNumber = item->roomNumber;

		int x = 0;
		int z = 0;

		if (item->pos.yRot > 0)
		{
			if (item->pos.yRot == ANGLE(90.0f))
				x = 512;
		}
		else if (item->pos.yRot < 0)
		{
			if (item->pos.yRot == -ANGLE(180.0f))
			{
				z = -512;
			}
			else if (item->pos.yRot == -ANGLE(90.0f))
			{
				x = -512;
			}
		}
		else
		{
			z = 512;
		}

		dartItem->pos.xPos = x + item->pos.xPos;
		dartItem->pos.yPos = item->pos.yPos - 512;
		dartItem->pos.zPos = z + item->pos.zPos;

		InitialiseItem(dartItemNumber);

		dartItem->pos.xRot = 0;
		dartItem->pos.yRot = item->pos.yRot + -ANGLE(180);
		dartItem->speed = 256;

		int xf = 0;
		int zf = 0;

		if (x)
			xf = abs(2 * x) - 1;
		else
			zf = abs(2 * z) - 1;

		for (int i = 0; i < 5; i++)
		{
			int random = -GetRandomControl();

			int xv = 0;
			int zv = 0;

			if (z >= 0)
				zv = zf & random;
			else
				zv = -(zf & random);

			if (x >= 0)
				xv = xf & random;
			else
				xv = -(xf & random);

			TriggerDartSmoke(dartItem->pos.xPos, dartItem->pos.yPos, dartItem->pos.zPos, xv, zv, 0);
		}

		AddActiveItem(dartItemNumber);
		dartItem->status = ITEM_ACTIVE;
		SoundEffect(SFX_TR4_DART_SPITT, &dartItem->pos, 0);
	}
}