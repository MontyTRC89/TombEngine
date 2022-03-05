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

namespace TEN::Entities::TR4
{
	BYTE DogAnims[] = { 20, 21, 22, 20 };
	BITE_INFO DogBite = { 0, 0, 100, 3 };

	enum DogState
	{
		DOG_STATE_NONE = 0,
		DOG_STATE_STOP = 1,
		DOG_STATE_WALK = 2,
		DOG_STATE_RUN = 3,
		DOG_STATE_STALK = 5,
		DOG_STATE_JUMP_ATTACK = 6,
		DOG_STATE_HOWL = 7,
		DOG_STATE_SLEEP = 8,
		DOG_STATE_CROUCH = 9,
		DOG_STATE_DEATH = 11,
		DOG_STATE_CROUCH_ATTACK = 12
	};

	// TODO
	enum DogAnim
	{

	};

	constexpr auto DOG_CROUCH_ATTACK_DAMAGE = 10;
	constexpr auto DOG_JUMP_ATTACK_DAMAGE = 20;

	void InitialiseTr4Dog(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];
		item->ActiveState = DOG_STATE_STOP;
		item->AnimNumber = Objects[item->ObjectNumber].animIndex + 8;

		// OCB 1 makes the dog sitting down until fired
		if (item->TriggerFlags)
		{
			item->AnimNumber = Objects[item->ObjectNumber].animIndex + 1;
			item->Status -= ITEM_INVISIBLE;
		}

