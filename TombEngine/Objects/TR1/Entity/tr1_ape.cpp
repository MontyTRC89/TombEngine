#include "framework.h"
#include "Objects/TR1/Entity/tr1_ape.h"

#include "Game/control/box.h"
#include "Game/control/control.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Game/misc.h"
#include "Game/people.h"
#include "Game/Lara/lara.h"
#include "Specific/level.h"
#include "Specific/setup.h"

using std::vector;

namespace TEN::Entities::TR1
{
	BITE_INFO ApeBite = { 0, -19, 75, 15 };
	const vector<int> ApeAttackJoints = { 8, 9, 10, 11, 12, 13, 14, 15 };

	constexpr auto APE_ATTACK_DAMAGE = 200;

	constexpr auto APE_ATTACK_RANGE = SECTOR(0.42f);
	constexpr auto APE_PANIC_RANGE = SECTOR(2);

	constexpr auto APE_JUMP_CHANCE = 0xa0;
	constexpr auto APE_POUND_CHEST_CHANCE = APE_JUMP_CHANCE + 0xA0;
	constexpr auto APE_POUND_GROUND_CHANCE = APE_POUND_CHEST_CHANCE + 0xA0;
	constexpr auto APE_RUN_LEFT_CHANCE = APE_POUND_GROUND_CHANCE + 0xA0;

	constexpr auto SHIFT = 75;

	#define APE_RUN_TURN_RATE_MAX ANGLE(5.0f)
	#define APE_DISPLAY_ANGLE ANGLE(45.0f)

	enum ApeState
	{
		APE_STATE_NONE = 0,
		APE_STATE_IDLE = 1,
		APE_STATE_WALK_FORWARD = 2,
		APE_STATE_RUN_FORWARD = 3,
		APE_STATE_ATTACK = 4,
		APE_STATE_DEATH = 5,
		APE_STATE_POUND_CHEST = 6,
		APE_STATE_POUND_GROUND = 7,
		APE_STATE_RUN_LEFT = 8,
		APE_STATE_RUN_RIGHT = 9,
		APE_STATE_JUMP = 10,
		APE_STATE_VAULT = 11
	};

	enum ApeAnim
	{
		APE_ANIM_IDLE = 0,
		APE_ANIM_POUND_CHEST = 1,
		APE_ANIM_POUND_GROUND = 2,
		APE_ANIM_IDLE_TO_RUN_FORWARD = 3,
		APE_ANIM_RUN_FORWARD_TO_IDLE = 4,
		APE_ANIM_JUMP = 5,
		APE_ANIM_RUN_FORWARD = 6,
		APE_ANIM_DEATH_1 = 7,
		APE_ANIM_DEATH_2 = 8,
		APE_ANIM_ATTACK_START = 9,
		APE_ANIM_ATTACK_CONTINUE_1 = 10,
		APE_ANIM_ATTACK_CONTINUE_2 = 11,
		APE_ANIM_ATTACK_END = 12,
		APE_ANIM_RUN_LEFT = 13,
		APE_ANIM_RUN_RIGHT = 14,
		APE_ANIM_RUN_LEFT_TO_IDLE = 15,
		APE_ANIM_RUN_RIGHT_TO_IDLE = 16,
		APE_ANIM_IDLE_TO_RUN_LEFT = 17,
		APE_ANIM_IDLE_TO_RUN_RIGHT = 18,
		APE_ANIM_VAULT = 19
	};

	enum ApeFlags
	{
		APE_FLAG_ATTACK = (1 << 0),
		APE_FLAG_TURN_LEFT = (1 << 1),
		APE_FLAG_TURN_RIGHT = (1 << 2)
	};

