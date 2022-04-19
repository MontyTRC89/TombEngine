#include "framework.h"
#include "tr5_explosion.h"
#include "Specific/level.h"
#include "Specific/setup.h"
#include "Game/control/control.h"
#include "Sound/sound.h"
#include "Game/effects/effects.h"
#include "Game/effects/tomb4fx.h"
#include "Game/animation.h"
#include "Objects/Generic/Traps/traps.h"
#include "Game/Lara/lara.h"
#include "Objects/TR5/Shatter/tr5_smashobject.h"
#include "Game/Lara/lara_one_gun.h"
#include "Game/effects/debris.h"
#include "Objects/Generic/Switches/generic_switch.h"
#include "Game/collision/collide_item.h"
#include "Game/control/box.h"
#include "Game/effects/lara_fx.h"
#include "Game/items.h"

using namespace TEN::Effects::Lara;
using namespace TEN::Entities::Switches;

void InitialiseExplosion(short itemNumber)
{
	ITEM_INFO* item;

	item = &g_Level.Items[itemNumber];
	if (item->triggerFlags >= 30000)
	{
		item->itemFlags[1] = 3;
		item->triggerFlags -= 30000;
	}
	else if (item->triggerFlags >= 20000)
	{
		item->itemFlags[1] = 2;
		item->triggerFlags -= 20000;
	}
	else if (item->triggerFlags >= 10000)
	{
		item->itemFlags[1] = 1;
		item->triggerFlags -= 10000;
	}

	if (item->triggerFlags >= 1000)
	{
		item->itemFlags[3] = 1;
		item->triggerFlags -= 1000;
	}

	item->itemFlags[2] = item->triggerFlags / 100;
	item->triggerFlags = 7 * (item->triggerFlags % 100);
}

