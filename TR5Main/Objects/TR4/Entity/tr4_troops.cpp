#include "framework.h"
#include "tr4_troops.h"
#include "Game/control/box.h"
#include "Game/items.h"
#include "Game/effects/effects.h"
#include "Game/Lara/lara.h"
#include "Game/people.h"
#include "Specific/setup.h"
#include "Game/control/lot.h"
#include "Specific/level.h"
#include "Game/itemdata/creature_info.h"
#include "Game/control/control.h"
#include "Game/animation.h"

BITE_INFO TroopsBite1 = { 0, 300, 64, 7 };

#define STATE_TROOPS_STOP						1
#define STATE_TROOPS_WALK						2
#define STATE_TROOPS_RUN						3
#define STATE_TROOPS_GUARD						4
#define STATE_TROOPS_ATTACK1					5
#define STATE_TROOPS_ATTACK2					6
#define STATE_TROOPS_DEATH						7
#define STATE_TROOPS_AIM1						8
#define STATE_TROOPS_AIM2						9
#define STATE_TROOPS_AIM3						10
#define STATE_TROOPS_ATTACK3					11
#define STATE_TROOPS_KILLED_BY_SCORPION			15
#define STATE_TROOPS_ATTACKED_BY_SCORPION		16
#define STATE_TROOPS_FLASHED					17

void InitialiseTroops(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	ClearItem(itemNumber);

	if (item->TriggerFlags == 1)
	{
		item->Animation.TargetState = item->Animation.ActiveState = STATE_TROOPS_ATTACKED_BY_SCORPION;
		item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 27;
	}
	else
	{
		item->Animation.TargetState = item->Animation.ActiveState = STATE_TROOPS_STOP;
		item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 12;
	}

	item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].FrameBase;
}

