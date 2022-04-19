#include "framework.h"
#include "tr4_crocodile.h"
#include "Game/control/box.h"
#include "Game/effects/effects.h"
#include "Game/people.h"
#include "Game/items.h"
#include "Specific/setup.h"
#include "Specific/level.h"
#include "Game/control/control.h"
#include "Game/Lara/lara.h"
#include "Game/animation.h"
#include "Game/misc.h"
#include "Game/itemdata/creature_info.h"
#include "Game/collision/collide_room.h"

static BITE_INFO CrocodileBite = { 0, -100, 500, 9 };

#define CROC_STATE_WALK_ANGLE ANGLE(3.0f)
#define CROC_SWIM_ANGLE ANGLE(3.0f)
#define CROC_STATE_RUN_ANGLE ANGLE(5.0f)
#define CROC_SWIM_SPEED 16;
#define CROC_DAMAGE 120;

constexpr auto CROC_ALERT_RANGE = SQUARE(SECTOR(1) + CLICK(2));
constexpr auto CROC_VISIBILITY_RANGE = SQUARE(SECTOR(5));
constexpr auto CROC_STATE_RUN_RANGE = SQUARE(SECTOR(1));
constexpr auto CROC_MAXRUN_RANGE = SQUARE(SECTOR(1) + CLICK(2));
constexpr auto CROC_ATTACK_RANGE = SQUARE(CLICK(3)); // NOTE: TR4 is CLICK(3), but the crocodile not go near lara to do damage in certain case !
constexpr auto CROC_TOUCHBITS = 768;

enum CrocodileState
{
	CROC_STATE_NONE_1 = 0,
	CROC_STATE_IDLE = 1,
	CROC_STATE_RUN = 2,
	CROC_STATE_WALK = 3,
	CROC_STATE_HIT = 4,
	CROC_STATE_ATTACK = 5,
	CROC_STATE_NONE_2 = 6,
	CROC_STATE_DEATH = 7,
	CROC_STATE_UNDERWATER_SWIM = 8,
	CROC_STATE_UNDERWATER_ATTACK = 9,
	CROC_STATE_UNDERWATER_DEATH = 10,
};

// TODO
enum CrocodileAnim
{
	CROC_ANIM_IDLE = 0,

	CROC_ANIM_DEATH = 11,
	CROC_ANIM_SWIM = 12,

	CROC_ANIM_UNDERWATER_DEATH = 16,
	CROC_ANIM_SWIM_MODE = 17,
	CROC_ANIM_LAND_MODE = 18
};

void InitialiseCrocodile(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	InitialiseCreature(itemNumber);

	if (TestEnvironment(ENV_FLAG_WATER, item))
	{
		item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + CROC_ANIM_SWIM;
		item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
		item->Animation.ActiveState = CROC_STATE_UNDERWATER_SWIM;
		item->Animation.TargetState = CROC_STATE_UNDERWATER_SWIM;
	}
	else
	{
		item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + CROC_ANIM_IDLE;
		item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
		item->Animation.ActiveState = CROC_STATE_IDLE;
		item->Animation.TargetState = CROC_STATE_IDLE;
	}
}

static bool CrocodileIsInWater(ITEM_INFO* item)
{
	auto* info = GetCreatureInfo(item);

	EntityStoringInfo storingInfo;
	storingInfo.x = item->Pose.Position.x;
	storingInfo.y = item->Pose.Position.y;
	storingInfo.z = item->Pose.Position.z;
	storingInfo.roomNumber = item->RoomNumber;

	GetFloor(storingInfo.x, storingInfo.y, storingInfo.z, &storingInfo.roomNumber);
	storingInfo.waterDepth = GetWaterSurface(storingInfo.x, storingInfo.y, storingInfo.z, storingInfo.roomNumber);

	if (storingInfo.waterDepth != NO_HEIGHT)
	{
		info->LOT.Step = SECTOR(20);
		info->LOT.Drop = -SECTOR(20);
		info->LOT.Fly = CROC_SWIM_SPEED;
		return true;
	}
	else
	{
		info->LOT.Step = CLICK(1);
		info->LOT.Drop = -CLICK(1);
		info->LOT.Fly = NO_FLYING;
		return false;
	}
}

