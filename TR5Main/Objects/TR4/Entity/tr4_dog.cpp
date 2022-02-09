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

namespace TEN::Entities::TR4
{
	enum DOG_STATES
	{
		STATE_DOG_NONE = 0,
		STATE_DOG_STOP = 1,
		STATE_DOG_WALK = 2,
		STATE_DOG_RUN = 3,
		STATE_DOG_STALK = 5,
		STATE_DOG_JUMP_ATTACK = 6,
		STATE_DOG_HOWL = 7,
		STATE_DOG_SLEEP = 8,
		STATE_DOG_CROUCH = 9,
		STATE_DOG_DEATH = 11,
		STATE_DOG_CROUCH_ATTACK = 12
	};

	constexpr auto DOG_CROUCH_ATTACK_DAMAGE = 10;
	constexpr auto DOG_JUMP_ATTACK_DAMAGE = 20;

	BYTE DogAnims[] = { 20, 21, 22, 20 };
	BITE_INFO DogBite = { 0, 0, 100, 3 };

	void InitialiseTr4Dog(short itemNum)
	{
		ITEM_INFO* item;

		item = &g_Level.Items[itemNum];
		item->ActiveState = STATE_DOG_STOP;
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

		short angle = 0;
		short joint2 = 0;
		short joint1 = 0;
		short joint0 = 0;

		ITEM_INFO* item = &g_Level.Items[itemNumber];
		CREATURE_INFO* creature = (CREATURE_INFO*)item->Data;
		OBJECT_INFO* obj = &Objects[item->ObjectNumber];

		if (item->HitPoints <= 0)
		{
			if (item->AnimNumber == obj->animIndex + 1)
			{
				item->HitPoints = obj->HitPoints;
			}
			else if (item->ActiveState != STATE_DOG_DEATH)
			{
				item->AnimNumber = obj->animIndex + DogAnims[GetRandomControl() & 3];
				item->ActiveState = STATE_DOG_DEATH;
				item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
			}
		}
		else
		{
			if (item->AIBits)
				GetAITarget(creature);
			else
				creature->enemy = LaraItem;

			AI_INFO info;
			CreatureAIInfo(item, &info);

			int distance;
			if (creature->enemy == LaraItem)
			{
				distance = info.distance;
			}
			else
			{
				int dx = LaraItem->Position.xPos - item->Position.xPos;
				int dz = LaraItem->Position.zPos - item->Position.zPos;
				phd_atan(dz, dx);
				distance = SQUARE(dx) + SQUARE(dz);
			}

			if (info.ahead)
			{
				joint2 = info.xAngle; // Maybe swapped
				joint1 = info.angle;
			}

			GetCreatureMood(item, &info, VIOLENT);
			CreatureMood(item, &info, VIOLENT);

			if (creature->mood == BORED_MOOD)
				creature->maximumTurn /= 2;

			angle = CreatureTurn(item, creature->maximumTurn);
			joint0 = 4 * angle;

			if (creature->hurtByLara || distance < SQUARE(3072) && !(item->AIBits & MODIFY))
			{
				AlertAllGuards(itemNumber);
				item->AIBits &= ~MODIFY;
			}

			short random = GetRandomControl();
			int frame = item->FrameNumber - g_Level.Anims[item->AnimNumber].frameBase;

			switch (item->ActiveState)
			{
			case STATE_DOG_NONE:
			case STATE_DOG_SLEEP:
				joint1 = 0;
				joint2 = 0;
				if (creature->mood && (item->AIBits) != MODIFY)
				{
					item->TargetState = STATE_DOG_STOP;
				}
				else
				{
					creature->flags++;
					creature->maximumTurn = 0;
					if (creature->flags > 300 && random < 128)
						item->TargetState = STATE_DOG_STOP;
				}
				break;

			case STATE_DOG_STOP:
			case STATE_DOG_CROUCH:
				if (item->ActiveState == STATE_DOG_CROUCH && item->RequiredState)
				{
					item->TargetState = item->RequiredState;
					break;
				}

				creature->maximumTurn = 0;
				if (item->AIBits & GUARD)
				{
					joint1 = AIGuard(creature);
					if (GetRandomControl())
						break;
					if (item->ActiveState == STATE_DOG_STOP)
					{
						item->TargetState = STATE_DOG_CROUCH;
						break;
					}
				}
				else
				{
					if (item->ActiveState == STATE_DOG_CROUCH && random < 128)
					{
						item->TargetState = STATE_DOG_STOP;
						break;
					}

					if (item->AIBits & PATROL1)
					{
						if (item->ActiveState == STATE_DOG_STOP)
							item->TargetState = STATE_DOG_WALK;
						else
							item->TargetState = STATE_DOG_STOP;
						break;
					}

					if (creature->mood == ESCAPE_MOOD)
					{
						if (Lara.target == item || !info.ahead || item->HitStatus)
						{
							item->RequiredState = STATE_DOG_RUN;
							item->TargetState = STATE_DOG_CROUCH;
						}
						else
						{
							item->TargetState = STATE_DOG_STOP;
						}
						break;
					}

					if (creature->mood != BORED_MOOD)
					{
						item->RequiredState = STATE_DOG_RUN;
						if (item->ActiveState == STATE_DOG_STOP)
							item->TargetState = STATE_DOG_CROUCH;
						break;
					}

					creature->flags = 0;
					creature->maximumTurn = ANGLE(1);

					if (random < 256)
					{
						if (item->AIBits & MODIFY)
						{
							if (item->ActiveState == STATE_DOG_STOP)
							{
								item->TargetState = STATE_DOG_SLEEP;
								creature->flags = 0;
								break;
							}
						}
					}

					if (random >= 4096)
					{
						if (!(random & 0x1F))
							item->TargetState = STATE_DOG_HOWL;
						break;
					}

					if (item->ActiveState == STATE_DOG_STOP)
					{
						item->TargetState = STATE_DOG_WALK;
						break;
					}
				}
				item->TargetState = STATE_DOG_STOP;
				break;

			case STATE_DOG_WALK:
				creature->maximumTurn = ANGLE(3);
				if (item->AIBits & PATROL1)
				{
					item->TargetState = STATE_DOG_WALK;
					break;
				}

				if (!creature->mood && random < 256)
				{
					item->TargetState = STATE_DOG_STOP;
					break;
				}
				item->TargetState = STATE_DOG_STALK;
				break;

			case STATE_DOG_RUN:
				creature->maximumTurn = ANGLE(6);
				if (creature->mood == ESCAPE_MOOD)
				{
					if (Lara.target != item && info.ahead)
						item->TargetState = STATE_DOG_CROUCH;
				}
				else if (creature->mood)
				{
					if (info.bite && info.distance < SQUARE(1024))
					{
						item->TargetState = STATE_DOG_JUMP_ATTACK;
					}
					else if (info.distance < SQUARE(1536))
					{
						item->RequiredState = STATE_DOG_STALK;
						item->TargetState = STATE_DOG_CROUCH;
					}
				}
				else
				{
					item->TargetState = STATE_DOG_CROUCH;
				}
				break;

			case STATE_DOG_STALK:
				creature->maximumTurn = ANGLE(3);
				if (creature->mood)
				{
					if (creature->mood == ESCAPE_MOOD)
					{
						item->TargetState = STATE_DOG_RUN;
					}
					else if (info.bite && info.distance < SQUARE(341))
					{
						item->TargetState = STATE_DOG_CROUCH_ATTACK;
						item->RequiredState = STATE_DOG_STALK;
					}
					else if (info.distance > SQUARE(1536) || item->HitStatus)
					{
						item->TargetState = STATE_DOG_RUN;
					}
				}
				else
				{
					item->TargetState = STATE_DOG_CROUCH;
				}
				break;

			case STATE_DOG_JUMP_ATTACK:
				if (info.bite
					&& item->TouchBits & 0x6648
					&& frame >= 4
					&& frame <= 14)
				{
					CreatureEffect2(item, &DogBite, 2, -1, DoBloodSplat);
					LaraItem->HitPoints -= DOG_JUMP_ATTACK_DAMAGE;
					LaraItem->HitStatus = true;
				}
				item->TargetState = STATE_DOG_RUN;
				break;

			case STATE_DOG_HOWL:
				joint1 = 0;
				joint2 = 0;
				break;

			case STATE_DOG_CROUCH_ATTACK:
				if (info.bite
					&& item->TouchBits & 0x48
					&& (frame >= 9
						&& frame <= 12
						|| frame >= 22
						&& frame <= 25))
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
