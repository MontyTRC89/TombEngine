#include "framework.h"
#include "tr5_larson.h"
#include "Game/items.h"
#include "Game/control/box.h"
#include "Game/effects/effects.h"
#include "Game/people.h"
#include "Game/Lara/lara.h"
#include "Specific/setup.h"
#include "Specific/level.h"
#include "Game/itemdata/creature_info.h"
#include "Game/control/control.h"
#include "Game/animation.h"

#define STATE_TR5_LARSON_STOP	1
#define STATE_TR5_LARSON_WALK	2
#define STATE_TR5_LARSON_RUN	3
#define STATE_TR5_LARSON_AIM	4
#define STATE_TR5_LARSON_DIE	5
#define STATE_TR5_LARSON_IDLE	6
#define STATE_TR5_LARSON_ATTACK	7

#define ANIMATION_TR5_PIERRE_DIE 12
#define ANIMATION_TR5_LARSON_DIE 15

#define TR5_LARSON_MIN_HP 40

BITE_INFO LarsonGun = { -55, 200, 5, 14 };
BITE_INFO PierreGun1 = { 60, 200, 0, 11 };
BITE_INFO PierreGun2 = { -57, 200, 0, 14 };

void InitialiseLarson(short itemNum)
{
	ItemInfo* item = &g_Level.Items[itemNum];

	ClearItem(itemNum);

	item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex;
	item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
	item->Animation.TargetState = STATE_TR5_LARSON_STOP;
	item->Animation.ActiveState = STATE_TR5_LARSON_STOP;

	if (!item->TriggerFlags)
		return;

	item->ItemFlags[3] = item->TriggerFlags;
	short rotY = item->Pose.Orientation.y;

	if (rotY > ANGLE(22.5f) && rotY < 28672)
	{
		item->Pose.Position.x += STEPUP_HEIGHT;
	}
	else if (rotY < -ANGLE(22.5f) && rotY > -28672)
	{
		item->Pose.Position.x -= STEPUP_HEIGHT;
	}
	else if (rotY < -20480 || rotY > 20480)
	{
		item->Pose.Position.z -= STEPUP_HEIGHT;
	}
	else if (rotY > -8192 || rotY < 8192)
	{
		item->Pose.Position.z += STEPUP_HEIGHT;
	}
}

