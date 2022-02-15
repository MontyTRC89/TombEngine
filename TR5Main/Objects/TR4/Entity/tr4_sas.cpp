#include "framework.h"
#include "tr4_sas.h"
#include "Game/control/box.h"
#include "Game/items.h"
#include "Game/people.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Specific/setup.h"
#include "Specific/level.h"
#include "Game/control/control.h"
#include "Game/animation.h"
#include "Game/effects/effects.h"
#include "Game/effects/tomb4fx.h"
#include "Specific/input.h"
#include "Game/Lara/lara_one_gun.h"
#include "Game/itemdata/creature_info.h"
#include "Game/collision/collide_item.h"

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

		item->AnimNumber = Objects[item->ObjectNumber].animIndex + ANIMATION_SAS_STAND;
		item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
		item->TargetState = STATE_SAS_STOP;
		item->ActiveState = STATE_SAS_STOP;
	}

	void SasControl(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;

		ITEM_INFO* item = &g_Level.Items[itemNumber];
		CREATURE_INFO* creature = (CREATURE_INFO*)item->Data;
		ITEM_INFO* enemyItem = creature->enemy;

		short tilt = 0;
		short angle = 0;
		short joint0 = 0;
		short joint1 = 0;
		short joint2 = 0;

		// Handle SAS firing
		if (item->FiredWeapon)
		{
			PHD_VECTOR pos;

			pos.x = sasGun.x;
			pos.y = sasGun.y;
			pos.z = sasGun.z;

			GetJointAbsPosition(item, &pos, sasGun.meshNum);
			TriggerDynamicLight(pos.x, pos.y, pos.z, 10, 24, 16, 4);
			item->FiredWeapon--;
		}

		if (item->HitPoints > 0)
		{
			if (item->AIBits)
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
				dx = LaraItem->Position.xPos - item->Position.xPos;
				dz = LaraItem->Position.zPos - item->Position.zPos;
				ang = phd_atan(dz, dx) - item->Position.yRot;
				distance = dx * dx + dz * dz;
			}

			GetCreatureMood(item, &info, creature->enemy != LaraItem);

			// Vehicle handling
			if (Lara.Vehicle != NO_ITEM && info.bite)
				creature->mood = ESCAPE_MOOD;

			CreatureMood(item, &info, creature->enemy != LaraItem);
			angle = CreatureTurn(item, creature->maximumTurn);

			if (item->HitStatus)
				AlertAllGuards(itemNumber);

			int angle1 = 0;
			int angle2 = 0;

			switch (item->ActiveState)
			{
			case STATE_SAS_STOP:
				creature->flags = 0;
				creature->maximumTurn = 0;
				joint2 = ang;
				if (item->AnimNumber == Objects[item->ObjectNumber].animIndex + ANIMATION_SAS_WALK_TO_STAND)
				{
					if (abs(info.angle) >= ANGLE(10))
					{
						if (info.angle >= 0)
							item->Position.yRot += ANGLE(10);
						else
							item->Position.yRot -= ANGLE(10);
					}
					else
					{
						item->Position.yRot += info.angle;
					}
				}
				else if (item->AIBits & MODIFY || Lara.Vehicle != NO_ITEM)
				{
					if (abs(info.angle) >= ANGLE(2))
					{
						if (info.angle >= 0)
							item->Position.yRot += ANGLE(2);
						else
							item->Position.yRot -= ANGLE(2);
					}
					else
					{
						item->Position.yRot += info.angle;
					}
				}

				if (item->AIBits & GUARD)
				{
					joint2 = AIGuard(creature);
					if (!GetRandomControl())
					{
						if (item->ActiveState == STATE_SAS_STOP)
						{
							item->TargetState = STATE_SAS_WAIT;
							break;
						}
						item->TargetState = STATE_SAS_STOP;
					}
				}
				else if (!(item->AIBits & PATROL1)
					|| item->AIBits & MODIFY
					|| Lara.Vehicle != NO_ITEM)
				{
					if (Targetable(item, &info))
					{
						if (info.distance < SQUARE(3072)
							|| info.zoneNumber != info.enemyZone)
						{
							if (GetRandomControl() & 1)
							{
								item->TargetState = STATE_SAS_SIGHT_AIM;
							}
							else if (GetRandomControl() & 1)
							{
								item->TargetState = STATE_SAS_HOLD_AIM;
							}
							else
							{
								item->TargetState = STATE_SAS_KNEEL_AIM;
							}
						}
						else if (!(item->AIBits & MODIFY))
						{
							item->TargetState = STATE_SAS_WALK;
						}
					}
					else
					{
						if (item->AIBits & MODIFY)
						{
							item->TargetState = STATE_SAS_STOP;
						}
						else
						{
							if (creature->mood == ESCAPE_MOOD)
							{
								item->TargetState = STATE_SAS_RUN;
							}
							else
							{
								if ((creature->alerted
									|| creature->mood != BORED_MOOD)
									&& (!(item->AIBits & FOLLOW)
										|| !creature->reachedGoal
										&& distance <= SQUARE(2048)))
								{
									if (creature->mood == BORED_MOOD
										|| info.distance <= SQUARE(2048))
									{
										item->TargetState = STATE_SAS_WALK;
										break;
									}
									item->TargetState = STATE_SAS_RUN;
								}
								else
								{
									item->TargetState = STATE_SAS_STOP;
								}
							}
						}
					}
				}
				else
				{
					item->TargetState = STATE_SAS_WALK;
					joint2 = 0;
				}
				break;

			case STATE_SAS_WAIT:
				joint2 = ang;
				creature->flags = 0;
				creature->maximumTurn = 0;

				if (item->AIBits & GUARD)
				{
					joint2 = AIGuard(creature);
					if (!GetRandomControl())
					{
						item->TargetState = STATE_SAS_STOP;
					}
				}
				else if (Targetable(item, &info)
					|| creature->mood != BORED_MOOD
					|| !info.ahead
					|| item->AIBits & MODIFY
					|| Lara.Vehicle != NO_ITEM)
				{
					item->TargetState = STATE_SAS_STOP;
				}
				break;

			case STATE_SAS_WALK:
				creature->flags = 0;
				creature->maximumTurn = ANGLE(5);
				joint2 = ang;

				if (item->AIBits & PATROL1)
				{
					item->TargetState = STATE_SAS_WALK;
				}
				else if (Lara.Vehicle == NO_ITEM
					|| !(item->AIBits & MODIFY)
					&& item->AIBits)
				{
					if (creature->mood == ESCAPE_MOOD)
					{
						item->TargetState = STATE_SAS_RUN;
					}
					else
					{
						if (item->AIBits & GUARD
							|| item->AIBits & FOLLOW
							&& (creature->reachedGoal
								|| distance > SQUARE(2048)))
						{
							item->TargetState = STATE_SAS_STOP;
							break;
						}
						if (Targetable(item, &info))
						{
							if (info.distance < SQUARE(3072)
								|| info.enemyZone != info.zoneNumber)
							{
								item->TargetState = STATE_SAS_STOP;
								break;
							}
							item->TargetState = STATE_SAS_WALK_AIM;
						}
						else if (creature->mood)
						{
							if (info.distance > SQUARE(2048))
							{
								item->TargetState = STATE_SAS_RUN;
							}
						}
						else if (info.ahead)
						{
							item->TargetState = STATE_SAS_STOP;
							break;
						}
					}
				}
				else
				{
					item->TargetState = STATE_SAS_STOP;
				}
				break;

			case STATE_SAS_RUN:
				if (info.ahead)
					joint2 = info.angle;
				creature->maximumTurn = ANGLE(10);
				tilt = angle / 2;

				if (Lara.Vehicle != NO_ITEM)
				{
					if (item->AIBits & MODIFY || !item->AIBits)
					{
						item->TargetState = STATE_SAS_WAIT;
						break;
					}
				}

				if (item->AIBits & GUARD
					|| item->AIBits & FOLLOW
					&& (creature->reachedGoal
						|| distance > SQUARE(2048)))
				{
					item->TargetState = STATE_SAS_WALK;
					break;
				}

				if (creature->mood != ESCAPE_MOOD)
				{
					if (Targetable(item, &info))
					{
						item->TargetState = STATE_SAS_WALK;
					}
					else
					{
						if (creature->mood != BORED_MOOD
							|| creature->mood == STALK_MOOD
							&& item->AIBits & FOLLOW
							&& info.distance < SQUARE(2048))
						{
							item->TargetState = STATE_SAS_WALK;
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
						if (item->ActiveState == STATE_SAS_SIGHT_AIM)
						{
							item->TargetState = STATE_SAS_SIGHT_SHOOT;
						}
						else if (item->ActiveState == STATE_SAS_KNEEL_AIM)
						{
							item->TargetState = STATE_SAS_KNEEL_SHOOT;
						}
						else if (GetRandomControl() & 1)
						{
							item->TargetState = STATE_SAS_HOLD_SHOOT;
						}
						else
						{
							item->TargetState = STATE_SAS_HOLD_PREPARE_GRENADE;
						}
					}
					else
					{
						item->TargetState = STATE_SAS_STOP;
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
						item->TargetState = STATE_SAS_WALK_SHOOT;
					}
					else
					{
						item->TargetState = STATE_SAS_WALK;
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

				if (item->FrameNumber == g_Level.Anims[item->AnimNumber].frameBase + 20)
				{
					if (!creature->enemy->Velocity)
					{
						angle2 += (GetRandomControl() & 0x1FF) - 256;
						joint1 = angle2;
						angle1 += (GetRandomControl() & 0x1FF) - 256;
						joint0 = angle1;
					}

					SasFireGrenade(item, angle2, angle1);

					if (Targetable(item, &info))
						item->TargetState = STATE_SAS_HOLD_PREPARE_GRENADE;
				}
				break;

			case STATE_SAS_HOLD_SHOOT:
			case STATE_SAS_KNEEL_SHOOT:
			case STATE_SAS_SIGHT_SHOOT:
			case STATE_SAS_WALK_SHOOT:
				if (item->ActiveState == STATE_SAS_HOLD_SHOOT
					|| item->ActiveState == STATE_SAS_KNEEL_SHOOT)
				{
					if (item->TargetState != STATE_SAS_STOP
						&& item->TargetState != STATE_SAS_KNEEL_STOP
						&& (creature->mood == ESCAPE_MOOD
							|| !Targetable(item, &info)))
					{
						if (item->ActiveState == STATE_SAS_HOLD_SHOOT)
							item->TargetState = STATE_SAS_STOP;
						else
							item->TargetState = STATE_SAS_KNEEL_STOP;
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
					item->FiredWeapon = 3;
				}
				break;

			case STATE_SAS_BLIND:
				if (!WeaponEnemyTimer && !(GetRandomControl() & 0x7F))
					item->TargetState = STATE_SAS_WAIT;
				break;

			default:
				break;
			}

			if (WeaponEnemyTimer > 100
				&& item->ActiveState != STATE_SAS_BLIND)
			{
				creature->maximumTurn = 0;
				item->AnimNumber = Objects[item->ObjectNumber].animIndex + ANIMATION_SAS_BLIND;
				item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase + (GetRandomControl() & 7);
				item->ActiveState = STATE_SAS_BLIND;
			}

		}
		else
		{
			if (item->ActiveState != STATE_SAS_DEATH)
			{
				item->AnimNumber = Objects[item->ObjectNumber].animIndex + ANIMATION_SAS_DEATH;
				item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
				item->ActiveState = STATE_SAS_DEATH;
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

			grenadeItem->Shade = -15856;
			grenadeItem->ObjectNumber = ID_GRENADE;
			grenadeItem->RoomNumber = item->RoomNumber;

			PHD_VECTOR pos;
			pos.x = sasGun.x;
			pos.y = sasGun.y;
			pos.z = sasGun.z;

			GetJointAbsPosition(item, &pos, sasGun.meshNum);

			grenadeItem->Position.xPos = pos.x;
			grenadeItem->Position.yPos = pos.y;
			grenadeItem->Position.zPos = pos.z;

			FLOOR_INFO* floor = GetFloor(pos.x, pos.y, pos.z, &grenadeItem->RoomNumber);
			int height = GetFloorHeight(floor, pos.x, pos.y, pos.z);

			if (height < grenadeItem->Position.yPos)
			{
				grenadeItem->Position.xPos = item->Position.xPos;
				grenadeItem->Position.yPos = height;
				grenadeItem->Position.zPos = item->Position.zPos;
				grenadeItem->RoomNumber = item->RoomNumber;
			}

			SmokeCountL = 32;
			SmokeWeapon = 5;

			for (int i = 0; i < 5; i++)
			{
				TriggerGunSmoke(pos.x, pos.y, pos.z, 0, 0, 0, 1, 5, 32);
			}

			InitialiseItem(itemNumber);

			grenadeItem->Position.xRot = angle1 + item->Position.xRot;
			grenadeItem->Position.yRot = angle2 + item->Position.yRot;
			grenadeItem->Position.zRot = 0;

			if (GetRandomControl() & 3)
			{
				grenadeItem->ItemFlags[0] = (int)GrenadeType::Normal;
			}
			else
			{
				grenadeItem->ItemFlags[0] = (int)GrenadeType::Super;
			}

			grenadeItem->ItemFlags[2] = 1;
			grenadeItem->Velocity = 128;
			grenadeItem->ActiveState = grenadeItem->Position.xRot;
			grenadeItem->VerticalVelocity = -128 * phd_sin(grenadeItem->Position.xRot);
			grenadeItem->TargetState = grenadeItem->Position.yRot;
			grenadeItem->RequiredState = 0;
			grenadeItem->HitPoints = 120;

			AddActiveItem(itemNumber);
		}
	}

	void InitialiseInjuredSas(short itemNumber)
	{
		ITEM_INFO* item = &g_Level.Items[itemNumber];

		if (item->TriggerFlags)
		{
			item->AnimNumber = Objects[item->ObjectNumber].animIndex;
			item->TargetState = item->ActiveState = 1;
		}
		else
		{
			item->AnimNumber = Objects[item->ObjectNumber].animIndex + 3;
			item->TargetState = item->ActiveState = 4;
		}

		item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
	}

	void InjuredSasControl(short itemNumber)
	{
		ITEM_INFO* item = &g_Level.Items[itemNumber];

		if (item->ActiveState == 1)
		{
			if (!(GetRandomControl() & 0x7F))
			{
				item->TargetState = 2;
				AnimateItem(item);
			}
			else if (!(byte)GetRandomControl())
			{
				item->TargetState = 3;
			}
		}
		else if (item->ActiveState == 4 && !(GetRandomControl() & 0x7F))
		{
			item->TargetState = 5;
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
			|| l->ActiveState != LS_IDLE
			|| l->AnimNumber != LA_STAND_IDLE
			|| Lara.Control.HandStatus != HandStatus::Free
			|| l->Airborne
			|| item->Flags & 0x3E00)
			&& (!(Lara.Control.IsMoving) || Lara.interactedItem != itemNumber))
		{
			if (item->Status == ITEM_ACTIVE)
			{
				if (item->FrameNumber == g_Level.Anims[item->AnimNumber].frameEnd)
				{
					int x = l->Position.xPos - 512 * phd_sin(l->Position.yRot);
					int y = l->Position.yPos;
					int z = l->Position.zPos - 512 * phd_cos(l->Position.yRot);

					TestTriggers(x, y, z, l->RoomNumber, true);

					RemoveActiveItem(itemNumber);
					item->Status = ITEM_NOT_ACTIVE;
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
					l->AnimNumber = LA_DRAG_BODY;
					l->ActiveState = LS_MISC_CONTROL;
					l->FrameNumber = g_Level.Anims[l->AnimNumber].frameBase;
					l->Position.yRot = item->Position.yRot;
					Lara.Control.IsMoving = false;
					ResetLaraFlex(l);
					Lara.Control.HandStatus = HandStatus::Busy;
					item->Flags |= 0x3E00;
					item->Status = ITEM_ACTIVE;
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
