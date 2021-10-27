#include "framework.h"
#include "tr4_sas.h"
#include "control/box.h"
#include "items.h"
#include "people.h"
#include "lara.h"
#include "setup.h"
#include "level.h"
#include "control/control.h"
#include "animation.h"
#include "effects/effects.h"
#include "effects/tomb4fx.h"
#include "Specific/input.h"
#include "Lara/lara_one_gun.h"
#include "itemdata/creature_info.h"

namespace TEN::Entities::TR4
{
	enum SAS_STATES
	{
		STATE_SAS_EMPTY,
		STATE_SAS_STOP,
		STATE_SAS_WALK,
		STATE_SAS_RUN,
		STATE_SAS_WAIT,
		STATE_SAS_SIGHT_SHOOT,
		STATE_SAS_WALK_SHOOT,
		STATE_SAS_DEATH,
		STATE_SAS_SIGHT_AIM,
		STATE_SAS_WALK_AIM,
		STATE_SAS_HOLD_AIM,
		STATE_SAS_HOLD_SHOOT,
		STATE_SAS_KNEEL_AIM,
		STATE_SAS_KNEEL_SHOOT,
		STATE_SAS_KNEEL_STOP,
		STATE_SAS_HOLD_PREPARE_GRENADE,
		STATE_SAS_HOLD_SHOOT_GRENADE,
		STATE_SAS_BLIND
	};

	enum SAS_ANIM
	{
		ANIMATION_SAS_WALK,
		ANIMATION_SAS_RUN,
		ANIMATION_SAS_SIGHT_SHOOT,
		ANIMATION_SAS_STAND_TO_SIGHT_AIM,
		ANIMATION_SAS_WALK_TO_WALK_AIM,
		ANIMATION_SAS_RUN_TO_WALK,
		ANIMATION_SAS_WALK_SHOOT,
		ANIMATION_SAS_SIGHT_AIM_TO_STAND,
		ANIMATION_SAS_WALK_AIM_TO_STAND,
		ANIMATION_SAS_STAND_TO_RUN,
		ANIMATION_SAS_STAND_TO_WAIT,
		ANIMATION_SAS_STAND_TO_WALK,
		ANIMATION_SAS_STAND,
		ANIMATION_SAS_WAIT_TO_STAND,
		ANIMATION_SAS_WAIT,
		ANIMATION_SAS_WAIT_TO_SIGHT_AIM,
		ANIMATION_SAS_WALK_TO_RUN,
		ANIMATION_SAS_WALK_TO_STAND,
		ANIMATION_SAS_WALK_AIM_TO_WALK,
		ANIMATION_SAS_DEATH,
		ANIMATION_SAS_STAND_TO_HOLD_AIM,
		ANIMATION_SAS_HOLD_SHOOT,
		ANIMATION_SAS_HOLD_AIM_TO_STAND,
		ANIMATION_SAS_HOLD_PREPARE_GRENADE,
		ANIMATION_SAS_HOLD_SHOOT_GRENADE,
		ANIMATION_SAS_STAND_TO_KNEEL_AIM,
		ANIMATION_SAS_KNEEL_SHOOT,
		ANIMATION_SAS_KNEEL_AIM_TO_STAND,
		ANIMATION_SAS_BLIND,
		ANIMATION_SAS_BLIND_TO_STAND
	};

	BITE_INFO sasGun = { 0, 300, 64, 7 };

	PHD_VECTOR SasDragBlokePosition = { 0, 0, -460 };
	OBJECT_COLLISION_BOUNDS SasDragBlokeBounds = 
	{ -256, 256, -64, 100, -200, -460, ANGLE(-10), ANGLE(10), ANGLE(-30), ANGLE(30), 0, 0 };

	void InitialiseSas(short itemNumber)
	{
		ITEM_INFO* item = &g_Level.Items[itemNumber];

		ClearItem(itemNumber);

		item->animNumber = Objects[item->objectNumber].animIndex + ANIMATION_SAS_STAND;
		item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
		item->goalAnimState = STATE_SAS_STOP;
		item->currentAnimState = STATE_SAS_STOP;
	}

