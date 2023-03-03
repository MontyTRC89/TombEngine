#include "framework.h"
#include "Objects/TR5/Entity/tr5_imp.h"

#include "Game/animation.h"
#include "Game/control/box.h"
#include "Game/control/control.h"
#include "Game/effects/effects.h"
#include "Game/itemdata/creature_info.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/misc.h"
#include "Math/Math.h"
#include "Objects/Generic/Object/burning_torch.h"
#include "Specific/level.h"
#include "Specific/setup.h"
#include "Game/Lara/lara_helpers.h"

using namespace TEN::Entities::Generic;
using namespace TEN::Math;

namespace TEN::Entities::Creatures::TR5
{
	constexpr auto IMP_SCARED_TURN_RATE_MAX = ANGLE(7.0f);
	constexpr auto IMP_WALK_TURN_RATE_MAX = ANGLE(7.0f);
	constexpr auto IMP_RUN_TURN_RATE_MAX = ANGLE(7.0f);
	constexpr auto IMP_ATTACKJUMP_OR_STONE_TURN_RATE_MAX = ANGLE(2.0f);
	constexpr auto IMP_SHOOT_DEATH_ANGLE_BACKWARD = ANGLE(67.5f);

	constexpr auto IMP_LOW_ATTACK_RANGE = SQUARE(SECTOR(0.5f));
	constexpr auto IMP_WALK_RANGE = SQUARE(SECTOR(2));
	constexpr auto IMP_TORCH_LIT_SCARED_RANGE = BLOCK(2);
	constexpr auto IMP_IDLE_ATTACK_RANGE = SQUARE(CLICK(4) / 6);
	constexpr auto IMP_ATTACK_DAMAGE = 3;
	constexpr auto ATTACK_VELOCITY = 10;
	constexpr auto MESHSWAP_RANDOM = 8;

	const auto ImpLeftHandBite = BiteInfo(Vector3(0.0f, 100.0f, 0.0f), 7);
	const auto ImpRightHandBite = BiteInfo(Vector3(0.0f, 100.0f, 0.0f), 9);
	const auto ImpHeadSwapJoints = std::vector <unsigned int> { 10 };
	
	enum ImpState
	{
		IMP_STATE_WALK = 0,
		IMP_STATE_IDLE = 1,
		IMP_STATE_RUN = 2,
		IMP_STATE_ATTACK_1 = 3,
		IMP_STATE_JUMP_ATTACK = 5,
		IMP_STATE_SCARED = 6,
		IMP_STATE_START_CLIMB = 7,
		IMP_STATE_START_ROLL = 8,
		IMP_STATE_DEATH = 9,
		IMP_STATE_THROW_STONES = 11
	};

	enum ImpAnim
	{
		IMP_ANIM_WALK = 0,
		IMP_ANIM_WAIT = 1,
		IMP_ANIM_RUN = 2,
		IMP_ANIM_ATTACK = 3,
		IMP_ANIM_CRY = 4,
		IMP_ANIM_JUMP_ATTACK = 5,  
		IMP_ANIM_SCARED_RUN_AWAY = 6,
		IMP_ANIM_CLIMB_UP = 7,
		IMP_ANIM_BARREL_ROLL = 8,
		IMP_ANIM_DEATH_BACKWARDS = 9,
		IMP_ANIM_DEATH_FORWARD = 10,
		IMP_ANIM_LEFT_FOOT_START_WALK = 11,
		IMP_ANIM_RIGHT_FOOT_START_WALK = 12,
		IMP_ANIM_SLOW_TO_WAIT_LEFT = 13,
		IMP_ANIM_SLOW_TO_WAIT_RIGHT = 14,
		IMP_ANIM_SLOW_TO_RUN = 15,
		IMP_ANIM_SLOW_TO_WALK = 16,
		IMP_ANIM_THROW_ROCK = 17
	};

	enum ImpOCB
	{
		IMP_OCB_CLIMB_UP_START = 1,
		IMP_OCB_BARREL_ROLL_START = 2,
		IMP_OCB_THROW_STONES = 3

	};

	static void RotateTowardTarget(ItemInfo& item, const AI_INFO& ai, short turnRate)
	{
		if (abs(ai.angle) < turnRate)
		{
			item.Pose.Orientation.y += ai.angle;
		}
		else if (ai.angle < 0)
		{
			item.Pose.Orientation.y -= turnRate;
		}
		else
		{
			item.Pose.Orientation.y += turnRate;
		}
	}

	void InitialiseImp(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];
		InitialiseCreature(itemNumber);

