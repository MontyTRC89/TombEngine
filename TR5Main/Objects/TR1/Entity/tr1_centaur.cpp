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
#include "Game/misc.h"
#include "Game/people.h"
#include "Sound/sound.h"
#include "Specific/level.h"
#include "Specific/setup.h"

BITE_INFO CentaurRocketBite = { 11, 415, 41, 13 };
BITE_INFO CentaurRearBite = { 50, 30, 0, 5 };

#define BOMB_SPEED 256
#define CENTAUR_TOUCH 0x30199
#define CENTAUR_TURN ANGLE(4.0f)
#define CENTAUR_REAR_CHANCE 0x60
#define CENTAUR_REAR_RANGE pow(SECTOR(1.5f), 2)
#define FLYER_PART_DAMAGE 100
#define CENTAUR_REAR_DAMAGE 200

enum CentaurState
{
	CENTAUR_STATE_NONE = 0,
	CENTAUR_STATE_IDLE = 1,
	CENTAUR_STATE_SHOOT = 2,
	CENTAUR_STATE_RUN = 3,
	CENTAUR_STATE_AIM = 4,
	CENTAUR_STATE_DEATH = 5,
	CENTAUR_STATE_WARNING = 6
};

// TODO
enum CentaurAnim
{
	CENTAUR_ANIM_DEATH = 8,
};

void ControlCentaurBomb(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	bool aboveWater = false;
	PHD_VECTOR oldPos = { item->Position.xPos, item->Position.yPos, item->Position.zPos };

	item->Position.zRot += ANGLE(35.0f);
	if (!TestEnvironment(ENV_FLAG_WATER, item->RoomNumber))
	{
		item->Position.xRot -= ANGLE(1.0f);
		if (item->Position.xRot < -ANGLE(90.0f))
			item->Position.xRot = -ANGLE(90.0f);

		aboveWater = true;
		item->Velocity = BOMB_SPEED * phd_cos(item->Position.xRot);
		item->VerticalVelocity = -BOMB_SPEED * phd_sin(item->Position.xRot);
	}
	else
	{
		aboveWater = true;
		item->VerticalVelocity += 3;

		if (item->Velocity)
		{
			item->Position.zRot += ((item->Velocity / 4) + 7) * ANGLE(1.0f);

			if (item->RequiredState)
				item->Position.yRot += ((item->Velocity / 2) + 7) * ANGLE(1.0f);
			else
				item->Position.xRot += ((item->Velocity / 2) + 7) * ANGLE(1.0f);

		}
	}

	item->Position.xPos += item->Velocity * phd_cos(item->Position.xRot) * phd_sin(item->Position.yRot);
	item->Position.yPos += item->Velocity * phd_sin(-item->Position.xRot);
	item->Position.zPos += item->Velocity * phd_cos(item->Position.xRot) * phd_cos(item->Position.yRot);

	auto probe = GetCollisionResult(item);

	if (probe.Position.Floor < item->Position.yPos ||
		probe.Position.Ceiling > item->Position.yPos)
	{
		item->Position.xPos = oldPos.x;
		item->Position.yPos = oldPos.y;
		item->Position.zPos = oldPos.z;

		if (TestEnvironment(ENV_FLAG_WATER, item->RoomNumber))
			TriggerUnderwaterExplosion(item, 0);
		else
		{
			item->Position.yPos -= CLICK(0.5f);
			TriggerShockwave(&item->Position, 48, 304, 96, 0, 96, 128, 24, 0, 0);

			TriggerExplosionSparks(oldPos.x, oldPos.y, oldPos.z, 3, -2, 0, item->RoomNumber);
			for (int x = 0; x < 2; x++)
				TriggerExplosionSparks(oldPos.x, oldPos.y, oldPos.z, 3, -1, 0, item->RoomNumber);
		}

		return;
	}

	if (item->RoomNumber != probe.RoomNumber)
		ItemNewRoom(itemNumber, probe.RoomNumber);

	if (TestEnvironment(ENV_FLAG_WATER, item->RoomNumber) && aboveWater)
		SetupRipple(item->Position.xPos, g_Level.Rooms[item->RoomNumber].minfloor, item->Position.zPos, (GetRandomControl() & 7) + 8, 0, Objects[ID_DEFAULT_SPRITES].meshIndex + SPR_RIPPLES);

	GetCollidedObjects(item, HARPOON_HIT_RADIUS, true, &CollidedItems[0], &CollidedMeshes[0], 0);

	if (!CollidedItems[0] && !CollidedMeshes[0])
		return;

	if (CollidedItems[0])
	{
		auto* currentItem = CollidedItems[0];

		int k = 0;
		do
		{
			auto* currentObject = &Objects[currentItem->ObjectNumber];

			if (currentItem->Status == ITEM_ACTIVE &&
				currentObject->intelligent && !currentObject->undead &&
				currentObject->collision)
			{
				DoExplosiveDamageOnBaddie(LaraItem, currentItem, item, WEAPON_CROSSBOW);
			}

			k++;
			currentItem = CollidedItems[k];

		} while (currentItem);
	}
}

