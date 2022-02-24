#include "framework.h"
#include "Objects/TR1/Entity/tr1_centaur.h"

#include "Game/animation.h"
#include "Game/control/box.h"
#include "Game/collision/collide_item.h"
#include "Game/collision/collide_room.h"
#include "Game/effects/effects.h"
#include "Game/effects/tomb4fx.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_one_gun.h"
#include "Game/people.h"
#include "Sound/sound.h"
#include "Specific/level.h"
#include "Specific/setup.h"

enum CentaurAnims
{ 
	CENTAUR_EMPTY, 
	CENTAUR_STOP, 
	CENTAUR_SHOOT, 
	CENTAUR_RUN, 
	CENTAUR_AIM, 
	CENTAUR_DEATH, 
	CENTAUR_WARNING};

BITE_INFO CentaurRocket = { 11, 415, 41, 13 };
BITE_INFO CentaurRear = { 50, 30, 0, 5 };

#define BOMB_SPEED 256
#define CENTAUR_TOUCH 0x30199
#define CENTAUR_DIE_ANIM 8
#define CENTAUR_TURN ANGLE(4.0f)
#define CENTAUR_REAR_CHANCE 0x60
#define CENTAUR_REAR_RANGE pow(SECTOR(3) / 2, 2)
#define FLYER_PART_DAMAGE 100
#define CENTAUR_REAR_DAMAGE 200

void ControlCentaurBomb(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	int oldX = item->Position.xPos;
	int oldY = item->Position.yPos;
	int oldZ = item->Position.zPos;

	bool aboveWater = false;

	item->Position.zRot += ANGLE(35.0f);
	if (!(g_Level.Rooms[item->RoomNumber].flags & ENV_FLAG_WATER))
	{
		item->Position.xRot -= ANGLE(1.0f);
		if (item->Position.xRot < -16384)
			item->Position.xRot = -16384;

		item->VerticalVelocity = -BOMB_SPEED * phd_sin(item->Position.xRot);
		item->Velocity = BOMB_SPEED * phd_cos(item->Position.xRot);
		aboveWater = true;
	}
	else
	{
		aboveWater = true;
		item->VerticalVelocity += 3;
		if (item->Velocity)
		{
			item->Position.zRot += (((item->Velocity / 4) + 7) * ANGLE(1));
			if (item->RequiredState)
				item->Position.yRot += (((item->Velocity / 2) + 7) * ANGLE(1));
			else
				item->Position.xRot += (((item->Velocity / 2) + 7) * ANGLE(1));

		}
	}

	item->Position.xPos += item->Velocity * phd_cos(item->Position.xRot) * phd_sin(item->Position.yRot);
	item->Position.yPos += item->Velocity * phd_sin(-item->Position.xRot);
	item->Position.zPos += item->Velocity * phd_cos(item->Position.xRot) * phd_cos(item->Position.yRot);

	auto probe = GetCollisionResult(item);

	if (probe.Position.Floor < item->Position.yPos ||
		probe.Position.Ceiling > item->Position.yPos)
	{
		item->Position.xPos = oldX;
		item->Position.yPos = oldY;
		item->Position.zPos = oldZ;

		if (TestEnvironment(ENV_FLAG_WATER, item->RoomNumber))
			TriggerUnderwaterExplosion(item, 0);
		else
		{
			item->Position.yPos -= CLICK(0.5f);
			TriggerShockwave(&item->Position, 48, 304, 96, 0, 96, 128, 24, 0, 0);

			TriggerExplosionSparks(oldX, oldY, oldZ, 3, -2, 0, item->RoomNumber);
			for (int x = 0; x < 2; x++)
				TriggerExplosionSparks(oldX, oldY, oldZ, 3, -1, 0, item->RoomNumber);
		}

		return;
	}

	if (item->RoomNumber != probe.RoomNumber)
		ItemNewRoom(itemNumber, probe.RoomNumber);

	if (TestEnvironment(ENV_FLAG_WATER, item->RoomNumber) && aboveWater)
		SetupRipple(item->Position.xPos, g_Level.Rooms[item->RoomNumber].minfloor, item->Position.zPos, (GetRandomControl() & 7) + 8, 0, Objects[ID_DEFAULT_SPRITES].meshIndex + SPR_RIPPLES);

	int n = 0;
	bool foundCollidedObjects = false;

	GetCollidedObjects(item, HARPOON_HIT_RADIUS, true, &CollidedItems[0], &CollidedMeshes[0], 0);

	if (!CollidedItems[0] && !CollidedMeshes[0])
		return;

	foundCollidedObjects = true;

	if (CollidedItems[0])
	{
		auto* currentItem = CollidedItems[0];

		int k = 0;
		do
		{
			auto* currentObj = &Objects[currentItem->ObjectNumber];

			if (currentObj->intelligent && currentObj->collision && currentItem->Status == ITEM_ACTIVE && !currentObj->undead)
				DoExplosiveDamageOnBaddie(LaraItem, currentItem, item, WEAPON_CROSSBOW);

			k++;
			currentItem = CollidedItems[k];

		} while (currentItem);
	}
}