void LarsonControl(short itemNumber)
{
	if (!CreatureActive(itemNumber))
		return;
	
	short tilt = 0;
	short angle = 0;
	short joint0 = 0;
	short joint1 = 0;
	short joint2 = 0;
	
	auto* item = &g_Level.Items[itemNumber];
	CreatureInfo* creature = (CreatureInfo*)item->Data;
	
	// In Streets of Rome when Larson HP are below 40 he runs way
	/*if (item->HitPoints <= TR5_LARSON_MIN_HP && !(item->flags & IFLAG_INVISIBLE))
	{
		item->HitPoints = TR5_LARSON_MIN_HP;
		creature->flags++;
	}*/

	// Fire weapon effects
	if (creature->FiredWeapon)
	{
		Vector3Int pos;

		pos.x = LarsonGun.x;
		pos.y = LarsonGun.y;
		pos.z = LarsonGun.z;

		GetJointAbsPosition(item, &pos, LarsonGun.meshNum);
		TriggerDynamicLight(pos.x, pos.y, pos.z, 2 * creature->FiredWeapon + 10, 192, 128, 32);
		
		creature->FiredWeapon--;
	}

	if (item->TriggerFlags)
	{
		if (CurrentLevel == 2)
		{
			item->ItemFlags[3] = 1;
			item->Animation.IsAirborne = false;
			item->HitStatus = false;
			item->Collidable = false;
			item->Status = ITEM_DEACTIVATED;
		}
		else
		{
			item->Animation.IsAirborne = false;
			item->HitStatus = false;
			item->Collidable = false;
			item->Status = ITEM_ACTIVE;
		}
		item->TriggerFlags = 0;
	}

	if (item->HitPoints > 0)
	{
		if (item->AIBits)
			GetAITarget(creature);
		else
			creature->Enemy = LaraItem;

		AI_INFO info;
		CreatureAIInfo(item, &info);

		if (info.ahead)
			joint2 = info.angle;

		// FIXME: this should make Larson running away, but it's broken
		/*if (creature->flags)
		{
			item->HitPoints = 60;
			item->IsAirborne = false;
			item->hitStatus = false;
			item->collidable = false;
			item->status = ITEM_DESACTIVATED;
			creature->flags = 0;
		}*/

		GetCreatureMood(item, &info, VIOLENT);
		CreatureMood(item, &info, VIOLENT);

		if (info.distance < SQUARE(2048) 
			&& LaraItem->Animation.Velocity > 20
			|| item->HitStatus
			|| TargetVisible(item, &info) != 0)
		{
			item->Status &= ~ITEM_ACTIVE;
			creature->Alerted = true;
		}

		angle = CreatureTurn(item, creature->MaxTurn);
		
		switch (item->Animation.ActiveState)
		{
		case STATE_TR5_LARSON_STOP:
			joint0 = info.angle / 2;
			joint2 = info.angle / 2;
			if (info.ahead)
				joint1 = info.xAngle;

			if (item->Animation.RequiredState)
			{
				item->Animation.TargetState = item->Animation.RequiredState;
			}
			else if (item->AIBits & AMBUSH)
			{
				item->Animation.TargetState = STATE_TR5_LARSON_RUN;
			}
			else if (Targetable(item, &info))
			{
				item->Animation.TargetState = STATE_TR5_LARSON_AIM;
			}
			else
			{
				if (item->AIBits & GUARD || CurrentLevel == 2 || item->ItemFlags[3])
				{
					creature->MaxTurn = 0;
					item->Animation.TargetState = STATE_TR5_LARSON_STOP;
					if (abs(info.angle) >= ANGLE(2))
					{
						if (info.angle > 0)
							item->Pose.Orientation.y += ANGLE(2);
						else
							item->Pose.Orientation.y -= ANGLE(2);
					}
					else
					{
						item->Pose.Orientation.y += info.angle;
					}
				}
				else
				{
					if (creature->Mood != MoodType::Bored)
					{
						if (creature->Mood == MoodType::Escape)
							item->Animation.TargetState = STATE_TR5_LARSON_RUN;
						else
							item->Animation.TargetState = STATE_TR5_LARSON_WALK;
					}
					else
					{
						item->Animation.TargetState = GetRandomControl() >= 96 ? 2 : 6;
					}
				}
			}
			break;

		case STATE_TR5_LARSON_WALK:
			if (info.ahead)
				joint2 = info.angle;
			
			creature->MaxTurn = ANGLE(7);
			if (creature->Mood == MoodType::Bored && GetRandomControl() < 96)
			{
				item->Animation.RequiredState = STATE_TR5_LARSON_IDLE;
				item->Animation.TargetState = STATE_TR5_LARSON_STOP;
				break;
			}

			if (creature->Mood == MoodType::Escape || item->AIBits & AMBUSH)
			{
				item->Animation.RequiredState = STATE_TR5_LARSON_RUN;
				item->Animation.TargetState = STATE_TR5_LARSON_STOP;
			}
			else if (Targetable(item, &info))
			{
				item->Animation.RequiredState = STATE_TR5_LARSON_AIM;
				item->Animation.TargetState = STATE_TR5_LARSON_STOP;
			}
			else if (!info.ahead || info.distance > SQUARE(3072))
			{
				item->Animation.RequiredState = STATE_TR5_LARSON_RUN;
				item->Animation.TargetState = STATE_TR5_LARSON_STOP;
			}
			break;

		case STATE_TR5_LARSON_RUN:
			if (info.ahead)
				joint2 = info.angle;
			creature->MaxTurn = ANGLE(11);
			tilt = angle / 2;

			if (creature->ReachedGoal)
			{
				item->Animation.TargetState = STATE_TR5_LARSON_STOP;
			}
			else if (item->AIBits & AMBUSH)
			{
				item->Animation.TargetState = STATE_TR5_LARSON_RUN;
			}
			else if (creature->Mood != MoodType::Bored || GetRandomControl() >= 96)
			{
				if (Targetable(item, &info))
				{
					item->Animation.RequiredState = STATE_TR5_LARSON_AIM;
					item->Animation.TargetState = STATE_TR5_LARSON_STOP;
				}
				else if (info.ahead)
				{
					if (info.distance <= SQUARE(3072))
					{
						item->Animation.RequiredState = STATE_TR5_LARSON_WALK;
						item->Animation.TargetState = STATE_TR5_LARSON_STOP;
					}
				}
			}
			else
			{
				item->Animation.RequiredState = STATE_TR5_LARSON_IDLE;
				item->Animation.TargetState = STATE_TR5_LARSON_STOP;
			}
			break;

		case STATE_TR5_LARSON_AIM:
			joint0 = info.angle / 2;
			joint2 = info.angle / 2;
			if (info.ahead)
				joint1 = info.xAngle;
			creature->MaxTurn = 0;
			if (abs(info.angle) >= ANGLE(2))
			{
				if (info.angle > 0)
					item->Pose.Orientation.y += ANGLE(2);
				else
					item->Pose.Orientation.y -= ANGLE(2);
			}
			else
			{
				item->Pose.Orientation.y += info.angle;
			}

			if (Targetable(item, &info))
				item->Animation.TargetState = STATE_TR5_LARSON_ATTACK;
			else
				item->Animation.TargetState = STATE_TR5_LARSON_STOP;
			break;

		case STATE_TR5_LARSON_IDLE:
			joint0 = info.angle / 2;
			joint2 = info.angle / 2;
			if (info.ahead)
				joint1 = info.xAngle;

			if (creature->Mood != MoodType::Bored)
			{
				item->Animation.TargetState = STATE_TR5_LARSON_STOP;
			}
			else
			{
				if (GetRandomControl() <= 96)
				{
					item->Animation.RequiredState = STATE_TR5_LARSON_WALK;
					item->Animation.TargetState = STATE_TR5_LARSON_STOP;
				}
			}
			break;

		case STATE_TR5_LARSON_ATTACK:
			joint0 = info.angle / 2;
			joint2 = info.angle / 2;
			if (info.ahead)
				joint1 = info.xAngle;
			creature->MaxTurn = 0;
			if (abs(info.angle) >= ANGLE(2))
			{
				if (info.angle > 0)
					item->Pose.Orientation.y += ANGLE(2);
				else
					item->Pose.Orientation.y -= ANGLE(2);
			}
			else
			{
				item->Pose.Orientation.y += info.angle;
			}
			if (item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameBase)
			{
				if (item->ObjectNumber == ID_PIERRE)
				{
					ShotLara(item, &info, &PierreGun1, joint0, 20);
					ShotLara(item, &info, &PierreGun2, joint0, 20);
				}
				else
				{
					ShotLara(item, &info, &LarsonGun, joint0, 20);
				}
				creature->FiredWeapon = 2;
			}
			if (creature->Mood == MoodType::Escape && GetRandomControl() > 0x2000)
				item->Animation.RequiredState = STATE_TR5_LARSON_STOP;
			break;

		default:
			break;

		}
	}
	else if (item->Animation.ActiveState == STATE_TR5_LARSON_DIE)
	{
		// When Larson dies, it activates trigger at start position
		if (item->ObjectNumber == ID_LARSON 
			&& item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameEnd)
		{
			short roomNumber = item->ItemFlags[2] & 0xFF;
			short floorHeight = item->ItemFlags[2] & 0xFF00;
			ROOM_INFO* r = &g_Level.Rooms[roomNumber];
			
			int x = r->x + (creature->Tosspad / 256 & 0xFF) * SECTOR(1) + 512;
			int y = r->minfloor + floorHeight;
			int z = r->z + (creature->Tosspad & 0xFF) * SECTOR(1) + 512;

			TestTriggers(x, y, z, roomNumber, true);

			joint0 = 0;
		}
	}
	else
	{
		// Die
		if (item->ObjectNumber == ID_PIERRE)
			item->Animation.AnimNumber = Objects[ID_PIERRE].animIndex + ANIMATION_TR5_PIERRE_DIE;
		else
			item->Animation.AnimNumber = Objects[ID_LARSON].animIndex + ANIMATION_TR5_LARSON_DIE;
		item->Animation.ActiveState = STATE_TR5_LARSON_DIE;
		item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
	}

	CreatureTilt(item, tilt);
	CreatureJoint(item, 0, joint0);
	CreatureJoint(item, 1, joint1);
	CreatureJoint(item, 2, joint2);
	CreatureAnimation(itemNumber, angle, 0);

	/*if (creature->reachedGoal)
	{
		if (CurrentLevel == 2)
		{
			item->TargetState = STATE_TR5_LARSON_STOP;
			item->RequiredState = STATE_TR5_LARSON_STOP;
			creature->reachedGoal = false;
			item->IsAirborne = false;
			item->hitStatus = false;
			item->collidable = false;
			item->status = ITEM_NOT_ACTIVE;
			item->triggerFlags = 0;
		}
		else
		{
			item->HitPoints = NOT_TARGETABLE;
			DisableEntityAI(itemNumber);
			KillItem(itemNumber);
		}
	}*/
}