void CrocodileControl(short itemNumber)
{
	if (!CreatureActive(itemNumber))
		return;

	auto* item = &g_Level.Items[itemNumber];
	auto* creature = GetCreatureInfo(item);
	auto* object = &Objects[item->ObjectNumber];

	short angle = 0;
	short boneAngle = 0;

	AI_INFO AI;

	if (item->HitPoints <= 0)
	{
		if (item->Animation.ActiveState != CROC_STATE_DEATH && item->Animation.ActiveState != CROC_STATE_UNDERWATER_DEATH)
		{
			if (TestEnvironment(ENV_FLAG_WATER, item))
			{
				item->Animation.AnimNumber = object->animIndex + CROC_ANIM_UNDERWATER_DEATH;
				item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
				item->Animation.ActiveState = CROC_STATE_UNDERWATER_DEATH;
				item->Animation.TargetState = CROC_STATE_UNDERWATER_DEATH;
			}
			else
			{
				item->Animation.AnimNumber = object->animIndex + CROC_ANIM_DEATH;
				item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
				item->Animation.ActiveState = CROC_STATE_DEATH;
				item->Animation.TargetState = CROC_STATE_DEATH;
			}
		}

		if (TestEnvironment(ENV_FLAG_WATER, item))
			CreatureFloat(itemNumber);
	}
	else
	{
		if (item->AIBits & ALL_AIOBJ)
			GetAITarget(creature);
		else if (creature->HurtByLara)
			creature->Enemy = LaraItem;

		CreatureAIInfo(item, &AI);

		GetCreatureMood(item, &AI, VIOLENT);
		CreatureMood(item, &AI, VIOLENT);

		angle = CreatureTurn(item, creature->MaxTurn);

		if ((item->HitStatus || AI.distance < CROC_ALERT_RANGE) ||
			(TargetVisible(item, &AI) && AI.distance < CROC_VISIBILITY_RANGE))
		{
			if (!creature->Alerted)
				creature->Alerted = true;

			AlertAllGuards(itemNumber);
		}

		boneAngle = angle * 4;

		switch (item->Animation.ActiveState)
		{
		case CROC_STATE_IDLE:
			creature->MaxTurn = 0;

			if (item->AIBits & GUARD)
			{
				boneAngle = item->ItemFlags[0];
				item->Animation.TargetState = CROC_STATE_IDLE;
				item->ItemFlags[0] = item->ItemFlags[1] + boneAngle;

				if (!(GetRandomControl() & 0x1F))
				{
					if (GetRandomControl() & 1)
						item->ItemFlags[1] = 0;
					else
						item->ItemFlags[1] = (GetRandomControl() & 1) != 0 ? 12 : -12;
				}

				if (item->ItemFlags[0] < -1024)
					item->ItemFlags[0] = -1024;
				else if (item->ItemFlags[0] > 1024)
					item->ItemFlags[0] = 1024;
			}
			else if (AI.bite && AI.distance < CROC_ATTACK_RANGE)
				item->Animation.TargetState = CROC_STATE_ATTACK;
			else
			{
				if (AI.ahead && AI.distance < CROC_STATE_RUN_RANGE)
					item->Animation.TargetState = CROC_STATE_WALK;
				else
					item->Animation.TargetState = CROC_STATE_RUN;
			}

			break;

		case CROC_STATE_WALK:
			creature->MaxTurn = CROC_STATE_WALK_ANGLE;

			// Land to water transition.
			if (CrocodileIsInWater(item))
			{
				item->Animation.RequiredState = CROC_STATE_UNDERWATER_SWIM;
				item->Animation.TargetState = CROC_STATE_UNDERWATER_SWIM;
				break;
			}

			if (item->Animation.RequiredState)
				item->Animation.TargetState = item->Animation.RequiredState;
			else if (AI.bite && AI.distance < CROC_ATTACK_RANGE)
				item->Animation.TargetState = CROC_STATE_IDLE;
			else if (!AI.ahead || AI.distance > CROC_MAXRUN_RANGE)
				item->Animation.TargetState = CROC_STATE_RUN;

			break;

		case CROC_STATE_RUN:
			creature->MaxTurn = CROC_STATE_RUN_ANGLE;

			// Land to water transition.
			if (CrocodileIsInWater(item))
			{
				item->Animation.RequiredState = CROC_STATE_WALK;
				item->Animation.TargetState = CROC_STATE_WALK;
				break;
			}

			if (item->Animation.RequiredState)
				item->Animation.TargetState = item->Animation.RequiredState;
			else if (AI.bite && AI.distance < CROC_ATTACK_RANGE)
				item->Animation.TargetState = CROC_STATE_IDLE;
			else if (AI.ahead && AI.distance < CROC_STATE_RUN_RANGE)
				item->Animation.TargetState = CROC_STATE_WALK;

			break;

		case CROC_STATE_ATTACK:
			if (item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameBase)
				item->Animation.RequiredState = 0;

			if (AI.bite && item->TouchBits & CROC_TOUCHBITS)
			{
				if (!item->Animation.RequiredState)
				{
					CreatureEffect2(item, &CrocodileBite, 10, -1, DoBloodSplat);
					item->Animation.RequiredState = CROC_STATE_IDLE;

					LaraItem->HitPoints -= CROC_DAMAGE;
					LaraItem->HitStatus = true;
				}
			}
			else
				item->Animation.TargetState = CROC_STATE_IDLE;
			
			break;

		case CROC_STATE_UNDERWATER_SWIM:
			creature->MaxTurn = CROC_SWIM_ANGLE;

			// Water to land transition.
			if (!CrocodileIsInWater(item))
			{
				item->Animation.AnimNumber = object->animIndex + CROC_ANIM_LAND_MODE;
				item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
				item->Animation.RequiredState = CROC_STATE_WALK;
				item->Animation.ActiveState = CROC_STATE_WALK;
				item->Animation.TargetState = CROC_STATE_WALK;
				break;
			}

			if (item->Animation.RequiredState)
				item->Animation.TargetState = item->Animation.RequiredState;
			else if (AI.bite)
			{
				if (item->TouchBits & 768)
					item->Animation.TargetState = CROC_STATE_UNDERWATER_ATTACK;
			}

			break;

		case CROC_STATE_UNDERWATER_ATTACK:
			if (item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameBase)
				item->Animation.RequiredState = CROC_STATE_NONE_1;

			if (AI.bite && item->TouchBits & CROC_TOUCHBITS)
			{
				if (!item->Animation.RequiredState)
				{
					CreatureEffect2(item, &CrocodileBite, 10, -1, DoBloodSplat);
					item->Animation.RequiredState = CROC_STATE_UNDERWATER_SWIM;

					LaraItem->HitPoints -= CROC_DAMAGE;
					LaraItem->HitStatus = true;
				}
			}
			else
				item->Animation.TargetState = CROC_STATE_UNDERWATER_SWIM;
			
			break;
		}
	}

	OBJECT_BONES boneRot;
	if (item->Animation.ActiveState == CROC_STATE_IDLE ||
		item->Animation.ActiveState == CROC_STATE_ATTACK ||
		item->Animation.ActiveState == CROC_STATE_UNDERWATER_ATTACK)
	{
		boneRot.bone0 = AI.angle;
		boneRot.bone1 = AI.angle;
		boneRot.bone2 = 0;
		boneRot.bone3 = 0;
	}
	else
	{
		boneRot.bone0 = boneAngle;
		boneRot.bone1 = boneAngle;
		boneRot.bone2 = -boneAngle;
		boneRot.bone3 = -boneAngle;
	}

	CreatureTilt(item, 0);
	CreatureJoint(item, 0, boneRot.bone0);
	CreatureJoint(item, 1, boneRot.bone1);
	CreatureJoint(item, 2, boneRot.bone2);
	CreatureJoint(item, 3, boneRot.bone3);

	if (item->Animation.ActiveState < CROC_STATE_UNDERWATER_SWIM)
		CalculateItemRotationToSurface(item, 2.0f);

	CreatureAnimation(itemNumber, angle, 0);

	if (item->Animation.ActiveState >= CROC_STATE_UNDERWATER_SWIM &&
		item->Animation.ActiveState <= CROC_STATE_UNDERWATER_DEATH)
	{
		CreatureUnderwater(item, CLICK(1));
	}
	else
		CreatureUnderwater(item, 0);
}
