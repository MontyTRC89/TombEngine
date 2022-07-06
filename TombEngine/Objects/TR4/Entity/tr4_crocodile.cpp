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

using std::vector;

namespace TEN::Entities::TR4
{
	BITE_INFO CrocodileBite = { 0, -100, 500, 9 };
	const vector<int> CrocodileBiteAttackJoints = { 8, 9 };

	constexpr auto CROC_SWIM_SPEED = 16;
	constexpr auto CROC_ATTACK_DAMAGE = 120;

	constexpr auto CROC_ALERT_RANGE = SQUARE(SECTOR(1) + CLICK(2));
	constexpr auto CROC_VISIBILITY_RANGE = SQUARE(SECTOR(5));
	constexpr auto CROC_STATE_RUN_RANGE = SQUARE(SECTOR(1));
	constexpr auto CROC_MAXRUN_RANGE = SQUARE(SECTOR(1) + CLICK(2));
	constexpr auto CROC_ATTACK_RANGE = SQUARE(CLICK(3)); // NOTE: It's CLICK(3) in TR4, but the crocodile does not go near Lara to do damage in certain cases.

	#define CROC_STATE_WALK_FORWARD_ANGLE ANGLE(3.0f)
	#define CROC_SWIM_ANGLE ANGLE(3.0f)
	#define CROC_STATE_RUN_FORWARD_ANGLE ANGLE(5.0f)

	enum CrocodileState
	{
		CROC_STATE_NONE_1 = 0,
		CROC_STATE_IDLE = 1,
		CROC_STATE_RUN_FORWARD = 2,
		CROC_STATE_WALK_FORWARD = 3,
		CROC_STATE_TURN_RIGHT = 4,
		CROC_STATE_BITE_ATTACK = 5,
		CROC_STATE_NONE_2 = 6,
		CROC_STATE_DEATH = 7,
		CROC_STATE_SWIM_FORWARD = 8,
		CROC_STATE_WATER_BITE_ATTACK = 9,
		CROC_STATE_WATER_DEATH = 10,
	};

	enum CrocodileAnim
	{
		CROC_ANIM_IDLE = 0,
		CROC_ANIM_IDLE_TO_RUN_FORWARD = 1,
		CROC_ANIM_RUN_FORWARD = 2,
		CROC_ANIM_RUN_FORWARD_TO_IDLE_RIGHT = 3,
		CROC_ANIM_RUN_FORWARD_TO_IDLE_LEFT = 4,
		CROC_ANIM_WALK_FORWARD = 5,
		CROC_ANIM_IDLE_TO_WALK_FORWARD = 6,
		CROC_ANIM_TURN_RIGHT_START = 7,
		CROC_ANIM_TURN_RIGHT_CONTINUE = 8,
		CROC_ANIM_TURN_RIGHT_END = 9,
		CROC_ANIM_BITE_ATTACK = 10,
		CROC_ANIM_LAND_DEATH = 11,
		CROC_ANIM_SWIM_FORWARD = 12,
		CROC_ANIM_WATER_BITE_ATTACK_START = 13,
		CROC_ANIM_WATER_BITE_ATTACK_CONTINUE = 14,
		CROC_ANIM_WATER_BITE_ATTACK_END = 15,
		CROC_ANIM_WATER_DEATH = 16,
		CROC_ANIM_LAND_TO_WATER = 17,
		CROC_ANIM_WATER_TO_LAND = 18
	};

	void InitialiseCrocodile(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		InitialiseCreature(itemNumber);

		if (TestEnvironment(ENV_FLAG_WATER, item))
		{
			item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + CROC_ANIM_SWIM_FORWARD;
			item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
			item->Animation.ActiveState = CROC_STATE_SWIM_FORWARD;
			item->Animation.TargetState = CROC_STATE_SWIM_FORWARD;
		}
		else
		{
			item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + CROC_ANIM_IDLE;
			item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
			item->Animation.ActiveState = CROC_STATE_IDLE;
			item->Animation.TargetState = CROC_STATE_IDLE;
		}
	}