		switch (item->TriggerFlags)
		{
		case IMP_OCB_BARREL_ROLL_START:
			SetAnimation(item, IMP_ANIM_BARREL_ROLL);
			break;
		case IMP_OCB_CLIMB_UP_START:
			SetAnimation(item, IMP_ANIM_CLIMB_UP);
			break;
		default:
			SetAnimation(item, IMP_ANIM_WAIT);
			break;
		}
	}

	void ImpThrowStones(ItemInfo* item)
	{
		auto pos1 = GetJointPosition(item, ImpRightHandBite.meshNum);
		auto pos2 = GetJointPosition(LaraItem, LM_HEAD);
		auto orient = Geometry::GetOrientToPoint(pos1.ToVector3(), pos2.ToVector3());
		
		int distance = (int)sqrt(SQUARE(pos2.x - pos1.x) + SQUARE(pos2.y - pos1.y) + SQUARE(pos2.z - pos1.z));
		if (distance < 8)
			distance = 8;
		orient.x += short(GetRandomControl() % (distance / 2) - (distance / 4));
		orient.y += short(GetRandomControl() % (distance / 4) - (distance / 8));

		short fxNumber = CreateNewEffect(item->RoomNumber);
		if (fxNumber != NO_ITEM)
		{
			auto* fx = &EffectList[fxNumber];

			fx->pos.Position = pos1;
			fx->roomNumber = item->RoomNumber;
			fx->speed = 4 * sqrt(distance);

			fx->pos.Orientation = EulerAngles(orient.x + (distance / 2), orient.y, 0);

			if (fx->speed < 256)
				fx->speed = 256;

			fx->fallspeed = 0;
			fx->objectNumber = ID_IMP_ROCK;
			fx->frameNumber = Objects[ID_IMP_ROCK].meshIndex;
			fx->color = Vector4::One;
			fx->counter = 0;
			fx->flag1 = 2;
			fx->flag2 = 0x2000;
		}
	}

	static bool IsTorchLitNearby(ItemInfo* item, CreatureInfo* creature)
	{
		if (creature->Enemy->IsLara())
		{
			auto* lara = GetLaraInfo(creature->Enemy);
			float distance = Vector3::Distance(item->Pose.Position.ToVector3(), creature->Enemy->Pose.Position.ToVector3());
			if (lara->Torch.IsLit && distance <= IMP_TORCH_LIT_SCARED_RANGE)
				return true;
		}

		auto foundTorchList = FoundCreatedItems(ID_BURNING_TORCH_ITEM);
		for (auto& torchItemNumber : foundTorchList)
		{
			if (torchItemNumber == NO_ITEM)
				continue;
			auto& torch = g_Level.Items[torchItemNumber];
			float distance = Vector3::Distance(item->Pose.Position.ToVector3(), torch.Pose.Position.ToVector3());
			if (torch.ItemFlags[3] != 0 && distance <= IMP_TORCH_LIT_SCARED_RANGE)
				return true;
		}

		return false;
	}

	void ImpControl(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;
		
		AI_INFO AI;
		short angle = 0;
		short otherAngle = 0;
		short torso_y = 0;
		short torso_x = 0;
		short head_y = 0;
		short head_x = 0;

		auto* item = &g_Level.Items[itemNumber];
		auto* creature = GetCreatureInfo(item);

		if (item->HitPoints <= 0)
		{
			if (item->Animation.ActiveState != IMP_STATE_DEATH)
			{
				CreatureAIInfo(item, &AI);
				if (AI.angle > -IMP_SHOOT_DEATH_ANGLE_BACKWARD && AI.angle < IMP_SHOOT_DEATH_ANGLE_BACKWARD)
					SetAnimation(item, IMP_ANIM_DEATH_BACKWARDS);
				else
					SetAnimation(item, IMP_ANIM_DEATH_FORWARD);
			}
		}
		else
		{
			if (item->AIBits)
				GetAITarget(creature);
			else if (creature->HurtByLara)
				creature->Enemy = LaraItem;

			CreatureAIInfo(item, &AI);

			int elevation = item->Pose.Position.y - creature->Enemy->Pose.Position.y + CLICK(1.5f);
			if (creature->Enemy->IsLara())
			{
				otherAngle = AI.angle;
				if (creature->Enemy->Animation.ActiveState == LS_CROUCH_IDLE ||
					creature->Enemy->Animation.ActiveState == LS_CROUCH_ROLL ||
					creature->Enemy->Animation.ActiveState > LS_MONKEY_TURN_180 &&
					creature->Enemy->Animation.ActiveState < LS_HANG_TO_CRAWL ||
					creature->Enemy->Animation.ActiveState == LS_CROUCH_TURN_LEFT ||
					creature->Enemy->Animation.ActiveState == LS_CROUCH_TURN_RIGHT)
				{
					elevation = item->Pose.Position.y - creature->Enemy->Pose.Position.y;
				}
			}
			else
				otherAngle = phd_atan(creature->Enemy->Pose.Position.z - item->Pose.Position.z, creature->Enemy->Pose.Position.x - item->Pose.Position.x) - item->Pose.Orientation.y;
			
			AI.xAngle = phd_atan(sqrt(AI.distance), elevation);

			GetCreatureMood(item, &AI, true);
			if (item->Animation.ActiveState == IMP_STATE_SCARED)
				creature->Mood = MoodType::Escape;
			CreatureMood(item, &AI, true);

			angle = CreatureTurn(item, creature->MaxTurn);

			if (Wibble & MESHSWAP_RANDOM)
				item->SetMeshSwapFlags (ImpHeadSwapJoints);
			else
				item->SetMeshSwapFlags(NO_JOINT_BITS);

			if (AI.ahead)
			{
				torso_x = AI.xAngle / 2;
				torso_y = AI.angle / 2;
				head_x = AI.xAngle / 2;
				head_y = AI.angle / 2;
			}

			bool isScared = IsTorchLitNearby(item, creature);
			switch (item->Animation.ActiveState)
			{
			case IMP_STATE_WALK:
				creature->MaxTurn = IMP_RUN_TURN_RATE_MAX;

				if (isScared)
				{
					item->Animation.TargetState = IMP_STATE_SCARED;
					break;
				}

				if (AI.distance > IMP_WALK_RANGE)
					item->Animation.TargetState = IMP_STATE_RUN;
				else if (AI.distance < IMP_LOW_ATTACK_RANGE)
					item->Animation.TargetState = IMP_STATE_IDLE;

				break;

			case IMP_STATE_IDLE:
				creature->MaxTurn = 0;
				creature->Flags = 0;

				if (isScared)
				{
					item->Animation.TargetState = IMP_STATE_SCARED;
					break;
				}

				if (AI.bite && AI.distance < IMP_IDLE_ATTACK_RANGE)
				{
					if (GetRandomControl() & 1)
						item->Animation.TargetState = IMP_STATE_ATTACK_1;
					else
						item->Animation.TargetState = IMP_STATE_JUMP_ATTACK;
				}
				else if (item->AIBits == FOLLOW)
				{
					item->Animation.TargetState = IMP_STATE_WALK;
				}
				else if(item->TriggerFlags == IMP_OCB_THROW_STONES)
				{
					item->Animation.TargetState = IMP_STATE_THROW_STONES;
				}
				else if (AI.distance > IMP_WALK_RANGE)
				{
					item->Animation.TargetState = IMP_STATE_RUN;
				}
				else if (AI.distance > IMP_LOW_ATTACK_RANGE)
				{
					item->Animation.TargetState = IMP_STATE_WALK;
				}

				break;

			case IMP_STATE_RUN:
				creature->MaxTurn = IMP_RUN_TURN_RATE_MAX;

				if (isScared)
				{
					item->Animation.TargetState = IMP_STATE_SCARED;
					break;
				}

				if (AI.distance < IMP_LOW_ATTACK_RANGE)
					item->Animation.TargetState = IMP_STATE_IDLE;
				else if (AI.distance < IMP_WALK_RANGE)
					item->Animation.TargetState = IMP_STATE_WALK;

				break;

			case IMP_STATE_ATTACK_1:
				creature->MaxTurn = 0;

				if (!(creature->Flags & 1) &&
					item->TouchBits.Test(ImpRightHandBite.meshNum))
				{
					DoDamage(creature->Enemy, IMP_ATTACK_DAMAGE);
					CreatureEffect2(item, ImpRightHandBite, ATTACK_VELOCITY, item->Pose.Orientation.y, DoBloodSplat);
					creature->Flags |= 1;
				}

				break;
			case IMP_STATE_JUMP_ATTACK:
				RotateTowardTarget(*item, AI, IMP_ATTACKJUMP_OR_STONE_TURN_RATE_MAX);

				if (!(creature->Flags & 1) && item->TouchBits.Test(ImpRightHandBite.meshNum))
				{
					DoDamage(creature->Enemy, IMP_ATTACK_DAMAGE);
					CreatureEffect(item, ImpRightHandBite, DoBloodSplat);
					creature->Flags |= 1;
				}

				if (!(creature->Flags & 2) && item->TouchBits.Test(ImpLeftHandBite.meshNum))
				{
					DoDamage(creature->Enemy, IMP_ATTACK_DAMAGE);
					CreatureEffect(item, ImpLeftHandBite, DoBloodSplat);
					creature->Flags |= 2;
				}

				break;

			case IMP_STATE_SCARED:
				creature->MaxTurn = IMP_SCARED_TURN_RATE_MAX;
				if (!isScared)
				{
					if (AI.distance > IMP_WALK_RANGE)
						item->Animation.TargetState = IMP_STATE_RUN;
					else
						item->Animation.TargetState = IMP_STATE_WALK;
				}
				break;

			case IMP_STATE_START_CLIMB:
			case IMP_STATE_START_ROLL:
				creature->MaxTurn = 0;
				break;

			case IMP_STATE_THROW_STONES:
				RotateTowardTarget(*item, AI, IMP_ATTACKJUMP_OR_STONE_TURN_RATE_MAX);
				
				if (item->Animation.FrameNumber == GetFrameNumber(item, 40))
					ImpThrowStones(item);

				break;
			}
		}

		CreatureJoint(item, 0, torso_x);
		CreatureJoint(item, 1, torso_y);
		CreatureJoint(item, 2, head_x);
		CreatureJoint(item, 3, head_y);
		CreatureAnimation(itemNumber, angle, 0);
	}
}