void TroopsControl(short itemNumber)
{
	if (!CreatureActive(itemNumber))
		return;

	ITEM_INFO* item = &g_Level.Items[itemNumber];
	CreatureInfo* creature = (CreatureInfo*)item->Data;
	OBJECT_INFO* obj = &Objects[item->ObjectNumber];
	
	short angle = 0;
	short tilt = 0;
	short joint0 = 0;
	short joint1 = 0;
	short joint2 = 0;
	short rot = 0;
	
	int dx = 0;
	int dy = 0;
	int dz = 0;
	
	int distance = 0;

	if (creature->FiredWeapon)
	{
		Vector3Int pos;

		pos.x = TroopsBite1.x;
		pos.y = TroopsBite1.y;
		pos.z = TroopsBite1.z;

		GetJointAbsPosition(item, &pos, TroopsBite1.meshNum);
		TriggerDynamicLight(pos.x, pos.y, pos.z, 2 * creature->FiredWeapon + 8, 24, 16, 4);

		creature->FiredWeapon--;
	}

	if (item->HitPoints <= 0)
	{
		if (item->Animation.ActiveState != STATE_TROOPS_DEATH 
			&& item->Animation.ActiveState != STATE_TROOPS_KILLED_BY_SCORPION)
		{
			if (creature->Enemy 
				&& creature->Enemy->ObjectNumber == ID_BIG_SCORPION 
				&& item->ItemFlags[0] < 80)
			{
				if (creature->Enemy->Animation.AnimNumber == Objects[ID_BIG_SCORPION].animIndex + 6)
				{
					item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 23;
					if (item->Animation.ActiveState == STATE_TROOPS_ATTACKED_BY_SCORPION)
					{
						item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].FrameBase + 37;
					}
					else
					{
						item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].FrameBase;
					}
					item->Animation.TargetState = STATE_TROOPS_KILLED_BY_SCORPION;
					item->Animation.ActiveState = STATE_TROOPS_KILLED_BY_SCORPION;

					angle = 0;

					item->Pose.Position.x = creature->Enemy->Pose.Position.x;
					item->Pose.Position.y = creature->Enemy->Pose.Position.y;
					item->Pose.Position.z = creature->Enemy->Pose.Position.z;

					item->Pose.Orientation.x = creature->Enemy->Pose.Orientation.x;
					item->Pose.Orientation.y = creature->Enemy->Pose.Orientation.y;
					item->Pose.Orientation.z = creature->Enemy->Pose.Orientation.z;

					creature->Enemy->TriggerFlags = 99;
				}
				else
				{
					angle = 0;
					item->ItemFlags[0]++;
				}
			}
			else
			{
				item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 19;
				item->Animation.ActiveState = STATE_TROOPS_DEATH;
				item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].FrameBase;
			}
		}
	}
	else
	{
		if (item->AIBits)
			GetAITarget(creature);
		else
		{
			// Search for active troops
			creature->Enemy = NULL;
			CreatureInfo* baddy = ActiveCreatures[0];
			int minDistance = 0x7FFFFFFF;

			for (int i = 0; i < ActiveCreatures.size(); i++)
			{
				baddy = ActiveCreatures[i];

				if (baddy->ItemNumber != NO_ITEM && baddy->ItemNumber != itemNumber)
				{
					ITEM_INFO* currentItem = &g_Level.Items[baddy->ItemNumber];

					if (currentItem->ObjectNumber != ID_LARA)
					{
						if (currentItem->ObjectNumber != ID_TROOPS &&
							(currentItem != LaraItem || creature->HurtByLara))
						{
							dx = currentItem->Pose.Position.x - item->Pose.Position.x;
							dy = currentItem->Pose.Position.y - item->Pose.Position.y;
							dz = currentItem->Pose.Position.z - item->Pose.Position.z;

							distance = SQUARE(dx) + SQUARE(dy) + SQUARE(dz);

							if (distance < minDistance)
							{
								minDistance = distance;
								creature->Enemy = currentItem;
							}
						}
					}
				}
			}
		}
		
		if (creature->HurtByLara && item->Animation.ActiveState != STATE_TROOPS_ATTACKED_BY_SCORPION)
			creature->Enemy = LaraItem;

		AI_INFO info;	
		CreatureAIInfo(item, &info);

		int distance = 0;
		if (creature->Enemy == LaraItem)
		{
			distance = info.distance;
			rot = info.angle;
		}
		else
		{
			dx = LaraItem->Pose.Position.x - item->Pose.Position.x;
			dz = LaraItem->Pose.Position.z - item->Pose.Position.z;
			distance = SQUARE(dx) + SQUARE(dz);
			rot = phd_atan(dz, dx) - item->Pose.Orientation.y;
		}

		if (!creature->HurtByLara && creature->Enemy == LaraItem)
			creature->Enemy = NULL;

		GetCreatureMood(item, &info, TIMID);
		CreatureMood(item, &info, TIMID);

		// Vehicle handling
		if (Lara.Vehicle != NO_ITEM && info.bite)
			creature->Mood = MoodType::Escape;

		angle = CreatureTurn(item, creature->MaxTurn);

		if (item->HitStatus)
			AlertAllGuards(itemNumber);

		switch (item->Animation.ActiveState)
		{
		case STATE_TROOPS_STOP:
			creature->Flags = 0;
			creature->MaxTurn = 0;
			joint2 = rot;

			if (item->Animation.AnimNumber == obj->animIndex + 17)
			{
				if (abs(info.angle) >= ANGLE(10))
				{
					if (info.angle >= 0)
					{
						item->Pose.Orientation.y += ANGLE(10);
					}
					else
					{
						item->Pose.Orientation.y -= ANGLE(10);
					}
				}
				else
				{
					item->Pose.Orientation.y += info.angle;
				}
			}

			if (item->AIBits & GUARD)
			{
				joint2 = AIGuard(creature);
				if (!GetRandomControl())
				{
					if (item->Animation.ActiveState == STATE_TROOPS_STOP)
					{
						item->Animation.TargetState = STATE_TROOPS_GUARD;
					}
					else
					{
						item->Animation.TargetState = STATE_TROOPS_STOP;
					}
				}
			}
			else if (item->AIBits & PATROL1)
			{
				item->Animation.TargetState = STATE_TROOPS_WALK;
				joint2 = 0;
			}
			else if (creature->Mood == MoodType::Escape)
			{
				item->Animation.TargetState = STATE_TROOPS_RUN;
			}
			else if (Targetable(item, &info))
			{
				if (info.distance < SQUARE(3072) || info.zoneNumber != info.enemyZone)
				{
					if (GetRandomControl() >= 16384)
					{
						item->Animation.TargetState = STATE_TROOPS_AIM3;
					}
					else
					{
						item->Animation.TargetState = STATE_TROOPS_AIM1;
					}
				}
				else
				{
					item->Animation.TargetState = STATE_TROOPS_WALK;
				}
			}
			else
			{
				if ((creature->Alerted 
					|| creature->Mood != MoodType::Bored)
					&& (!(item->AIBits & FOLLOW) 
						|| !(item->AIBits & MODIFY) 
						&& distance <= SQUARE(2048)))
				{
					if (creature->Mood == MoodType::Bored || info.distance <= SQUARE(2048))
					{
						item->Animation.TargetState = STATE_TROOPS_WALK;
						break;
					}
					item->Animation.TargetState = STATE_TROOPS_RUN;
				}
				else
				{
					item->Animation.TargetState = STATE_TROOPS_STOP;
				}
			}

			break;

		case STATE_TROOPS_WALK:
			creature->Flags = 0;
			joint2 = rot;
			creature->MaxTurn = ANGLE(5);

			if (item->AIBits & PATROL1)
			{
				item->Animation.TargetState = STATE_TROOPS_WALK;
			}
			else if (creature->Mood == MoodType::Escape)
			{
				item->Animation.TargetState = STATE_TROOPS_RUN;
			}
			else
			{
				if ((item->AIBits & GUARD)
					|| (item->AIBits & FOLLOW) 
					&& (creature->ReachedGoal 
						|| distance > SQUARE(2048)))
				{
					item->Animation.TargetState = STATE_TROOPS_STOP;
				}
				else if (Targetable(item, &info))
				{
					if (info.distance < SQUARE(3072) || info.enemyZone != info.zoneNumber)
					{
						item->Animation.TargetState = STATE_TROOPS_STOP;
					}
					else
					{
						item->Animation.TargetState = STATE_TROOPS_AIM2;
					}
				}
				else if (creature->Mood != MoodType::Bored)
				{
					if (info.distance > SQUARE(2048))
					{
						item->Animation.TargetState = STATE_TROOPS_RUN;
					}
				}
				else if (info.ahead)
				{
					item->Animation.TargetState = STATE_TROOPS_STOP;
				}
			}

			break;

		case STATE_TROOPS_RUN:
			if (info.ahead)
			{
				joint2 = info.angle;
			}
			creature->MaxTurn = ANGLE(10);
			tilt = angle / 2;

			if ((item->AIBits & GUARD) 
				|| (item->AIBits & FOLLOW) 
				&& (creature->ReachedGoal 
					|| distance > SQUARE(2048)))
			{
				item->Animation.TargetState = STATE_TROOPS_WALK;
			}
			else if (creature->Mood != MoodType::Escape)
			{
				if (Targetable(item, &info))
				{
					item->Animation.TargetState = STATE_TROOPS_WALK;
				}
				else if (creature->Mood == MoodType::Bored 
					|| creature->Mood == MoodType::Stalk 
					&& !(item->AIBits & FOLLOW) 
					&& info.distance < SQUARE(2048))
				{
					item->Animation.TargetState = STATE_TROOPS_WALK;
				}
			}

			break;

		case STATE_TROOPS_GUARD:
			creature->Flags = 0;
			creature->MaxTurn = 0;
			joint2 = rot;

			if (item->AIBits & GUARD)
			{
				joint2 = AIGuard(creature);
				if (!GetRandomControl())
				{
					item->Animation.TargetState = STATE_TROOPS_STOP;
				}
			}
			else if (Targetable(item, &info))
			{
				item->Animation.TargetState = STATE_TROOPS_ATTACK1;
			}
			else if (creature->Mood != MoodType::Bored || !info.ahead)
			{
				item->Animation.TargetState = STATE_TROOPS_STOP;
			}

			break;

		case STATE_TROOPS_ATTACK1:
		case STATE_TROOPS_ATTACK2:
			if (info.ahead)
			{
				joint0 = info.angle;
				joint1 = info.xAngle;
			}

			if (creature->Flags)
			{
				creature->Flags--;
			}
			else
			{
				ShotLara(item, &info, &TroopsBite1, joint0, 23);
				creature->Flags = 5;
			}

			break;

		case STATE_TROOPS_AIM1:
		case STATE_TROOPS_AIM3:
			creature->Flags = 0;

			if (info.ahead)
			{
				joint1 = info.xAngle;
				joint0 = info.angle;

				if (Targetable(item, &info))
				{
					item->Animation.TargetState = item->Animation.ActiveState != STATE_TROOPS_AIM1 ? STATE_TROOPS_ATTACK3 : STATE_TROOPS_ATTACK1;
				}
				else
				{
					item->Animation.TargetState = STATE_TROOPS_STOP;
				}
			}

			break;

		case STATE_TROOPS_AIM2:
			creature->Flags = 0;

			if (info.ahead)
			{
				joint1 = info.xAngle;
				joint0 = info.angle;

				if (Targetable(item, &info))
				{
					item->Animation.TargetState = STATE_TROOPS_ATTACK2;
				}
				else
				{
					item->Animation.TargetState = STATE_TROOPS_WALK;
				}
			}

			break;

		case STATE_TROOPS_ATTACK3:
			if (item->Animation.TargetState != STATE_TROOPS_STOP
				&& (creature->Mood == MoodType::Escape || 
					info.distance > SQUARE(3072) || 
					!Targetable(item, &info)))
			{
				item->Animation.TargetState = STATE_TROOPS_STOP;
			}

			if (info.ahead)
			{
				joint0 = info.angle;
				joint1 = info.xAngle;
			}

			if (creature->Flags)
			{
				creature->Flags--;
			}
			else
			{
				ShotLara(item, &info, &TroopsBite1, joint0, 23);
				creature->Flags = 5;
			}

			break;

		case STATE_TROOPS_ATTACKED_BY_SCORPION:
			creature->MaxTurn = 0;
			break;

		case STATE_TROOPS_FLASHED:
			if (!WeaponEnemyTimer && !(GetRandomControl() & 0x7F))
			{
				item->Animation.TargetState = STATE_TROOPS_GUARD;
			}

			break;

		default:
			break;
		}

		if (WeaponEnemyTimer > 100)
		{
			if (item->Animation.ActiveState != STATE_TROOPS_FLASHED 
				&& item->Animation.ActiveState != STATE_TROOPS_ATTACKED_BY_SCORPION)
			{
				creature->MaxTurn = 0;
				item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 28;
				item->Animation.ActiveState = STATE_TROOPS_FLASHED;
				item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].FrameBase + (GetRandomControl() & 7);
			}
		}
	}

	CreatureTilt(item, tilt);
	CreatureJoint(item, 0, joint0);
	CreatureJoint(item, 1, joint1);
	CreatureJoint(item, 2, joint2);
	CreatureAnimation(itemNumber, angle, 0);
}