	void SasControl(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;

		ITEM_INFO* item = &g_Level.Items[itemNumber];
		CREATURE_INFO* creature = (CREATURE_INFO*)item->data;
		ITEM_INFO* enemyItem = creature->enemy;

		short tilt = 0;
		short angle = 0;
		short joint0 = 0;
		short joint1 = 0;
		short joint2 = 0;

		// Handle SAS firing
		if (item->firedWeapon)
		{
			PHD_VECTOR pos;

			pos.x = sasGun.x;
			pos.y = sasGun.y;
			pos.z = sasGun.z;

			GetJointAbsPosition(item, &pos, sasGun.meshNum);
			TriggerDynamicLight(pos.x, pos.y, pos.z, 10, 24, 16, 4);
			item->firedWeapon--;
		}

		if (item->hitPoints > 0)
		{
			if (item->aiBits)
				GetAITarget(creature);
			else
				creature->enemy = LaraItem;

			AI_INFO info;

			int distance = 0;
			int ang = 0;
			int dx = 0;
			int dz = 0;

			CreatureAIInfo(item, &info);

			if (creature->enemy == LaraItem)
			{
				ang = info.angle;
				distance = info.distance;
			}
			else
			{
				dx = LaraItem->pos.xPos - item->pos.xPos;
				dz = LaraItem->pos.zPos - item->pos.zPos;
				ang = phd_atan(dz, dx) - item->pos.yRot;
				distance = dx * dx + dz * dz;
			}

			GetCreatureMood(item, &info, creature->enemy != LaraItem);

			// Vehicle handling
			if (Lara.Vehicle != NO_ITEM && info.bite)
				creature->mood = ESCAPE_MOOD;

			CreatureMood(item, &info, creature->enemy != LaraItem);
			angle = CreatureTurn(item, creature->maximumTurn);

			if (item->hitStatus)
				AlertAllGuards(itemNumber);

			int angle1 = 0;
			int angle2 = 0;

			switch (item->currentAnimState)
			{
			case STATE_SAS_STOP:
				creature->flags = 0;
				creature->maximumTurn = 0;
				joint2 = ang;
				if (item->animNumber == Objects[item->objectNumber].animIndex + ANIMATION_SAS_WALK_TO_STAND)
				{
					if (abs(info.angle) >= ANGLE(10))
					{
						if (info.angle >= 0)
							item->pos.yRot += ANGLE(10);
						else
							item->pos.yRot -= ANGLE(10);
					}
					else
					{
						item->pos.yRot += info.angle;
					}
				}
				else if (item->aiBits & MODIFY || Lara.Vehicle != NO_ITEM)
				{
					if (abs(info.angle) >= ANGLE(2))
					{
						if (info.angle >= 0)
							item->pos.yRot += ANGLE(2);
						else
							item->pos.yRot -= ANGLE(2);
					}
					else
					{
						item->pos.yRot += info.angle;
					}
				}

				if (item->aiBits & GUARD)
				{
					joint2 = AIGuard(creature);
					if (!GetRandomControl())
					{
						if (item->currentAnimState == STATE_SAS_STOP)
						{
							item->goalAnimState = STATE_SAS_WAIT;
							break;
						}
						item->goalAnimState = STATE_SAS_STOP;
					}
				}
				else if (!(item->aiBits & PATROL1)
					|| item->aiBits & MODIFY
					|| Lara.Vehicle != NO_ITEM)
				{
					if (Targetable(item, &info))
					{
						if (info.distance < SQUARE(3072)
							|| info.zoneNumber != info.enemyZone)
						{
							if (GetRandomControl() & 1)
							{
								item->goalAnimState = STATE_SAS_SIGHT_AIM;
							}
							else if (GetRandomControl() & 1)
							{
								item->goalAnimState = STATE_SAS_HOLD_AIM;
							}
							else
							{
								item->goalAnimState = STATE_SAS_KNEEL_AIM;
							}
						}
						else if (!(item->aiBits & MODIFY))
						{
							item->goalAnimState = STATE_SAS_WALK;
						}
					}
					else
					{
						if (item->aiBits & MODIFY)
						{
							item->goalAnimState = STATE_SAS_STOP;
						}
						else
						{
							if (creature->mood == ESCAPE_MOOD)
							{
								item->goalAnimState = STATE_SAS_RUN;
							}
							else
							{
								if ((creature->alerted
									|| creature->mood != BORED_MOOD)
									&& (!(item->aiBits & FOLLOW)
										|| !creature->reachedGoal
										&& distance <= SQUARE(2048)))
								{
									if (creature->mood == BORED_MOOD
										|| info.distance <= SQUARE(2048))
									{
										item->goalAnimState = STATE_SAS_WALK;
										break;
									}
									item->goalAnimState = STATE_SAS_RUN;
								}
								else
								{
									item->goalAnimState = STATE_SAS_STOP;
								}
							}
						}
					}
				}
				else
				{
					item->goalAnimState = STATE_SAS_WALK;
					joint2 = 0;
				}
				break;

			case STATE_SAS_WAIT:
				joint2 = ang;
				creature->flags = 0;
				creature->maximumTurn = 0;

				if (item->aiBits & GUARD)
				{
					joint2 = AIGuard(creature);
					if (!GetRandomControl())
					{
						item->goalAnimState = STATE_SAS_STOP;
					}
				}
				else if (Targetable(item, &info)
					|| creature->mood != BORED_MOOD
					|| !info.ahead
					|| item->aiBits & MODIFY
					|| Lara.Vehicle != NO_ITEM)
				{
					item->goalAnimState = STATE_SAS_STOP;
				}
				break;

			case STATE_SAS_WALK:
				creature->flags = 0;
				creature->maximumTurn = ANGLE(5);
				joint2 = ang;

				if (item->aiBits & PATROL1)
				{
					item->goalAnimState = STATE_SAS_WALK;
				}
				else if (Lara.Vehicle == NO_ITEM
					|| !(item->aiBits & MODIFY)
					&& item->aiBits)
				{
					if (creature->mood == ESCAPE_MOOD)
					{
						item->goalAnimState = STATE_SAS_RUN;
					}
					else
					{
						if (item->aiBits & GUARD
							|| item->aiBits & FOLLOW
							&& (creature->reachedGoal
								|| distance > SQUARE(2048)))
						{
							item->goalAnimState = STATE_SAS_STOP;
							break;
						}
						if (Targetable(item, &info))
						{
							if (info.distance < SQUARE(3072)
								|| info.enemyZone != info.zoneNumber)
							{
								item->goalAnimState = STATE_SAS_STOP;
								break;
							}
							item->goalAnimState = STATE_SAS_WALK_AIM;
						}
						else if (creature->mood)
						{
							if (info.distance > SQUARE(2048))
							{
								item->goalAnimState = STATE_SAS_RUN;
							}
						}
						else if (info.ahead)
						{
							item->goalAnimState = STATE_SAS_STOP;
							break;
						}
					}
				}
				else
				{
					item->goalAnimState = STATE_SAS_STOP;
				}
				break;

			case STATE_SAS_RUN:
				if (info.ahead)
					joint2 = info.angle;
				creature->maximumTurn = ANGLE(10);
				tilt = angle / 2;

				if (Lara.Vehicle != NO_ITEM)
				{
					if (item->aiBits & MODIFY || !item->aiBits)
					{
						item->goalAnimState = STATE_SAS_WAIT;
						break;
					}
				}

				if (item->aiBits & GUARD
					|| item->aiBits & FOLLOW
					&& (creature->reachedGoal
						|| distance > SQUARE(2048)))
				{
					item->goalAnimState = STATE_SAS_WALK;
					break;
				}

				if (creature->mood != ESCAPE_MOOD)
				{
					if (Targetable(item, &info))
					{
						item->goalAnimState = STATE_SAS_WALK;
					}
					else
					{
						if (creature->mood != BORED_MOOD
							|| creature->mood == STALK_MOOD
							&& item->aiBits & FOLLOW
							&& info.distance < SQUARE(2048))
						{
							item->goalAnimState = STATE_SAS_WALK;
						}
					}
				}
				break;

			case STATE_SAS_SIGHT_AIM:
			case STATE_SAS_HOLD_AIM:
			case STATE_SAS_KNEEL_AIM:
				creature->flags = 0;
				if (info.ahead)
				{
					joint1 = info.xAngle;
					joint0 = info.angle;
					if (Targetable(item, &info))
					{
						if (item->currentAnimState == STATE_SAS_SIGHT_AIM)
						{
							item->goalAnimState = STATE_SAS_SIGHT_SHOOT;
						}
						else if (item->currentAnimState == STATE_SAS_KNEEL_AIM)
						{
							item->goalAnimState = STATE_SAS_KNEEL_SHOOT;
						}
						else if (GetRandomControl() & 1)
						{
							item->goalAnimState = STATE_SAS_HOLD_SHOOT;
						}
						else
						{
							item->goalAnimState = STATE_SAS_HOLD_PREPARE_GRENADE;
						}
					}
					else
					{
						item->goalAnimState = STATE_SAS_STOP;
					}
				}
				break;

			case STATE_SAS_WALK_AIM:
				creature->flags = 0;
				if (info.ahead)
				{
					joint1 = info.xAngle;
					joint0 = info.angle;
					if (Targetable(item, &info))
					{
						item->goalAnimState = STATE_SAS_WALK_SHOOT;
					}
					else
					{
						item->goalAnimState = STATE_SAS_WALK;
					}
				}
				break;

			case STATE_SAS_HOLD_PREPARE_GRENADE:
				if (info.ahead)
				{
					joint1 = info.xAngle;
					joint0 = info.angle;
				}
				break;

			case STATE_SAS_HOLD_SHOOT_GRENADE:
				if (info.ahead)
				{
					angle1 = info.angle;
					angle2 = info.xAngle;
					joint1 = info.xAngle;
					joint0 = info.angle;

					if (info.distance > SQUARE(3072))
					{
						angle2 = sqrt(info.distance) + info.xAngle - 1024;
						joint1 = angle2;
					}
				}
				else
				{
					angle1 = 0;
					angle2 = 0;
				}

				if (item->frameNumber == g_Level.Anims[item->animNumber].frameBase + 20)
				{
					if (!creature->enemy->speed)
					{
						angle2 += (GetRandomControl() & 0x1FF) - 256;
						joint1 = angle2;
						angle1 += (GetRandomControl() & 0x1FF) - 256;
						joint0 = angle1;
					}

					SasFireGrenade(item, angle2, angle1);

					if (Targetable(item, &info))
						item->goalAnimState = STATE_SAS_HOLD_PREPARE_GRENADE;
				}
				break;

			case STATE_SAS_HOLD_SHOOT:
			case STATE_SAS_KNEEL_SHOOT:
			case STATE_SAS_SIGHT_SHOOT:
			case STATE_SAS_WALK_SHOOT:
				if (item->currentAnimState == STATE_SAS_HOLD_SHOOT
					|| item->currentAnimState == STATE_SAS_KNEEL_SHOOT)
				{
					if (item->goalAnimState != STATE_SAS_STOP
						&& item->goalAnimState != STATE_SAS_KNEEL_STOP
						&& (creature->mood == ESCAPE_MOOD
							|| !Targetable(item, &info)))
					{
						if (item->currentAnimState == STATE_SAS_HOLD_SHOOT)
							item->goalAnimState = STATE_SAS_STOP;
						else
							item->goalAnimState = STATE_SAS_KNEEL_STOP;
					}
				}

				if (info.ahead)
				{
					joint0 = info.angle;
					joint1 = info.xAngle;
				}

				if (creature->flags)
					creature->flags -= 1;
				else
				{
					ShotLara(item, &info, &sasGun, joint0, 15);
					creature->flags = 5;
					item->firedWeapon = 3;
				}
				break;

			case STATE_SAS_BLIND:
				if (!WeaponEnemyTimer && !(GetRandomControl() & 0x7F))
					item->goalAnimState = STATE_SAS_WAIT;
				break;

			default:
				break;
			}

			if (WeaponEnemyTimer > 100
				&& item->currentAnimState != STATE_SAS_BLIND)
			{
				creature->maximumTurn = 0;
				item->animNumber = Objects[item->objectNumber].animIndex + ANIMATION_SAS_BLIND;
				item->frameNumber = g_Level.Anims[item->animNumber].frameBase + (GetRandomControl() & 7);
				item->currentAnimState = STATE_SAS_BLIND;
			}

		}
		else
		{
			if (item->currentAnimState != STATE_SAS_DEATH)
			{
				item->animNumber = Objects[item->objectNumber].animIndex + ANIMATION_SAS_DEATH;
				item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
				item->currentAnimState = STATE_SAS_DEATH;
			}
		}

		CreatureTilt(item, tilt);
		CreatureJoint(item, 0, joint0);
		CreatureJoint(item, 1, joint1);
		CreatureJoint(item, 2, joint2);

		CreatureAnimation(itemNumber, angle, 0);
	}

