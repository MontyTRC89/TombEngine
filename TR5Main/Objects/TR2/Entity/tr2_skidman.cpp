#include "framework.h"
#include "Objects/TR2/Entity/tr2_skidman.h"

#include "Game/animation.h"
#include "Game/items.h"
#include "Game/collision/collide_item.h"
#include "Game/collision/sphere.h"
#include "Game/control/box.h"
#include "Game/control/lot.h"
#include "Game/itemdata/creature_info.h"
#include "Game/Lara/lara.h"
#include "Game/people.h"
#include "Objects/TR2/Vehicles/snowmobile.h"
#include "Objects/TR2/Vehicles/skidoo_info.h"
#include "Sound/sound.h"
#include "Specific/level.h"
#include "Specific/setup.h"

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
		item = &g_Level.Items[itemNum];
		skidoo = &g_Level.Items[skidoo_item];
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
		item->data = skidoo_item;

		g_Level.NumItems++;
	}
	else
	{
		TENLog("Can't create skidoo for rider!", LogLevel::Error);
	}
}

void SkidManCollision(short itemNum, ITEM_INFO* laraitem, COLL_INFO* coll)
{
	ITEM_INFO* item;

	item = &g_Level.Items[itemNum];
	if (!TestBoundsCollide(item, laraitem, coll->Setup.Radius))
		return;
	if (!TestCollision(item, laraitem))
		return;

	if (coll->Setup.EnableObjectPush)
	{
		if (item->speed > 0)
			ItemPushItem(item, laraitem, coll, coll->Setup.EnableSpaz, 0);
		else
			ItemPushItem(item, laraitem, coll, 0, 0);
	}

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

	rider = &g_Level.Items[riderNum];
	if (rider->data == NULL)
	{
		TENLog("Rider data does not contain the skidoo itemNumber!", LogLevel::Error);
		return;
	}

	item_number = (short)rider->data;
	item = &g_Level.Items[item_number];

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
			rider->pos.xPos = item->pos.xPos;
			rider->pos.yPos = item->pos.yPos;
			rider->pos.zPos = item->pos.zPos;
			rider->pos.yRot = item->pos.yRot;
			rider->roomNumber = item->roomNumber;

			rider->animNumber = Objects[ID_SNOWMOBILE_DRIVER].animIndex + SMAN_DEATH_ANIM;
			rider->frameNumber = g_Level.Anims[rider->animNumber].frameBase;
			rider->currentAnimState = SMAN_DEATH;

			if (Lara.target == item)
				Lara.target = NULL;
		}
		else
			AnimateItem(rider);

		if (item->currentAnimState == SMAN_MOVING || item->currentAnimState == SMAN_WAIT)
			item->goalAnimState = SMAN_WAIT;
		else
			item->goalAnimState = SMAN_MOVING;
	}
	else
	{
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

	if (rider->currentAnimState != SMAN_DEATH)
	{
		if (!skidman->flags && abs(info.angle) < SMAN_TARGET_ANGLE && LaraItem->hitPoints > 0)
		{
			damage = (Lara.Vehicle != NO_ITEM) ? 10 : 50;

			if (ShotLara(item, &info, &skidooLeft, 0, damage) + ShotLara(item, &info, &skidooRight, 0, damage))
				skidman->flags = 5;
		}

		if (skidman->flags)
		{
			SoundEffect(43, &item->pos, 0);
			skidman->flags--;
		}
	}

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

	if (rider->currentAnimState != SMAN_DEATH)
	{
		rider->pos.xPos = item->pos.xPos;
		rider->pos.yPos = item->pos.yPos;
		rider->pos.zPos = item->pos.zPos;
		rider->pos.yRot = item->pos.yRot;
		if (item->roomNumber != rider->roomNumber)
			ItemNewRoom(riderNum, item->roomNumber);

		rider->animNumber = item->animNumber + (Objects[ID_SNOWMOBILE_DRIVER].animIndex - Objects[ID_SNOWMOBILE_GUN].animIndex);
		rider->frameNumber = item->frameNumber + (g_Level.Anims[rider->animNumber].frameBase - g_Level.Anims[item->animNumber].frameBase);
	}
	else if (rider->status == ITEM_DEACTIVATED && item->speed == 0 && item->fallspeed == 0)
	{
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