		item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
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
			if (item->AnimNumber == object->animIndex + 1)
				item->HitPoints = object->HitPoints;
			else if (item->ActiveState != DOG_STATE_DEATH)
			{
				item->AnimNumber = object->animIndex + DogAnims[GetRandomControl() & 3];
				item->ActiveState = DOG_STATE_DEATH;
				item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
			}
		}
		else
		{
			if (item->AIBits)
				GetAITarget(creature);
			else
				creature->enemy = LaraItem;

			AI_INFO AI;
			CreatureAIInfo(item, &AI);

			int distance;
			if (creature->enemy == LaraItem)
				distance = AI.distance;
			else
			{
				int dx = LaraItem->Position.xPos - item->Position.xPos;
				int dz = LaraItem->Position.zPos - item->Position.zPos;
				phd_atan(dz, dx);
				distance = pow(dx, 2) + pow(dz, 2);
			}

			if (AI.ahead)
			{
				joint2 = AI.xAngle; // Maybe swapped
				joint1 = AI.angle;
			}

			GetCreatureMood(item, &AI, VIOLENT);
			CreatureMood(item, &AI, VIOLENT);

			if (creature->mood == BORED_MOOD)
				creature->maximumTurn /= 2;

			angle = CreatureTurn(item, creature->maximumTurn);
			joint0 = 4 * angle;

			if (creature->hurtByLara || distance < pow(SECTOR(3), 2) && !(item->AIBits & MODIFY))
			{
				AlertAllGuards(itemNumber);
				item->AIBits &= ~MODIFY;
			}

			short random = GetRandomControl();
			int frame = item->FrameNumber - g_Level.Anims[item->AnimNumber].frameBase;

			switch (item->ActiveState)
			{
			case DOG_STATE_NONE:
			case DOG_STATE_SLEEP:
				joint1 = 0;
				joint2 = 0;

				if (creature->mood && item->AIBits != MODIFY)
					item->TargetState = DOG_STATE_STOP;
				else
				{
					creature->flags++;
					creature->maximumTurn = 0;

					if (creature->flags > 300 && random < 128)
						item->TargetState = DOG_STATE_STOP;
				}

				break;

			case DOG_STATE_STOP:
			case DOG_STATE_CROUCH:
				creature->maximumTurn = 0;

				if (item->ActiveState == DOG_STATE_CROUCH && item->RequiredState)
				{
					item->TargetState = item->RequiredState;
					break;
				}

				if (item->AIBits & GUARD)
				{
					joint1 = AIGuard(creature);

					if (GetRandomControl())
						break;

					if (item->ActiveState == DOG_STATE_STOP)
					{
						item->TargetState = DOG_STATE_CROUCH;
						break;
					}
				}
				else
				{
					if (item->ActiveState == DOG_STATE_CROUCH && random < 128)
					{
						item->TargetState = DOG_STATE_STOP;
						break;
					}

					if (item->AIBits & PATROL1)
					{
						if (item->ActiveState == DOG_STATE_STOP)
							item->TargetState = DOG_STATE_WALK;
						else
							item->TargetState = DOG_STATE_STOP;

						break;
					}

					if (creature->mood == ESCAPE_MOOD)
					{
						if (Lara.target == item || !AI.ahead || item->HitStatus)
						{
							item->RequiredState = DOG_STATE_RUN;
							item->TargetState = DOG_STATE_CROUCH;
						}
						else
							item->TargetState = DOG_STATE_STOP;

						break;
					}

					if (creature->mood != BORED_MOOD)
					{
						item->RequiredState = DOG_STATE_RUN;

						if (item->ActiveState == DOG_STATE_STOP)
							item->TargetState = DOG_STATE_CROUCH;

						break;
					}

					creature->flags = 0;
					creature->maximumTurn = ANGLE(1.0f);

					if (random < 256)
					{
						if (item->AIBits & MODIFY)
						{
							if (item->ActiveState == DOG_STATE_STOP)
							{
								item->TargetState = DOG_STATE_SLEEP;
								creature->flags = 0;
								break;
							}
						}
					}

					if (random >= 4096)
					{
						if (!(random & 0x1F))
							item->TargetState = DOG_STATE_HOWL;

						break;
					}

					if (item->ActiveState == DOG_STATE_STOP)
					{
						item->TargetState = DOG_STATE_WALK;
						break;
					}
				}

				item->TargetState = DOG_STATE_STOP;
				break;

			case DOG_STATE_WALK:
				creature->maximumTurn = ANGLE(3.0f);

				if (item->AIBits & PATROL1)
				{
					item->TargetState = DOG_STATE_WALK;
					break;
				}

				if (!creature->mood && random < 256)
				{
					item->TargetState = DOG_STATE_STOP;
					break;
				}

				item->TargetState = DOG_STATE_STALK;
				break;

			case DOG_STATE_RUN:
				creature->maximumTurn = ANGLE(6.0f);

				if (creature->mood == ESCAPE_MOOD)
				{
					if (Lara.target != item && AI.ahead)
						item->TargetState = DOG_STATE_CROUCH;
				}
				else if (creature->mood)
				{
					if (AI.bite && AI.distance < pow(SECTOR(1), 2))
						item->TargetState = DOG_STATE_JUMP_ATTACK;
					else if (AI.distance < pow(SECTOR(1.5f), 2))
					{
						item->RequiredState = DOG_STATE_STALK;
						item->TargetState = DOG_STATE_CROUCH;
					}
				}
				else
					item->TargetState = DOG_STATE_CROUCH;

				break;

			case DOG_STATE_STALK:
				creature->maximumTurn = ANGLE(3.0f);

				if (creature->mood)
				{
					if (creature->mood == ESCAPE_MOOD)
						item->TargetState = DOG_STATE_RUN;
					else if (AI.bite && AI.distance < pow(341, 2))
					{
						item->TargetState = DOG_STATE_CROUCH_ATTACK;
						item->RequiredState = DOG_STATE_STALK;
					}
					else if (AI.distance > pow(SECTOR(1.5f), 2) || item->HitStatus)
						item->TargetState = DOG_STATE_RUN;
				}
				else
					item->TargetState = DOG_STATE_CROUCH;

				break;

			case DOG_STATE_JUMP_ATTACK:
				if (AI.bite &&
					item->TouchBits & 0x6648 &&
					frame >= 4 &&
					frame <= 14)
				{
					CreatureEffect2(item, &DogBite, 2, -1, DoBloodSplat);

					LaraItem->HitPoints -= DOG_JUMP_ATTACK_DAMAGE;
					LaraItem->HitStatus = true;
				}

				item->TargetState = DOG_STATE_RUN;
				break;

			case DOG_STATE_HOWL:
				joint1 = 0;
				joint2 = 0;
				break;

			case DOG_STATE_CROUCH_ATTACK:
				if (AI.bite &&
					item->TouchBits & 0x48 &&
					(frame >= 9 &&
						frame <= 12 ||
						frame >= 22 &&
						frame <= 25))
				{
					CreatureEffect2(item, &DogBite, 2, -1, DoBloodSplat);

					LaraItem->HitPoints -= DOG_CROUCH_ATTACK_DAMAGE;
					LaraItem->HitStatus = true;
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