	void SasFireGrenade(ITEM_INFO* item, short angle1, short angle2)
	{
		short itemNumber = CreateItem();
		if (itemNumber != NO_ITEM)
		{
			ITEM_INFO* grenadeItem = &g_Level.Items[itemNumber];

			grenadeItem->shade = -15856;
			grenadeItem->objectNumber = ID_GRENADE;
			grenadeItem->roomNumber = item->roomNumber;

			PHD_VECTOR pos;
			pos.x = sasGun.x;
			pos.y = sasGun.y;
			pos.z = sasGun.z;

			GetJointAbsPosition(item, &pos, sasGun.meshNum);

			grenadeItem->pos.xPos = pos.x;
			grenadeItem->pos.yPos = pos.y;
			grenadeItem->pos.zPos = pos.z;

			FLOOR_INFO* floor = GetFloor(pos.x, pos.y, pos.z, &grenadeItem->roomNumber);
			int height = GetFloorHeight(floor, pos.x, pos.y, pos.z);

			if (height < grenadeItem->pos.yPos)
			{
				grenadeItem->pos.xPos = item->pos.xPos;
				grenadeItem->pos.yPos = height;
				grenadeItem->pos.zPos = item->pos.zPos;
				grenadeItem->roomNumber = item->roomNumber;
			}

			SmokeCountL = 32;
			SmokeWeapon = 5;

			for (int i = 0; i < 5; i++)
			{
				TriggerGunSmoke(pos.x, pos.y, pos.z, 0, 0, 0, 1, 5, 32);
			}

			InitialiseItem(itemNumber);

			grenadeItem->pos.xRot = angle1 + item->pos.xRot;
			grenadeItem->pos.yRot = angle2 + item->pos.yRot;
			grenadeItem->pos.zRot = 0;

			if (GetRandomControl() & 3)
			{
				grenadeItem->itemFlags[0] = GRENADE_NORMAL;
			}
			else
			{
				grenadeItem->itemFlags[0] = GRENADE_SUPER;
			}

			grenadeItem->itemFlags[2] = 1;
			grenadeItem->speed = 128;
			grenadeItem->currentAnimState = grenadeItem->pos.xRot;
			grenadeItem->fallspeed = -128 * phd_sin(grenadeItem->pos.xRot);
			grenadeItem->goalAnimState = grenadeItem->pos.yRot;
			grenadeItem->requiredAnimState = 0;
			grenadeItem->hitPoints = 120;

			AddActiveItem(itemNumber);
		}
	}