	void ApeVault(short itemNumber, short angle)
	{
		auto* item = &g_Level.Items[itemNumber];
		auto* creature = GetCreatureInfo(item);

		if (creature->Flags & APE_FLAG_TURN_LEFT)
		{
			item->Pose.Orientation.y -= ANGLE(90.0f);
			creature->Flags -= APE_FLAG_TURN_LEFT;
		}
		else if (item->Flags & APE_FLAG_TURN_RIGHT)
		{
			item->Pose.Orientation.y += ANGLE(90.0f);
			creature->Flags -= APE_FLAG_TURN_RIGHT;
		}

		long long xx = item->Pose.Position.z / SECTOR(1);
		long long yy = item->Pose.Position.x / SECTOR(1);
		long long y = item->Pose.Position.y;

		CreatureAnimation(itemNumber, angle, 0);

		if (item->Pose.Position.y > (y - CLICK(1.5f)))
			return;

		long long xFloor = item->Pose.Position.z / SECTOR(1);
		long long yFloor = item->Pose.Position.x / SECTOR(1);
		if (xx == xFloor)
		{
			if (yy == yFloor)
				return;

			if (yy < yFloor)
			{
				item->Pose.Position.x = (yFloor * SECTOR(1)) - SHIFT;
				item->Pose.Orientation.y = ANGLE(90.0f);
			}
			else
			{
				item->Pose.Position.x = (yy * SECTOR(1)) + SHIFT;
				item->Pose.Orientation.y = -ANGLE(90.0f);
			}
		}
		else if (yy == yFloor)
		{
			if (xx < xFloor)
			{
				item->Pose.Position.z = (xFloor * SECTOR(1)) - SHIFT;
				item->Pose.Orientation.y = 0;
			}
			else
			{
				item->Pose.Position.z = (xx * SECTOR(1)) + SHIFT;
				item->Pose.Orientation.y = -ANGLE(180.0f);
			}
		}
		else
		{
			// diagonal
		}

		switch (CreatureVault(itemNumber, angle, 2, SHIFT))
		{
		case 2:
			item->Pose.Position.y = y;
			item->Animation.AnimNumber = Objects[ID_APE].animIndex + APE_ANIM_VAULT;
			item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
			item->Animation.ActiveState = APE_STATE_VAULT;
			break;

		default:
			return;
		}
	}

