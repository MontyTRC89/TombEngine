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

struct LASER_HEAD_INFO
{
	short baseItem;
	short arcsItems[8];
	short puzzleItem;
};

struct LASER_HEAD_STRUCT
{
	PHD_VECTOR pos;
	ENERGY_ARC* fireArcs[2];
	ENERGY_ARC* chargeArcs[8];
	bool LOS;
	byte byte1;
	byte byte2;
	short unk1;
	short unk2;
};

LASER_HEAD_STRUCT LaserHeadData;

void InitialiseLaserHead(short itemNumber)
{
	ITEM_INFO* item = &Items[itemNumber];

	item->data = (LASER_HEAD_INFO*)GameMalloc(sizeof(LASER_HEAD_INFO));
	LASER_HEAD_INFO* info = (LASER_HEAD_INFO*)item->data;

	for (int i = 0; i < LevelItems; i++)
	{
		if (Items[i].objectNumber == ID_ANIMATING12)
		{
			info->baseItem = i;
			break;
		}
	}

	short rotation = 0;
	int j = 0;
	for (int i = 0; i < LevelItems; i++)
	{
		// ID_WINGED_MUMMY beccause we recycled MIP slots
		if (Items[i].objectNumber == ID_WINGED_MUMMY && Items[i].pos.yRot == rotation)
		{
			info->arcsItems[j] = i;
			rotation += ANGLE(45);
		}
	}

	for (int i = 0; i < LevelItems; i++)
	{
		if (Items[i].objectNumber == ID_PUZZLE_ITEM4)
		{
			info->puzzleItem = i;
			break;
		}
	}

	item->pos.yPos -= 640;
	item->itemFlags[1] = item->pos.yPos - 640;
	item->currentAnimState = 0;
	item->itemFlags[3] = 90;

	ZeroMemory(&LaserHeadData, sizeof(LASER_HEAD_STRUCT));
}

void ControlLaserHead(short itemNumber)
{

}