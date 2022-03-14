#include "framework.h"
#include "tr5_cyborg.h"
#include "Game/items.h"
#include "Game/collision/collide_room.h"
#include "Game/control/box.h"
#include "Game/effects/effects.h"
#include "Game/people.h"
#include "Game/animation.h"
#include "Game/effects/tomb4fx.h"
#include "Game/Lara/lara.h"
#include "Game/misc.h"
#include "Specific/setup.h"
#include "Specific/level.h"
#include "Sound/sound.h"
#include "Game/itemdata/creature_info.h"
#include "Game/effects/lightning.h"
#include "Game/effects/lara_fx.h"

using namespace TEN::Effects::Lara;
using namespace TEN::Effects::Lightning;

BITE_INFO CyborgGunBite = { 0, 300, 64, 7 };
byte HitmanJoints[12] = { 15, 14, 13, 6, 5, 12, 7, 4, 10, 11, 19 };

enum CyborgState
{
	CYBORG_STATE_IDLE = 1,
	CYBORG_STATE_WALK = 2,
	CYBORG_STATE_RUN = 3,
	CYBORG_STATE_START_END_MONKEY = 4,
	CYBORG_STATE_MONKEY = 5,

	CYBORG_STATE_JUMP = 15,
	CYBORG_STATE_JUMP_2_BLOCKS = 16,

	CYBORG_STATE_AIM = 38,
	CYBORG_STATE_FIRE = 39,

	CYBORG_STATE_GASSED = 42,
	CYBORG_STATE_DEATH = 43
};

// TODO
enum CyborgAnim
{

};

void InitialiseCyborg(short itemNumber)
{
    auto* item = &g_Level.Items[itemNumber];

    ClearItem(itemNumber);

    item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 4;
    item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
    item->Animation.TargetState = CYBORG_STATE_IDLE;
    item->Animation.ActiveState = CYBORG_STATE_IDLE;
}

static void TriggerHitmanSparks(int x, int y, int z, short xv, short yv, short zv)
{
	int dx = LaraItem->Position.xPos - x;
	int dz = LaraItem->Position.zPos - z;

	if (dx >= -SECTOR(16) && dx <= SECTOR(16) &&
		dz >= -SECTOR(16) && dz <= SECTOR(16))
	{
		auto* spark = &Sparks[GetFreeSpark()];

		spark->sR = -1;
		spark->sG = -1;
		spark->sB = -1;
		spark->dR = -1;
		spark->on = 1;
		spark->colFadeSpeed = 3;
		spark->fadeToBlack = 5;
		spark->dG = (rand() & 127) + 64;
		spark->dB = -64 - spark->dG;
		spark->life = 10;
		spark->sLife = 10;
		spark->transType = TransTypeEnum::COLADD;
		spark->friction = 34;
		spark->scalar = 1;
		spark->flags = SP_SCALE;
		spark->x = (rand() & 7) + x - 3;
		spark->y = ((rand() / 8) & 7) + y - 3;
		spark->z = ((rand() / 64) & 7) + z - 3;
		spark->xVel = (byte)(rand() / 4) + xv - 128;
		spark->yVel = (byte)(rand() / 16) + yv - 128;
		spark->zVel = (byte)(rand() / 64) + zv - 128;
		spark->sSize = spark->size= ((rand() / 512) & 3) + 4;
		spark->dSize = ((rand() / 4096) & 1) + 1;
		spark->maxYvel = 0;
		spark->gravity = 0;
	}
}