	static bool CrocodileIsInWater(ItemInfo* item)
	{
		auto* info = GetCreatureInfo(item);

		short roomNumber = item->RoomNumber;
		GetFloor(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, &roomNumber);
		int waterDepth = GetWaterSurface(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, roomNumber);

		if (waterDepth != NO_HEIGHT)
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
			if (item->Animation.ActiveState != CROC_STATE_DEATH && item->Animation.ActiveState != CROC_STATE_WATER_DEATH)
			{
				if (TestEnvironment(ENV_FLAG_WATER, item))
				{
					item->Animation.AnimNumber = object->animIndex + CROC_ANIM_WATER_DEATH;
					item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
					item->Animation.ActiveState = CROC_STATE_WATER_DEATH;
					item->Animation.TargetState = CROC_STATE_WATER_DEATH;
					item->HitPoints = NOT_TARGETABLE;
				}
				else
				{
					item->Animation.AnimNumber = object->animIndex + CROC_ANIM_LAND_DEATH;
					item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
					item->Animation.ActiveState = CROC_STATE_DEATH;
					item->Animation.TargetState = CROC_STATE_DEATH;
				}
			}

			if (TestEnvironment(ENV_FLAG_WATER, item))
			{
				CreatureFloat(itemNumber);
				return;
			}
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
					item->Animation.TargetState = CROC_STATE_BITE_ATTACK;
				else
				{
					if (AI.ahead && AI.distance < CROC_STATE_RUN_RANGE)
						item->Animation.TargetState = CROC_STATE_WALK_FORWARD;
					else
						item->Animation.TargetState = CROC_STATE_RUN_FORWARD;
				}

				break;

			case CROC_STATE_WALK_FORWARD:
				creature->MaxTurn = CROC_STATE_WALK_FORWARD_ANGLE;

				// Land to water transition.
				if (CrocodileIsInWater(item))
				{
					item->Animation.RequiredState = CROC_STATE_SWIM_FORWARD;
					item->Animation.TargetState = CROC_STATE_SWIM_FORWARD;
					break;
				}

				if (item->Animation.RequiredState)
					item->Animation.TargetState = item->Animation.RequiredState;
				else if (AI.bite && AI.distance < CROC_ATTACK_RANGE)
					item->Animation.TargetState = CROC_STATE_IDLE;
				else if (!AI.ahead || AI.distance > CROC_MAXRUN_RANGE)
					item->Animation.TargetState = CROC_STATE_RUN_FORWARD;

				break;

			case CROC_STATE_RUN_FORWARD:
				creature->MaxTurn = CROC_STATE_RUN_FORWARD_ANGLE;

				// Land to water transition.
				if (CrocodileIsInWater(item))
				{
					item->Animation.RequiredState = CROC_STATE_WALK_FORWARD;
					item->Animation.TargetState = CROC_STATE_WALK_FORWARD;
					break;
				}

				if (item->Animation.RequiredState)
					item->Animation.TargetState = item->Animation.RequiredState;
				else if (AI.bite && AI.distance < CROC_ATTACK_RANGE)
					item->Animation.TargetState = CROC_STATE_IDLE;
				else if (AI.ahead && AI.distance < CROC_STATE_RUN_RANGE)
					item->Animation.TargetState = CROC_STATE_WALK_FORWARD;

				break;

			case CROC_STATE_BITE_ATTACK:
				if (item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameBase)
					item->Animation.RequiredState = 0;

				if (AI.bite && item->TestBits(JointBitType::Touch, CrocodileBiteAttackJoints))
				{
					if (!item->Animation.RequiredState)
					{
						CreatureEffect2(item, &CrocodileBite, 10, -1, DoBloodSplat);
						DoDamage(creature->Enemy, CROC_ATTACK_DAMAGE);
						item->Animation.RequiredState = CROC_STATE_IDLE;
					}
				}
				else
					item->Animation.TargetState = CROC_STATE_IDLE;

				break;

			case CROC_STATE_SWIM_FORWARD:
				creature->MaxTurn = CROC_SWIM_ANGLE;

				// Water to land transition.
				if (!CrocodileIsInWater(item))
				{
					item->Animation.AnimNumber = object->animIndex + CROC_ANIM_WATER_TO_LAND;
					item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
					item->Animation.RequiredState = CROC_STATE_WALK_FORWARD;
					item->Animation.ActiveState = CROC_STATE_WALK_FORWARD;
					item->Animation.TargetState = CROC_STATE_WALK_FORWARD;
					break;
				}

				if (item->Animation.RequiredState)
					item->Animation.TargetState = item->Animation.RequiredState;
				else if (AI.bite)
				{
					if (item->TouchBits & 768)
						item->Animation.TargetState = CROC_STATE_WATER_BITE_ATTACK;
				}

				break;

			case CROC_STATE_WATER_BITE_ATTACK:
				if (item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameBase)
					item->Animation.RequiredState = CROC_STATE_NONE_1;

				if (AI.bite && item->TestBits(JointBitType::Touch, CrocodileBiteAttackJoints))
				{
					if (!item->Animation.RequiredState)
					{
						CreatureEffect2(item, &CrocodileBite, 10, -1, DoBloodSplat);
						DoDamage(creature->Enemy, CROC_ATTACK_DAMAGE);
						item->Animation.RequiredState = CROC_STATE_SWIM_FORWARD;
					}
				}
				else
					item->Animation.TargetState = CROC_STATE_SWIM_FORWARD;

				break;
			}
		}

		OBJECT_BONES boneRot;
		if (item->Animation.ActiveState == CROC_STATE_IDLE ||
			item->Animation.ActiveState == CROC_STATE_BITE_ATTACK ||
			item->Animation.ActiveState == CROC_STATE_WATER_BITE_ATTACK)
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

		if (item->Animation.ActiveState < CROC_STATE_SWIM_FORWARD)
			CalculateItemRotationToSurface(item, 2.0f);

		CreatureAnimation(itemNumber, angle, 0);

		if (item->Animation.ActiveState >= CROC_STATE_SWIM_FORWARD &&
			item->Animation.ActiveState <= CROC_STATE_WATER_DEATH)
		{
			CreatureUnderwater(item, CLICK(1));
		}
		else
			CreatureUnderwater(item, 0);
	}
}
