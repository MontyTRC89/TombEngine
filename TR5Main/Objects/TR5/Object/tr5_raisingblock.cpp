#include "framework.h"
#include "tr5_raisingblock.h"
#include "Game/items.h"
#include "Specific/level.h"
#include "Specific/setup.h"
#include "Game/collision/collide_room.h"
#include "Game/animation.h"
#include "Game/control/control.h"
#include "Game/control/box.h"
#include "Objects/objectslist.h"
#include "Sound/sound.h"
#include "Game/camera.h"
#include "Game/collision/floordata.h"

using namespace TEN::Floordata;

void InitialiseRaisingBlock(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	short roomNumber = item->roomNumber;
	FLOOR_INFO* floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
	if(floor->Box != NO_BOX)
		g_Level.Boxes[floor->Box].flags &= ~BLOCKED;

	// Set mutators to 0 by default
	for (int i = 0; i < item->mutator.size(); i++)
		item->mutator[i].Scale.y = 0;

	if (item->triggerFlags < 0)
	{
		item->aiBits |= ALL_AIOBJ;
		AddActiveItem(itemNumber);
		item->status = ITEM_ACTIVE;
	}

	TEN::Floordata::UpdateBridgeItem(itemNumber);
}

void ControlRaisingBlock(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	if (TriggerActive(item))
	{
		if (!item->itemFlags[2])
		{
			if (item->objectNumber == ID_RAISING_BLOCK1)
			{
				if (item->triggerFlags == -1)
				{
					//AlterFloorHeight(item, -255);
				}
				else if (item->triggerFlags == -3)
				{
					//AlterFloorHeight(item, -1023);
				}
				else
				{
					//AlterFloorHeight(item, -item->itemFlags[7]);
				}
			}
			else
			{
				//AlterFloorHeight(item, -item->itemFlags[7]);
			}

			item->itemFlags[2] = 1;
		}

		if (item->triggerFlags < 0)
		{
			item->itemFlags[1] = 1;
		}
		else if (item->itemFlags[1] < 4096)
		{
			SoundEffect(SFX_TR4_BLK_PLAT_RAISE_AND_LOW, &item->pos, 0);

			item->itemFlags[1] += 64;

			if (item->triggerFlags > 0)
			{
				if (abs(item->pos.xPos - Camera.pos.x) < 10240 &&
					abs(item->pos.xPos - Camera.pos.x) < 10240 &&
					abs(item->pos.xPos - Camera.pos.x) < 10240)
				{
					if (item->itemFlags[1] == 64 || item->itemFlags[1] == 4096)
						Camera.bounce = -32;
					else
						Camera.bounce = -16;
				}
			}
		}
	}
	else if (item->itemFlags[1] <= 0 || item->triggerFlags < 0)
	{
		if (item->itemFlags[2])
		{
			item->itemFlags[1] = 0;

			if (item->objectNumber == ID_RAISING_BLOCK1)
			{
				if (item->triggerFlags == -1)
				{
					//AlterFloorHeight(item, 255);
					item->itemFlags[2] = 0;
				}
				else if (item->triggerFlags == -3)
				{
					//AlterFloorHeight(item, 1023);
					item->itemFlags[2] = 0;
				}
				else
				{
					//AlterFloorHeight(item, item->itemFlags[7]);
				}
			}
			else
			{
				//AlterFloorHeight(item, item->itemFlags[7]);
			}

			item->itemFlags[2] = 0;
		}
	}
	else
	{
		SoundEffect(SFX_TR4_BLK_PLAT_RAISE_AND_LOW, &item->pos, 0);

		if (item->triggerFlags >= 0)
		{
			if (abs(item->pos.xPos - Camera.pos.x) < 10240 &&
				abs(item->pos.xPos - Camera.pos.x) < 10240 &&
				abs(item->pos.xPos - Camera.pos.x) < 10240)
			{
				if (item->itemFlags[1] == 64 || item->itemFlags[1] == 4096)
					Camera.bounce = -32;
				else
					Camera.bounce = -16;
			}
		}

		item->itemFlags[1] -= 64;
	}

	// Update bone mutators
	if (item->triggerFlags > -1)
	{
		for (int i = 0; i < item->mutator.size(); i++)
			item->mutator[i].Scale = Vector3(1.0f, item->itemFlags[1] / 4096.0f, 1.0f);
	}
}

std::optional<int> RaisingBlockFloor(short itemNumber, int x, int y, int z)
{
	auto bboxHeight = GetBridgeItemIntersect(itemNumber, x, y, z, false);

	if (bboxHeight.has_value())
	{
		auto item = &g_Level.Items[itemNumber];

		auto bounds = GetBoundsAccurate(item);
		auto height = abs(bounds->Y2 - bounds->Y1);

		auto currentHeight = item->pos.yPos - height * item->itemFlags[1] / 4096;
		return std::optional{ currentHeight };
	}

	return bboxHeight;
}

std::optional<int> RaisingBlockCeiling(short itemNumber, int x, int y, int z)
{
	auto bboxHeight = GetBridgeItemIntersect(itemNumber, x, y, z, true);

	if (bboxHeight.has_value())
		return std::optional{ bboxHeight.value() + 1 };
	else 
		return bboxHeight;
}

int RaisingBlockFloorBorder(short itemNumber)
{
	return GetBridgeBorder(itemNumber, false);
}

int RaisingBlockCeilingBorder(short itemNumber)
{
	return GetBridgeBorder(itemNumber, true);
}
