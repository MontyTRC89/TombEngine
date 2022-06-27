#include "framework.h"
#include "Objects/TR3/Entity/tr3_mp_gun.h"

#include "Game/animation.h"
#include "Game/collision/collide_room.h"
#include "Game/control/box.h"
#include "Game/control/lot.h"
#include "Game/collision/sphere.h"
#include "Game/effects/effects.h"
#include "Game/itemdata/creature_info.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/misc.h"
#include "Game/people.h"
#include "Sound/sound.h"
#include "Specific/level.h"
#include "Specific/setup.h"

namespace TEN::Entities::TR3
{
	BITE_INFO MPGunBite = { 0, 160, 40, 13 };

	enum MPGunState
	{
		MPGUN_STATE_NONE = 0,
		MPGUN_STATE_WAIT = 1,
		MPGUN_STATE_WALK = 2,
		MPGUN_STATE_RUN = 3,
		MPGUN_STATE_AIM_1 = 4,
		MPGUN_STATE_SHOOT_1 = 5,
		MPGUN_STATE_AIM_2 = 6,
		MPGUN_STATE_SHOOT_2 = 7,
		MPGUN_STATE_SHOOT_3A = 8,
		MPGUN_STATE_SHOOT_3B = 9,
		MPGUN_STATE_SHOOT_4A = 10,
		MPGUN_STATE_AIM_3 = 11,
		MPGUN_STATE_AIM_4 = 12,
		MPGUN_STATE_DEATH = 13,
		MPGUN_STATE_SHOOT_4B = 14,
		MPGUN_STATE_CROUCH = 15,
		MPGUN_STATE_CROUCHED = 16,
		MPGUN_STATE_CROUCH_AIM = 17,
		MPGUN_STATE_CROUCH_SHOT = 18,
		MPGUN_STATE_CROUCH_WALK = 19,
		MPGUN_STATE_STAND = 20
	};

	// TODO
	enum MPGunAnim
	{

	};

