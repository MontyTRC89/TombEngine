#include "framework.h"
#include "tr5_guard.h"
#include "Game/items.h"
#include "Game/collision/collide_room.h"
#include "Game/control/box.h"
#include "Game/people.h"
#include "Game/effects/effects.h"
#include "Game/effects/tomb4fx.h"
#include "Game/control/los.h"
#include "Specific/setup.h"
#include "Game/animation.h"
#include "Specific/level.h"
#include "Game/Lara/lara.h"
#include "Game/misc.h"
#include "Sound/sound.h"
#include "Game/itemdata/creature_info.h"

BITE_INFO SwatGunBite = { 80, 200, 13, 0 };
BITE_INFO SniperGunBite = { 0, 480, 110, 13 };
BITE_INFO ArmedMafia2GunBite = { -50, 220, 60, 13 };

enum GuardState
{
	GUARD_STATE_IDLE = 1,
	GUARD_STATE_TURN_180 = 2,
	GUARD_STATE_FIRE_SINGLE = 3,
	GUARD_STATE_AIM = 4,
	GUARD_STATE_WALK = 5,
	GUARD_STATE_DEATH_1 = 6,
	GUARD_STATE_RUN = 7,
	GUARD_STATE_DEATH_2 = 8,
	GUARD_STATE_RECOIL = 11,

	GUARD_STATE_CROUCH = 13,
	GUARD_STATE_ROPE_DOWN = 14,
	GUARD_STATE_SIT = 15,
	GUARD_STATE_STAND_UP = 16,

	GUARD_STATE_WAITING_ON_WALL = 19,

	GUARD_STATE_IDLE_START_JUMP = 26,
	GUARD_STATE_JUMP_1_BLOCK = 27,
	GUARD_STATE_JUMP_2_BLOCKS = 28,
	GUARD_STATE_LAND = 29,
	GUARD_STATE_HUNT = 30,
	GUARD_STATE_HUNT_IDLE = 31,

	GUARD_STATE_FIRE_FAST = 35,
	GUARD_STATE_INSERT_CODE = 36,
	GUARD_STATE_START_USE_COMPUTER = 37,
	GUARD_STATE_USE_COMPUTER = 38,
	GUARD_STATE_SURRENDER = 39,
};

enum Mafia2State
{
	MAFIA2_STATE_IDLE = 1,
	MAFIA2_STATE_TURN_180_UNDRAW_WEAPON = 2,
	MAFIA2_STATE_FIRE = 3,
	MAFIA2_STATE_AIM = 4,
	MAFIA2_STATE_WALK = 5,
	MAFIA2_STATE_DEATH_1 = 6,
	MAFIA2_STATE_RUN = 7,
	MAFIA2_STATE_DEATH_2 = 8,

	MAFIA2_STATE_IDLE_START_JUMP = 26,
	MAFIA2_STATE_JUMP_1_BLOCK = 27,
	MAFIA2_STATE_JUMP_2_BLOCKS = 28,
	MAFIA2_STATE_LAND = 29,

	MAFIA2_STATE_TURN_180 = 32,

	MAFIA2_STATE_UNDRAW_GUNS = 37
};

enum SniperState
{
	SNIPER_STATE_IDLE = 1,
	SNIPER_STATE_UNCOVER = 2,
	SNIPER_STATE_AIM = 3,
	SNIPER_STATE_FIRE = 4,
	SNIPER_STATE_COVER = 5,
	SNIPER_STATE_DEATH = 6
};

enum GuardAnim
{
	ANIMATION_GUARD_DEATH1 = 11,
	ANIMATION_GUARD_DEATH2 = 16,
	ANIMATION_GUARD_START_JUMP = 41
};

void InitialiseGuard(short itemNum)
{
	auto* item = &g_Level.Items[itemNum];
	ClearItem(itemNum);

   int anim = Objects[ID_SWAT].animIndex;
	if (!Objects[ID_SWAT].loaded)
		anim = Objects[ID_GUARD1].animIndex;

	short roomItemNumber;
	switch (item->TriggerFlags)
	{
		case 0:
		case 10:
			item->Animation.AnimNumber = anim;
			item->Animation.TargetState = GUARD_STATE_IDLE;
			break;

		case 1:
			item->Animation.AnimNumber = anim + 23;
			item->Animation.TargetState = GUARD_STATE_RECOIL;
			break;

		case 2:
			item->Animation.AnimNumber = anim + 25;
			item->Animation.TargetState = GUARD_STATE_CROUCH;
			// TODO: item->flags2 ^= (item->flags2 ^ ((item->flags2 & 0xFE) + 2)) & 6;
			break;

		case 3:
			item->Animation.AnimNumber = anim + 28;
			item->Animation.TargetState = GUARD_STATE_SIT;
			item->MeshSwapBits = 9216;

			roomItemNumber = g_Level.Rooms[item->RoomNumber].itemNumber;
			if (roomItemNumber != NO_ITEM)
			{
				ItemInfo* item2 = nullptr;
				while (true)
				{
					item2 = &g_Level.Items[roomItemNumber];
					if (item2->ObjectNumber >= ID_ANIMATING1 &&
						item2->ObjectNumber <= ID_ANIMATING15 &&
						item2->RoomNumber == item->RoomNumber &&
						item2->TriggerFlags == 3)
					{
						break;
					}

					roomItemNumber = item2->NextItem;
					if (roomItemNumber == NO_ITEM)
					{
						item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
						item->Animation.ActiveState = item->Animation.TargetState;
						break;
					}
				}

				item2->MeshBits = -5;
			}

			break;

		case 4:
			item->Animation.AnimNumber = anim + 30;
			item->Animation.TargetState = 17;
			item->MeshSwapBits = 8192;
			break;

		case 5:
			item->Animation.AnimNumber = anim + 26;
			item->Animation.TargetState = GUARD_STATE_ROPE_DOWN;
			item->Pose.Position.y = GetCollision(item).Position.Ceiling - SECTOR(2);
			break;

		case 6:
			item->Animation.AnimNumber = anim + 32;
			item->Animation.TargetState = GUARD_STATE_WAITING_ON_WALL;
			break;

		case 7:
		case 9:
			item->Animation.AnimNumber = anim + 59;
			item->Animation.TargetState = GUARD_STATE_USE_COMPUTER;
			item->Pose.Position.x -= CLICK(2) * phd_sin(item->Pose.Orientation.y);
			item->Pose.Position.z -= CLICK(2) * phd_cos(item->Pose.Orientation.y);
			break;

		case 8:
			item->Animation.TargetState = GUARD_STATE_HUNT_IDLE;
			item->Animation.AnimNumber = anim + 46;
			break;

		case 11:
			item->Animation.TargetState = GUARD_STATE_RUN;
			item->Animation.AnimNumber = anim + 12;
			break;

		default:
			break;
	}
}

