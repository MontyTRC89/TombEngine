#include "framework.h"
#include "newobjects.h"
#include "items.h"
#include "collide.h"
#include "lara.h"
#include "lot.h"
#include "box.h"
#include "people.h"
#include "sphere.h"
#include "setup.h"
#include "level.h"
#include "sound.h"
#include "snowmobile.h"

enum SKIDMAN_STATE { SMAN_EMPTY, SMAN_WAIT, SMAN_MOVING, SMAN_STARTLEFT, SMAN_STARTRIGHT, SMAN_LEFT, SMAN_RIGHT, SMAN_DEATH };

#define SMAN_MIN_TURN (ANGLE(6.0f)/3)
#define SMAN_TARGET_ANGLE ANGLE(15.0f)
#define SMAN_WAIT_RANGE SQUARE(WALL_SIZE*4)
#define SMAN_DEATH_ANIM 10

BITE_INFO skidooLeft = { 240, -190, 540, 0 };
BITE_INFO skidooRight = { -240, -190, 540, 0 };

void InitialiseSkidman(short itemNum)
{
	short skidoo_item;
	ITEM_INFO* item, *skidoo;

	skidoo_item = CreateItem();
	if (skidoo_item != NO_ITEM)
	{
		item = &Items[itemNum];
		skidoo = &Items[skidoo_item];
		skidoo->objectNumber = ID_SNOWMOBILE_GUN;
		skidoo->pos.xPos = item->pos.xPos;
		skidoo->pos.yPos = item->pos.yPos;
		skidoo->pos.zPos = item->pos.zPos;
		skidoo->pos.yRot = item->pos.yRot;
		skidoo->roomNumber = item->roomNumber;
		skidoo->flags = ITEM_INVISIBLE;
		skidoo->shade = -1;

		InitialiseItem(skidoo_item);

		// The skidman remembers his skidoo
		item->data = (void*)skidoo_item;

		LevelItems++;
	}
	else
	{
		printf("FATAL: cannot create skidoo for SKIDMAN !");
	}
}

void SkidManCollision(short itemNum, ITEM_INFO* laraitem, COLL_INFO* coll)
{
	ITEM_INFO* item;

	item = &Items[itemNum];
	if (!TestBoundsCollide(item, laraitem, coll->radius))
		return;
	if (!TestCollision(item, laraitem))
		return;

	if (coll->enableBaddiePush)
	{
		if (item->speed > 0)
			ItemPushLara(item, laraitem, coll, coll->enableSpaz, 0);
		else
			ItemPushLara(item, laraitem, coll, 0, 0);
	}

	/* If Lara is walking and hit by skidoo, gets hurt a lot */
	if (Lara.Vehicle == NO_ITEM && item->speed > 0)
	{
		laraitem->hitStatus = true;
		laraitem->hitPoints -= 100;
	}
}

