#include "framework.h"
#include "Objects/TR1/Entity/tr1_centaur.h"

#include "Game/animation.h"
#include "Game/control/box.h"
#include "Game/collision/collide_item.h"
#include "Game/effects/effects.h"
#include "Game/effects/tomb4fx.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_one_gun.h"
#include "Game/people.h"
#include "Sound/sound.h"
#include "Specific/level.h"
#include "Specific/setup.h"

enum centaur_anims { 
	CENTAUR_EMPTY, 
	CENTAUR_STOP, 
	CENTAUR_SHOOT, 
	CENTAUR_RUN, 
	CENTAUR_AIM, 
	CENTAUR_DEATH, 
	CENTAUR_WARNING};

BITE_INFO centaur_rocket = { 11, 415, 41, 13 };
BITE_INFO centaur_rear = { 50, 30, 0, 5 };

#define BOMB_SPEED		256

#define CENTAUR_TOUCH 0x30199

#define CENTAUR_DIE_ANIM 8

#define CENTAUR_TURN ANGLE(4)

#define CENTAUR_REAR_CHANCE 0x60

#define CENTAUR_REAR_RANGE SQUARE(WALL_SIZE*3/2)

#define FLYER_PART_DAMAGE 100

#define CENTAUR_REAR_DAMAGE 200

void ControlCentaurBomb(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	int oldX = item->pos.xPos;
	int oldY = item->pos.yPos;
	int oldZ = item->pos.zPos;
	short roomNumber = item->roomNumber;

	bool aboveWater = false;

	item->pos.zRot += ANGLE(35);
	if (!(g_Level.Rooms[item->roomNumber].flags & ENV_FLAG_WATER))
	{
		item->pos.xRot -= ANGLE(1);
		if (item->pos.xRot < -16384)
			item->pos.xRot = -16384;
		item->VerticalVelocity = -BOMB_SPEED * phd_sin(item->pos.xRot);
		item->Velocity = BOMB_SPEED * phd_cos(item->pos.xRot);
		aboveWater = true;
	}
	else
	{
		aboveWater = true;
		item->VerticalVelocity += 3;
		if (item->Velocity)
		{
			item->pos.zRot += (((item->Velocity / 4) + 7) * ANGLE(1));
			if (item->requiredState)
				item->pos.yRot += (((item->Velocity / 2) + 7) * ANGLE(1));
			else
				item->pos.xRot += (((item->Velocity / 2) + 7) * ANGLE(1));

		}
	}

	item->pos.xPos += item->Velocity * phd_cos(item->pos.xRot) * phd_sin(item->pos.yRot);
	item->pos.yPos += item->Velocity * phd_sin(-item->pos.xRot);
	item->pos.zPos += item->Velocity * phd_cos(item->pos.xRot) * phd_cos(item->pos.yRot);

	roomNumber = item->roomNumber;
	FLOOR_INFO * floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);

	if (GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos) < item->pos.yPos ||
		GetCeiling(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos) > item->pos.yPos)
	{
		item->pos.xPos = oldX;
		item->pos.yPos = oldY;
		item->pos.zPos = oldZ;
		if (g_Level.Rooms[item->roomNumber].flags & ENV_FLAG_WATER)
		{
			TriggerUnderwaterExplosion(item, 0);
		}
		else
		{
			item->pos.yPos -= 128;
			TriggerShockwave(&item->pos, 48, 304, 96, 0, 96, 128, 24, 0, 0);

			TriggerExplosionSparks(oldX, oldY, oldZ, 3, -2, 0, item->roomNumber);
			for (int x = 0; x < 2; x++)
				TriggerExplosionSparks(oldX, oldY, oldZ, 3, -1, 0, item->roomNumber);
		}
		return;
	}

	if (item->roomNumber != roomNumber)
		ItemNewRoom(itemNumber, roomNumber);

	if ((g_Level.Rooms[item->roomNumber].flags & ENV_FLAG_WATER) && aboveWater)
	{
		SetupRipple(item->pos.xPos, g_Level.Rooms[item->roomNumber].minfloor, item->pos.zPos, (GetRandomControl() & 7) + 8, 0, Objects[ID_DEFAULT_SPRITES].meshIndex + SPR_RIPPLES);
	}

	int n = 0;
	bool foundCollidedObjects = false;

	GetCollidedObjects(item, HARPOON_HIT_RADIUS, true, &CollidedItems[0], &CollidedMeshes[0], 0);

	if (!CollidedItems[0] && !CollidedMeshes[0])
		return;

	foundCollidedObjects = true;

	if (CollidedItems[0])
	{
		ITEM_INFO* currentItem = CollidedItems[0];

		int k = 0;
		do
		{
			OBJECT_INFO* currentObj = &Objects[currentItem->objectNumber];

			if (currentObj->intelligent && currentObj->collision && currentItem->status == ITEM_ACTIVE && !currentObj->undead)
			{
				DoExplosiveDamageOnBaddie(currentItem, item, WEAPON_CROSSBOW);
			}

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
		PHD_VECTOR pos;
		ITEM_INFO* item;

		item = &g_Level.Items[itemNum];

		item->objectNumber = ID_PROJ_BOMB;
		item->shade = 16 * 256;
		item->roomNumber = v->roomNumber;

		pos.x = 11;
		pos.y = 415;
		pos.z = 41;
		GetJointAbsPosition(v, &pos, 13);

		item->pos.xPos = pos.x;
		item->pos.yPos = pos.y;
		item->pos.zPos = pos.z;
		InitialiseItem(itemNum);

		item->pos.xRot = 0;
		item->pos.yRot = v->pos.yRot;
		item->pos.zRot = 0;

		item->VerticalVelocity = -BOMB_SPEED * phd_cos(item->pos.xRot);
		item->Velocity = BOMB_SPEED *phd_cos(item->pos.xRot);
		item->itemFlags[0] = 1;

		AddActiveItem(itemNum);
	}
}