void InitialiseSniper(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	ClearItem(itemNumber);

	item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex;
	item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
	item->Animation.TargetState = SNIPER_STATE_IDLE;
	item->Animation.ActiveState = SNIPER_STATE_IDLE;

	item->Pose.Position.x += SECTOR(1) * phd_sin(item->Pose.Orientation.y + ANGLE(90.0f));
	item->Pose.Position.y += CLICK(2);
	item->Pose.Position.z += SECTOR(1) * phd_cos(item->Pose.Orientation.y + ANGLE(90.0f));
}

void InitialiseGuardLaser(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	ClearItem(itemNumber);

	item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 6;
	item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
	item->Animation.TargetState = GUARD_STATE_IDLE;
	item->Animation.ActiveState = GUARD_STATE_IDLE;
}

void ControlGuardLaser(short itemNumber)
{

}

void GuardControl(short itemNumber)
{
	if (!CreatureActive(itemNumber))
		return;

	int animIndex = 0;
	if (Objects[ID_SWAT].loaded)
		animIndex= Objects[ID_SWAT].animIndex;
	else
		animIndex = Objects[ID_GUARD1].animIndex;

	auto* item = &g_Level.Items[itemNumber];
	auto* creature = GetCreatureInfo(item);

	short angle = 0;
	short joint0 = 0;
	short joint1 = 0;
	short joint2 = 0;

	int x = item->Pose.Position.x;
	int z = item->Pose.Position.z;
	int dx = 870 * phd_sin(item->Pose.Orientation.y);
	int dz = 870 * phd_cos(item->Pose.Orientation.y);

	x += dx;
	z += dz;
	int height1 = GetCollision(x, item->Pose.Position.y, z, item->RoomNumber).Position.Floor;

	x += dx;
	z += dz;
	int height2 = GetCollision(x, item->Pose.Position.y, z, item->RoomNumber).Position.Floor;

	x += dx;
	z += dz;
	int height3 = GetCollision(x, item->Pose.Position.y, z, item->RoomNumber).Position.Floor;

	bool canJump1block = true;
	if (item->BoxNumber == LaraItem->BoxNumber ||
		item->Pose.Position.y >= (height1 - CLICK(1.5f)) ||
		item->Pose.Position.y >= (height2 + CLICK(1)) ||
		item->Pose.Position.y <= (height2 - CLICK(1)))
	{
		canJump1block = false;
	}

	bool canJump2blocks = true;
	if (item->BoxNumber == LaraItem->BoxNumber ||
		item->Pose.Position.y >= (height1 - CLICK(1.5f)) ||
		item->Pose.Position.y >= (height2 - CLICK(1.5f)) ||
		item->Pose.Position.y >= (height3 + CLICK(1)) ||
		item->Pose.Position.y <= (height3 - CLICK(1)))
	{
		canJump2blocks = false;
	}

	if (creature->FiredWeapon)
	{
		creature->FiredWeapon--;

		auto pos = Vector3Int(SwatGunBite.x, SwatGunBite.y, SwatGunBite.z);
		GetJointAbsPosition(item, &pos, SwatGunBite.meshNum);

		TriggerDynamicLight(pos.x, pos.y, pos.z, 2 * creature->FiredWeapon + 10, 192, 128, 32);
	}

	if (item->AIBits)
		GetAITarget(creature);
	else
		creature->Enemy = LaraItem;

	AI_INFO AI;
	CreatureAIInfo(item, &AI);

	AI_INFO laraAI;
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
	
	if (item->HitPoints <= 0)
	{
		if (item->Animation.ActiveState != GUARD_STATE_DEATH_1 &&
			item->Animation.ActiveState != GUARD_STATE_DEATH_2)
		{
			if (laraAI.angle >= ANGLE(67.5f) || laraAI.angle <= -ANGLE(67.5f))
			{
				item->Animation.AnimNumber = animIndex + ANIMATION_GUARD_DEATH2;
				item->Animation.ActiveState = GUARD_STATE_DEATH_2;
				item->Pose.Orientation.y += laraAI.angle + -ANGLE(180.0f);
			}
			else
			{
				item->Animation.AnimNumber = animIndex + ANIMATION_GUARD_DEATH1;
				item->Animation.ActiveState = GUARD_STATE_DEATH_1;
				item->Pose.Orientation.y += laraAI.angle;
			}

			item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
		}
	}
	else
	{
		GetCreatureMood(item, &AI, creature->Enemy != LaraItem);
		if (item->ObjectNumber == ID_SCIENTIST)
		{
			if (item->HitPoints >= Objects[ID_SCIENTIST].HitPoints)
			{
				if (creature->Enemy == LaraItem)
					creature->Mood = MoodType::Bored;
			}
			else
				creature->Mood = MoodType::Escape;
		}
		if (TestEnvironment(ENV_FLAG_NO_LENSFLARE, item->RoomNumber)) // TODO: CHECK
		{
			if (item->ObjectNumber == ID_SWAT_PLUS)
			{
				item->ItemFlags[0]++;
				if (item->ItemFlags[0] > 60 && !(GetRandomControl() & 0xF))
				{
					SoundEffect(SFX_TR5_BIO_BREATHE_OUT, &item->Pose);
					item->ItemFlags[0] = 0;
				}
			}
			else
			{
				creature->Mood = MoodType::Escape;

				if (!(GlobalCounter & 7))
					item->HitPoints--;

				if (item->HitPoints <= 0)
				{
					item->Animation.AnimNumber = animIndex + ANIMATION_GUARD_DEATH2;
					item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
					item->Animation.ActiveState = GUARD_STATE_DEATH_2;
				}
			}
		}

		CreatureMood(item, &AI, creature->Enemy != LaraItem);
		auto* enemy = creature->Enemy;

		angle = CreatureTurn(item, creature->MaxTurn);
		creature->Enemy = LaraItem;

		if ((laraAI.distance < pow(SECTOR(2), 2) && LaraItem->Animation.Velocity > 20) ||
			item->HitStatus ||
			TargetVisible(item, &laraAI))
		{
			if (!(item->AIBits & FOLLOW) &&
				item->ObjectNumber != ID_SCIENTIST &&
				abs(item->Pose.Position.y - LaraItem->Pose.Position.y) < SECTOR(1.25f))
			{
				creature->Enemy = LaraItem;
				AlertAllGuards(itemNumber);
			}
		}

		creature->Enemy = enemy;

		GameVector src;
		src.x = item->Pose.Position.x;
		src.y = item->Pose.Position.y - CLICK(1.5f);
		src.z = item->Pose.Position.z;
		src.roomNumber = item->RoomNumber;

		auto* frame = GetBestFrame(LaraItem);

		GameVector dest;
		dest.x = LaraItem->Pose.Position.x;
		dest.y = LaraItem->Pose.Position.y + ((frame->boundingBox.Y2 + 3 * frame->boundingBox.Y1) / 4);
		dest.z = LaraItem->Pose.Position.z;

		bool los = !LOS(&src, &dest) && item->TriggerFlags != 10;

		creature->MaxTurn = 0;

		ItemInfo* currentItem = nullptr;
		short currentItemNumber;

		switch (item->Animation.ActiveState)
		{
		case GUARD_STATE_IDLE:
			creature->LOT.IsJumping = false;
			creature->Flags = 0;
			joint2 = laraAI.angle;

			if (AI.ahead)
			{
				if (!(item->AIBits & FOLLOW))
				{
					joint0 = AI.angle / 2;
					joint1 = AI.xAngle;
				}
			}

			if (item->ObjectNumber == ID_SCIENTIST && item == Lara.TargetEntity)
				item->Animation.TargetState = GUARD_STATE_SURRENDER;
			else if (item->Animation.RequiredState)
				item->Animation.TargetState = item->Animation.RequiredState;
			else if (item->AIBits & GUARD)
			{
				if (item->AIBits & MODIFY)
					joint2 = 0;
				else
					joint2 = AIGuard(creature);
			
				if (item->AIBits & PATROL1)
				{
					item->TriggerFlags--;
					if (item->TriggerFlags < 1)
						item->AIBits &= ~GUARD;
				}
			}
			else if (creature->Enemy == LaraItem &&
				(laraAI.angle > ANGLE(112.5f) || laraAI.angle < -ANGLE(112.5f)) &&
				item->ObjectNumber != ID_SCIENTIST)
			{
				item->Animation.TargetState = GUARD_STATE_TURN_180;
			}
			else if (item->AIBits & PATROL1)
				item->Animation.TargetState = GUARD_STATE_WALK;
			else if (item->AIBits & AMBUSH)
				item->Animation.TargetState = GUARD_STATE_RUN;
			else if (Targetable(item, &AI) && item->ObjectNumber != ID_SCIENTIST)
			{
				if (AI.distance >= pow(SECTOR(4), 2) && AI.zoneNumber == AI.enemyZone)
				{
					if (!(item->AIBits & MODIFY))
						item->Animation.TargetState = GUARD_STATE_WALK;
				}
				else
					item->Animation.TargetState = GUARD_STATE_AIM;
			}
			else if (canJump1block || canJump2blocks)
			{
				item->Animation.AnimNumber = animIndex + 41;
				item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
				item->Animation.ActiveState = GUARD_STATE_IDLE_START_JUMP;
				creature->MaxTurn = 0;

				if (canJump1block)
					item->Animation.TargetState = GUARD_STATE_JUMP_1_BLOCK;
				else
					item->Animation.TargetState = GUARD_STATE_JUMP_2_BLOCKS;

				creature->LOT.IsJumping = true;
			}
			else if (los)
				item->Animation.TargetState = GUARD_STATE_HUNT_IDLE;
			else if (creature->Mood != MoodType::Bored)
			{
				if (AI.distance < pow(SECTOR(3), 2) || item->AIBits & FOLLOW)
					item->Animation.TargetState = GUARD_STATE_WALK;
				else
					item->Animation.TargetState = GUARD_STATE_RUN;
			}
			else
				item->Animation.TargetState = GUARD_STATE_IDLE;
			
			if (item->TriggerFlags == 11)
				item->TriggerFlags = 0;

			break;

		case GUARD_STATE_TURN_180:
			creature->Flags = 0;

			if (AI.angle >= 0)
				item->Pose.Orientation.y -= ANGLE(2.0f);
			else
				item->Pose.Orientation.y += ANGLE(2.0f);

			if (item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameEnd)
				item->Pose.Orientation.y += -ANGLE(180.0f);

			break;

		case GUARD_STATE_FIRE_SINGLE:
		case GUARD_STATE_FIRE_FAST:
			joint0 = laraAI.angle / 2;
			joint2 = laraAI.angle / 2;

			if (AI.ahead)
				joint1 = AI.xAngle;
			if (abs(AI.angle) >= ANGLE(2.0f))
			{
				if (AI.angle >= 0)
					item->Pose.Orientation.y += ANGLE(2.0f);
				else
					item->Pose.Orientation.y -= ANGLE(2.0f);
			}
			else
				item->Pose.Orientation.y += AI.angle;

			if (item->Animation.ActiveState == GUARD_STATE_FIRE_FAST)
			{
				if (creature->Flags)
				{
					if (item->Animation.FrameNumber < g_Level.Anims[item->Animation.AnimNumber].frameBase + 10 &&
						(item->Animation.FrameNumber - g_Level.Anims[item->Animation.AnimNumber].frameBase) & 1)
					{
						creature->Flags = 0;
					}
				}
			}

			if (!creature->Flags)
			{
				creature->FiredWeapon = 2;
				creature->Flags = 1;

				if (item->Animation.ActiveState == GUARD_STATE_FIRE_SINGLE)
					ShotLara(item, &AI, &SwatGunBite, joint0, 30);
				else
					ShotLara(item, &AI, &SwatGunBite, joint0, 10);
				
				// TODO: just for testing energy arcs
				/*pos1.x = SwatGunBite.x;
				pos1.y = SwatGunBite.y;
				pos1.z = SwatGunBite.z;
				GetJointAbsPosition(item, &pos1, SwatGunBite.meshNum);
				TriggerEnergyArc(&pos1, (Vector3Int*)& LaraItem->pos, 192, 128, 192, 256, 150, 256, 0, ENERGY_ARC_STRAIGHT_LINE);*/

			}

			break;

		case GUARD_STATE_AIM:
			creature->Flags = 0;
			joint0 = laraAI.angle / 2;
			joint2 = laraAI.angle / 2;

			if (AI.ahead)
				joint1 = AI.xAngle;

			if (abs(AI.angle) >= ANGLE(2.0f))
			{
				if (AI.angle >= 0)
					item->Pose.Orientation.y += ANGLE(2.0f);
				else
					item->Pose.Orientation.y -= ANGLE(2.0f);
			}
			else
				item->Pose.Orientation.y += AI.angle;

			if (!Targetable(item, &AI))
				item->Animation.TargetState = GUARD_STATE_IDLE;
			else if (item->ObjectNumber == ID_GUARD1 || item->ObjectNumber == ID_GUARD2)
				item->Animation.TargetState = GUARD_STATE_FIRE_SINGLE;
			else
				item->Animation.TargetState = GUARD_STATE_FIRE_FAST;

			break;

		case GUARD_STATE_WALK:
			creature->MaxTurn = ANGLE(5.0f);
			creature->LOT.IsJumping = false;

			if (!Targetable(item, &AI) ||
				AI.distance >= pow(SECTOR(4), 2) && AI.zoneNumber == AI.enemyZone ||
				item->ObjectNumber == ID_SCIENTIST ||
				item->AIBits & AMBUSH || item->AIBits & PATROL1) // TODO: CHECK
			{
				if (canJump1block || canJump2blocks)
				{
					item->Animation.AnimNumber = animIndex + 41;
					item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
					item->Animation.ActiveState = GUARD_STATE_IDLE_START_JUMP;
					creature->MaxTurn = 0;

					if (canJump1block)
						item->Animation.TargetState = GUARD_STATE_JUMP_1_BLOCK;
					else
						item->Animation.TargetState = GUARD_STATE_JUMP_2_BLOCKS;

					creature->LOT.IsJumping = true;
				}
				else if (AI.distance >= pow(SECTOR(1), 2))
				{
					if (!los || item->AIBits)
					{
						if (AI.distance > pow(SECTOR(3), 2))
						{
							if (!(item->InDrawRoom))
								item->Animation.TargetState = GUARD_STATE_RUN;
						}
					}
					else
						item->Animation.TargetState = GUARD_STATE_IDLE;
				}
				else
					item->Animation.TargetState = GUARD_STATE_IDLE;
			}
			else
				item->Animation.TargetState = GUARD_STATE_AIM;
			
			break;

		case GUARD_STATE_RUN:
			creature->MaxTurn = ANGLE(10.0f);
			creature->LOT.IsJumping = false;

			if (Targetable(item, &AI) &&
				(AI.distance < pow(SECTOR(4), 2) || AI.enemyZone == AI.zoneNumber) &&
				item->ObjectNumber != ID_SCIENTIST)
			{
				item->Animation.TargetState = GUARD_STATE_AIM;
			}
			else if (canJump1block || canJump2blocks)
			{
				item->Animation.AnimNumber = animIndex + 50;
				item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
				item->Animation.ActiveState = GUARD_STATE_IDLE_START_JUMP;
				creature->MaxTurn = 0;

				if (canJump1block)
					item->Animation.TargetState = GUARD_STATE_JUMP_1_BLOCK;
				else
					item->Animation.TargetState = GUARD_STATE_JUMP_2_BLOCKS;

				creature->LOT.IsJumping = true;
			}
			else if (los)
				item->Animation.TargetState = GUARD_STATE_IDLE;
			else if (AI.distance < pow(SECTOR(3), 2))
				item->Animation.TargetState = GUARD_STATE_WALK;
			if (item->TriggerFlags == 11)
			{
				creature->MaxTurn = 0;
				creature->LOT.IsJumping = true;
			}
			
			break;

		case GUARD_STATE_ROPE_DOWN:
			joint2 = laraAI.angle;

			if (item->Pose.Position.y <= (item->Floor - SECTOR(2)) ||
				item->TriggerFlags != 5)
			{
				if (item->Pose.Position.y >= (item->Floor - CLICK(2)))
					item->Animation.TargetState = GUARD_STATE_AIM;
			}
			else
			{
				item->TriggerFlags = 0;
				TestTriggers(item, true);
				SoundEffect(SFX_TR4_LARA_POLE_SLIDE_LOOP, &item->Pose);
			}
			if (abs(AI.angle) >= ANGLE(2.0f))
			{
				if ((AI.angle & 0x8000) == 0)
					item->Pose.Orientation.y += ANGLE(2.0f);
				else
					item->Pose.Orientation.y -= ANGLE(2.0f);
			}
			else
				item->Pose.Orientation.y += AI.angle;
			
			break;

		case GUARD_STATE_SIT:
			joint2 = AIGuard(creature);

			if (creature->Alerted)
				item->Animation.TargetState = GUARD_STATE_STAND_UP;

			break;

		case GUARD_STATE_STAND_UP:
		case 18:
			if (item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameBase)
			{
				TestTriggers(item, true);
				break;
			}

			if (item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameBase + 44)
			{
				item->MeshSwapBits = NO_JOINT_BITS;

				short currentItemNumber = g_Level.Rooms[item->RoomNumber].itemNumber;
				if (currentItemNumber == NO_ITEM)
					break;

				while (true)
				{
					currentItem = &g_Level.Items[currentItemNumber];

					if (currentItem->ObjectNumber >= ID_ANIMATING1 &&
						currentItem->ObjectNumber <= ID_ANIMATING15 &&
						currentItem->RoomNumber == item->RoomNumber)
					{
						if (currentItem->TriggerFlags > 2 && currentItem->TriggerFlags < 5)
							break;
					}

					currentItemNumber = currentItem->NextItem;
					if (currentItemNumber == -1)
						break;
				}

				if (currentItemNumber == NO_ITEM)
					break;

				currentItem->MeshBits = -3;
			}
			else if (item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameEnd)
				item->Pose.Orientation.y -= ANGLE(90.0f);
			
			break;

		case 17:
			joint2 = 0;

			if (!item->HitStatus &&
				LaraItem->Animation.Velocity < 40 &&
				!Lara.Control.Weapon.HasFired)
			{
				creature->Alerted = false;
			}

			if (creature->Alerted)
				item->Animation.TargetState = 18;

			break;

		case GUARD_STATE_WAITING_ON_WALL:
			joint2 = AIGuard(creature);

			if (creature->Alerted)
				item->Animation.TargetState = GUARD_STATE_IDLE;

			break;

		case GUARD_STATE_HUNT:
		case GUARD_STATE_HUNT_IDLE:
			creature->MaxTurn = ANGLE(5.0f);
			creature->LOT.IsJumping = false;

			if (item->Animation.ActiveState == GUARD_STATE_HUNT_IDLE)
			{
				if (item->TriggerFlags != 8 || !los || item->HitStatus)
					item->Animation.TargetState = GUARD_STATE_HUNT;
			}

			if (canJump1block || canJump2blocks || AI.distance < pow(SECTOR(1), 2) || !los || item->HitStatus)
				item->Animation.TargetState = GUARD_STATE_IDLE;

			break;

		case GUARD_STATE_INSERT_CODE:
			if (item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameBase + 39)
				TestTriggers(item, true);
			
			break;

		case GUARD_STATE_START_USE_COMPUTER:
			currentItem = nullptr;

			for (currentItemNumber = g_Level.Rooms[item->RoomNumber].itemNumber; currentItemNumber != NO_ITEM; currentItemNumber = currentItem->NextItem)
			{
				currentItem = &g_Level.Items[currentItemNumber];

				if (item->ObjectNumber == ID_PUZZLE_HOLE8)
					break;
			}

			if (item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameBase)
			{
				currentItem->MeshBits = 0x1FFF;
				item->Pose.Position.x = currentItem->Pose.Position.x - CLICK(1);
				item->Pose.Orientation.y = currentItem->Pose.Orientation.y;
				item->Pose.Position.z = currentItem->Pose.Position.z + CLICK(0.5f);
				item->MeshSwapBits = 1024;
			}
			else
			{
				if (item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameBase + 32)
					currentItem->MeshBits = 16381;
				else if (item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameBase + 74)
					currentItem->MeshBits = 278461;
				else if (item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameBase + 120)
					currentItem->MeshBits = 802621;
				else if (item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameBase + 157)
					currentItem->MeshBits = 819001;
				else if (item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameBase + 190)
					currentItem->MeshBits = 17592121;
				else if (item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameBase + g_Level.Anims[item->Animation.AnimNumber].frameEnd)
				{
					currentItem->MeshBits = 0x1FFF;
					TestTriggers(item, true);
					item->Animation.RequiredState = GUARD_STATE_WALK;
					item->MeshSwapBits = NO_JOINT_BITS;
				}
			}

			break;

		case GUARD_STATE_USE_COMPUTER:
			if ((item->ObjectNumber != ID_SCIENTIST || item != Lara.TargetEntity) &&
				(GetRandomControl() & 0x7F || item->TriggerFlags >= 10 || item->TriggerFlags == 9))
			{
				if (item->AIBits & GUARD)
				{
					joint2 = AIGuard(creature);

					if (item->AIBits & PATROL1)
					{
						item->TriggerFlags--;
						if (item->TriggerFlags < 1)
							item->AIBits = PATROL1 | MODIFY;
					}
				}
			}
			else
				item->Animation.TargetState = GUARD_STATE_IDLE;
			
			break;

		case GUARD_STATE_SURRENDER:
			if (item != Lara.TargetEntity && !(GetRandomControl() & 0x3F))
			{
				if (item->TriggerFlags == 7 || item->TriggerFlags == 9)
					item->Animation.RequiredState = GUARD_STATE_USE_COMPUTER;

				item->Animation.TargetState = GUARD_STATE_IDLE;
			}

			if (item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameBase + 39)
				TestTriggers(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, enemy->RoomNumber, true);

			break;

		default:
			break;
		}
	}

	CreatureJoint(item, 0, joint0);
	CreatureJoint(item, 1, joint1);
	CreatureJoint(item, 2, joint2);

	if (creature->ReachedGoal && creature->Enemy)
	{
		auto* enemy = creature->Enemy;
		
		if (enemy->Flags != 4)
		{
			if (enemy->Flags & 0x10)
			{
				item->Animation.TargetState = GUARD_STATE_IDLE;
				item->Animation.RequiredState = GUARD_STATE_USE_COMPUTER;
				item->TriggerFlags = 300;
				item->AIBits = GUARD | PATROL1;
			}
			else
			{
				if (enemy->Flags & 0x20)
				{
					item->Animation.TargetState = GUARD_STATE_IDLE;
					item->Animation.RequiredState = 36;
					item->AIBits = PATROL1 | MODIFY;
				}
				else
				{
					TestTriggers(creature->Enemy->Pose.Position.x, creature->Enemy->Pose.Position.y, creature->Enemy->Pose.Position.z, enemy->RoomNumber, true);
					item->Animation.RequiredState = GUARD_STATE_WALK;

					if (creature->Enemy->Flags & 2)
						item->ItemFlags[3] = (creature->Tosspad & 0xFF) - 1;

					if (creature->Enemy->Flags & 8)
					{
						item->Animation.RequiredState = GUARD_STATE_IDLE;
						item->TriggerFlags = 300;
						item->AIBits |= GUARD | PATROL1;
					}
				}
			}
		}
		else
		{
			item->Animation.TargetState = GUARD_STATE_IDLE;
			item->Animation.RequiredState = 37;
		}
	}
	if ((item->Animation.ActiveState >= 20 ||
		item->Animation.ActiveState == GUARD_STATE_DEATH_1 ||
		item->Animation.ActiveState == GUARD_STATE_DEATH_2) &&
		item->Animation.ActiveState != GUARD_STATE_HUNT)
	{
		CreatureAnimation(itemNumber, angle, 0);
	}
	else
	{
		switch (CreatureVault(itemNumber, angle, 2, 256) + 4)
		{
		case 0:
			item->Animation.AnimNumber = animIndex + 38;
			item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
			item->Animation.ActiveState = 23;
			creature->MaxTurn = 0;
			break;

		case 1:
			item->Animation.AnimNumber = animIndex + 39;
			item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
			item->Animation.ActiveState = 24;
			creature->MaxTurn = 0;
			break;

		case 2:
			item->Animation.AnimNumber = animIndex + 40;
			item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
			item->Animation.ActiveState = 25;
			creature->MaxTurn = 0;
			break;

		case 6:
			item->Animation.AnimNumber = animIndex + 35;
			item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
			item->Animation.ActiveState = 20;
			creature->MaxTurn = 0;
			break;

		case 7:
			item->Animation.AnimNumber = animIndex + 36;
			item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
			item->Animation.ActiveState = 21;
			creature->MaxTurn = 0;
			break;

		case 8:
			item->Animation.AnimNumber = animIndex + 37;
			item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
			item->Animation.ActiveState = 22;
			creature->MaxTurn = 0;
			break;
		}
	}
}