static void RocketGun(ITEM_INFO* v)
{
	short itemNum;
	itemNum = CreateItem();
	if (itemNum != NO_ITEM)
	{
		auto* item = &g_Level.Items[itemNum];

		item->ObjectNumber = ID_PROJ_BOMB;
		item->Shade = 16 * 256;
		item->RoomNumber = v->RoomNumber;

		PHD_VECTOR pos = { 11, 415, 41 };
		GetJointAbsPosition(v, &pos, 13);

		item->Position.xPos = pos.x;
		item->Position.yPos = pos.y;
		item->Position.zPos = pos.z;
		InitialiseItem(itemNum);

		item->Position.xRot = 0;
		item->Position.yRot = v->Position.yRot;
		item->Position.zRot = 0;

		item->VerticalVelocity = -BOMB_SPEED * phd_cos(item->Position.xRot);
		item->Velocity = BOMB_SPEED *phd_cos(item->Position.xRot);
		item->ItemFlags[0] = 1;

		AddActiveItem(itemNum);
	}
}

void CentaurControl(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];
	auto* creatureInfo = (CREATURE_INFO*)item->Data;

	if (!CreatureActive(itemNumber))
		return;

	short head = 0;
	short angle = 0;

	if (item->HitPoints <= 0)
	{
		if (item->ActiveState != CENTAUR_DEATH)
		{
			item->AnimNumber = Objects[ID_CENTAUR_MUTANT].animIndex + CENTAUR_DIE_ANIM;
			item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
			item->ActiveState = CENTAUR_DEATH;
		}
	}
	else
	{
		AI_INFO info;
		CreatureAIInfo(item, &info);

		if (info.ahead)
			head = info.angle;

		CreatureMood(item, &info, VIOLENT);

		angle = CreatureTurn(item, CENTAUR_TURN);

		switch (item->ActiveState)
		{
		case CENTAUR_STOP:
			CreatureJoint(item, 17, 0);
			if (item->RequiredState)
				item->TargetState = item->RequiredState;
			else if (info.bite && info.distance < CENTAUR_REAR_RANGE)
				item->TargetState = CENTAUR_RUN;
			else if (Targetable(item, &info))
				item->TargetState = CENTAUR_AIM;
			else
				item->TargetState = CENTAUR_RUN;

			break;

		case CENTAUR_RUN:
			if (info.bite && info.distance < CENTAUR_REAR_RANGE)
			{
				item->RequiredState = CENTAUR_WARNING;
				item->TargetState = CENTAUR_STOP;
			}
			else if (Targetable(item, &info))
			{
				item->RequiredState = CENTAUR_AIM;
				item->TargetState = CENTAUR_STOP;
			}
			else if (GetRandomControl() < CENTAUR_REAR_CHANCE)
			{
				item->RequiredState = CENTAUR_WARNING;
				item->TargetState = CENTAUR_STOP;
			}

			break;

		case CENTAUR_AIM:
			if (item->RequiredState)
				item->TargetState = item->RequiredState;
			else if (Targetable(item, &info))
				item->TargetState = CENTAUR_SHOOT;
			else
				item->TargetState = CENTAUR_STOP;

			break;

		case CENTAUR_SHOOT:
			if (!item->RequiredState)
			{
				item->RequiredState = CENTAUR_AIM;
				RocketGun(item);
			}

			break;

		case CENTAUR_WARNING:
			if (!item->RequiredState && item->TouchBits & CENTAUR_TOUCH)
			{
				CreatureEffect(item, &CentaurRear, DoBloodSplat);

				LaraItem->HitPoints -= CENTAUR_REAR_DAMAGE;
				LaraItem->HitStatus = 1;

				item->RequiredState = CENTAUR_STOP;
			}

			break;
		}
	}

	CreatureJoint(item, 0, head);
	CreatureAnimation(itemNumber, angle, 0);

	if (item->Status == ITEM_DEACTIVATED)
	{
		SoundEffect(171, &item->Position, NULL);
		ExplodingDeath(itemNumber, 0xffffffff, FLYER_PART_DAMAGE);
		KillItem(itemNumber);
		item->Status = ITEM_DEACTIVATED;
	}
}
