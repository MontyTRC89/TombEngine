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
	auto* item = &g_Level.Items[itemNumber];

	if (item->TriggerFlags >= 30000)
	{
		item->ItemFlags[1] = 3;
		item->TriggerFlags -= 30000;
	}
	else if (item->TriggerFlags >= 20000)
	{
		item->ItemFlags[1] = 2;
		item->TriggerFlags -= 20000;
	}
	else if (item->TriggerFlags >= 10000)
	{
		item->ItemFlags[1] = 1;
		item->TriggerFlags -= 10000;
	}

	if (item->TriggerFlags >= 1000)
	{
		item->ItemFlags[3] = 1;
		item->TriggerFlags -= 1000;
	}

	item->ItemFlags[2] = item->TriggerFlags / 100;
	item->TriggerFlags = 7 * (item->TriggerFlags % 100);
}

void ExplosionControl(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	if (TriggerActive(item))
	{
		item->Flags |= IFLAG_INVISIBLE;
		if (item->ItemFlags[0] < item->TriggerFlags)
			++item->ItemFlags[0];
		else if (item->ItemFlags[0] == item->TriggerFlags)
		{
			int flag;

			++item->ItemFlags[0];
			if (g_Level.Rooms[item->RoomNumber].flags & ENV_FLAG_WATER)
				flag = 1;
			else
				flag = item->ItemFlags[1] == 1 ? 2 : 0;
			
			SoundEffect(SFX_TR4_EXPLOSION1, &item->Pose, SoundEnvironment::Land, 1.5f);
			SoundEffect(SFX_TR4_EXPLOSION2, &item->Pose);
			TriggerExplosionSparks(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, 3, -2, flag, item->RoomNumber);
			
			for (int i = 0; i < item->ItemFlags[2]; ++i)
				TriggerExplosionSparks(item->Pose.Position.x + (GetRandomControl() % 128 - 64) * item->ItemFlags[2], item->Pose.Position.y + (GetRandomControl() % 128 - 64) * item->ItemFlags[2], item->Pose.Position.z + (GetRandomControl() % 128 - 64) * item->ItemFlags[2], 2, 0, i, item->RoomNumber);
			
			PHD_3DPOS pos;
			pos.Position.x = item->Pose.Position.x;
			pos.Position.y = item->Pose.Position.y - 128;
			pos.Position.z = item->Pose.Position.z;
			
			if (item->ItemFlags[3])
			{
				if (flag == 2)
					TriggerShockwave(&pos, 48, 32 * item->ItemFlags[2] + 304, 4 * item->ItemFlags[2] + 96, 128, 96, 0, 24, 2048, 0);
				else
					TriggerShockwave(&pos, 48, 32 * item->ItemFlags[2] + 304, 4 * item->ItemFlags[2] + 96, 0, 96, 128, 24, 2048, 0);
			}
			else if (flag == 2)
			{
				Vector3Int vec = { 0, 0, 0 };
				GetLaraJointPosition(&vec, 0);

				int dx = vec.x - item->Pose.Position.x;
				int dy = vec.y - item->Pose.Position.y;
				int dz = vec.z - item->Pose.Position.z;
				
				if (abs(dx) < SECTOR(1) &&
					abs(dy) < SECTOR(1) &&
					abs(dz) < SECTOR(1))
				{
					int distance = sqrt(pow(dx, 2) + pow(dy, 2) + pow(dz, 2));
					if (distance < SECTOR(2))
					{
						DoDamage(LaraItem, distance / 16);

						if (distance < CLICK(3))
							LaraBurn(LaraItem);
					}
				}
			}

			GetCollidedObjects(item, 2048, true, CollidedItems, CollidedMeshes, 1);
			if (CollidedItems[0] || CollidedMeshes[0])
			{
				int i = 0;
				while (CollidedItems[i])
				{
					if (CollidedItems[i]->ObjectNumber >= ID_SMASH_OBJECT1 && CollidedItems[i]->ObjectNumber <= ID_SMASH_OBJECT16)
					{
						TriggerExplosionSparks(CollidedItems[i]->Pose.Position.x, CollidedItems[i]->Pose.Position.y, CollidedItems[i]->Pose.Position.z, 3, -2, 0, CollidedItems[i]->RoomNumber);
						CollidedItems[i]->Pose.Position.y -= 128;
						TriggerShockwave(&CollidedItems[i]->Pose, 48, 304, 96, 0, 96, 128, 24, 0, 0);
						CollidedItems[i]->Pose.Position.y += 128;
						ExplodeItemNode(CollidedItems[i], 0, 0, 80);
						SmashObject(CollidedItems[i] - g_Level.Items.data());
						KillItem(CollidedItems[i] - g_Level.Items.data());
					}
					else if (CollidedItems[i]->ObjectNumber != ID_SWITCH_TYPE7 && CollidedItems[i]->ObjectNumber != ID_SWITCH_TYPE8)
					{
						if (Objects[CollidedItems[i]->ObjectNumber].intelligent)
							DoExplosiveDamageOnBaddy(LaraItem, CollidedItems[i], item, LaraWeaponType::GrenadeLauncher);
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
						TriggerExplosionSparks(CollidedMeshes[i]->pos.Position.x, CollidedMeshes[i]->pos.Position.y, CollidedMeshes[i]->pos.Position.z, 3, -2, 0, item->RoomNumber);
						CollidedMeshes[i]->pos.Position.y -= 128;
						TriggerShockwave(&CollidedMeshes[i]->pos, 40, 176, 64, 0, 96, 128, 16, 0, 0);
						CollidedMeshes[i]->pos.Position.y += 128;
						SoundEffect(GetShatterSound(CollidedMeshes[i]->staticNumber), &CollidedMeshes[i]->pos);
						ShatterObject(NULL, CollidedMeshes[i], -128, item->RoomNumber, 0);
						SmashedMeshRoom[SmashedMeshCount] = item->RoomNumber;
						SmashedMesh[SmashedMeshCount] = CollidedMeshes[i];
						++SmashedMeshCount;
						CollidedMeshes[i]->flags &= ~StaticMeshFlags::SM_VISIBLE;
					}

					++i;
				}

				AlertNearbyGuards(item);
			}

			if (item->ItemFlags[1] >= 2)
			{
				if (item->ItemFlags[1] == 3)
				{
					short triggerItems[8];
					for (int i = GetSwitchTrigger(item, triggerItems, 1); i > 0; --i)
						g_Level.Items[triggerItems[i - 1]].ItemFlags[0] = 0;
					
					item->ItemFlags[0] = 0;
				}
			}
			else
				KillItem(itemNumber);
		}
	}
}