void SniperControl(short itemNumber)
{
	if (!CreatureActive(itemNumber))
		return;

	auto* item = &g_Level.Items[itemNumber];
	auto* creature = GetCreatureInfo(item);

	short angle = 0;
	short joint0 = 0;
	short joint1 = 0;
	short joint2 = 0;

	if (creature->FiredWeapon)
	{
		auto pos = Vector3Int(SniperGunBite.x, SniperGunBite.y, SniperGunBite.z);
		GetJointAbsPosition(item, &pos, SniperGunBite.meshNum);

		TriggerDynamicLight(pos.x, pos.y, pos.z, 2 * creature->FiredWeapon + 10, 192, 128, 32);
		creature->FiredWeapon--;
	}

	if (item->HitPoints > 0)
	{
		if (item->AIBits)
			GetAITarget(creature);
		else if (creature->HurtByLara)
			creature->Enemy = LaraItem;
		
		AI_INFO AI;
		CreatureAIInfo(item, &AI);

		GetCreatureMood(item, &AI, VIOLENT);
		CreatureMood(item, &AI, VIOLENT);

		angle = CreatureTurn(item, creature->MaxTurn);
		creature->MaxTurn = 0;

		if (AI.ahead)
		{
			joint0 = AI.angle / 2;
			joint1 = AI.xAngle;
			joint2 = AI.angle / 2;
		}

		switch (item->Animation.ActiveState)
		{
		case SNIPER_STATE_IDLE:
			item->MeshBits = 0;

			if (TargetVisible(item, &AI))
				item->Animation.TargetState = SNIPER_STATE_UNCOVER;

			break;

		case SNIPER_STATE_UNCOVER:
			item->MeshBits = ALL_JOINT_BITS;
			break;

		case 3:
			creature->Flags = 0;
			if (!TargetVisible(item, &AI) ||
				item->HitStatus &&
				GetRandomControl() & 1)
			{
				item->Animation.TargetState = SNIPER_STATE_COVER;
			}
			else if (!(GetRandomControl() & 0x1F))
				item->Animation.TargetState = SNIPER_STATE_FIRE;
			
			break;

		case SNIPER_STATE_FIRE:
			if (!creature->Flags)
			{
				ShotLara(item, &AI, &SniperGunBite, joint0, 100);
				creature->FiredWeapon = 2;
				creature->Flags = 1;
			}

			break;

		default:
			break;
		}
	}
	else
	{
		item->HitPoints = 0;

		if (item->Animation.ActiveState != SNIPER_STATE_DEATH)
		{
			item->Animation.AnimNumber = Objects[ID_SNIPER].animIndex + 5;
			item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
			item->Animation.ActiveState = SNIPER_STATE_DEATH;
		}
	}

	CreatureTilt(item, 0);
	CreatureJoint(item, 0, joint0);
	CreatureJoint(item, 1, joint1);
	CreatureJoint(item, 2, joint2);
	CreatureAnimation(itemNumber, angle, 0);
}