static void RocketGun(ITEM_INFO* centaurItem)
{
	short itemNumber;
	itemNumber = CreateItem();

	if (itemNumber != NO_ITEM)
	{
		auto* projectileItem = &g_Level.Items[itemNumber];

		projectileItem->ObjectNumber = ID_PROJ_BOMB;
		projectileItem->Shade = 16 * 256;
		projectileItem->RoomNumber = centaurItem->RoomNumber;

		PHD_VECTOR pos = { 11, 415, 41 };
		GetJointAbsPosition(centaurItem, &pos, 13);

		projectileItem->Position.xPos = pos.x;
		projectileItem->Position.yPos = pos.y;
		projectileItem->Position.zPos = pos.z;
		InitialiseItem(itemNumber);

		projectileItem->Position.xRot = 0;
		projectileItem->Position.yRot = centaurItem->Position.yRot;
		projectileItem->Position.zRot = 0;

		projectileItem->Velocity = BOMB_SPEED * phd_cos(projectileItem->Position.xRot);
		projectileItem->VerticalVelocity = -BOMB_SPEED * phd_cos(projectileItem->Position.xRot);
		projectileItem->ItemFlags[0] = 1;

		AddActiveItem(itemNumber);
	}
}

void CentaurControl(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];
	auto* info = GetCreatureInfo(item);

	if (!CreatureActive(itemNumber))
		return;

	short head = 0;
	short angle = 0;

	if (item->HitPoints <= 0)
	{
		if (item->ActiveState != CENTAUR_STATE_DEATH)
		{
			item->AnimNumber = Objects[ID_CENTAUR_MUTANT].animIndex + CENTAUR_ANIM_DEATH;
			item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
			item->ActiveState = CENTAUR_STATE_DEATH;
		}
	}
	else
	{
		AI_INFO aiInfo;
		CreatureAIInfo(item, &aiInfo);

		if (aiInfo.ahead)
			head = aiInfo.angle;

		CreatureMood(item, &aiInfo, VIOLENT);

		angle = CreatureTurn(item, CENTAUR_TURN);

		switch (item->ActiveState)
		{
		case CENTAUR_STATE_IDLE:
			CreatureJoint(item, 17, 0);
			if (item->RequiredState)
				item->TargetState = item->RequiredState;
			else if (aiInfo.bite && aiInfo.distance < CENTAUR_REAR_RANGE)
				item->TargetState = CENTAUR_STATE_RUN;
			else if (Targetable(item, &aiInfo))
				item->TargetState = CENTAUR_STATE_AIM;
			else
				item->TargetState = CENTAUR_STATE_RUN;

			break;

		case CENTAUR_STATE_RUN:
			if (aiInfo.bite && aiInfo.distance < CENTAUR_REAR_RANGE)
			{
				item->RequiredState = CENTAUR_STATE_WARNING;
				item->TargetState = CENTAUR_STATE_IDLE;
			}
			else if (Targetable(item, &aiInfo))
			{
				item->RequiredState = CENTAUR_STATE_AIM;
				item->TargetState = CENTAUR_STATE_IDLE;
			}
			else if (GetRandomControl() < CENTAUR_REAR_CHANCE)
			{
				item->RequiredState = CENTAUR_STATE_WARNING;
				item->TargetState = CENTAUR_STATE_IDLE;
			}

			break;

		case CENTAUR_STATE_AIM:
			if (item->RequiredState)
				item->TargetState = item->RequiredState;
			else if (Targetable(item, &aiInfo))
				item->TargetState = CENTAUR_STATE_SHOOT;
			else
				item->TargetState = CENTAUR_STATE_IDLE;

			break;

		case CENTAUR_STATE_SHOOT:
			if (!item->RequiredState)
			{
				item->RequiredState = CENTAUR_STATE_AIM;
				RocketGun(item);
			}

			break;

		case CENTAUR_STATE_WARNING:
			if (!item->RequiredState && item->TouchBits & CENTAUR_TOUCH)
			{
				CreatureEffect(item, &CentaurRearBite, DoBloodSplat);
				item->RequiredState = CENTAUR_STATE_IDLE;

				LaraItem->HitPoints -= CENTAUR_REAR_DAMAGE;
				LaraItem->HitStatus = 1;
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
