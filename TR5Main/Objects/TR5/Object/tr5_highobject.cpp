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

		if (currentItem->ObjectNumber != ID_TRIGGER_TRIGGERER)
		{
			if (currentItem->ObjectNumber == ID_PUZZLE_ITEM4_COMBO2)
			{
				item->ItemFlags[3] |= (i * 256);
				currentItem->Position.yPos = item->Position.yPos - 512;
				continue;
			}
		}

		if (currentItem->TriggerFlags == 111)
		{
			item->ItemFlags[3] |= i;
			continue;
		}

		if (currentItem->TriggerFlags != 112)
		{
			if (currentItem->ObjectNumber == ID_PUZZLE_ITEM4_COMBO2)
			{
				item->ItemFlags[3] |= (i * 256);
				currentItem->Position.yPos = item->Position.yPos - 512;
				continue;
			}
		}
		else
		{
			x = currentItem->Position.xPos;
			y = currentItem->Position.yPos;
			z = currentItem->Position.zPos;
		}
	}

	for (int i = 0; i < g_Level.NumItems; i++)
	{
		ITEM_INFO* currentItem = &g_Level.Items[i];

		if (currentItem->ObjectNumber == ID_PULLEY
			&& currentItem->Position.xPos == x
			&& currentItem->Position.yPos == y
			&& currentItem->Position.zPos == z)
		{
			item->ItemFlags[2] |= i;
			break;
		}
	}
}

void ControlHighObject1(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	if (!TriggerActive(item))
	{
		if (item->ItemFlags[0] == 4)
		{
			item->ItemFlags[1]--;

			if (!item->ItemFlags[1])
			{
				ITEM_INFO* targetItem = &g_Level.Items[item->ItemFlags[3] & 0xFF];
				targetItem->Flags = (item->Flags & 0xC1FF) | 0x20;
				item->ItemFlags[0] = 6;
				item->ItemFlags[1] = 768;
				TestTriggers(item, true);
			}

			return;
		}

		if (item->ItemFlags[0] == 6)
		{
			item->ItemFlags[1] -= 8;

			if (item->ItemFlags[1] >= 0)
			{
				int flags = 0;

				if (item->ItemFlags[1] >= 256)
				{
					if (item->ItemFlags[1] <= 512)
						flags = 31;
					else
						flags = (768 - item->ItemFlags[1]) / 8;
				}
				else
				{
					flags = item->ItemFlags[1] / 8;
				}

				SoundEffect(SFX_TR4_BLK_PLAT_RAISE_AND_LOW, &item->Position, (flags * 256) | 8);

				item->Position.yPos += 8;

				ITEM_INFO* targetItem = &g_Level.Items[(item->ItemFlags[3] / 256) & 0xFF];
				targetItem->Flags |= 0x20u;
				targetItem->Position.yPos = item->Position.yPos - 560;
			}

			if (item->ItemFlags[1] < -60)
			{
				ITEM_INFO* targetItem = &g_Level.Items[item->ItemFlags[2] & 0xFF];
				targetItem->ItemFlags[1] = 0;
				targetItem->Flags |= 0x20u;
				item->ItemFlags[0] = 0;
				item->ItemFlags[1] = 0;

				RemoveActiveItem(itemNumber);

				item->Flags &= 0xC1FF;
				item->Status = ITEM_NOT_ACTIVE;

				return;
			}
		}
	}
	else if (item->ItemFlags[0] >= 3)
	{
		if (item->ItemFlags[0] == 4)
		{
			item->ItemFlags[0] = 5;
			item->ItemFlags[1] = 0;
		}
		else if (item->ItemFlags[0] == 5 && !item->ItemFlags[1] && g_Level.Items[(item->ItemFlags[3] / 256) & 0xFF].Flags < 0)
		{
			DoFlipMap(3);
			FlipMap[3] ^= 0x3E00u;
			item->ItemFlags[1] = 1;
		}
	}
	else
	{
		if (item->ItemFlags[1] >= 256)
		{
			item->ItemFlags[1] = 0;
			item->ItemFlags[0]++;

			if (item->ItemFlags[0] == 3)
			{
				item->ItemFlags[1] = 30 * item->TriggerFlags;
				item->ItemFlags[0] = 4;

				short targetItemNumber = item->ItemFlags[3] & 0xFF;
				ITEM_INFO* targetItem = &g_Level.Items[targetItemNumber];

				AddActiveItem(targetItemNumber);

				targetItem->Flags |= 0x3E20u;
				targetItem->Status = ITEM_ACTIVE;

				targetItemNumber = item->ItemFlags[2] & 0xFF;
				targetItem = &g_Level.Items[targetItemNumber];

				targetItem->ItemFlags[1] = 1;
				targetItem->Flags |= 0x20;
				targetItem->Flags &= 0xC1FF;

				return;
			}

			RemoveActiveItem(itemNumber);

			item->Flags &= 0xC1FF;
			item->Status = ITEM_NOT_ACTIVE;

			return;
		}

		int flags = 0;

		if (item->ItemFlags[1] >= 31)
		{
			if (item->ItemFlags[1] <= 224)
				flags = 31;
			else
				flags = 255 - item->ItemFlags[1];
		}
		else
		{
			flags = item->ItemFlags[1];
		}

		SoundEffect(SFX_TR4_BLK_PLAT_RAISE_AND_LOW, &item->Position, (flags * 256) | 8);

		item->ItemFlags[1] += 16;
		item->Position.yPos -= 16;

		short targetItemNumber = (item->ItemFlags[3] / 256) & 0xFF;
		ITEM_INFO* targetItem = &g_Level.Items[targetItemNumber];
		targetItem->Flags |= 0x20;
		targetItem->Position.yPos = item->Position.yPos - 560;
	}
}