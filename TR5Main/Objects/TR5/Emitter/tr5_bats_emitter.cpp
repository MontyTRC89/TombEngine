#include "framework.h"
#include "Objects/TR5/Emitter/tr5_bats_emitter.h"
#include "Specific/level.h"
#include "Game/control/control.h"
#include "Specific/setup.h"
#include "Game/effects/effects.h"
#include "Game/effects/tomb4fx.h"
#include "Sound/sound.h"
#include "Game/Lara/lara.h"
#include "Game/animation.h"
#include <Game\items.h>
#include "Game/items.h"

int NextBat;
BAT_STRUCT Bats[NUM_BATS];

void InitialiseLittleBats(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	if (item->pos.yRot == 0)
	{
		item->pos.zPos += 512;
	}
	else if (item->pos.yRot == -ANGLE(180))
	{
		item->pos.zPos -= 512;
	}
	else if (item->pos.yRot == -ANGLE(90))
	{
		item->pos.xPos -= 512;
	}
	else if (item->pos.yRot == ANGLE(90))
	{
		item->pos.xPos += 512;
	}

	if (Objects[ID_BATS_EMITTER].loaded)
		ZeroMemory(Bats, NUM_BATS * sizeof(BAT_STRUCT));

	//LOWORD(item) = sub_402F27(ebx0, Bats, 0, 1920);
}

void LittleBatsControl(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	if (TriggerActive(item))
	{
		if (item->triggerFlags)
		{
			TriggerLittleBat(item);
			item->triggerFlags--;
		}
		else
		{
			KillItem(itemNumber);
		}
	}
}

short GetNextBat()
{
	short batNumber = NextBat;
	int index = 0;
	BAT_STRUCT* bat = &Bats[NextBat];

	while (bat->on)
	{
		if (batNumber == NUM_BATS - 1)
		{
			bat = (BAT_STRUCT*)Bats;
			batNumber = 0;
		}
		else
		{
			batNumber++;
			bat++;
		}

		index++;

		if (index >= NUM_BATS)
			return NO_ITEM;
	}

	NextBat = (batNumber + 1) & (NUM_BATS - 1);

	return batNumber;
}

void TriggerLittleBat(ITEM_INFO* item)
{
	short batNumber = GetNextBat();

	if (batNumber != NO_ITEM)
	{
		BAT_STRUCT* bat = &Bats[batNumber];

		bat->roomNumber = item->roomNumber;
		bat->pos.xPos = item->pos.xPos;
		bat->pos.yPos = item->pos.yPos;
		bat->pos.zPos = item->pos.zPos;
		bat->pos.yRot = (GetRandomControl() & 0x7FF) + item->pos.yRot + -ANGLE(180) - 1024;
		bat->on = 1;
		bat->flags = 0;
		bat->pos.xRot = (GetRandomControl() & 0x3FF) - 512;
		bat->speed = (GetRandomControl() & 0x1F) + 16;
		bat->laraTarget = GetRandomControl() & 0x1FF;
		bat->counter = 20 * ((GetRandomControl() & 7) + 15);
	}
}

void UpdateBats()
{
	if (!Objects[ID_BATS_EMITTER].loaded)
		return;

	BOUNDING_BOX* bounds = GetBoundsAccurate(LaraItem);

	int x1 = LaraItem->pos.xPos + bounds->X1 - (bounds->X1 / 4);
	int x2 = LaraItem->pos.xPos + bounds->X2 - (bounds->X2 / 4);

	int y1 = LaraItem->pos.yPos + bounds->Y1 - (bounds->Y1 / 4);
	int y2 = LaraItem->pos.yPos + bounds->Y1 - (bounds->Y1 / 4);

	int z1 = LaraItem->pos.zPos + bounds->Z1 - (bounds->Z1 / 4);
	int z2 = LaraItem->pos.zPos + bounds->Z1 - (bounds->Z1 / 4);

	int minDistance = MAXINT;
	int minIndex = -1;

	for (int i = 0; i < NUM_BATS; i++)
	{
		BAT_STRUCT* bat = &Bats[i];

		if (!bat->on)
			continue;

		if ((Lara.burn || LaraItem->hitPoints <= 0)
			&& bat->counter > 90
			&& !(GetRandomControl() & 7))
			bat->counter = 90;

		if (!(--bat->counter))
		{
			bat->on = 0;
			continue;
		}

		if (!(GetRandomControl() & 7))
		{
			bat->laraTarget = GetRandomControl() % 640 + 128;
			bat->xTarget = (GetRandomControl() & 0x7F) - 64;
			bat->zTarget = (GetRandomControl() & 0x7F) - 64;
		}

		short angles[2];
		phd_GetVectorAngles(
			LaraItem->pos.xPos + 8 * bat->xTarget - bat->pos.xPos,
			LaraItem->pos.yPos - bat->laraTarget - bat->pos.yPos,
			LaraItem->pos.zPos + 8 * bat->zTarget - bat->pos.zPos,
			angles);

		int distance = SQUARE(LaraItem->pos.zPos - bat->pos.zPos) +
			SQUARE(LaraItem->pos.xPos - bat->pos.xPos);
		if (distance < minDistance)
		{
			minDistance = distance;
			minIndex = i;
		}

		distance = sqrt(distance) / 8;
		if (distance < 48)
			distance = 48;
		else if (distance > 128)
			distance = 128;

		if (bat->speed < distance)
			bat->speed++;
		else if (bat->speed > distance)
			bat->speed--;

		if (bat->counter > 90)
		{
			short speed = bat->speed * 128;

			short xAngle = abs(angles[1] - bat->pos.xRot) / 8;
			short yAngle = abs(angles[0] - bat->pos.yRot) / 8;

			if (xAngle < -speed)
				xAngle = -speed;
			else if (xAngle > speed)
				xAngle = speed;

			if (yAngle < -speed)
				yAngle = -speed;
			else if (yAngle > speed)
				yAngle = speed;

			bat->pos.yRot += yAngle;
			bat->pos.xRot += xAngle;
		}

		int sp = bat->speed * phd_cos(bat->pos.xRot);

		bat->pos.xPos += sp * phd_sin(bat->pos.yRot);
		bat->pos.yPos += bat->speed * phd_sin(-bat->pos.xRot);
		bat->pos.zPos += sp * phd_cos(bat->pos.yRot);

		if ((i % 2 == 0)
			&& bat->pos.xPos > x1
			&& bat->pos.xPos < x2
			&& bat->pos.yPos > y1
			&& bat->pos.yPos < y2
			&& bat->pos.zPos > z1
			&& bat->pos.zPos < z2)
		{
			TriggerBlood(bat->pos.xPos, bat->pos.yPos, bat->pos.zPos, 2 * GetRandomControl(), 2);
			if (LaraItem->hitPoints > 0)
				LaraItem->hitPoints -= 2;
		}
	}

	if (minIndex != -1)
	{
		BAT_STRUCT* bat = &Bats[minIndex];
		if (!(GetRandomControl() & 4))
			SoundEffect(157,&bat->pos, 0);
	}
}