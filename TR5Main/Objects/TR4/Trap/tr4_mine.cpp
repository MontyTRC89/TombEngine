#include "framework.h"
#include "tr4_mine.h"
#include "level.h"
#include "sphere.h"
#include "sound.h"
#include "effect2.h"
#include "tomb4fx.h"
#include "items.h"
#include "collide.h"
#include "objectslist.h"

void InitialiseMine(short itemNum)
{
	ITEM_INFO* item = &Items[itemNum];
	if (item->triggerFlags)
		item->meshBits = 0;
}

void MineControl(short itemNum)
{
	ITEM_INFO* item = &Items[itemNum];

	int num = GetSpheres(item, CreatureSpheres, SPHERES_SPACE_WORLD, Matrix::Identity);
	if (item->itemFlags[0] >= 150)
	{
		SoundEffect(SFX_EXPLOSION1, &item->pos, 0);
		SoundEffect(SFX_EXPLOSION2, &item->pos, 0);
		SoundEffect(SFX_EXPLOSION1, &item->pos, PITCH_SHIFT | 0x1800000);

		if (num > 0)
		{
			SPHERE* sphere = &CreatureSpheres[0];

			for (int i = 0; i < num; i++)
			{
				if (i >= 7 && i != 9)
				{
					TriggerExplosionSparks(sphere->x, sphere->y, sphere->z, 3, -2, 0, -item->roomNumber);
					TriggerExplosionSparks(sphere->x, sphere->y, sphere->z, 3, -1, 0, -item->roomNumber);
					TriggerShockwave((PHD_3DPOS*)sphere, 48, 304, (GetRandomControl() & 0x1F) + 112, 0, 96, 128, 32, 2048, 0);
				}

				sphere++;
			}

			for (int i = 0; i < num; i++)
				ExplodeItemNode(item, i, 0, -128);
		}

		FlashFadeR = 255;
		FlashFadeG = 192;
		FlashFadeB = 64;
		FlashFader = 32;

		short currentItemNumber = Rooms[item->roomNumber].itemNumber;

		// Make the sentry gun explode?
		while (currentItemNumber != NO_ITEM)
		{
			ITEM_INFO* currentItem = &Items[currentItemNumber];

			if (currentItem->objectNumber == ID_SENTRY_GUN)
				currentItem->meshBits &= ~0x40;

			currentItemNumber = currentItem->nextItem;
		}

		KillItem(itemNum);
	}
	else
	{
		item->itemFlags[0]++;

		int something = 4 * item->itemFlags[0];
		if (something > 255)
			something = 0;

		for (int i = 0; i < num; i++)
		{
			SPHERE* sphere = &CreatureSpheres[i];

			if (i == 0 || i > 5)
				AddFire(sphere->x, sphere->y, sphere->z, 2, item->roomNumber, something);

			sphere++;
		}

		SoundEffect(SFX_LOOP_FOR_SMALL_FIRES, &item->pos, 0);
	}
}

void MineCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll)
{
	ITEM_INFO* item = &Items[itemNum];

	if (item->triggerFlags && !item->itemFlags[3])
	{
		if (l->animNumber != 432 || l->frameNumber < Anims[item->animNumber].frameBase + 57)
		{
			if (TestBoundsCollide(item, l, 512))
			{
				TriggerExplosionSparks(item->pos.xPos, item->pos.yPos, item->pos.zPos, 3, -2, 0, item->roomNumber);
				for (int i = 0; i < 2; i++)
					TriggerExplosionSparks(item->pos.xPos, item->pos.yPos, item->pos.zPos, 3, -1, 0, item->roomNumber);

				item->meshBits = 1;

				ExplodeItemNode(item, 0, 0, 128);
				KillItem(itemNum);

				l->animNumber = 438;
				l->frameNumber = Anims[item->animNumber].frameBase;
				l->currentAnimState = 8;
				l->speed = 0;

				SoundEffect(SFX_TR4_MINE_EXP_OVERLAY, &item->pos, 0);
			}
		}
		else
		{
			for (int i = 0; i < LevelItems; i++)
			{
				ITEM_INFO* currentItem = &Items[i];

				// Explode other mines
				if (currentItem->objectNumber == ID_MINE && currentItem->status != ITEM_INVISIBLE && !currentItem->triggerFlags)
				{
					TriggerExplosionSparks(
						currentItem->pos.xPos,
						currentItem->pos.yPos,
						currentItem->pos.zPos,
						3,
						-2,
						0,
						currentItem->roomNumber);

					for (int j = 0; j < 2; j++)
						TriggerExplosionSparks(
							currentItem->pos.xPos,
							currentItem->pos.yPos,
							currentItem->pos.zPos,
							3,
							-1,
							0,
							currentItem->roomNumber);

					currentItem->meshBits = 1;

					ExplodeItemNode(currentItem, 0, 0, -32);
					KillItem(i);

					if (!(GetRandomControl() & 3))
						SoundEffect(SFX_TR4_MINE_EXP_OVERLAY, &currentItem->pos, 0);

					currentItem->status = ITEM_INVISIBLE;
				}
			}
		}
	}
}