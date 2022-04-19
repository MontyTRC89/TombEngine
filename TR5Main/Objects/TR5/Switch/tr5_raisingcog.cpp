#include "framework.h"
#include "tr5_raisingcog.h"
#include "Specific/level.h"
#include "Game/control/control.h"
#include "Game/items.h"
#include "Sound/sound.h"
#include "Game/spotcam.h"
#include "Objects/objectslist.h"
#include "Objects/Generic/Switches/generic_switch.h"
#include "Game/animation.h"

using namespace TEN::Entities::Switches;

void InitialiseRaisingCog(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	short itemNos[32];
	int numSwitchItems = GetSwitchTrigger(item, itemNos, 1);

	if (numSwitchItems > 0)
	{
		for (int i = 0; i < numSwitchItems; i++)
		{
			ITEM_INFO* currentItem = &g_Level.Items[itemNos[i]];

			if (currentItem->objectNumber == ID_TRIGGER_TRIGGERER)
			{
				item->itemFlags[1] = currentItem->roomNumber;
			}

			if (currentItem->objectNumber == ID_PULLEY || currentItem->objectNumber == ID_TRIGGER_TRIGGERER)
			{
				currentItem->itemFlags[1] = 1;
				// FIXME: no more hardcoding!
				//PulleyItemNumber = itemNos[i];
			}
		}
	}
}

void RaisingCogControl(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	if (TriggerActive(item))
	{
		if (item->itemFlags[0] >= 3)
		{
			AnimateItem(item);
		}
		else
		{
			if (item->itemFlags[2] >= 256)
			{
				item->itemFlags[2] = 0;
				item->itemFlags[0]++;

				if (item->itemFlags[0] == 3)
				{
					short itemNos[32];
					short numItems = GetSwitchTrigger(item, itemNos, 1);

					if (numItems > 0)
					{
						for (int i = 0; i < numItems; i++)
						{
							ITEM_INFO* currentItem = &g_Level.Items[itemNos[i]];

							if (item->objectNumber == ID_PULLEY)
							{
								if (currentItem->roomNumber == item->itemFlags[1])
								{
									currentItem->itemFlags[1] = 0;
									currentItem->collidable = true;
								}
								else
								{
									currentItem->itemFlags[1] = 1;
								}
							}
							else if (item->objectNumber == ID_TRIGGER_TRIGGERER)
							{
								AddActiveItem(itemNos[i]);
								currentItem->status = ITEM_ACTIVE;
								currentItem->aiBits = (GUARD | MODIFY | AMBUSH | PATROL1 | FOLLOW);
							}
						}
					}
				}

				RemoveActiveItem(itemNumber);
				item->status = ITEM_NOT_ACTIVE;
				item->aiBits = 0;
			}
			else
			{
				if (!item->itemFlags[2])
				{
					InitialiseSpotCam(item->itemFlags[2]);
					UseSpotCam = 1;
				}

				int flags = 0;

				if (item->itemFlags[2] >= 31)
				{
					if (item->itemFlags[2] <= 224)
						flags = 31;
					else
						flags = 255 - item->itemFlags[2];
				}
				else
				{
					flags = item->itemFlags[2];
				}

				SoundEffect(SFX_TR4_BLK_PLAT_RAISE_AND_LOW,&item->pos, (flags * 256) | 8);

				item->itemFlags[2] += 2;
				item->pos.yPos -= 2;
			}
		}
	}
}