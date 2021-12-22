#include "framework.h"
#include "tr4_obelisk.h"
#include "Game/items.h"
#include "Specific/level.h"
#include "Specific/setup.h"
#include "Game/control/control.h"
#include "Sound/sound.h"
#include "Game/Lara/lara.h"
#include "Game/effects/effects.h"
#include "Specific/input.h"
#include "Game/animation.h"
#include "Game/effects/lightning.h"

using namespace TEN::Effects::Lightning;

void InitialiseObelisk(short itemNumber)
{
	ITEM_INFO* item;
	ITEM_INFO* item2;

	item = &g_Level.Items[itemNumber];
	item->animNumber = Objects[item->objectNumber].animIndex + 3;;
	item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
	AddActiveItem(itemNumber);
	item->status = ITEM_ACTIVE;

	if (item->triggerFlags == 2)
	{
		for (int i = 0; i < g_Level.NumItems; i++)
		{
			item2 = &g_Level.Items[i];

			if (item2->objectNumber == ID_OBELISK)
				item->itemFlags[0]++;

			if (item2->objectNumber == ID_ANIMATING3)
				item->itemFlags[2] = i;
		}
	}

}

void ObeliskControl(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];
	short someNumber;
	PHD_3DPOS pos;
	PHD_3DPOS pos2;

	if (TriggerActive(item))
	{
		if (item->itemFlags[3] > 346)
			return;

		item->itemFlags[3]++;

		byte r = (GetRandomControl() & 0x1F) + 224;
		byte g = r - (GetRandomControl() & 0x1F) - 32;
		byte b = g - (GetRandomControl() & 0x1F) - 128;

		if (!(GlobalCounter & 1))
		{
			someNumber = 8192;
			if (GetRandomControl() & 1)
			{
				if (item->itemFlags[3] < 256 
					&& (GetRandomControl() & 1) 
					&& !(GlobalCounter & 3))
				{
					SoundEffect(SFX_TR4_ELEC_ONE_SHOT, &item->pos, 0);
					someNumber = (GetRandomControl() & 0xFFF) + 3456;
				}

				pos.xPos = item->pos.xPos + (3456 * phd_sin(item->pos.yRot + ANGLE(90)));
				pos.yPos = item->pos.yPos - 256;
				pos.zPos = item->pos.zPos + (3456 * phd_cos(item->pos.yRot + ANGLE(90)));

				pos2.xPos = item->pos.xPos + (someNumber * phd_sin(item->pos.yRot + ANGLE(90)));
				pos2.yPos = item->pos.yPos;
				pos2.xPos = item->pos.zPos + (someNumber * phd_cos(item->pos.zRot + ANGLE(90)));

				if (abs(pos.xPos - LaraItem->pos.xPos) < SECTOR(20)
					&& abs(pos.yPos - LaraItem->pos.yPos) < SECTOR(20)
					&& abs(pos.zPos - LaraItem->pos.zPos) < SECTOR(20)
					&& abs(pos2.xPos - LaraItem->pos.xPos) < SECTOR(20)
					&& abs(pos2.yPos - LaraItem->pos.yPos) < SECTOR(20)
					&& abs(pos2.zPos - LaraItem->pos.zPos) < SECTOR(20))
				{
					if (!(GlobalCounter & 3))
					{
						TriggerLightning(
							(PHD_VECTOR*)&pos,
							(PHD_VECTOR*)&pos2,
							(GetRandomControl() & 0x1F) + 32,
							r,
							g,
							b,
							24,
							1,
							32,
							5);
					}

					TriggerLightningGlow(pos.xPos, pos.yPos, pos.zPos, 48, r, g, b);
				}
			}
		}

		if (item->itemFlags[3] >= 256 && item->triggerFlags == 2)
		{
			pos.xPos = item->pos.xPos + 8192 * phd_sin(item->pos.yRot);
			pos.yPos = item->pos.yPos;
			pos.zPos = item->pos.zPos + 8192 * phd_cos(item->pos.yRot + ANGLE(90));

			SoundEffect(SFX_TR4_ELEC_ARCING_LOOP, &pos, 0);

			if (GlobalCounter & 1)
			{
				pos2.xPos = (GetRandomControl() & 0x3FF) + pos.xPos - 512;
				pos2.yPos = (GetRandomControl() & 0x3FF) + pos.yPos - 512;
				pos2.zPos = (GetRandomControl() & 0x3FF) + pos.zPos - 512;

				if (abs(pos.xPos - LaraItem->pos.xPos) < SECTOR(20)
					&& abs(pos.yPos - LaraItem->pos.yPos) < SECTOR(20)
					&& abs(pos.zPos - LaraItem->pos.zPos) < SECTOR(20)
					&& abs(pos2.xPos - LaraItem->pos.xPos) < SECTOR(20)
					&& abs(pos2.yPos - LaraItem->pos.yPos) < SECTOR(20)
					&& abs(pos2.zPos - LaraItem->pos.zPos) < SECTOR(20))
				{
					if (item->itemFlags[2] != NO_ITEM)
					{
						ITEM_INFO* item2 = &g_Level.Items[item->itemFlags[2]];
						ExplodeItemNode(item2, 0, 0, 128);
						KillItem(item->itemFlags[2]);

						TriggerExplosionSparks(pos.xPos, pos.yPos, pos.zPos, 3, -2, 0, item2->roomNumber);
						TriggerExplosionSparks(pos.xPos, pos.yPos, pos.zPos, 3, -1, 0, item2->roomNumber);

						item->itemFlags[2] = NO_ITEM;
						item2 = FindItem(ID_PUZZLE_ITEM1_COMBO1);
						item2->status = ITEM_NOT_ACTIVE;

						SoundEffect(SFX_TR4_EXPLOSION1, &item2->pos, 0);
						SoundEffect(SFX_TR4_EXPLOSION2, &item2->pos, 0);
					}

					TriggerLightning(
						(PHD_VECTOR*)&pos,
						(PHD_VECTOR*)&pos2,
						(GetRandomControl() & 0xF) + 16,
						r,
						g,
						b,
						24,
						3,
						24,
						3);
				}
			}
		}	
	}
	else
	{	
		AnimateItem(item);

		OBJECT_INFO* obj = &Objects[item->objectNumber];
		bool flag = false;

		if (item->animNumber == obj->animIndex + 2)
		{
			item->pos.yRot -= ANGLE(90);
			if (TrInput & IN_ACTION)
			{
				item->animNumber = obj->animIndex + 1;
				item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			}
			else
			{
				flag = true;
			}
		}

		if (item->animNumber == obj->animIndex + 6)
		{
			item->pos.yRot += ANGLE(90);
			if (!(TrInput & IN_ACTION))
			{
				item->animNumber = obj->animIndex + 3;
				item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
				flag = false;
			}
			else
			{
				item->animNumber = obj->animIndex + 5;
				item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			}
		}

		if (flag)
		{
			item->animNumber = obj->animIndex + 3;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
		}

		if (item->triggerFlags == 2)
		{
			for (int i = 0; i < g_Level.Items.size(); i++)
			{
				ITEM_INFO* currentItem = &g_Level.Items[i];

				if (currentItem->objectNumber == ID_PULLEY)
				{
					currentItem->itemFlags[1] =
						(item->pos.yRot != -ANGLE(90)
							|| g_Level.Items[item->itemFlags[0]].pos.yRot != ANGLE(90)
							|| g_Level.Items[item->itemFlags[1]].pos.yRot != 0 ? 0 : 1) ^ 1;
					break;
				}
			}
		}
	}
}
