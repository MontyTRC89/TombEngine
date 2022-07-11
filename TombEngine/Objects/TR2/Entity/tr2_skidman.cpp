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
#include "Game/misc.h"
#include "Game/people.h"
#include "Objects/TR2/Vehicles/skidoo.h"
#include "Objects/TR2/Vehicles/skidoo_info.h"
#include "Sound/sound.h"
#include "Specific/level.h"
#include "Specific/setup.h"

enum SnowmobileManState
{
	SMAN_STATE_NONE = 0,
	SMAN_STATE_WAIT = 1,
	SMAN_STATE_MOVING = 2,
	SMAN_STATE_START_LEFT = 3,
	SMAN_STATE_START_RIGHT = 4,
	SMAN_STATE_LEFT = 5,
	SMAN_STATE_RIGHT = 6,
	SMAN_STATE_DEATH = 7
};

// TODO
enum SkidooManAnim
{
	SMAN_ANIM_DEATH = 10
};

#define SMAN_MIN_TURN (ANGLE(2.0f))
#define SMAN_TARGET_ANGLE ANGLE(15.0f)
#define SMAN_WAIT_RANGE pow(SECTOR(4), 2)

BITE_INFO SkidooBiteLeft = { 240, -190, 540, 0 };
BITE_INFO SkidooBiteRight = { -240, -190, 540, 0 };

void InitialiseSkidooMan(short itemNumber)
{
	short skidooItemNumber = CreateItem();
	if (skidooItemNumber != NO_ITEM)
	{
		auto* riderItem = &g_Level.Items[itemNumber];
		auto* skidooItem = &g_Level.Items[skidooItemNumber];

		skidooItem->ObjectNumber = ID_SNOWMOBILE_GUN;
		skidooItem->Pose.Position.x = riderItem->Pose.Position.x;
		skidooItem->Pose.Position.y = riderItem->Pose.Position.y;
		skidooItem->Pose.Position.z = riderItem->Pose.Position.z;
		skidooItem->Pose.Orientation.y = riderItem->Pose.Orientation.y;
		skidooItem->RoomNumber = riderItem->RoomNumber;
		skidooItem->Flags = ITEM_INVISIBLE;
		skidooItem->Color = Vector4(0.5f, 0.5f, 0.5f, 1.0f);

		InitialiseItem(skidooItemNumber);

		// The rider remembers his skidoo.
		riderItem->Data = skidooItemNumber;

		g_Level.NumItems++;
	}
	else
		TENLog("Can't create skidoo for rider!", LogLevel::Error);
}

void SkidooManCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
{
	auto* item = &g_Level.Items[itemNumber];
	
	if (!TestBoundsCollide(item, laraItem, coll->Setup.Radius))
		return;

	if (!TestCollision(item, laraItem))
		return;

	if (coll->Setup.EnableObjectPush)
	{
		if (item->Animation.Velocity > 0)
			ItemPushItem(item, laraItem, coll, coll->Setup.EnableSpasm, 0);
		else
			ItemPushItem(item, laraItem, coll, 0, 0);
	}

	if (Lara.Vehicle == NO_ITEM && item->Animation.Velocity > 0)
	{
		DoDamage(laraItem, 100);
	}
}