void InitialiseMafia2(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	ClearItem(itemNumber);

	item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex;
	item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
	item->Animation.TargetState = GUARD_STATE_IDLE;
	item->Animation.ActiveState = GUARD_STATE_IDLE;
	item->MeshSwapBits = 9216;
}

void Mafia2Control(short itemNumber)
{
	if (!CreatureActive(itemNumber))
		return;

	auto* item = &g_Level.Items[itemNumber];
	auto* creature = GetCreatureInfo(item);

	short angle = 0;
	short joint0 = 0;
	short joint1 = 0;
	short joint2 = 0;

	// Can mafia jump? Check for a distances of 1 and 2 sectors.
	int x = item->Pose.Position.x;
	int y = item->Pose.Position.y;
	int z = item->Pose.Position.z;

	int dx = 870 * phd_sin(item->Pose.Orientation.y);
	int dz = 870 * phd_cos(item->Pose.Orientation.y);

	x += dx;
	z += dz;
	int height1 = GetCollision(x, y, z, item->RoomNumber).Position.Floor;

	x += dx;
	z += dz;
	int height2 = GetCollision(x, y, z, item->RoomNumber).Position.Floor;
	
	x += dx;
	z += dz;
	int height3 = GetCollision(x, y, z, item->RoomNumber).Position.Floor;
	
	int height = 0;
	bool canJump1Sector = true;
	if (item->BoxNumber == LaraItem->BoxNumber ||
		y >= (height1 - CLICK(1.5f)) ||
		y >= (height2 + CLICK(1)) ||
		y <= (height2 - CLICK(1)))
	{
		height = height2;
		canJump1Sector = false;
	}

	bool canJump2Sectors = true;
	if (item->BoxNumber == LaraItem->BoxNumber ||
		y >= (height1 - CLICK(1.5f)) ||
		y >= (height - CLICK(1.5f)) ||
		y >= (height3 + CLICK(1)) ||
		y <= (height3 - CLICK(1)))
	{
		canJump2Sectors = false;
	}

	if (creature->FiredWeapon)
	{
		auto pos = Vector3Int(ArmedMafia2GunBite.x, ArmedMafia2GunBite.y, ArmedMafia2GunBite.z);
		GetJointAbsPosition(item, &pos, ArmedMafia2GunBite.meshNum);

		TriggerDynamicLight(pos.x, pos.y, pos.z, 4 * creature->FiredWeapon + 8, 24, 16, 4);
		creature->FiredWeapon--;
	}

	AI_INFO AI;
	ZeroMemory(&AI, sizeof(AI_INFO));

	if (item->HitPoints > 0)
	{
		if (item->AIBits)
			GetAITarget(creature);
		else
			creature->Enemy = LaraItem;

		CreatureAIInfo(item, &AI);

		AI_INFO laraAI;
		if (creature->Enemy == LaraItem)
		{
			laraAI.angle = AI.angle;
			laraAI.distance = AI.distance;
		}
		else
		{
			dx = LaraItem->Pose.Position.x - item->Pose.Position.x;
			dz = LaraItem->Pose.Position.z - item->Pose.Position.z;
			laraAI.angle = phd_atan(dz, dx) - item->Pose.Orientation.y;
			laraAI.distance = pow(dx, 2) + pow(dz, 2);
		}

		GetCreatureMood(item, &AI, creature->Enemy != LaraItem);
		CreatureMood(item, &AI, creature->Enemy != LaraItem);
		creature->Enemy = LaraItem;
		angle = CreatureTurn(item, creature->MaxTurn);

		if ((laraAI.distance < pow(SECTOR(2), 2) && LaraItem->Animation.Velocity > 20) ||
			item->HitStatus ||
			TargetVisible(item, &laraAI))
		{
			if (!(item->AIBits & FOLLOW))
			{
				creature->Enemy = LaraItem;
				AlertAllGuards(itemNumber);
			}
		}
		switch (item->Animation.ActiveState)
		{
		case MAFIA2_STATE_IDLE:
			creature->MaxTurn = 0;
			creature->Flags = 0;
			creature->LOT.IsJumping = false;
			joint2 = laraAI.angle;

			if (AI.ahead && !(item->AIBits & GUARD))
			{
				joint0 = AI.angle / 2;
				joint1 = AI.xAngle;
			}
			if (item->AIBits & GUARD)
			{
				joint2 = AIGuard(creature);
				break;
			}
			if (laraAI.angle <= ANGLE(112.5f) && laraAI.angle >= -ANGLE(112.5f))
			{
				if (item->MeshSwapBits == 9216)
				{
					item->Animation.TargetState = MAFIA2_STATE_UNDRAW_GUNS;
					break;
				}
			}
			else if (item->MeshSwapBits == 9216)
			{
				item->Animation.TargetState = MAFIA2_STATE_TURN_180;
				break;
			}
			if (Targetable(item, &AI))
			{
				if (AI.distance < pow(SECTOR(1), 2) || AI.zoneNumber != AI.enemyZone)
					item->Animation.TargetState = MAFIA2_STATE_AIM;
				else if (!(item->AIBits & MODIFY))
					item->Animation.TargetState = MAFIA2_STATE_WALK;
			}
			else
			{
				if (item->AIBits & PATROL1)
					item->Animation.TargetState = MAFIA2_STATE_WALK;
				else
				{
					if (canJump1Sector || canJump2Sectors)
					{
						item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 41;
						item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
						item->Animation.ActiveState = MAFIA2_STATE_IDLE_START_JUMP;
						creature->MaxTurn = 0;

						if (canJump2Sectors)
							item->Animation.TargetState = MAFIA2_STATE_JUMP_2_BLOCKS;
						else
							item->Animation.TargetState = MAFIA2_STATE_JUMP_1_BLOCK;

						creature->LOT.IsJumping = true;
						break;
					}

					if (creature->Mood != MoodType::Bored)
					{
						if (AI.distance >= pow(SECTOR(3), 2))
							item->Animation.TargetState = MAFIA2_STATE_WALK;
					}
					else
						item->Animation.TargetState = MAFIA2_STATE_IDLE;
				}
			}

			break;

		case MAFIA2_STATE_TURN_180_UNDRAW_WEAPON:
		case MAFIA2_STATE_TURN_180:
			creature->MaxTurn = 0;

			if (AI.angle >= 0)
				item->Pose.Orientation.y -= ANGLE(2.0f);
			else
				item->Pose.Orientation.y += ANGLE(2.0f);

			if (item->Animation.FrameNumber != g_Level.Anims[item->Animation.AnimNumber].frameBase + 16 ||
				item->MeshSwapBits != 9216)
			{
				if (item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameEnd)
					item->Pose.Orientation.y += -ANGLE(180.0f);
			}
			else
				item->MeshSwapBits = 128;
			
			break;

		case MAFIA2_STATE_FIRE:
			creature->MaxTurn = 0;
			joint0 = laraAI.angle / 2;
			joint2 = laraAI.angle / 2;

			if (AI.ahead)
				joint1 = AI.xAngle;

			if (abs(AI.angle) >= ANGLE(2.0f))
			{
				if (AI.angle >= 0)
					item->Pose.Orientation.y += ANGLE(2.0f);
				else
					item->Pose.Orientation.y -= ANGLE(2.0f);
			}
			else
				item->Pose.Orientation.y += AI.angle;
			
			if (!creature->Flags)
			{
				ShotLara(item, &AI, &ArmedMafia2GunBite, laraAI.angle / 2, 35);
				creature->Flags = 1;
				creature->FiredWeapon = 2;
			}
			
			break;

		case MAFIA2_STATE_AIM:
			creature->MaxTurn = 0;
			creature->Flags = 0;
			joint0 = laraAI.angle / 2;
			joint2 = laraAI.angle / 2;

			if (AI.ahead)
				joint1 = AI.xAngle;

			if (abs(AI.angle) >= ANGLE(2.0f))
			{
				if (AI.angle >= 0)
					item->Pose.Orientation.y += ANGLE(2.0f);
				else
					item->Pose.Orientation.y -= ANGLE(2.0f);
			}
			else
				item->Pose.Orientation.y += AI.angle;

			if (Targetable(item, &AI))
				item->Animation.TargetState = MAFIA2_STATE_FIRE;
			else if (laraAI.angle > 20480 || laraAI.angle < -20480)
				item->Animation.TargetState = 32;
			else
				item->Animation.TargetState = MAFIA2_STATE_IDLE;
			
			break;

		case MAFIA2_STATE_WALK:
			creature->MaxTurn = ANGLE(5.0f);
			creature->LOT.IsJumping = false;

			if (Targetable(item, &AI) &&
				(AI.distance < pow(SECTOR(1), 2) || AI.zoneNumber != AI.enemyZone))
			{
				item->Animation.TargetState = MAFIA2_STATE_AIM;
			}
			else
			{
				if (canJump1Sector || canJump2Sectors)
				{
					item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 41;
					item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
					item->Animation.ActiveState = MAFIA2_STATE_IDLE_START_JUMP;
					creature->MaxTurn = 0;

					if (canJump2Sectors)
						item->Animation.TargetState = MAFIA2_STATE_JUMP_2_BLOCKS;
					else
						item->Animation.TargetState = MAFIA2_STATE_JUMP_1_BLOCK;

					creature->LOT.IsJumping = true;
					break;
				}

				if (AI.distance >= pow(SECTOR(1), 2))
				{
					if (AI.distance > pow(SECTOR(3), 2))
						item->Animation.TargetState = MAFIA2_STATE_RUN;
				}
				else
					item->Animation.TargetState = MAFIA2_STATE_IDLE;
			}

			break;

		case MAFIA2_STATE_RUN:
			creature->MaxTurn = ANGLE(10.0f);
			creature->LOT.IsJumping = false;

			if (Targetable(item, &AI) &&
				(AI.distance < pow(SECTOR(1), 2) || AI.zoneNumber != AI.enemyZone))
			{
				item->Animation.TargetState = MAFIA2_STATE_AIM;
			}
			else if (canJump1Sector || canJump2Sectors)
			{
				item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 50;
				item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
				item->Animation.ActiveState = MAFIA2_STATE_IDLE_START_JUMP;
				creature->MaxTurn = 0;

				if (canJump2Sectors)
					item->Animation.TargetState = MAFIA2_STATE_JUMP_2_BLOCKS;
				else
					item->Animation.TargetState = MAFIA2_STATE_JUMP_1_BLOCK;

				creature->LOT.IsJumping = true;
			}
			else if (AI.distance < pow(SECTOR(3), 2))
				item->Animation.TargetState = MAFIA2_STATE_WALK;
			
			break;

		case MAFIA2_STATE_UNDRAW_GUNS:
			creature->MaxTurn = 0;

			if (AI.angle >= 0)
				item->Pose.Orientation.y += ANGLE(2.0f);
			else
				item->Pose.Orientation.y -= ANGLE(2.0f);

			if (item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameBase + 16 &&
				item->MeshSwapBits == 9216)
			{
				item->MeshSwapBits = 128;
			}

			break;

		default:
			break;
		}
	}
	else
	{
		if (item->Animation.ActiveState != MAFIA2_STATE_DEATH_1 &&
			item->Animation.ActiveState != MAFIA2_STATE_DEATH_2)
		{
			if (AI.angle >= ANGLE(67.5f) || AI.angle <= -ANGLE(67.5f))
			{
				item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 16;
				item->Animation.ActiveState = MAFIA2_STATE_DEATH_2;
				item->Pose.Orientation.y += AI.angle - ANGLE(18.0f);
			}
			else
			{
				item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 11;
				item->Animation.ActiveState = MAFIA2_STATE_DEATH_1;
				item->Pose.Orientation.y += AI.angle;
			}

			item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
		}
	}

	CreatureJoint(item, 0, joint0);
	CreatureJoint(item, 1, joint1);
	CreatureJoint(item, 2, joint2);

	if (item->Animation.ActiveState >= 20 ||
		item->Animation.ActiveState == MAFIA2_STATE_DEATH_1 ||
		item->Animation.ActiveState == MAFIA2_STATE_DEATH_2)
	{
		CreatureAnimation(itemNumber, angle, 0);
	}
	else
	{
		switch (CreatureVault(itemNumber, angle, 2, CLICK(2)) + 4)
		{
		case 0:
			item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 38;
			item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
			item->Animation.ActiveState = 23;
			break;

		case 1:
			item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 39;
			item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
			item->Animation.ActiveState = 24;
			creature->MaxTurn = 0;
			break;

		case 2:
			item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 40;
			item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
			item->Animation.ActiveState = 25;
			creature->MaxTurn = 0;
			break;

		case 6:
			item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 35;
			item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
			item->Animation.ActiveState = 20;
			creature->MaxTurn = 0;
			break;

		case 7:
			item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 36;
			item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
			item->Animation.ActiveState = 21;
			creature->MaxTurn = 0;
			break;

		case 8:
			item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 37;
			item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
			item->Animation.ActiveState = 22;
			creature->MaxTurn = 0;
			break;

		default:
			return;
		}
	}
}