void ExplosionControl(short itemNumber)
{
	ITEM_INFO* item;
	int flag, i, dx, dy, dz, distance;
	PHD_3DPOS pos;
	PHD_VECTOR vec;
	short triggerItems[8];

	item = &g_Level.Items[itemNumber];
	if (TriggerActive(item))
	{
		item->flags |= IFLAG_INVISIBLE;
		if (item->itemFlags[0] < item->triggerFlags)
		{
			++item->itemFlags[0];
		}
		else if (item->itemFlags[0] == item->triggerFlags)
		{
			++item->itemFlags[0];
			if (g_Level.Rooms[item->roomNumber].flags & ENV_FLAG_WATER)
			{
				flag = 1;
			}
			else
			{
				flag = item->itemFlags[1] == 1 ? 2 : 0;
			}
			SoundEffect(SFX_TR4_EXPLOSION1,&item->pos, 25165828);
			SoundEffect(SFX_TR4_EXPLOSION2,&item->pos, 0);
			TriggerExplosionSparks(item->pos.xPos, item->pos.yPos, item->pos.zPos, 3, -2, flag, item->roomNumber);
			for (i = 0; i < item->itemFlags[2]; ++i)
				TriggerExplosionSparks(item->pos.xPos + (GetRandomControl() % 128 - 64) * item->itemFlags[2], item->pos.yPos + (GetRandomControl() % 128 - 64) * item->itemFlags[2], item->pos.zPos + (GetRandomControl() % 128 - 64) * item->itemFlags[2], 2, 0, i, item->roomNumber);
			pos.xPos = item->pos.xPos;
			pos.yPos = item->pos.yPos - 128;
			pos.zPos = item->pos.zPos;
			if (item->itemFlags[3])
			{
				if (flag == 2)
					TriggerShockwave(&pos, 48, 32 * item->itemFlags[2] + 304, 4 * item->itemFlags[2] + 96, 128, 96, 0, 24, 2048, 0);
				else
					TriggerShockwave(&pos, 48, 32 * item->itemFlags[2] + 304, 4 * item->itemFlags[2] + 96, 0, 96, 128, 24, 2048, 0);
			}
			else if (flag == 2)
			{
				vec.x = 0;
				vec.y = 0;
				vec.z = 0;
				GetLaraJointPosition(&vec, 0);
				dx = vec.x - item->pos.xPos;
				dy = vec.y - item->pos.yPos;
				dz = vec.z - item->pos.zPos;
				if (abs(dx) < 1024 && abs(dy) < 1024 && abs(dz) < 1024)
				{
					distance = sqrt(SQUARE(dx) + SQUARE(dy) + SQUARE(dz));
					if (distance < 2048)
					{
						LaraItem->hitPoints -= distance / 16;
						if (distance < 768)
							LaraBurn(LaraItem);
					}
				}
			}
			GetCollidedObjects(item, 2048, 1, CollidedItems, CollidedMeshes, 1);
			if (CollidedItems[0] || CollidedMeshes[0])
			{
				i = 0;
				while (CollidedItems[i])
				{
					if (CollidedItems[i]->objectNumber >= ID_SMASH_OBJECT1 && CollidedItems[i]->objectNumber <= ID_SMASH_OBJECT16)
					{
						TriggerExplosionSparks(CollidedItems[i]->pos.xPos, CollidedItems[i]->pos.yPos, CollidedItems[i]->pos.zPos, 3, -2, 0, CollidedItems[i]->roomNumber);
						CollidedItems[i]->pos.yPos -= 128;
						TriggerShockwave(&CollidedItems[i]->pos, 48, 304, 96, 0, 96, 128, 24, 0, 0);
						CollidedItems[i]->pos.yPos += 128;
						ExplodeItemNode(CollidedItems[i], 0, 0, 80);
						SmashObject(CollidedItems[i] - g_Level.Items.data());
						KillItem(CollidedItems[i] - g_Level.Items.data());
					}
					else if (CollidedItems[i]->objectNumber != ID_SWITCH_TYPE7 && CollidedItems[i]->objectNumber != ID_SWITCH_TYPE8)
					{
						if (Objects[CollidedItems[i]->objectNumber].intelligent)
							DoExplosiveDamageOnBaddie(CollidedItems[i], item, WEAPON_GRENADE_LAUNCHER);
					}
					else
					{
						/* @FIXME This calls CrossbowHitSwitchType78() */
					}
					++i;
				}
				i = 0;
				while (CollidedMeshes[i])
				{
					if (StaticObjects[CollidedMeshes[i]->staticNumber].shatterType != SHT_NONE)
					{
						TriggerExplosionSparks(CollidedMeshes[i]->pos.xPos, CollidedMeshes[i]->pos.yPos, CollidedMeshes[i]->pos.zPos, 3, -2, 0, item->roomNumber);
						CollidedMeshes[i]->pos.yPos -= 128;
						TriggerShockwave(&CollidedMeshes[i]->pos, 40, 176, 64, 0, 96, 128, 16, 0, 0);
						CollidedMeshes[i]->pos.yPos += 128;
						SoundEffect(GetShatterSound(CollidedMeshes[i]->staticNumber),&CollidedMeshes[i]->pos, 0);
						ShatterObject(NULL, CollidedMeshes[i], -128, item->roomNumber, 0);
						SmashedMeshRoom[SmashedMeshCount] = item->roomNumber;
						SmashedMesh[SmashedMeshCount] = CollidedMeshes[i];
						++SmashedMeshCount;
						CollidedMeshes[i]->flags &= ~StaticMeshFlags::SM_VISIBLE;
					}
					++i;
				}
				AlertNearbyGuards(item);
			}
			if (item->itemFlags[1] >= 2)
			{
				if (item->itemFlags[1] == 3)
				{
					for (i = GetSwitchTrigger(item, triggerItems, 1); i > 0; --i)
					{
						g_Level.Items[triggerItems[i - 1]].itemFlags[0] = 0;
					}
					item->itemFlags[0] = 0;
				}
			}
			else
			{
				KillItem(itemNumber);
			}
		}
	}
}
