#include "framework.h"
#include "tr4_obelisk.h"
#include "items.h"
#include "level.h"
#include "setup.h"
#include "control.h"
#include "Sound\sound.h"
#include "lara.h"
#include "effects\effects.h"
#include "items.h"
#include "effects\tomb4fx.h"

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
	return;//unfinished
	ITEM_INFO* item;
	short number;

	item = &g_Level.Items[itemNumber];

	if (TriggerActive(item))
	{
		if (item->itemFlags[3] > 346)
			return;

		item->itemFlags[3]++;

		if ((GlobalCounter & 1) || (item->itemFlags[3] < 256 && !(GetRandomControl() & 1)))
		{

			if (!(GlobalCounter & 1))
				number = 0x2000;
			wtf:
			if (item->itemFlags[3] >= 256 && item->triggerFlags == 2)
			{
				PHD_3DPOS pos;
				PHD_3DPOS pos2;
				int m = phd_sin(item->pos.yRot + 0x4000);

				pos.xPos = item->pos.xPos + (m << 15);//check cos and sin
				pos.yPos = item->pos.yPos;
				m = phd_cos(item->pos.yRot + 0x4000);
				pos.zPos = item->pos.zPos + (m << 15);
				SoundEffect(197, &pos, 0);

				if ((GlobalCounter & 1) != 0)
				{
					pos2.xPos = (GetRandomControl() & 0x3FF) + pos.xPos - 512;
					pos2.yPos = (GetRandomControl() & 0x3FF) + pos.yPos - 512;
					pos2.zPos = (GetRandomControl() & 0x3FF) + pos.zPos - 512;

					if (abs(pos.xPos - LaraItem->pos.xPos) < 0x5000)
					{
						if (abs(pos.yPos - LaraItem->pos.yPos) < 0x5000)
						{
							if (abs(pos.zPos - LaraItem->pos.zPos) < 0x5000
								&& abs(pos2.xPos - LaraItem->pos.xPos) < 0x5000
								&& abs(pos2.yPos - LaraItem->pos.yPos) < 0x5000
								&& abs(pos2.zPos - LaraItem->pos.zPos) < 0x5000)
							{
								if (item->itemFlags[2] != NO_ITEM)
								{
									ITEM_INFO* item2 = &g_Level.Items[item->itemFlags[2]];
									ExplodeItemNode(item2, 0, 0, 128);
									KillItem(item->itemFlags[2]);
									TriggerExplosionSparks(pos.xPos, pos.yPos, pos.zPos, 3, -2, 0, item2->roomNumber);
									TriggerExplosionSparks(pos.xPos, pos.yPos, pos.zPos, 3, -1, 0, item2->roomNumber);
									item->itemFlags[2] = NO_ITEM;
									item2 = find_a_fucking_item(ID_PUZZLE_ITEM1_COMBO1);
									item2->status = ITEM_NOT_ACTIVE;
									SoundEffect(105, &item2->pos, 0);
									SoundEffect(106, &item2->pos, 0);
								}

								int r = (GetRandomControl() & 0x1F) + 224;
								int b = (GetRandomControl() & 0x1F) + 192;//this feels wrong :)
								int g = (GetRandomControl() & 0x1F) + 64;
								int clr = (g | ((b | (r << 8)) << 8)) | 0x18000000;
								TriggerLightning((PHD_VECTOR*)&pos, (PHD_VECTOR*)&pos2, (GetRandomControl() & 0xF) + 16, r, g, b, 3, 3, 24, 3);
								TriggerLightningGlow(pos.xPos, pos.yPos, pos.zPos, 0x100, r | 0x4000, g, b);
							}
						}
					}
				}
			}

			return;
		}

		if (item->itemFlags[3] < 256 && !(GlobalCounter & 3))
		{
			SoundEffect(198, &item->pos, 0);
			number = (GetRandomControl() & 0xFFF) + 3456;
		}

		PHD_3DPOS* pos;
		PHD_3DPOS* pos2;

		pos->xPos = item->pos.xPos + (3456 * phd_sin(item->pos.yRot + 0x4000));
		pos->yPos = item->pos.yPos - 256;
		pos->zPos = item->pos.zPos + (3456 * phd_sin(item->pos.yRot + 0x4000));

		pos2->xPos = item->pos.xPos + (number * phd_sin(item->pos.yRot + 0x4000));
		pos2->yPos = item->pos.yPos;
		pos2->xPos = item->pos.zPos + (number * phd_sin(item->pos.zRot + 0x4000));

		if (abs(pos->xPos - LaraItem->pos.xPos) < 0x5000)
		{
			if (abs(pos->yPos - LaraItem->pos.yPos) < 0x5000)
			{
				if (abs(pos->zPos - LaraItem->pos.zPos) < 0x5000)
				{
					if (abs(pos2->xPos - LaraItem->pos.xPos) < 0x5000)
					{
						if (abs(pos2->yPos - LaraItem->pos.yPos) < 0x5000)
						{
							if (abs(pos2->zPos - LaraItem->pos.zPos) < 0x5000)
							{
								if (!(GlobalCounter & 3))
								{
								/*	v23 = v39;
									BYTE1(v23) = HIBYTE(v39) | 0x18;
									v37 = v42 | ((itemNumbera | (v23 << 8)) << 8);
									v24 = GetRandomControl();
									TriggerLightning(&v45, &v46, (v24 & 0x1F) + 32, v37, 1, 32, 5);
									v18 = v45.z;
									v16 = v45.x;*/
								}

								/*
								v25 = v39;
								BYTE1(v25) = HIBYTE(v39) | 0x30;
								LODWORD(v13) = TriggerLightningGlow(v16, v45.y, v18, v42 | ((itemNumbera | (v25 << 8)) << 8));
								*/

							}
						}
					}
				}
			}
		}
		goto wtf;
	}



}
