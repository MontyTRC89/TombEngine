#include "framework.h"
#include "tr4_dog.h"
#include "Game/control/box.h"
#include "Game/effects/effects.h"
#include "Specific/setup.h"
#include "Specific/level.h"
#include "Game/Lara/lara.h"
#include "Game/itemdata/creature_info.h"
#include "Game/control/control.h"
#include "Game/items.h"
#include "Game/misc.h"

using std::vector;

namespace TEN::Entities::TR4
{
	BITE_INFO DogBite = { 0, 0, 100, 3 };
	const vector<int> DogJumpAttackJoints = { 3, 6, 9, 10, 13, 14 };
	const vector<int> DogBiteAttackJoints = { 3, 6 };

	constexpr auto DOG_BITE_ATTACK_DAMAGE = 10;
	constexpr auto DOG_JUMP_ATTACK_DAMAGE = 20;
	constexpr auto DOG_BITE_ATTACK_RANGE = SECTOR(0.55);
	constexpr auto DOG_JUMP_ATTACK_RANGE = SECTOR(1);
	
	enum DogState
	{
		DOG_STATE_NONE = 0,
		DOG_STATE_IDLE = 1,
		DOG_STATE_WALK_FORWARD = 2,
		DOG_STATE_RUN_FORWARD = 3,
		DOG_STATE_STALK = 5,
		DOG_STATE_JUMP_ATTACK = 6,
		DOG_STATE_HOWL = 7,
		DOG_STATE_SLEEP = 8,
		DOG_STATE_STALK_IDLE = 9,
		DOG_STATE_RUN_FORWARD_RIGHT = 10,	// Unused.
		DOG_STATE_DEATH = 11,
		DOG_STATE_BITE_ATTACK = 12
	};

	enum DogAnim
	{
		DOG_ANIM_SLEEP = 0,
		DOG_ANIM_AWAKEN = 1,
		DOG_ANIM_IDLE_TO_WALK_FORWARD = 2,
		DOG_ANIM_WALK_FORWARD = 3,
		DOG_ANIM_WALK_FORWARD_TO_STALK = 4,
		DOG_ANIM_STALK_FORWARD = 5,
		DOG_ANIM_STALK_FORWARD_TO_RUN_FORWARD = 6,
		DOG_ANIM_RUN_FORWARD = 7,
		DOG_ANIM_IDLE = 8,
		DOG_ANIM_JUMP_ATTACK = 9,
		DOG_ANIM_STALK_IDLE_TO_STALK_FORWARD = 10,
		DOG_ANIM_STALK_FORWARD_TO_STALK_IDLE_START = 11,
		DOG_ANIM_STALK_FORWARD_TO_STALK_IDLE_END = 12,
		DOG_ANIM_STALK_IDLE_TO_RUN_FORWARD = 13,
		DOG_ANIM_STALK_IDLE = 14,
		DOG_ANIM_RUN_FORWARD_TO_STALK_IDLE = 15,
		DOG_ANIM_HOWL = 16,
		DOG_ANIM_IDLE_TO_STALK_IDLE = 17,
		DOG_ANIM_RUN_FORWARD_RIGHT = 18,	// Unused.
		DOG_ANIM_WALK_FORWARD_TO_IDLE = 19,
		DOG_ANIM_DEATH_1 = 20,
		DOG_ANIM_DEATH_2 = 21,
		DOG_ANIM_DEATH_3 = 22,
		DOG_ANIM_BITE_ATTACK = 23,
		DOG_ANIM_STALK_IDLE_TO_IDLE = 24
	};

	int DeathAnims[] = { DOG_ANIM_DEATH_1, DOG_ANIM_DEATH_2, DOG_ANIM_DEATH_3, DOG_ANIM_DEATH_1 };

	void InitialiseTr4Dog(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		item->Animation.ActiveState = DOG_STATE_IDLE;
		item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + DOG_ANIM_IDLE;

		// OCB 1 makes the dog sitting down until fired
		if (item->TriggerFlags)
		{
			item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + DOG_ANIM_AWAKEN;
			item->Status -= ITEM_INVISIBLE;
		}

		item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
	}