void SkidooManControl(short riderItemNumber)
{

	auto* riderItem = &g_Level.Items[riderItemNumber];
	if (riderItem->Data == NULL)
	{
		TENLog("Rider data does not contain the skidoo itemNumber!", LogLevel::Error);
		return;
	}

	short itemNumber = (short)riderItem->Data;
	auto* item = &g_Level.Items[itemNumber];

	if (!item->Data)
	{
		EnableEntityAI(itemNumber, TRUE);
		item->Status = ITEM_ACTIVE;
	}

	auto* creatureInfo = GetCreatureInfo(item);
	short angle = 0;
	int damage;

	AI_INFO AI;
	if (item->HitPoints <= 0)
	{
		if (riderItem->Animation.ActiveState != SMAN_STATE_DEATH)
		{
			riderItem->Pose.Position.x = item->Pose.Position.x;
			riderItem->Pose.Position.y = item->Pose.Position.y;
			riderItem->Pose.Position.z = item->Pose.Position.z;
			riderItem->Pose.Orientation.y = item->Pose.Orientation.y;
			riderItem->RoomNumber = item->RoomNumber;

			riderItem->Animation.AnimNumber = Objects[ID_SNOWMOBILE_DRIVER].animIndex + SMAN_ANIM_DEATH;
			riderItem->Animation.FrameNumber = g_Level.Anims[riderItem->Animation.AnimNumber].frameBase;
			riderItem->Animation.ActiveState = SMAN_STATE_DEATH;

			if (Lara.TargetEntity == item)
				Lara.TargetEntity = NULL;
		}
		else
			AnimateItem(riderItem);

		if (item->Animation.ActiveState == SMAN_STATE_MOVING || item->Animation.ActiveState == SMAN_STATE_WAIT)
			item->Animation.TargetState = SMAN_STATE_WAIT;
		else
			item->Animation.TargetState = SMAN_STATE_MOVING;
	}
	else
	{
		CreatureAIInfo(item, &AI);

		GetCreatureMood(item, &AI, VIOLENT);
		CreatureMood(item, &AI, VIOLENT);

		angle = CreatureTurn(item, ANGLE(3.0f));

		switch (item->Animation.ActiveState)
		{
		case SMAN_STATE_WAIT:
			if (creatureInfo->Mood == MoodType::Bored)
				break;
			else if (abs(AI.angle) < SMAN_TARGET_ANGLE && AI.distance < SMAN_WAIT_RANGE)
				break;

			item->Animation.TargetState = SMAN_STATE_MOVING;
			break;

		case SMAN_STATE_MOVING:
			if (creatureInfo->Mood == MoodType::Bored)
				item->Animation.TargetState = SMAN_STATE_WAIT;
			else if (abs(AI.angle) < SMAN_TARGET_ANGLE && AI.distance < SMAN_WAIT_RANGE)
				item->Animation.TargetState = SMAN_STATE_WAIT;
			else if (angle < -SMAN_MIN_TURN)
				item->Animation.TargetState = SMAN_STATE_START_LEFT;
			else if (angle > SMAN_MIN_TURN)
				item->Animation.TargetState = SMAN_STATE_START_RIGHT;

			break;

		case SMAN_STATE_START_LEFT:
		case SMAN_STATE_LEFT:
			if (angle < -SMAN_MIN_TURN)
				item->Animation.TargetState = SMAN_STATE_LEFT;
			else
				item->Animation.TargetState = SMAN_STATE_MOVING;

			break;

		case SMAN_STATE_START_RIGHT:
		case SMAN_STATE_RIGHT:
			if (angle < -SMAN_MIN_TURN)
				item->Animation.TargetState = SMAN_STATE_LEFT;
			else
				item->Animation.TargetState = SMAN_STATE_MOVING;

			break;
		}
	}

	if (riderItem->Animation.ActiveState != SMAN_STATE_DEATH)
	{
		if (!creatureInfo->Flags && abs(AI.angle) < SMAN_TARGET_ANGLE && LaraItem->HitPoints > 0)
		{
			damage = (Lara.Vehicle != NO_ITEM) ? 10 : 50;

			if (ShotLara(item, &AI, &SkidooBiteLeft, 0, damage) + ShotLara(item, &AI, &SkidooBiteRight, 0, damage))
				creatureInfo->Flags = 5;
		}

		if (creatureInfo->Flags)
		{
			SoundEffect(SFX_TR4_BADDY_UZI, &item->Pose);
			creatureInfo->Flags--;
		}
	}

	if (item->Animation.ActiveState == SMAN_STATE_WAIT)
	{
		SoundEffect(SFX_TR2_VEHICLE_SNOWMOBILE_IDLE, &item->Pose);
		creatureInfo->JointRotation[0] = 0;
	}
	else
	{
		creatureInfo->JointRotation[0] = (creatureInfo->JointRotation[0] == 1) ? 2 : 1;
		DoSnowEffect(item);
		SoundEffect(SFX_TR2_VEHICLE_SNOWMOBILE_IDLE, &item->Pose, SoundEnvironment::Land, 0.5f + item->Animation.Velocity / 100.0f); // SKIDOO_MAX_VELOCITY.  TODO: Check actual sound!
	}

	CreatureAnimation(itemNumber, angle, 0);

	if (riderItem->Animation.ActiveState != SMAN_STATE_DEATH)
	{
		riderItem->Pose.Position.x = item->Pose.Position.x;
		riderItem->Pose.Position.y = item->Pose.Position.y;
		riderItem->Pose.Position.z = item->Pose.Position.z;
		riderItem->Pose.Orientation.y = item->Pose.Orientation.y;

		if (item->RoomNumber != riderItem->RoomNumber)
			ItemNewRoom(riderItemNumber, item->RoomNumber);

		riderItem->Animation.AnimNumber = item->Animation.AnimNumber + (Objects[ID_SNOWMOBILE_DRIVER].animIndex - Objects[ID_SNOWMOBILE_GUN].animIndex);
		riderItem->Animation.FrameNumber = item->Animation.FrameNumber + (g_Level.Anims[riderItem->Animation.AnimNumber].frameBase - g_Level.Anims[item->Animation.AnimNumber].frameBase);
	}
	else if (riderItem->Status == ITEM_DEACTIVATED &&
			 item->Animation.Velocity == 0 &&
			 item->Animation.VerticalVelocity == 0)
	{
		RemoveActiveItem(riderItemNumber);
		riderItem->Collidable = false;
		riderItem->HitPoints = NOT_TARGETABLE;
		riderItem->Flags |= IFLAG_INVISIBLE;

		DisableEntityAI(itemNumber);
		item->ObjectNumber = ID_SNOWMOBILE;
		item->Status = ITEM_DEACTIVATED;
		InitialiseSkidoo(itemNumber);

		((SkidooInfo*)item->Data)->Armed = true;
	}
}
