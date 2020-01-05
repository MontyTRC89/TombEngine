#include "../newobjects.h"
#include "../oldobjects.h"
#include "../../Game/lara.h"
#include "../../Game/draw.h"
#include "../../Global/global.h"
#include "../../Game/items.h"
#include "../../Game/collide.h"
#include "../../Game/effects.h"
#include "../../Game/laramisc.h"
#include "../../Game/Box.h"

void InitialiseRaisingBlock(short itemNumber)
{
	ITEM_INFO* item = &Items[itemNumber];
	
	short roomNumber = item->roomNumber;
	FLOOR_INFO* floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);

	short boxIndex = floor->box;
	Boxes[boxIndex].overlapIndex &= ~BLOCKED;
	
	if (item->triggerFlags < 0)
	{
		item->aiBits |= (GUARD | FOLLOW | AMBUSH | PATROL1 | MODIFY);
		AddActiveItem(itemNumber);
		item->status = ITEM_ACTIVE;
	}
}

void ControlRaisingBlock(short itemNumber)
{
	ITEM_INFO* item = &Items[itemNumber];

	if (TriggerActive(item))
	{
		if (!item->itemFlags[2])
		{
			if (item->objectNumber == ID_RAISING_BLOCK1)
			{
				if (item->triggerFlags == -1)
				{
					AlterFloorHeight(item, -255);
				}
				else if (item->triggerFlags == -3)
				{
					AlterFloorHeight(item, -1023);
				}
				else
				{
					AlterFloorHeight(item, -1024);
				}
			}
			else
			{
				AlterFloorHeight(item, -2048);
			}

			item->itemFlags[2] = 1;
		}

		if (item->triggerFlags < 0)
		{
			item->itemFlags[1] = 1;
		}
		else if (item->itemFlags[1] < 4096)
		{
			SoundEffect(SFX_BLK_PLAT_RAISE_LOW, &item->pos, 0);

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
					AlterFloorHeight(item, 255);
					item->itemFlags[2] = 0;
				}
				else if (item->triggerFlags == -3)
				{
					AlterFloorHeight(item, 1023);
					item->itemFlags[2] = 0;
				}
				else
				{
					AlterFloorHeight(item, 1024);
				}
			}
			else
			{
				AlterFloorHeight(item, 2048);
			}

			item->itemFlags[2] = 0;
		}
	}
	else
	{
		SoundEffect(SFX_BLK_PLAT_RAISE_LOW, &item->pos, 0);

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
}