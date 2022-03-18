#include "framework.h"
#include "tr5_highobject.h"
#include "Game/items.h"
#include "Specific/level.h"
#include "Game/room.h"
#include "Sound/sound.h"
#include "Objects/objectslist.h"

void InitialiseHighObject1(short itemNumber)
{
	int x = 0;
	int y = 0;
	int z = 0;

	ITEM_INFO* item = &g_Level.Items[itemNumber];

	for (int i = 0; i < g_Level.NumItems; i++)
	{
		ITEM_INFO* currentItem = &g_Level.Items[i];

		if (currentItem->objectNumber != ID_TRIGGER_TRIGGERER)
		{
			if (currentItem->objectNumber == ID_PUZZLE_ITEM4_COMBO2)
			{
				item->itemFlags[3] |= (i * 256);
				currentItem->pos.yPos = item->pos.yPos - 512;
				continue;
			}
		}

		if (currentItem->triggerFlags == 111)
		{
			item->itemFlags[3] |= i;
			continue;
		}

		if (currentItem->triggerFlags != 112)
		{
			if (currentItem->objectNumber == ID_PUZZLE_ITEM4_COMBO2)
			{
				item->itemFlags[3] |= (i * 256);
				currentItem->pos.yPos = item->pos.yPos - 512;
				continue;
			}
		}
		else
		{
			x = currentItem->pos.xPos;
			y = currentItem->pos.yPos;
			z = currentItem->pos.zPos;
		}
	}

	for (int i = 0; i < g_Level.NumItems; i++)
	{
		ITEM_INFO* currentItem = &g_Level.Items[i];

		if (currentItem->objectNumber == ID_PULLEY
			&& currentItem->pos.xPos == x
			&& currentItem->pos.yPos == y
			&& currentItem->pos.zPos == z)
		{
			item->itemFlags[2] |= i;
			break;
		}
	}
}

void ControlHighObject1(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	if (!TriggerActive(item))
	{
		if (item->itemFlags[0] == 4)
		{
			item->itemFlags[1]--;

			if (!item->itemFlags[1])
			{
				ITEM_INFO* targetItem = &g_Level.Items[item->itemFlags[3] & 0xFF];
				targetItem->flags = (item->flags & 0xC1FF) | 0x20;
				item->itemFlags[0] = 6;
				item->itemFlags[1] = 768;
				TestTriggers(item, true);
			}

			return;
		}

		if (item->itemFlags[0] == 6)
		{
			item->itemFlags[1] -= 8;

			if (item->itemFlags[1] >= 0)
			{
				int flags = 0;

				if (item->itemFlags[1] >= 256)
				{
					if (item->itemFlags[1] <= 512)
						flags = 31;
					else
						flags = (768 - item->itemFlags[1]) / 8;
				}
				else
				{
					flags = item->itemFlags[1] / 8;
				}

				SoundEffect(SFX_TR4_BLK_PLAT_RAISE_AND_LOW, &item->pos, (flags * 256) | 8);

				item->pos.yPos += 8;

				ITEM_INFO* targetItem = &g_Level.Items[(item->itemFlags[3] / 256) & 0xFF];
				targetItem->flags |= 0x20u;
				targetItem->pos.yPos = item->pos.yPos - 560;
			}

			if (item->itemFlags[1] < -60)
			{
				ITEM_INFO* targetItem = &g_Level.Items[item->itemFlags[2] & 0xFF];
				targetItem->itemFlags[1] = 0;
				targetItem->flags |= 0x20u;
				item->itemFlags[0] = 0;
				item->itemFlags[1] = 0;

				RemoveActiveItem(itemNumber);

				item->flags &= 0xC1FF;
				item->status = ITEM_NOT_ACTIVE;

				return;
			}
		}
	}
	else if (item->itemFlags[0] >= 3)
	{
		if (item->itemFlags[0] == 4)
		{
			item->itemFlags[0] = 5;
			item->itemFlags[1] = 0;
		}
		else if (item->itemFlags[0] == 5 && !item->itemFlags[1] && g_Level.Items[(item->itemFlags[3] / 256) & 0xFF].flags < 0)
		{
			DoFlipMap(3);
			FlipMap[3] ^= 0x3E00u;
			item->itemFlags[1] = 1;
		}
	}
	else
	{
		if (item->itemFlags[1] >= 256)
		{
			item->itemFlags[1] = 0;
			item->itemFlags[0]++;

			if (item->itemFlags[0] == 3)
			{
				item->itemFlags[1] = 30 * item->triggerFlags;
				item->itemFlags[0] = 4;

				short targetItemNumber = item->itemFlags[3] & 0xFF;
				ITEM_INFO* targetItem = &g_Level.Items[targetItemNumber];

				AddActiveItem(targetItemNumber);

				targetItem->flags |= 0x3E20u;
				targetItem->status = ITEM_ACTIVE;

				targetItemNumber = item->itemFlags[2] & 0xFF;
				targetItem = &g_Level.Items[targetItemNumber];

				targetItem->itemFlags[1] = 1;
				targetItem->flags |= 0x20;
				targetItem->flags &= 0xC1FF;

				return;
			}

			RemoveActiveItem(itemNumber);

			item->flags &= 0xC1FF;
			item->status = ITEM_NOT_ACTIVE;

			return;
		}

		int flags = 0;

		if (item->itemFlags[1] >= 31)
		{
			if (item->itemFlags[1] <= 224)
				flags = 31;
			else
				flags = 255 - item->itemFlags[1];
		}
		else
		{
			flags = item->itemFlags[1];
		}

		SoundEffect(SFX_TR4_BLK_PLAT_RAISE_AND_LOW, &item->pos, (flags * 256) | 8);

		item->itemFlags[1] += 16;
		item->pos.yPos -= 16;

		short targetItemNumber = (item->itemFlags[3] / 256) & 0xFF;
		ITEM_INFO* targetItem = &g_Level.Items[targetItemNumber];
		targetItem->flags |= 0x20;
		targetItem->pos.yPos = item->pos.yPos - 560;
	}
}