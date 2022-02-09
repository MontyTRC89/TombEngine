#include "framework.h"
#include "tr4_mine.h"
#include "Specific/level.h"
#include "Game/collision/sphere.h"
#include "Sound/sound.h"
#include "Game/effects/effects.h"
#include "Game/effects/tomb4fx.h"
#include "Game/effects/weather.h"
#include "Game/items.h"
#include "Game/collision/collide_item.h"
#include "Objects/objectslist.h"

using namespace TEN::Effects::Environment;

namespace TEN::Entities::TR4
{
	void InitialiseMine(short itemNum)
	{
		ITEM_INFO* item = &g_Level.Items[itemNum];
		if (item->TriggerFlags)
			item->MeshBits = 0;
	}

	void MineControl(short itemNum)
	{
		ITEM_INFO* item = &g_Level.Items[itemNum];

		int num = GetSpheres(item, CreatureSpheres, SPHERES_SPACE_WORLD, Matrix::Identity);
		if (item->ItemFlags[0] >= 150)
		{
			SoundEffect(SFX_TR4_EXPLOSION1, &item->Position, 0);
			SoundEffect(SFX_TR4_EXPLOSION2, &item->Position, 0);
			SoundEffect(SFX_TR4_EXPLOSION1, &item->Position, 0, 0.7f, 0.5f);

			if (num > 0)
			{
				for (int i = 0; i < num; i++)
				{
					if (i >= 7 && i != 9)
					{
						SPHERE* sphere = &CreatureSpheres[i];

						TriggerExplosionSparks(sphere->x, sphere->y, sphere->z, 3, -2, 0, -item->RoomNumber);
						TriggerExplosionSparks(sphere->x, sphere->y, sphere->z, 3, -1, 0, -item->RoomNumber);
						TriggerShockwave((PHD_3DPOS*)sphere, 48, 304, (GetRandomControl() & 0x1F) + 112, 0, 96, 128, 32, 2048, 0);
					}
				}

				for (int i = 0; i < num; i++)
					ExplodeItemNode(item, i, 0, -128);
			}

			Weather.Flash(255, 192, 64, 0.03f);

			short currentItemNumber = g_Level.Rooms[item->RoomNumber].itemNumber;

			// Make the sentry gun explode?
			while (currentItemNumber != NO_ITEM)
			{
				ITEM_INFO* currentItem = &g_Level.Items[currentItemNumber];

				if (currentItem->ObjectNumber == ID_SENTRY_GUN)
					currentItem->MeshBits &= ~0x40;

				currentItemNumber = currentItem->NextItem;
			}

			KillItem(itemNum);
		}
		else
		{
			item->ItemFlags[0]++;

			int fireOn = 4 * item->ItemFlags[0];
			if (fireOn > 255)
				fireOn = 0;

			for (int i = 0; i < num; i++)
			{
				if (i == 0 || i > 5)
				{
					SPHERE* sphere = &CreatureSpheres[i];
					AddFire(sphere->x, sphere->y, sphere->z, 2, item->RoomNumber, fireOn);
				}
			}

			SoundEffect(SFX_TR4_LOOP_FOR_SMALL_FIRES, &item->Position, 0);
		}
	}

	void MineCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll)
	{
		ITEM_INFO* item = &g_Level.Items[itemNum];

		if (item->TriggerFlags && !item->ItemFlags[3])
		{
			if (l->AnimNumber != LA_DETONATOR_USE
				|| l->FrameNumber < g_Level.Anims[l->AnimNumber].frameBase + 57)
			{
				if (TestBoundsCollide(item, l, 512))
				{
					TriggerExplosionSparks(item->Position.xPos, item->Position.yPos, item->Position.zPos, 3, -2, 0, item->RoomNumber);
					for (int i = 0; i < 2; i++)
						TriggerExplosionSparks(item->Position.xPos, item->Position.yPos, item->Position.zPos, 3, -1, 0, item->RoomNumber);

					item->MeshBits = 1;

					ExplodeItemNode(item, 0, 0, 128);
					KillItem(itemNum);

					l->AnimNumber = LA_MINE_DEATH;
					l->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
					l->ActiveState = LS_DEATH;
					l->Velocity = 0;

					SoundEffect(SFX_TR4_MINE_EXP_OVERLAY, &item->Position, 0);
				}
			}
			else
			{
				for (int i = 0; i < g_Level.NumItems; i++)
				{
					ITEM_INFO* currentItem = &g_Level.Items[i];

					// Explode other mines
					if (currentItem->ObjectNumber == ID_MINE
						&& currentItem->Status != ITEM_INVISIBLE
						&& currentItem->TriggerFlags == 0)
					{
						TriggerExplosionSparks(
							currentItem->Position.xPos,
							currentItem->Position.yPos,
							currentItem->Position.zPos,
							3,
							-2,
							0,
							currentItem->RoomNumber);

						for (int j = 0; j < 2; j++)
							TriggerExplosionSparks(
								currentItem->Position.xPos,
								currentItem->Position.yPos,
								currentItem->Position.zPos,
								3,
								-1,
								0,
								currentItem->RoomNumber);

						currentItem->MeshBits = 1;

						ExplodeItemNode(currentItem, 0, 0, -32);
						KillItem(i);

						if (!(GetRandomControl() & 3))
							SoundEffect(SFX_TR4_MINE_EXP_OVERLAY, &currentItem->Position, 0);

						currentItem->Status = ITEM_INVISIBLE;
					}
				}
			}
		}
	}
}