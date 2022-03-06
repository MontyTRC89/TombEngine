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
#include "Objects/TR2/Vehicles/skidoo.h"
#include "Objects/TR2/Vehicles/skidoo_info.h"
#include "Sound/sound.h"
#include "Specific/level.h"
#include "Specific/setup.h"

enum SKIDMAN_STATE
{
	SMAN_EMPTY,
	SMAN_WAIT,
	SMAN_MOVING,
	SMAN_STARTLEFT,
	SMAN_STARTRIGHT,
	SMAN_LEFT,
	SMAN_RIGHT,
	SMAN_DEATH
};

#define SMAN_MIN_TURN (ANGLE(6.0f)/3)
#define SMAN_TARGET_ANGLE ANGLE(15.0f)
#define SMAN_WAIT_RANGE SQUARE(SECTOR(4))
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
		skidoo->ObjectNumber = ID_SNOWMOBILE_GUN;
		skidoo->Position.xPos = item->Position.xPos;
		skidoo->Position.yPos = item->Position.yPos;
		skidoo->Position.zPos = item->Position.zPos;
		skidoo->Position.yRot = item->Position.yRot;
		skidoo->RoomNumber = item->RoomNumber;
		skidoo->Flags = ITEM_INVISIBLE;
		skidoo->Shade = -1;

		InitialiseItem(skidoo_item);

		// The skidman remembers his skidoo
		item->Data = skidoo_item;

		g_Level.NumItems++;
	}
	else
		TENLog("Can't create skidoo for rider!", LogLevel::Error);
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
		if (item->Velocity > 0)
			ItemPushItem(item, laraitem, coll, coll->Setup.EnableSpasm, 0);
		else
			ItemPushItem(item, laraitem, coll, 0, 0);
	}

	if (Lara.Vehicle == NO_ITEM && item->Velocity > 0)
	{
		laraitem->HitStatus = true;
		laraitem->HitPoints -= 100;
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
	if (rider->Data == NULL)
	{
		TENLog("Rider data does not contain the skidoo itemNumber!", LogLevel::Error);
		return;
	}

	item_number = (short)rider->Data;
	item = &g_Level.Items[item_number];

	if (!item->Data)
	{
		EnableBaddieAI(item_number, TRUE);
		item->Status = ITEM_ACTIVE;
	}

	skidman = (CREATURE_INFO*)item->Data;
	angle = 0;

	if (item->HitPoints <= 0)
	{
		if (rider->ActiveState != SMAN_DEATH)
		{
			rider->Position.xPos = item->Position.xPos;
			rider->Position.yPos = item->Position.yPos;
			rider->Position.zPos = item->Position.zPos;
			rider->Position.yRot = item->Position.yRot;
			rider->RoomNumber = item->RoomNumber;

			rider->AnimNumber = Objects[ID_SNOWMOBILE_DRIVER].animIndex + SMAN_DEATH_ANIM;
			rider->FrameNumber = g_Level.Anims[rider->AnimNumber].frameBase;
			rider->ActiveState = SMAN_DEATH;

			if (Lara.target == item)
				Lara.target = NULL;
		}
		else
			AnimateItem(rider);

		if (item->ActiveState == SMAN_MOVING || item->ActiveState == SMAN_WAIT)
			item->TargetState = SMAN_WAIT;
		else
			item->TargetState = SMAN_MOVING;
	}
	else
	{
		CreatureAIInfo(item, &info);

		GetCreatureMood(item, &info, VIOLENT);
		CreatureMood(item, &info, VIOLENT);

		angle = CreatureTurn(item, ANGLE(6)/2);

		switch (item->ActiveState)
		{
		case SMAN_WAIT:
			if (skidman->mood == MoodType::Bored)
				break;
			else if (abs(info.angle) < SMAN_TARGET_ANGLE && info.distance < SMAN_WAIT_RANGE)
				break;
			item->TargetState = SMAN_MOVING;
			break;

		case SMAN_MOVING:
			if (skidman->mood == MoodType::Bored)
				item->TargetState = SMAN_WAIT;
			else if (abs(info.angle) < SMAN_TARGET_ANGLE && info.distance < SMAN_WAIT_RANGE)
				item->TargetState = SMAN_WAIT;
			else if (angle < -SMAN_MIN_TURN)
				item->TargetState = SMAN_STARTLEFT;
			else if (angle > SMAN_MIN_TURN)
				item->TargetState = SMAN_STARTRIGHT;
			break;

		case SMAN_STARTLEFT:
		case SMAN_LEFT:
			if (angle < -SMAN_MIN_TURN)
				item->TargetState = SMAN_LEFT;
			else
				item->TargetState = SMAN_MOVING;
			break;

		case SMAN_STARTRIGHT:
		case SMAN_RIGHT:
			if (angle < -SMAN_MIN_TURN)
				item->TargetState = SMAN_LEFT;
			else
				item->TargetState = SMAN_MOVING;
			break;
		}
	}

	if (rider->ActiveState != SMAN_DEATH)
	{
		if (!skidman->flags && abs(info.angle) < SMAN_TARGET_ANGLE && LaraItem->HitPoints > 0)
		{
			damage = (Lara.Vehicle != NO_ITEM) ? 10 : 50;

			if (ShotLara(item, &info, &skidooLeft, 0, damage) + ShotLara(item, &info, &skidooRight, 0, damage))
				skidman->flags = 5;
		}

		if (skidman->flags)
		{
			SoundEffect(43, &item->Position, 0);
			skidman->flags--;
		}
	}

	if (item->ActiveState == SMAN_WAIT)
	{
		SoundEffect(153, &item->Position, 0);
		skidman->jointRotation[0] = 0;
	}
	else
	{
		skidman->jointRotation[0] = (skidman->jointRotation[0] == 1) ? 2 : 1;
		DoSnowEffect(item);
		SoundEffect(155, &item->Position, 4 + ((0x10000 - (100 - item->Velocity) * 100) << 8));
	}

	CreatureAnimation(item_number, angle, 0);

	if (rider->ActiveState != SMAN_DEATH)
	{
		rider->Position.xPos = item->Position.xPos;
		rider->Position.yPos = item->Position.yPos;
		rider->Position.zPos = item->Position.zPos;
		rider->Position.yRot = item->Position.yRot;
		if (item->RoomNumber != rider->RoomNumber)
			ItemNewRoom(riderNum, item->RoomNumber);

		rider->AnimNumber = item->AnimNumber + (Objects[ID_SNOWMOBILE_DRIVER].animIndex - Objects[ID_SNOWMOBILE_GUN].animIndex);
		rider->FrameNumber = item->FrameNumber + (g_Level.Anims[rider->AnimNumber].frameBase - g_Level.Anims[item->AnimNumber].frameBase);
	}
	else if (rider->Status == ITEM_DEACTIVATED && item->Velocity == 0 && item->VerticalVelocity == 0)
	{
		RemoveActiveItem(riderNum);
		rider->Collidable = false;
		rider->HitPoints = -16384;
		rider->Flags |= ONESHOT;

		DisableEntityAI(item_number);
		item->ObjectNumber = ID_SNOWMOBILE;
		item->Status = ITEM_DEACTIVATED;
		InitialiseSkidoo(item_number);

		((SkidooInfo*)item->Data)->Armed = true;
	}
}
