#include "framework.h"
#include "tr5_deathslide.h"
#include "input.h"
#include "trmath.h"
#include "lara.h"
#include "setup.h"
#include "laramisc.h"
#include "sound.h"

short DeathSlideBounds[12] = { -256, 256, -100, 100, 256, 512, 0, 0, -ANGLE(25.0f), ANGLE(25.0f), 0, 0 };
PHD_VECTOR DeathSlidePosition(0, 0, 371);

void InitialiseDeathSlide(short itemNumber)
{
	ITEM_INFO* item = &Items[itemNumber];
	GAME_VECTOR* pos = (GAME_VECTOR*)game_malloc(sizeof(GAME_VECTOR));
	item->data = pos;
	pos->x = item->pos.xPos;
	pos->y = item->pos.yPos;
	pos->z = item->pos.zPos;
	pos->roomNumber = item->roomNumber;
}

void DeathSlideCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* coll)
{
	if (!(TrInput & IN_ACTION) || l->gravityStatus || Lara.gunStatus != LG_NO_ARMS || l->currentAnimState != STATE_LARA_STOP)
		return;

	ITEM_INFO* item = &Items[itemNumber];
	if (item->status != ITEM_NOT_ACTIVE)
		return;

	if (TestLaraPosition(DeathSlideBounds, item, LaraItem))
	{
		AlignLaraPosition(&DeathSlidePosition, item, LaraItem);
		Lara.gunStatus = LG_HANDS_BUSY;

		l->goalAnimState = STATE_LARA_ZIPLINE_RIDE;
		do
			AnimateItem(l);
		while (l->currentAnimState != STATE_LARA_GRABBING);

		if (!item->active)
			AddActiveItem(itemNumber);

		item->status = ITEM_ACTIVE;
		item->flags |= ONESHOT;
	}
}

void ControlDeathSlide(short itemNumber)
{
	ITEM_INFO* item = &Items[itemNumber];

	if (item->status == ITEM_ACTIVE)
	{
		if (!(item->flags & ONESHOT))
		{
			GAME_VECTOR* old = (GAME_VECTOR*)item->data;

			item->pos.xPos = old->x;
			item->pos.yPos = old->y;
			item->pos.zPos = old->z;

			if (old->roomNumber != item->roomNumber)
				ItemNewRoom(itemNumber, old->roomNumber);

			item->status = ITEM_NOT_ACTIVE;
			item->currentAnimState = item->goalAnimState = 1;
			item->animNumber = Objects[item->objectNumber].animIndex;
			item->frameNumber = Anims[item->animNumber].frameBase;

			RemoveActiveItem(itemNumber);

			return;
		}

		if (item->currentAnimState == 1)
		{
			AnimateItem(item);
			return;
		}

		AnimateItem(item);

		if (item->fallspeed < 100)
			item->fallspeed += 5;

		int c = phd_cos(item->pos.yRot);
		int s = phd_sin(item->pos.yRot);

		item->pos.zPos += item->fallspeed * c >> W2V_SHIFT;
		item->pos.xPos += item->fallspeed * s >> W2V_SHIFT;
		item->pos.yPos += item->fallspeed >> 2;

		short roomNumber = item->roomNumber;
		GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
		if (roomNumber != item->roomNumber)
			ItemNewRoom(itemNumber, roomNumber);

		if (LaraItem->currentAnimState == STATE_LARA_ZIPLINE_RIDE)
		{
			LaraItem->pos.xPos = item->pos.xPos;
			LaraItem->pos.yPos = item->pos.yPos;
			LaraItem->pos.zPos = item->pos.zPos;
		}

		int x = item->pos.xPos + (1024 * s >> W2V_SHIFT);
		int y = item->pos.yPos + 64;
		int z = item->pos.zPos + (1024 * c >> W2V_SHIFT);

		FLOOR_INFO* floor = GetFloor(x, y, z, &roomNumber);

		if (GetFloorHeight(floor, x, y, z) <= y + 256 || GetCeiling(floor, x, y, z) >= y - 256)
		{
			if (LaraItem->currentAnimState == STATE_LARA_ZIPLINE_RIDE)
			{
				LaraItem->goalAnimState = STATE_LARA_JUMP_FORWARD;
				AnimateLara(LaraItem);
				LaraItem->gravityStatus = true;
				LaraItem->speed = item->fallspeed;
				LaraItem->fallspeed = item->fallspeed >> 2;
			}

			// TODO: sounds
			// Stop
			SoundEffect(SFX_COGS_ROME, &item->pos, 0);
			RemoveActiveItem(itemNumber);
			item->status = ITEM_NOT_ACTIVE;
			item->flags -= ONESHOT;
		}
		else
		{
			// Whizz
			SoundEffect(SFX_GOD_HEAD_LASER_LOOPS, &item->pos, 0);
		}
	}
}