	void Tr4DogControl(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;

		auto* item = &g_Level.Items[itemNumber];
		auto* creature = GetCreatureInfo(item);
		auto* object = &Objects[item->ObjectNumber];

		short angle = 0;
		short joint2 = 0;
		short joint1 = 0;
		short joint0 = 0;

		if (item->HitPoints <= 0)
		{
			if (item->Animation.AnimNumber == object->animIndex + 1)
				item->HitPoints = object->HitPoints;
			else if (item->Animation.ActiveState != DOG_STATE_DEATH)
			{
				item->Animation.AnimNumber = object->animIndex + DeathAnims[GetRandomControl() & 3];
				item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
				item->Animation.ActiveState = DOG_STATE_DEATH;
			}
		}
		else
		{
			if (item->AIBits)
				GetAITarget(creature);
			else
				creature->Enemy = LaraItem;

			AI_INFO AI;
			CreatureAIInfo(item, &AI);

			int distance;
			if (creature->Enemy == LaraItem)
			{
				distance = AI.distance;
			}
			else
			{
				int dx = LaraItem->Pose.Position.x - item->Pose.Position.x;
				int dz = LaraItem->Pose.Position.z - item->Pose.Position.z;
				phd_atan(dz, dx);
				distance = pow(dx, 2) + pow(dz, 2);
			}

			if (AI.ahead)
			{
				joint2 = AI.xAngle; // TODO: Maybe swapped
				joint1 = AI.angle;
			}

			GetCreatureMood(item, &AI, VIOLENT);
			CreatureMood(item, &AI, VIOLENT);

			if (creature->Mood == MoodType::Bored)
				creature->MaxTurn /= 2;

			angle = CreatureTurn(item, creature->MaxTurn);
			joint0 = angle * 4;

			if (creature->HurtByLara || distance < pow(SECTOR(3), 2) && !(item->AIBits & MODIFY))
			{
				AlertAllGuards(itemNumber);
				item->AIBits &= ~MODIFY;
			}

			short random = GetRandomControl();
			int frame = item->Animation.FrameNumber - g_Level.Anims[item->Animation.AnimNumber].frameBase;

			switch (item->Animation.ActiveState)
			{
			case DOG_STATE_NONE:
			case DOG_STATE_SLEEP:
				joint1 = 0;
				joint2 = 0;

				if (creature->Mood != MoodType::Bored && item->AIBits != MODIFY)
					item->Animation.TargetState = DOG_STATE_IDLE;
				else
				{
					creature->Flags++;
					creature->MaxTurn = 0;

					if (creature->Flags > 300 && random < 128)
						item->Animation.TargetState = DOG_STATE_IDLE;
				}

				break;

			case DOG_STATE_IDLE:
			case DOG_STATE_STALK_IDLE:
				creature->MaxTurn = 0;

				if (item->Animation.ActiveState == DOG_STATE_STALK_IDLE &&
					item->Animation.RequiredState)
				{
					item->Animation.TargetState = item->Animation.RequiredState;
					break;
				}

				if (item->AIBits & GUARD)
				{
					joint1 = AIGuard(creature);

					if (GetRandomControl() & 0xFF)
						break;

					if (item->Animation.ActiveState == DOG_STATE_IDLE)
					{
						item->Animation.TargetState = DOG_STATE_STALK_IDLE;
						break;
					}
				}
				else
				{
					if (item->Animation.ActiveState == DOG_STATE_STALK_IDLE && random < 128)
					{
						item->Animation.TargetState = DOG_STATE_IDLE;
						break;
					}

					if (item->AIBits & PATROL1)
					{
						if (item->Animation.ActiveState == DOG_STATE_IDLE)
							item->Animation.TargetState = DOG_STATE_WALK_FORWARD;
						else
							item->Animation.TargetState = DOG_STATE_IDLE;

						break;
					}

					if (creature->Mood == MoodType::Escape)
					{
						if (Lara.TargetEntity == item || !AI.ahead || item->HitStatus)
						{
							item->Animation.TargetState = DOG_STATE_STALK_IDLE;
							item->Animation.RequiredState = DOG_STATE_RUN_FORWARD;
						}
						else
							item->Animation.TargetState = DOG_STATE_IDLE;

						break;
					}

					if (creature->Mood != MoodType::Bored)
					{
						if (item->Animation.ActiveState == DOG_STATE_IDLE)
							item->Animation.TargetState = DOG_STATE_STALK_IDLE;

						item->Animation.RequiredState = DOG_STATE_RUN_FORWARD;
						break;
					}

					creature->Flags = 0;
					creature->MaxTurn = ANGLE(1.0f);

					if (random < 256)
					{
						if (item->AIBits & MODIFY)
						{
							if (item->Animation.ActiveState == DOG_STATE_IDLE)
							{
								item->Animation.TargetState = DOG_STATE_SLEEP;
								creature->Flags = 0;
								break;
							}
						}
					}

					if (random >= 4096)
					{
						if (!(random & 0x1F))
							item->Animation.TargetState = DOG_STATE_HOWL;

						break;
					}

					if (item->Animation.ActiveState == DOG_STATE_IDLE)
					{
						item->Animation.TargetState = DOG_STATE_WALK_FORWARD;
						break;
					}
				}

				item->Animation.TargetState = DOG_STATE_IDLE;
				break;

			case DOG_STATE_WALK_FORWARD:
				creature->MaxTurn = ANGLE(3.0f);

				if (item->AIBits & PATROL1)
					item->Animation.TargetState = DOG_STATE_WALK_FORWARD;
				else if (creature->Mood == MoodType::Bored && random < 256)
					item->Animation.TargetState = DOG_STATE_IDLE;
				else
					item->Animation.TargetState = DOG_STATE_STALK;

				break;

			case DOG_STATE_RUN_FORWARD:
				creature->MaxTurn = ANGLE(6.0f);

				if (creature->Mood == MoodType::Escape)
				{
					if (Lara.TargetEntity != item && AI.ahead)
						item->Animation.TargetState = DOG_STATE_STALK_IDLE;
				}
				else if (creature->Mood != MoodType::Bored)
				{
					if (AI.bite && AI.distance < pow(DOG_JUMP_ATTACK_RANGE, 2))
						item->Animation.TargetState = DOG_STATE_JUMP_ATTACK;
					else if (AI.distance < pow(SECTOR(1.5f), 2))
					{
						item->Animation.TargetState = DOG_STATE_STALK_IDLE;
						item->Animation.RequiredState = DOG_STATE_STALK;
					}
				}
				else
					item->Animation.TargetState = DOG_STATE_STALK_IDLE;

				break;

			case DOG_STATE_STALK:
				creature->MaxTurn = ANGLE(3.0f);

				if (creature->Mood != MoodType::Bored)
				{
					if (creature->Mood == MoodType::Escape)
						item->Animation.TargetState = DOG_STATE_RUN_FORWARD;
					else if (AI.bite && AI.distance < pow(DOG_BITE_ATTACK_RANGE, 2))
					{
						item->Animation.TargetState = DOG_STATE_BITE_ATTACK;
						item->Animation.RequiredState = DOG_STATE_STALK;
					}
					else if (AI.distance > pow(SECTOR(1.5f), 2) || item->HitStatus)
						item->Animation.TargetState = DOG_STATE_RUN_FORWARD;
				}
				else
					item->Animation.TargetState = DOG_STATE_STALK_IDLE;

				break;

			case DOG_STATE_JUMP_ATTACK:
				if (AI.bite &&
					item->TestBits(JointBitType::Touch, DogJumpAttackJoints) &&
					frame >= 4 &&
					frame <= 14)
				{
					DoDamage(creature->Enemy, DOG_JUMP_ATTACK_DAMAGE);
					CreatureEffect2(item, &DogBite, 2, -1, DoBloodSplat);
				}

				item->Animation.TargetState = DOG_STATE_RUN_FORWARD;
				break;

			case DOG_STATE_HOWL:
				joint1 = 0;
				joint2 = 0;
				break;

			case DOG_STATE_BITE_ATTACK:
				if (AI.bite &&
					item->TestBits(JointBitType::Touch, DogBiteAttackJoints) &&
					(frame >= 9 &&
						frame <= 12 ||
						frame >= 22 &&
						frame <= 25))
				{
					DoDamage(creature->Enemy, DOG_BITE_ATTACK_DAMAGE);
					CreatureEffect2(item, &DogBite, 2, -1, DoBloodSplat);
				}

				break;

			default:
				break;
			}
		}

		CreatureTilt(item, 0);
		CreatureJoint(item, 0, joint0);
		CreatureJoint(item, 1, joint1);
		CreatureJoint(item, 2, joint2);
		CreatureAnimation(itemNumber, angle, 0);
	}
}