	void InitialiseInjuredSas(short itemNumber)
	{
		ITEM_INFO* item = &g_Level.Items[itemNumber];

		if (item->triggerFlags)
		{
			item->animNumber = Objects[item->objectNumber].animIndex;
			item->goalAnimState = item->currentAnimState = 1;
		}
		else
		{
			item->animNumber = Objects[item->objectNumber].animIndex + 3;
			item->goalAnimState = item->currentAnimState = 4;
		}

		item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
	}

	void InjuredSasControl(short itemNumber)
	{
		ITEM_INFO* item = &g_Level.Items[itemNumber];

		if (item->currentAnimState == 1)
		{
			if (!(GetRandomControl() & 0x7F))
			{
				item->goalAnimState = 2;
				AnimateItem(item);
			}
			else if (!(byte)GetRandomControl())
			{
				item->goalAnimState = 3;
			}
		}
		else if (item->currentAnimState == 4 && !(GetRandomControl() & 0x7F))
		{
			item->goalAnimState = 5;
			AnimateItem(item);
		}
		else
		{
			AnimateItem(item);
		}
	}

	void SasDragBlokeCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* c)
	{
		ITEM_INFO* item = &g_Level.Items[itemNumber];

		if ((!(TrInput & IN_ACTION)
			|| l->currentAnimState != LS_STOP
			|| l->animNumber != LA_STAND_IDLE
			|| Lara.gunStatus
			|| l->gravityStatus
			|| item->flags & 0x3E00)
			&& (!(Lara.isMoving) || Lara.interactedItem != itemNumber))
		{
			if (item->status == ITEM_ACTIVE)
			{
				if (item->frameNumber == g_Level.Anims[item->animNumber].frameEnd)
				{
					int x = l->pos.xPos - 512 * phd_sin(l->pos.yRot);
					int y = l->pos.yPos;
					int z = l->pos.zPos - 512 * phd_cos(l->pos.yRot);

					TestTriggers(x, y, z, l->roomNumber, true);

					RemoveActiveItem(itemNumber);
					item->status = ITEM_NOT_ACTIVE;
				}
			}

			ObjectCollision(itemNumber, l, c);
		}
		else
		{
			if (TestLaraPosition(&SasDragBlokeBounds, item, l))
			{
				if (MoveLaraPosition(&SasDragBlokePosition, item, l))
				{
					l->animNumber = LA_DRAG_BODY;
					l->currentAnimState = LS_MISC_CONTROL;
					l->frameNumber = g_Level.Anims[l->animNumber].frameBase;
					l->pos.yRot = item->pos.yRot;
					Lara.isMoving = false;
					Lara.headXrot = 0;
					Lara.headYrot = 0;
					Lara.torsoXrot = 0;
					Lara.torsoYrot = 0;
					Lara.gunStatus = LG_HANDS_BUSY;
					item->flags |= 0x3E00;
					item->status = ITEM_ACTIVE;
					AddActiveItem(itemNumber);
				}
				else
				{
					Lara.interactedItem = itemNumber;
				}
			}
		}
	}
}
