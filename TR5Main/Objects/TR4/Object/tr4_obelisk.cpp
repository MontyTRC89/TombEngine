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
	auto* item = &g_Level.Items[itemNumber];

	item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 3;;
	item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;

	AddActiveItem(itemNumber);
	item->Status = ITEM_ACTIVE;

	if (item->TriggerFlags == 2)
	{
		for (int i = 0; i < g_Level.NumItems; i++)
		{
			auto* currentItem = &g_Level.Items[i];

			if (currentItem->ObjectNumber == ID_OBELISK)
				item->ItemFlags[0]++;

			if (currentItem->ObjectNumber == ID_ANIMATING3)
				item->ItemFlags[2] = i;
		}
	}

}

void ObeliskControl(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	short someNumber;
	PHD_3DPOS pos;
	PHD_3DPOS pos2;

	if (TriggerActive(item))
	{
		if (item->ItemFlags[3] > 346)
			return;

		item->ItemFlags[3]++;

		byte r = (GetRandomControl() & 0x1F) + 224;
		byte g = r - (GetRandomControl() & 0x1F) - 32;
		byte b = g - (GetRandomControl() & 0x1F) - 128;

		if (!(GlobalCounter & 1))
		{
			someNumber = 8192;
			if (GetRandomControl() & 1)
			{
				if (item->ItemFlags[3] < 256 
					&& (GetRandomControl() & 1) 
					&& !(GlobalCounter & 3))
				{
					SoundEffect(SFX_TR4_ELEC_ONE_SHOT, &item->Position, 0);
					someNumber = (GetRandomControl() & 0xFFF) + 3456;
				}

				pos.xPos = item->Position.xPos + (3456 * phd_sin(item->Position.yRot + ANGLE(90.0f)));
				pos.yPos = item->Position.yPos - CLICK(1);
				pos.zPos = item->Position.zPos + (3456 * phd_cos(item->Position.yRot + ANGLE(90.0f)));

				pos2.xPos = item->Position.xPos + (someNumber * phd_sin(item->Position.yRot + ANGLE(90.0f)));
				pos2.yPos = item->Position.yPos;
				pos2.xPos = item->Position.zPos + (someNumber * phd_cos(item->Position.zRot + ANGLE(90.0f)));

				if (abs(pos.xPos - LaraItem->Position.xPos) < SECTOR(20) &&
					abs(pos.yPos - LaraItem->Position.yPos) < SECTOR(20) &&
					abs(pos.zPos - LaraItem->Position.zPos) < SECTOR(20) &&
					abs(pos2.xPos - LaraItem->Position.xPos) < SECTOR(20) &&
					abs(pos2.yPos - LaraItem->Position.yPos) < SECTOR(20) &&
					abs(pos2.zPos - LaraItem->Position.zPos) < SECTOR(20))
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

		if (item->ItemFlags[3] >= 256 && item->TriggerFlags == 2)
		{
			pos.xPos = item->Position.xPos + SECTOR(8) * phd_sin(item->Position.yRot);
			pos.yPos = item->Position.yPos;
			pos.zPos = item->Position.zPos + SECTOR(8) * phd_cos(item->Position.yRot + ANGLE(90.0f));

			SoundEffect(SFX_TR4_ELEC_ARCING_LOOP, &pos, 0);

			if (GlobalCounter & 1)
			{
				pos2.xPos = (GetRandomControl() & 0x3FF) + pos.xPos - 512;
				pos2.yPos = (GetRandomControl() & 0x3FF) + pos.yPos - 512;
				pos2.zPos = (GetRandomControl() & 0x3FF) + pos.zPos - 512;

				if (abs(pos.xPos - LaraItem->Position.xPos) < SECTOR(20) &&
					abs(pos.yPos - LaraItem->Position.yPos) < SECTOR(20) &&
					abs(pos.zPos - LaraItem->Position.zPos) < SECTOR(20) &&
					abs(pos2.xPos - LaraItem->Position.xPos) < SECTOR(20) &&
					abs(pos2.yPos - LaraItem->Position.yPos) < SECTOR(20) &&
					abs(pos2.zPos - LaraItem->Position.zPos) < SECTOR(20))
				{
					if (item->ItemFlags[2] != NO_ITEM)
					{
						auto* item2 = &g_Level.Items[item->ItemFlags[2]];
						ExplodeItemNode(item2, 0, 0, 128);
						KillItem(item->ItemFlags[2]);

						TriggerExplosionSparks(pos.xPos, pos.yPos, pos.zPos, 3, -2, 0, item2->RoomNumber);
						TriggerExplosionSparks(pos.xPos, pos.yPos, pos.zPos, 3, -1, 0, item2->RoomNumber);

						item->ItemFlags[2] = NO_ITEM;
						item2 = FindItem(ID_PUZZLE_ITEM1_COMBO1);
						item2->Status = ITEM_NOT_ACTIVE;

						SoundEffect(SFX_TR4_EXPLOSION1, &item2->Position, 0);
						SoundEffect(SFX_TR4_EXPLOSION2, &item2->Position, 0);
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

		auto* obj = &Objects[item->ObjectNumber];
		bool flag = false;

		if (item->Animation.AnimNumber == obj->animIndex + 2)
		{
			item->Position.yRot -= ANGLE(90.0f);

			if (TrInput & IN_ACTION)
			{
				item->Animation.AnimNumber = obj->animIndex + 1;
				item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
			}
			else
				flag = true;
		}

		if (item->Animation.AnimNumber == obj->animIndex + 6)
		{
			item->Position.yRot += ANGLE(90.0f);

			if (!(TrInput & IN_ACTION))
			{
				item->Animation.AnimNumber = obj->animIndex + 3;
				item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
				flag = false;
			}
			else
			{
				item->Animation.AnimNumber = obj->animIndex + 5;
				item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
			}
		}

		if (flag)
		{
			item->Animation.AnimNumber = obj->animIndex + 3;
			item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
		}

		if (item->TriggerFlags == 2)
		{
			for (int i = 0; i < g_Level.Items.size(); i++)
			{
				auto* currentItem = &g_Level.Items[i];

				if (currentItem->ObjectNumber == ID_PULLEY)
				{
					currentItem->ItemFlags[1] =
						(item->Position.yRot != -ANGLE(90.0f) ||
							g_Level.Items[item->ItemFlags[0]].Position.yRot != ANGLE(90.0f) ||
							g_Level.Items[item->ItemFlags[1]].Position.yRot != 0 ? 0 : 1) ^ 1;

					break;
				}
			}
		}
	}
}