	void ApeControl(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;

		auto* item = &g_Level.Items[itemNumber];
		auto* creatureInfo = GetCreatureInfo(item);

		short head = 0;
		short angle = 0;

		if (item->HitPoints <= 0)
		{
			if (item->Animation.ActiveState != APE_STATE_DEATH)
			{
				item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + APE_ANIM_DEATH_1 + (short)(GetRandomControl() / 0x4000);
				item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
				item->Animation.ActiveState = APE_STATE_DEATH;
			}
		}
		else
		{
			AI_INFO AI;
			CreatureAIInfo(item, &AI);

			if (AI.ahead)
				head = AI.angle;

			GetCreatureMood(item, &AI, TIMID);
			CreatureMood(item, &AI, TIMID);

			angle = CreatureTurn(item, creatureInfo->MaxTurn);

			if (item->HitStatus || AI.distance < pow(APE_PANIC_RANGE, 2))
				creatureInfo->Flags |= APE_FLAG_ATTACK;

			short random;

			switch (item->Animation.ActiveState)
			{
			case APE_STATE_IDLE:
				if (creatureInfo->Flags & APE_FLAG_TURN_LEFT)
				{
					item->Pose.Orientation.y -= ANGLE(90.0f);
					creatureInfo->Flags -= APE_FLAG_TURN_LEFT;
				}
				else if (item->Flags & APE_FLAG_TURN_RIGHT)
				{
					item->Pose.Orientation.y += ANGLE(90);
					creatureInfo->Flags -= APE_FLAG_TURN_RIGHT;
				}

				if (item->Animation.RequiredState)
					item->Animation.TargetState = item->Animation.RequiredState;
				else if (AI.bite && AI.distance < pow(APE_ATTACK_RANGE, 2))
					item->Animation.TargetState = APE_STATE_ATTACK;
				else if (!(creatureInfo->Flags & APE_FLAG_ATTACK) &&
					AI.zoneNumber == AI.enemyZone && AI.ahead)
				{
					random = (short)(GetRandomControl() / 32);
					if (random < APE_JUMP_CHANCE)
						item->Animation.TargetState = APE_STATE_JUMP;
					else if (random < APE_POUND_CHEST_CHANCE)
						item->Animation.TargetState = APE_STATE_POUND_CHEST;
					else if (random < APE_POUND_GROUND_CHANCE)
						item->Animation.TargetState = APE_STATE_POUND_GROUND;
					else if (random < APE_RUN_LEFT_CHANCE)
					{
						item->Animation.TargetState = APE_STATE_RUN_LEFT;
						creatureInfo->MaxTurn = 0;
					}
					else
					{
						item->Animation.TargetState = APE_STATE_RUN_RIGHT;
						creatureInfo->MaxTurn = 0;
					}
				}
				else
					item->Animation.TargetState = APE_STATE_RUN_FORWARD;

				break;

			case APE_STATE_RUN_FORWARD:
				creatureInfo->MaxTurn = APE_RUN_TURN_RATE_MAX;

				if (creatureInfo->Flags == 0 &&
					AI.angle > -APE_DISPLAY_ANGLE &&
					AI.angle < APE_DISPLAY_ANGLE)
				{
					item->Animation.TargetState = APE_STATE_IDLE;
				}
				else if (AI.ahead && item->TestBits(JointBitType::Touch, ApeAttackJoints))
				{
					item->Animation.RequiredState = APE_STATE_ATTACK;
					item->Animation.TargetState = APE_STATE_IDLE;
				}
				else if (creatureInfo->Mood != MoodType::Escape)
				{
					random = (short)GetRandomControl();
					if (random < APE_JUMP_CHANCE)
					{
						item->Animation.RequiredState = APE_STATE_JUMP;
						item->Animation.TargetState = APE_STATE_IDLE;
					}
					else if (random < APE_POUND_CHEST_CHANCE)
					{
						item->Animation.RequiredState = APE_STATE_POUND_CHEST;
						item->Animation.TargetState = APE_STATE_IDLE;
					}
					else if (random < APE_POUND_GROUND_CHANCE)
					{
						item->Animation.RequiredState = APE_STATE_POUND_GROUND;
						item->Animation.TargetState = APE_STATE_IDLE;
					}
				}

				break;

			case APE_STATE_RUN_LEFT:
				if (!(creatureInfo->Flags & APE_FLAG_TURN_RIGHT))
				{
					item->Pose.Orientation.y -= ANGLE(90.0f);
					creatureInfo->Flags |= APE_FLAG_TURN_RIGHT;
				}

				item->Animation.TargetState = APE_STATE_IDLE;
				break;

			case APE_STATE_RUN_RIGHT:
				if (!(creatureInfo->Flags & APE_FLAG_TURN_LEFT))
				{
					item->Pose.Orientation.y += ANGLE(90.0f);
					creatureInfo->Flags |= APE_FLAG_TURN_LEFT;
				}

				item->Animation.TargetState = APE_STATE_IDLE;
				break;

			case APE_STATE_ATTACK:
				if (!item->Animation.RequiredState &&
					item->TestBits(JointBitType::Touch, ApeAttackJoints))
				{
					CreatureEffect(item, &ApeBite, DoBloodSplat);
					item->Animation.RequiredState = APE_STATE_IDLE;
					DoDamage(creatureInfo->Enemy, APE_ATTACK_DAMAGE);
				}

				break;
			}
		}

		CreatureJoint(item, 0, head);

		if (item->Animation.ActiveState != APE_STATE_VAULT)
			ApeVault(itemNumber, angle);
		else
			CreatureAnimation(itemNumber, angle, 0);
	}
}