	void MPGunControl(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;

		auto* item = &g_Level.Items[itemNumber];
		auto* creature = GetCreatureInfo(item);

		short head = 0;
		short angle = 0;
		short tilt = 0;
		short torsoX = 0;
		short torsoY = 0;

		if (creature->FiredWeapon)
		{
			Vector3Int pos = { MPGunBite.x, MPGunBite.y, MPGunBite.z };
			GetJointAbsPosition(item, &pos, MPGunBite.meshNum);

			TriggerDynamicLight(pos.x, pos.y, pos.z, (creature->FiredWeapon * 2) + 4, 24, 16, 4);
			creature->FiredWeapon--;
		}

		if (item->BoxNumber != NO_BOX && (g_Level.Boxes[item->BoxNumber].flags & BLOCKED))
		{
			DoLotsOfBlood(item->Pose.Position.x, item->Pose.Position.y - (GetRandomControl() & 255) - 32, item->Pose.Position.z, (GetRandomControl() & 127) + 128, GetRandomControl() * 2, item->RoomNumber, 3);
			DoDamage(item, 20);
		}

		AI_INFO AI;
		AI_INFO laraAI;
		if (item->HitPoints <= 0)
		{
			item->HitPoints = 0;
			if (item->Animation.ActiveState != 13)
			{
				item->Animation.AnimNumber = Objects[ID_MP_WITH_GUN].animIndex + 14;
				item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
				item->Animation.ActiveState = 13;
			}
			else if (!(GetRandomControl() & 3) && item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameBase + 1)
			{
				CreatureAIInfo(item, &AI);

				if (Targetable(item, &AI))
				{
					if (AI.angle > -ANGLE(45.0f) &&
						AI.angle < ANGLE(45.0f))
					{
						head = AI.angle;
						torsoY = AI.angle;
						ShotLara(item, &AI, &MPGunBite, torsoY, 32);
						SoundEffect(SFX_TR3_OIL_SMG_FIRE, &item->Pose, SoundEnvironment::Land, 1.0f, 0.7f);
						creature->FiredWeapon = 1;
					}
				}
			}
		}
		else
		{
			if (item->AIBits)
				GetAITarget(creature);
			else
			{
				creature->Enemy = LaraItem;

				int dx = LaraItem->Pose.Position.x - item->Pose.Position.x;
				int dz = LaraItem->Pose.Position.z - item->Pose.Position.z;

				laraAI.distance = pow(dx, 2) + pow(dx, 2);

				for (int slot = 0; slot < ActiveCreatures.size(); slot++)
				{
					auto* currentCreature = ActiveCreatures[slot];
					if (currentCreature->ItemNumber == NO_ITEM || currentCreature->ItemNumber == itemNumber)
						continue;

					auto* target = &g_Level.Items[currentCreature->ItemNumber];
					if (target->ObjectNumber != ID_LARA)
						continue;

					dx = target->Pose.Position.x - item->Pose.Position.x;
					dz = target->Pose.Position.z - item->Pose.Position.z;

					int distance = pow(dx, 2) + pow(dz, 2);
					if (distance < laraAI.distance)
						creature->Enemy = target;
				}
			}

			CreatureAIInfo(item, &AI);

			if (creature->Enemy == LaraItem)
			{
				laraAI.angle = AI.angle;
				laraAI.distance = AI.distance;
			}
			else
			{
				int dx = LaraItem->Pose.Position.x - item->Pose.Position.x;
				int dz = LaraItem->Pose.Position.z - item->Pose.Position.z;
				laraAI.angle = phd_atan(dz, dx) - item->Pose.Orientation.y;
				laraAI.distance = pow(dx, 2) + pow(dz, 2);
			}

			GetCreatureMood(item, &AI, creature->Enemy != LaraItem ? VIOLENT : TIMID);
			CreatureMood(item, &AI, creature->Enemy != LaraItem ? VIOLENT : TIMID);

			angle = CreatureTurn(item, creature->MaxTurn);

			int x = item->Pose.Position.x + SECTOR(1) * phd_sin(item->Pose.Orientation.y + laraAI.angle);
			int y = item->Pose.Position.y;
			int z = item->Pose.Position.z + SECTOR(1) * phd_cos(item->Pose.Orientation.y + laraAI.angle);

			int height = GetCollision(x, y, z, item->RoomNumber).Position.Floor;
			bool cover = (item->Pose.Position.y > (height + CLICK(3)) && item->Pose.Position.y < (height + CLICK(4.5f)) && laraAI.distance > pow(SECTOR(1), 2));

			auto* enemy = creature->Enemy;
			creature->Enemy = LaraItem;

			if (laraAI.distance < pow(SECTOR(1), 2) || item->HitStatus || TargetVisible(item, &laraAI))
			{
				if (!creature->Alerted)
					SoundEffect(SFX_TR3_AMERCAN_HOY, &item->Pose);

				AlertAllGuards(itemNumber);
			}

			creature->Enemy = enemy;

			switch (item->Animation.ActiveState)
			{
			case MPGUN_STATE_WAIT:
				creature->MaxTurn = 0;
				head = laraAI.angle;

				if (item->Animation.AnimNumber == Objects[item->ObjectNumber].animIndex + 17 ||
					item->Animation.AnimNumber == Objects[item->ObjectNumber].animIndex + 27 ||
					item->Animation.AnimNumber == Objects[item->ObjectNumber].animIndex + 28)
				{
					if (abs(AI.angle) < ANGLE(10.0f))
						item->Pose.Orientation.y += AI.angle;
					else if (AI.angle < 0)
						item->Pose.Orientation.y -= ANGLE(10.0f);
					else
						item->Pose.Orientation.y += ANGLE(10.0f);
				}

				if (item->AIBits & GUARD)
				{
					head = AIGuard(creature);
					item->Animation.TargetState = MPGUN_STATE_WAIT;
					break;
				}

				else if (item->AIBits & PATROL1)
				{
					item->Animation.TargetState = MPGUN_STATE_WALK;
					head = 0;
				}

				else if (cover && (Lara.TargetEntity == item || item->HitStatus))
					item->Animation.TargetState = MPGUN_STATE_CROUCH;
				else if (item->Animation.RequiredState == MPGUN_STATE_CROUCH)
					item->Animation.TargetState = MPGUN_STATE_CROUCH;
				else if (creature->Mood == MoodType::Escape)
					item->Animation.TargetState = MPGUN_STATE_RUN;
				else if (Targetable(item, &AI))
				{
					int random = GetRandomControl();
					if (random < 0x2000)
						item->Animation.TargetState = MPGUN_STATE_SHOOT_1;
					else if (random < 0x4000)
						item->Animation.TargetState = MPGUN_STATE_SHOOT_2;
					else
						item->Animation.TargetState = MPGUN_STATE_AIM_3;
				}
				else if (creature->Mood == MoodType::Bored ||
					(item->AIBits & FOLLOW && (creature->ReachedGoal || laraAI.distance > pow(SECTOR(2), 2))))
				{
					if (AI.ahead)
						item->Animation.TargetState = MPGUN_STATE_WAIT;
					else
						item->Animation.TargetState = MPGUN_STATE_WALK;
				}
				else
					item->Animation.TargetState = MPGUN_STATE_RUN;

				break;

			case MPGUN_STATE_WALK:
				creature->MaxTurn = ANGLE(6.0f);
				head = laraAI.angle;

				if (item->AIBits & PATROL1)
				{
					item->Animation.TargetState = MPGUN_STATE_WALK;
					head = 0;
				}
				else if (cover && (Lara.TargetEntity == item || item->HitStatus))
				{
					item->Animation.RequiredState = MPGUN_STATE_CROUCH;
					item->Animation.TargetState = MPGUN_STATE_WAIT;
				}
				else if (creature->Mood == MoodType::Escape)
					item->Animation.TargetState = MPGUN_STATE_RUN;
				else if (Targetable(item, &AI))
				{
					if (AI.distance > pow(CLICK(1.5f), 2) && AI.zoneNumber == AI.enemyZone)
						item->Animation.TargetState = MPGUN_STATE_AIM_4;
					else
						item->Animation.TargetState = MPGUN_STATE_WAIT;
				}
				else if (creature->Mood == MoodType::Bored)
				{
					if (AI.ahead)
						item->Animation.TargetState = MPGUN_STATE_WALK;
					else
						item->Animation.TargetState = MPGUN_STATE_WAIT;
				}
				else
					item->Animation.TargetState = MPGUN_STATE_RUN;

				break;

			case MPGUN_STATE_RUN:
				creature->MaxTurn = ANGLE(10.0f);
				tilt = angle / 2;

				if (AI.ahead)
					head = AI.angle;

				if (item->AIBits & GUARD)
					item->Animation.TargetState = MPGUN_STATE_WAIT;
				else if (cover && (Lara.TargetEntity == item || item->HitStatus))
				{
					item->Animation.RequiredState = MPGUN_STATE_CROUCH;
					item->Animation.TargetState = MPGUN_STATE_WAIT;
				}
				else if (creature->Mood == MoodType::Escape)
					break;
				else if (Targetable(item, &AI) ||
					(item->AIBits & FOLLOW && (creature->ReachedGoal || laraAI.distance > pow(SECTOR(2), 2))))
				{
					item->Animation.TargetState = MPGUN_STATE_WAIT;
				}
				else if (creature->Mood == MoodType::Bored)
					item->Animation.TargetState = MPGUN_STATE_WALK;

				break;

			case MPGUN_STATE_AIM_1:
				if (AI.ahead)
				{
					torsoX = AI.xAngle;
					torsoY = AI.angle;
				}

				if (item->Animation.AnimNumber == Objects[ID_MP_WITH_GUN].animIndex + 12 ||
					(item->Animation.AnimNumber == Objects[ID_MP_WITH_GUN].animIndex + 1 && item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameBase + 10))
				{
					if (!ShotLara(item, &AI, &MPGunBite, torsoY, 32))
						item->Animation.RequiredState = MPGUN_STATE_WAIT;
				}
				else if (item->HitStatus && !(GetRandomControl() & 0x3) && cover)
				{
					item->Animation.RequiredState = MPGUN_STATE_CROUCH;
					item->Animation.TargetState = MPGUN_STATE_WAIT;
				}

				break;

			case MPGUN_STATE_SHOOT_1:
				if (AI.ahead)
				{
					torsoX = AI.xAngle;
					torsoY = AI.angle;
				}

				if (item->Animation.RequiredState == MPGUN_STATE_WAIT)
					item->Animation.TargetState = MPGUN_STATE_WAIT;

				break;

			case MPGUN_STATE_SHOOT_2:
				if (AI.ahead)
				{
					torsoX = AI.xAngle;
					torsoY = AI.angle;
				}

				if (item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameBase)
				{
					if (!ShotLara(item, &AI, &MPGunBite, torsoY, 32))
						item->Animation.TargetState = MPGUN_STATE_WAIT;
				}
				else if (item->HitStatus && !(GetRandomControl() & 0x3) && cover)
				{
					item->Animation.RequiredState = MPGUN_STATE_CROUCH;
					item->Animation.TargetState = MPGUN_STATE_WAIT;
				}

				break;

			case MPGUN_STATE_SHOOT_3A:
			case MPGUN_STATE_SHOOT_3B:
				if (AI.ahead)
				{
					torsoX = AI.xAngle;
					torsoY = AI.angle;
				}

				if (item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameBase ||
					item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameBase + 11)
				{
					if (!ShotLara(item, &AI, &MPGunBite, torsoY, 32))
						item->Animation.TargetState = MPGUN_STATE_WAIT;
				}
				else if (item->HitStatus && !(GetRandomControl() & 0x3) && cover)
				{
					item->Animation.RequiredState = MPGUN_STATE_CROUCH;
					item->Animation.TargetState = MPGUN_STATE_WAIT;
				}

				break;

			case MPGUN_STATE_AIM_4:
				if (AI.ahead)
				{
					torsoX = AI.xAngle;
					torsoY = AI.angle;
				}

				if ((item->Animation.AnimNumber == Objects[ID_MP_WITH_GUN].animIndex + 18 && item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameBase + 17) ||
					(item->Animation.AnimNumber == Objects[ID_MP_WITH_GUN].animIndex + 19 && item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameBase + 6))
				{
					if (!ShotLara(item, &AI, &MPGunBite, torsoY, 32))
						item->Animation.RequiredState = MPGUN_STATE_WALK;
				}
				else if (item->HitStatus && !(GetRandomControl() & 0x3) && cover)
				{
					item->Animation.RequiredState = MPGUN_STATE_CROUCH;
					item->Animation.TargetState = MPGUN_STATE_WAIT;
				}

				if (AI.distance < pow(SECTOR(1.5f), 2))
					item->Animation.RequiredState = MPGUN_STATE_WALK;

				break;

			case MPGUN_STATE_SHOOT_4A:
			case MPGUN_STATE_SHOOT_4B:
				if (AI.ahead)
				{
					torsoX = AI.xAngle;
					torsoY = AI.angle;
				}

				if (item->Animation.RequiredState == MPGUN_STATE_WALK)
					item->Animation.TargetState = MPGUN_STATE_WALK;

				if (item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameBase + 16)
				{
					if (!ShotLara(item, &AI, &MPGunBite, torsoY, 32))
						item->Animation.TargetState = MPGUN_STATE_WALK;
				}

				if (AI.distance < pow(SECTOR(1.5f), 2))
					item->Animation.TargetState = MPGUN_STATE_WALK;

				break;

			case MPGUN_STATE_CROUCHED:
				creature->MaxTurn = 0;

				if (AI.ahead)
					head = AI.angle;

				if (Targetable(item, &AI))
					item->Animation.TargetState = MPGUN_STATE_CROUCH_AIM;
				else if (item->HitStatus || !cover || (AI.ahead && !(GetRandomControl() & 0x1F)))
					item->Animation.TargetState = MPGUN_STATE_STAND;
				else
					item->Animation.TargetState = MPGUN_STATE_CROUCH_WALK;

				break;

			case MPGUN_STATE_CROUCH_AIM:
				creature->MaxTurn = ANGLE(1.0f);

				if (AI.ahead)
					torsoY = AI.angle;

				if (Targetable(item, &AI))
					item->Animation.TargetState = MPGUN_STATE_CROUCH_SHOT;
				else
					item->Animation.TargetState = MPGUN_STATE_CROUCHED;

				break;

			case MPGUN_STATE_CROUCH_SHOT:
				if (AI.ahead)
					torsoY = AI.angle;

				if (item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameBase)
				{
					if (!ShotLara(item, &AI, &MPGunBite, torsoY, 32) || !(GetRandomControl() & 0x7))
						item->Animation.TargetState = MPGUN_STATE_CROUCHED;
				}

				break;

			case MPGUN_STATE_CROUCH_WALK:
				creature->MaxTurn = ANGLE(6.0f);

				if (AI.ahead)
					head = AI.angle;

				if (Targetable(item, &AI) || item->HitStatus || !cover || (AI.ahead && !(GetRandomControl() & 0x1F)))
					item->Animation.TargetState = MPGUN_STATE_CROUCHED;

				break;
			}
		}

		CreatureTilt(item, tilt);
		CreatureJoint(item, 0, torsoY);
		CreatureJoint(item, 1, torsoX);
		CreatureJoint(item, 2, head);
		CreatureAnimation(itemNumber, angle, tilt);
	}
}