void CentaurControl(short itemNum)
{
	ITEM_INFO *item;
	CREATURE_INFO *centaur;
	short angle, head, fx_number;
	AI_INFO info;

	item = &g_Level.Items[itemNum];

	if (!CreatureActive(itemNum))
		return;

	centaur = (CREATURE_INFO *)item->data;
	head = angle = 0;

	if (item->hitPoints <= 0)
	{
		if (item->activeState != CENTAUR_DEATH)
		{
			item->animNumber = Objects[ID_CENTAUR_MUTANT].animIndex + CENTAUR_DIE_ANIM;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			item->activeState = CENTAUR_DEATH;
		}
	}
	else
	{
		CreatureAIInfo(item, &info);

		if (info.ahead)
			head = info.angle;

		CreatureMood(item, &info, VIOLENT);

		angle = CreatureTurn(item, CENTAUR_TURN);

		switch (item->activeState)
		{
		case CENTAUR_STOP:
			CreatureJoint(item, 17, 0);
			if (item->requiredState)
				item->targetState = item->requiredState;
			else if (info.bite && info.distance < CENTAUR_REAR_RANGE)
				item->targetState = CENTAUR_RUN;
			else if (Targetable(item, &info))
				item->targetState = CENTAUR_AIM;
			else
				item->targetState = CENTAUR_RUN;
			break;

		case CENTAUR_RUN:
			if (info.bite && info.distance < CENTAUR_REAR_RANGE)
			{
				item->requiredState = CENTAUR_WARNING;
				item->targetState = CENTAUR_STOP;
			}
			else if (Targetable(item, &info))
			{
				item->requiredState = CENTAUR_AIM;
				item->targetState = CENTAUR_STOP;
			}
			else if (GetRandomControl() < CENTAUR_REAR_CHANCE)
			{
				item->requiredState = CENTAUR_WARNING;
				item->targetState = CENTAUR_STOP;
			}
			break;

		case CENTAUR_AIM:
			if (item->requiredState)
				item->targetState = item->requiredState;
			else if (Targetable(item, &info))
				item->targetState = CENTAUR_SHOOT;
			else
				item->targetState = CENTAUR_STOP;
			break;

		case CENTAUR_SHOOT:
			if (!item->requiredState)
			{
				item->requiredState = CENTAUR_AIM;
				RocketGun(item);
			}
			break;

		case CENTAUR_WARNING:
			if (!item->requiredState && (item->touchBits & CENTAUR_TOUCH))
			{
				CreatureEffect(item, &centaur_rear, DoBloodSplat);

				LaraItem->hitPoints -= CENTAUR_REAR_DAMAGE;
				LaraItem->hitStatus = 1;

				item->requiredState = CENTAUR_STOP;
			}
			break;
		}
	}

	CreatureJoint(item, 0, head);
	CreatureAnimation(itemNum, angle, 0);

	if (item->status == ITEM_DEACTIVATED)
	{
		SoundEffect(171, &item->pos, NULL);
		ExplodingDeath(itemNum, 0xffffffff, FLYER_PART_DAMAGE);
		KillItem(itemNum);
		item->status = ITEM_DEACTIVATED;
	}
}