void CyborgControl(short itemNumber)
{
	if (CreatureActive(itemNumber))
	{
		auto* item = &g_Level.Items[itemNumber];
		auto* creature = GetCreatureInfo(item);
		auto* object = &Objects[item->ObjectNumber];

		short angle = 0;
		short joint2 = 0;
		short joint1 = 0;
		short joint0 = 0;

		int x = item->Position.xPos;
		int z = item->Position.zPos;

		int dx = 808 * phd_sin(item->Position.yRot);
		int dz = 808 * phd_cos(item->Position.yRot);

		x += dx;
		z += dz;
		int height1 = GetCollision(x, item->Position.yPos, z, item->RoomNumber).Position.Floor;

		x += dx;
		z += dz;
		int height2 = GetCollision(x, item->Position.yPos, z, item->RoomNumber).Position.Floor;

		x += dx;
		z += dz;
		auto probe = GetCollision(x, item->Position.yPos, z, item->RoomNumber);
		short roomNumber = probe.RoomNumber;
		int height3 = probe.Position.Floor;

		bool canJump1block = true;
		if (item->BoxNumber == LaraItem->BoxNumber ||
			item->Position.yPos >= (height1 - CLICK(1.5f)) ||
			item->Position.yPos >= (height2 + CLICK(2)) ||
			item->Position.yPos <= (height2 - CLICK(2)))
		{
			canJump1block = false;
		}

		bool canJump2blocks = true;
		if (item->BoxNumber == LaraItem->BoxNumber ||
			item->Position.yPos >= (height1 - CLICK(1.5f)) ||
			item->Position.yPos >= (height2 - CLICK(1.5f)) ||
			item->Position.yPos >= (height3 + CLICK(2)) ||
			item->Position.yPos <= (height3 - CLICK(2)))
		{
			canJump2blocks = false;
		}

		if (creature->FiredWeapon)
		{
			PHD_VECTOR pos = { CyborgGunBite.x, CyborgGunBite.y, CyborgGunBite.z };
			GetJointAbsPosition(item, &pos, CyborgGunBite.meshNum);

			TriggerDynamicLight(pos.x, pos.y, pos.z, 2 * creature->FiredWeapon + 10, 192, 128, 32);
			creature->FiredWeapon--;
		}

		if (item->AIBits)
			GetAITarget(creature);
		else
			creature->Enemy = LaraItem;

		AI_INFO AI;
		CreatureAIInfo(item, &AI);

		if (item->HitStatus)
		{
			if (!(GetRandomControl() & 7))
			{
				if (item->ItemFlags[0] < 11)
				{
					item->SwapMeshFlags |= 1 << HitmanJoints[item->ItemFlags[0]];
					item->ItemFlags[0]++;
				}
			}
		}

		byte random = (byte)GetRandomControl();
		if (g_Level.Rooms[item->RoomNumber].flags & ENV_FLAG_WATER)
			random &= 31;
		if (random < item->ItemFlags[0])
		{
			PHD_VECTOR pos = { 0, 0, 50 };
			GetJointAbsPosition(item, &pos, HitmanJoints[random]);

			TriggerLightningGlow(pos.x, pos.y, pos.z, 48, 32, 32, 64);
			TriggerHitmanSparks(pos.x, pos.y, pos.z, -1, -1, -1);
			TriggerDynamicLight(pos.x, pos.y, pos.z, (GetRandomControl() & 3) + 16, 31, 63, 127);

			SoundEffect(SFX_TR5_HITMAN_ELEC_SHORT, &item->Position, 0);

			if (random == 5 || random == 7 || random == 10)
			{
				PHD_VECTOR pos2 = { 0, 0, 50 };

				switch (random)
				{
				case 5:
					GetJointAbsPosition(item, &pos2, 15);
					break;

				case 7:
					GetJointAbsPosition(item, &pos2, 6);

					if (TestEnvironment(ENV_FLAG_WATER, item) && item->HitPoints > 0)
					{
						item->Animation.ActiveState = CYBORG_STATE_DEATH;
						item->Animation.AnimNumber = object->animIndex + 69;
						item->HitPoints = 0;
						item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
						DropEntityPickups(item);
					}

					break;

				case 10:
					GetJointAbsPosition(item, &pos2, 12);
					break;
				}
				
				//TriggerEnergyArc((PHD_VECTOR*)& src, (PHD_VECTOR*)& src.x_rot, (GetRandomControl() & 7) + 8, 404701055, 13, 64, 3);
			}
		}

		if (item->HitPoints > 0)
		{
			AI_INFO laraAI;
			if (creature->Enemy == LaraItem)
			{
				laraAI.angle = AI.angle;
				laraAI.distance = AI.distance;
			}
			else
			{
				int dx = LaraItem->Position.xPos - item->Position.xPos;
				int dz = LaraItem->Position.zPos - item->Position.zPos;
				laraAI.angle = phd_atan(dz, dx) - item->Position.yRot;
				laraAI.distance = pow(dx, 2) + pow(dz, 2);
			}

			GetCreatureMood(item, &AI, creature->Enemy != LaraItem);

			if (TestEnvironment(ENV_FLAG_NO_LENSFLARE, item)) // Gassed room?
			{
				if (!(GlobalCounter & 7))
					item->HitPoints--;

				creature->Mood = MoodType::Escape;

				if (item->HitPoints <= 0)
				{
					item->Animation.ActiveState = CYBORG_STATE_GASSED;
					item->Animation.AnimNumber = object->animIndex + 68;
					item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
				}
			}

			CreatureMood(item, &AI, creature->Enemy != LaraItem);
			
			angle = CreatureTurn(item, creature->MaxTurn);
			
			if (laraAI.distance < pow(SECTOR(2), 2) &&
				LaraItem->Animation.Velocity> 20 ||
				item->HitStatus ||
				TargetVisible(item, &laraAI))
			{
				if (!(item->AIBits & FOLLOW))
				{
					creature->Enemy = LaraItem;
					AlertAllGuards(itemNumber);
				}
			}

			int height;

			switch (item->Animation.ActiveState)
			{
			case CYBORG_STATE_IDLE:
				creature->MaxTurn = 0;
				creature->Flags = 0;
				creature->LOT.IsJumping = false;
				joint2 = laraAI.angle;

				if (AI.ahead && item->AIBits != GUARD)
				{
					joint0 = AI.angle / 2;
					joint1 = AI.xAngle;
				}
				
				if (item->Animation.RequiredState)
					item->Animation.TargetState = item->Animation.RequiredState;
				else
				{
					if (item->AIBits & GUARD)
					{
						joint2 = AIGuard(creature);
						
						if (item->AIBits & PATROL1)
						{
							item->TriggerFlags--;
							if (item->TriggerFlags < 1)
								item->AIBits |= PATROL1;
						}
					}
					else if (Targetable(item, &AI))
					{
						if (AI.distance < pow(SECTOR(4), 2) || AI.zoneNumber != AI.enemyZone)
							item->Animation.TargetState = CYBORG_STATE_AIM;
						else if (item->AIBits != MODIFY)
							item->Animation.TargetState = CYBORG_STATE_WALK;
					}
					else
					{
						if (item->AIBits & PATROL1)
							item->Animation.TargetState = CYBORG_STATE_WALK;
						else
						{
							if (canJump1block || canJump2blocks)
							{
								creature->MaxTurn = 0;
								item->Animation.AnimNumber = object->animIndex + 22;
								item->Animation.ActiveState = CYBORG_STATE_JUMP;
								item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;

								if (canJump2blocks)
									item->Animation.TargetState = CYBORG_STATE_JUMP_2_BLOCKS;
								creature->LOT.IsJumping = true;
							}
							else if (!creature->MonkeySwingAhead)
							{
								if (creature->Mood != MoodType::Bored)
								{
									if (AI.distance < pow(SECTOR(3), 2) || item->AIBits & FOLLOW)
										item->Animation.TargetState = CYBORG_STATE_WALK;
									else
										item->Animation.TargetState = CYBORG_STATE_RUN;
								}
								else
									item->Animation.TargetState = CYBORG_STATE_IDLE;
							}
							else
							{
								probe = GetCollision(item->Position.xPos, item->Position.yPos, item->Position.zPos, roomNumber);
								roomNumber = probe.RoomNumber;
								height = probe.Position.Floor;

								if (probe.Position.Ceiling == height - SECTOR(1.5f))
									item->Animation.TargetState = CYBORG_STATE_START_END_MONKEY;
								else
									item->Animation.TargetState = CYBORG_STATE_WALK;
							}
						}
					}
				}

				break;

			case CYBORG_STATE_WALK:
				creature->LOT.IsJumping = false;
				creature->MaxTurn = ANGLE(5.0f);

				if (Targetable(item, &AI) &&
					(AI.distance < pow(SECTOR(4), 2) ||
						AI.zoneNumber != AI.enemyZone))
				{
					item->Animation.TargetState = CYBORG_STATE_IDLE;
					item->Animation.RequiredState = CYBORG_STATE_AIM;
				}
				else
				{
					if (canJump1block || canJump2blocks)
					{
						item->Animation.AnimNumber = object->animIndex + 22;
						item->Animation.ActiveState = CYBORG_STATE_JUMP;
						item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
						creature->MaxTurn = 0;

						if (canJump2blocks)
							item->Animation.TargetState = CYBORG_STATE_JUMP_2_BLOCKS;

						creature->LOT.IsJumping = true;
					}
					else if (!creature->MonkeySwingAhead)
					{
						if (AI.distance >= pow(SECTOR(1), 2))
						{
							if (AI.distance > pow(SECTOR(3), 2))
							{
								if (!item->AIBits)
									item->Animation.TargetState = CYBORG_STATE_RUN;
							}
						}
						else
							item->Animation.TargetState = CYBORG_STATE_IDLE;
					}
					else
						item->Animation.TargetState = CYBORG_STATE_IDLE;
				}

				break;

			case CYBORG_STATE_RUN:
				creature->LOT.IsJumping = false;
				creature->MaxTurn = ANGLE(10);

				if (Targetable(item, &AI) &&
					(AI.distance < pow(SECTOR(4), 2) ||
						AI.zoneNumber != AI.enemyZone))
				{
					item->Animation.TargetState = CYBORG_STATE_IDLE;
					item->Animation.RequiredState = CYBORG_STATE_AIM;
				}
				else if (canJump1block || canJump2blocks)
				{
					item->Animation.AnimNumber = object->animIndex + 22;
					item->Animation.ActiveState = CYBORG_STATE_JUMP;
					item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
					creature->MaxTurn = 0;

					if (canJump2blocks)
						item->Animation.TargetState = CYBORG_STATE_JUMP_2_BLOCKS;

					creature->LOT.IsJumping = true;
				}
				else
				{
					if (creature->MonkeySwingAhead)
						item->Animation.TargetState = CYBORG_STATE_IDLE;
					else if (AI.distance < pow(SECTOR(3), 2))
						item->Animation.TargetState = CYBORG_STATE_WALK;
				}

				break;

			case CYBORG_STATE_START_END_MONKEY:
				creature->MaxTurn = 0;
				
				if (item->BoxNumber == creature->LOT.TargetBox ||
					!creature->MonkeySwingAhead)
				{
					probe = GetCollision(item->Position.xPos, item->Position.yPos, item->Position.zPos, roomNumber);
					roomNumber = probe.RoomNumber;
					height = probe.Position.Floor;

					if (probe.Position.Ceiling == height - SECTOR(1.5f), 2)
						item->Animation.TargetState = CYBORG_STATE_IDLE;
				}
				else
					item->Animation.TargetState = CYBORG_STATE_MONKEY;
				
				break;

			case CYBORG_STATE_MONKEY:
				creature->LOT.IsMonkeying = true;
				creature->LOT.IsJumping = true;
				creature->MaxTurn = ANGLE(5.0f);
				
				if (item->BoxNumber == creature->LOT.TargetBox ||
					!creature->MonkeySwingAhead)
				{
					probe = GetCollision(item->Position.xPos, item->Position.yPos, item->Position.zPos, roomNumber);
					roomNumber = probe.RoomNumber;
					height = probe.Position.Floor;

					if (probe.Position.Ceiling == height - SECTOR(1.5f), 2)
						item->Animation.TargetState = CYBORG_STATE_START_END_MONKEY;
				}

				break;

			case CYBORG_STATE_AIM:
				creature->MaxTurn = 0;
				creature->Flags = 0;
				joint0 = laraAI.angle / 2;
				joint2 = laraAI.angle / 2;

				if (AI.ahead)
					joint1 = AI.xAngle;

				if (abs(AI.angle) >= ANGLE(2.0f))
				{
					if (AI.angle >= 0)
						item->Position.yRot += ANGLE(2.0f);
					else
						item->Position.yRot -= ANGLE(2.0f);
				}
				else
					item->Position.yRot += AI.angle;

				if (Targetable(item, &AI) &&
					(AI.distance < pow(SECTOR(4), 2) ||
						AI.zoneNumber != AI.enemyZone))
					item->Animation.TargetState = CYBORG_STATE_FIRE;
				else
					item->Animation.TargetState = CYBORG_STATE_IDLE;

				break;

			case CYBORG_STATE_FIRE:
				joint0 = laraAI.angle / 2;
				joint2 = laraAI.angle / 2;

				if (AI.ahead)
					joint1 = AI.xAngle;

				creature->MaxTurn = 0;

				if (abs(AI.angle) >= ANGLE(2.0f))
				{
					if (AI.angle >= 0)
						item->Position.yRot += ANGLE(2.0f);
					else
						item->Position.yRot -= ANGLE(2.0f);
				}
				else
					item->Position.yRot += AI.angle;

				if (item->Animation.FrameNumber > g_Level.Anims[item->Animation.AnimNumber].frameBase + 6 &&
					item->Animation.FrameNumber < g_Level.Anims[item->Animation.AnimNumber].frameBase + 16 &&
					((byte)item->Animation.FrameNumber - (byte)g_Level.Anims[item->Animation.AnimNumber].frameBase) & 1)
				{
					creature->FiredWeapon = 1;
					ShotLara(item, &AI, &CyborgGunBite, joint0, 12);
				}

				break;

			default:
				break;
			}
		}
		else if (item->Animation.ActiveState == 43 && !Lara.Burn)
		{
			PHD_VECTOR pos = { 0, 0, 0 };
			GetLaraJointPosition(&pos, LM_LFOOT);
			
			short roomNumberLeft = LaraItem->RoomNumber;
			GetFloor(pos.x, pos.y, pos.z, &roomNumberLeft);
			
			pos = { 0, 0, 0 };
			GetLaraJointPosition(&pos, LM_RFOOT);

			short roomNumberRight = LaraItem->RoomNumber;
			GetFloor(pos.x, pos.y, pos.z, &roomNumberRight);

			auto* roomRight = &g_Level.Rooms[roomNumberRight];
			auto* roomLeft = &g_Level.Rooms[roomNumberLeft];

			short flipNumber = g_Level.Rooms[item->RoomNumber].flipNumber;

			if (TestEnvironment(ENV_FLAG_WATER, roomRight->flags) ||
				TestEnvironment(ENV_FLAG_WATER, roomLeft->flags))
			{
				if (roomLeft->flipNumber == flipNumber || roomRight->flipNumber == flipNumber)
				{
					LaraBurn(LaraItem);
					Lara.BurnCount = 48;
					Lara.BurnBlue = 1;
					LaraItem->HitPoints = 0;
				}
			}
		}

		CreatureJoint(item, 0, joint0);
		CreatureJoint(item, 1, joint1);
		CreatureJoint(item, 2, joint2);

		if (creature->ReachedGoal)
		{
			if (creature->Enemy)
			{
				TestTriggers(
					creature->Enemy->Position.xPos,
					creature->Enemy->Position.yPos,
					creature->Enemy->Position.zPos, roomNumber, true);
				
				item->Animation.RequiredState = CYBORG_STATE_WALK;

				if (creature->Enemy->Flags & 2)
					item->ItemFlags[3] = (creature->Tosspad & 0xFF) - 1;

				if (creature->Enemy->Flags & 8)
				{
					item->Animation.RequiredState = CYBORG_STATE_IDLE;
					item->TriggerFlags = 300;
					item->AIBits = GUARD | PATROL1;
				}
				
				item->ItemFlags[3]++;
				creature->ReachedGoal = false;
				creature->Enemy = NULL;
			}
		}
		
		if (item->Animation.ActiveState >= 15 || item->Animation.ActiveState == 5)
			CreatureAnimation(itemNumber, angle, 0);
		else
		{
			switch (CreatureVault(itemNumber, angle, 2, 260) + 4)
			{
			case 0:
				creature->MaxTurn = 0;
				item->Animation.AnimNumber = object->animIndex + 35;
				item->Animation.ActiveState = 25;
				item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
				break;

			case 1:
				creature->MaxTurn = 0;
				item->Animation.AnimNumber = object->animIndex + 41;
				item->Animation.ActiveState = 24;
				item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
				break;

			case 2:
				creature->MaxTurn = 0;
				item->Animation.AnimNumber = object->animIndex + 42;
				item->Animation.ActiveState = 23;
				item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
				break;

			case 6:
				creature->MaxTurn = 0;
				item->Animation.AnimNumber = object->animIndex + 29;
				item->Animation.ActiveState = 19;
				item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
				break;

			case 7:
				creature->MaxTurn = 0;
				item->Animation.AnimNumber = object->animIndex + 28;
				item->Animation.ActiveState = 18;
				item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
				break;

			case 8:
				creature->MaxTurn = 0;
				item->Animation.AnimNumber = object->animIndex + 27;
				item->Animation.ActiveState = 17;
				item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
				break;

			default:
				return;
			}
		}
	}
}
