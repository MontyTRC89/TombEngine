#include "../oldobjects.h"
#include "../../Game/sphere.h"
#include "../../Game/items.h"
#include "../../Game/tomb4fx.h"
#include "../../Game/effect2.h"
#include "../../Game/Box.h"
#include "../../Game/people.h"
#include "../../Game/debris.h"
#include "../../Game/draw.h"
#include "../../Game/control.h"
#include "../../Game/effects.h"

int NumBats;
BAT_STRUCT* Bats;

void InitialiseLittleBats(short itemNumber)
{
	ITEM_INFO* item = &Items[itemNumber];

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

	if (Objects[ID_BATS].loaded)
		ZeroMemory(Bats, NUM_BATS * sizeof(BAT_STRUCT));

	//LOWORD(item) = sub_402F27(ebx0, Bats, 0, 1920);
}

void ControlLittleBats(short itemNumber)
{
	ITEM_INFO* item = &Items[itemNumber];
	
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

short GetFreeBat()
{
	short batNumber = NumBats;
	int index = 0;
	BAT_STRUCT* bat = &Bats[NumBats];

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

	NumBats = (batNumber + 1) & (NUM_BATS - 1);

	return batNumber;
}

void TriggerLittleBat(ITEM_INFO* item)
{
	short batNumber = GetFreeBat();

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