void SkidManControl(short riderNum)
{
	ITEM_INFO* item, * rider;
	CREATURE_INFO* skidman;
	AI_INFO info;
	short angle, item_number;
	int damage;

	rider = &Items[riderNum];
	if (rider->data == NULL)
	{
		printf("FATAL: rider data not contains the skidoo itemNumber !");
		return;
	}

	item_number = (short)rider->data;
	item = &Items[item_number];

	/* Need to activate AI for skidoo (it's the skidoo that holds the brain) if not done yet */
	if (!item->data)
	{
		EnableBaddieAI(item_number, TRUE);
		item->status = ITEM_ACTIVE;
	}

	skidman = (CREATURE_INFO*)item->data;
	angle = 0;

	if (item->hitPoints <= 0)
	{
		if (rider->currentAnimState != SMAN_DEATH)
		{
			/* Separate skidoo from rider */
			rider->pos.xPos = item->pos.xPos;
			rider->pos.yPos = item->pos.yPos;
			rider->pos.zPos = item->pos.zPos;
			rider->pos.yRot = item->pos.yRot;
			rider->roomNumber = item->roomNumber;

			rider->animNumber = Objects[ID_SNOWMOBILE_DRIVER].animIndex + SMAN_DEATH_ANIM;
			rider->frameNumber = Anims[rider->animNumber].frameBase;
			rider->currentAnimState = SMAN_DEATH;

			/* Break Lara's lock */
			if (Lara.target == item)
				Lara.target = NULL;
		}
		else
			AnimateItem(rider);

		/* Make skidoo stop */
		if (item->currentAnimState == SMAN_MOVING || item->currentAnimState == SMAN_WAIT)
			item->goalAnimState = SMAN_WAIT;
		else
			item->goalAnimState = SMAN_MOVING;
	}
	else
	{
		/* As skidoo has the brain, it needs to know if the rider was hurt */
		CreatureAIInfo(item, &info);

		GetCreatureMood(item, &info, VIOLENT);
		CreatureMood(item, &info, VIOLENT);

		angle = CreatureTurn(item, ANGLE(6)/2);

		switch (item->currentAnimState)
		{
		case SMAN_WAIT:
			if (skidman->mood == BORED_MOOD)
				break;
			else if (abs(info.angle) < SMAN_TARGET_ANGLE && info.distance < SMAN_WAIT_RANGE)
				break;
			item->goalAnimState = SMAN_MOVING;
			break;

		case SMAN_MOVING:
			if (skidman->mood == BORED_MOOD)
				item->goalAnimState = SMAN_WAIT;
			else if (abs(info.angle) < SMAN_TARGET_ANGLE && info.distance < SMAN_WAIT_RANGE)
				item->goalAnimState = SMAN_WAIT;
			else if (angle < -SMAN_MIN_TURN)
				item->goalAnimState = SMAN_STARTLEFT;
			else if (angle > SMAN_MIN_TURN)
				item->goalAnimState = SMAN_STARTRIGHT;
			break;

		case SMAN_STARTLEFT:
		case SMAN_LEFT:
			if (angle < -SMAN_MIN_TURN)
				item->goalAnimState = SMAN_LEFT;
			else
				item->goalAnimState = SMAN_MOVING;
			break;

		case SMAN_STARTRIGHT:
		case SMAN_RIGHT:
			if (angle < -SMAN_MIN_TURN)
				item->goalAnimState = SMAN_LEFT;
			else
				item->goalAnimState = SMAN_MOVING;
			break;
		}
	}


	/* Shoot them guns */
	if (rider->currentAnimState != SMAN_DEATH)
	{
		if (!skidman->flags && abs(info.angle) < SMAN_TARGET_ANGLE && LaraItem->hitPoints > 0)
		{
			damage = (Lara.Vehicle != NO_ITEM) ? 10 : 50; // more damage if Lara on foot

			if (ShotLara(item, &info, &skidooLeft, 0, damage) + ShotLara(item, &info, &skidooRight, 0, damage))
				skidman->flags = 5;
		}

		if (skidman->flags)
		{
			SoundEffect(43, &item->pos, 0);
			skidman->flags--;
		}
	}

	/* Use 'head_rotation' to store which track is required */
	if (item->currentAnimState == SMAN_WAIT)
	{
		SoundEffect(153, &item->pos, 0);
		skidman->jointRotation[0] = 0;
	}
	else
	{
		skidman->jointRotation[0] = (skidman->jointRotation[0] == 1) ? 2 : 1;
		DoSnowEffect(item);
		SoundEffect(155, &item->pos, 4 + ((0x10000 - (100 - item->speed) * 100) << 8));
	}

	CreatureAnimation(item_number, angle, 0);

	/* Move rider to save position and animation as skidoo */
	if (rider->currentAnimState != SMAN_DEATH)
	{
		rider->pos.xPos = item->pos.xPos;
		rider->pos.yPos = item->pos.yPos;
		rider->pos.zPos = item->pos.zPos;
		rider->pos.yRot = item->pos.yRot;
		if (item->roomNumber != rider->roomNumber)
			ItemNewRoom(riderNum, item->roomNumber);

		rider->animNumber = item->animNumber + (Objects[ID_SNOWMOBILE_DRIVER].animIndex - Objects[ID_SNOWMOBILE_GUN].animIndex);
		rider->frameNumber = item->frameNumber + (Anims[rider->animNumber].frameBase - Anims[item->animNumber].frameBase);
	}
	else if (rider->status == ITEM_DEACTIVATED && item->speed == 0 && item->fallspeed == 0)
	{
		/* If rider has reached end of his death animation, turn his skidoo into one that Lara can ride */
		RemoveActiveItem(riderNum);
		rider->collidable = false;
		rider->hitPoints = -16384;
		rider->flags |= ONESHOT;

		DisableBaddieAI(item_number);
		item->objectNumber = ID_SNOWMOBILE;
		item->status = ITEM_DEACTIVATED;
		InitialiseSkidoo(item_number);

		((SKIDOO_INFO*)item->data)->armed = true;
	}
}