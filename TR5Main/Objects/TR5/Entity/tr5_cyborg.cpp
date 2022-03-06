#include "framework.h"
#include "tr5_cyborg.h"
#include "Game/items.h"
#include "Game/control/box.h"
#include "Game/effects/effects.h"
#include "Game/people.h"
#include "Game/animation.h"
#include "Game/effects/tomb4fx.h"
#include "Game/Lara/lara.h"
#include "Specific/setup.h"
#include "Specific/level.h"
#include "Sound/sound.h"
#include "Game/itemdata/creature_info.h"
#include "Game/effects/lightning.h"
#include "Game/effects/lara_fx.h"

using namespace TEN::Effects::Lara;
using namespace TEN::Effects::Lightning;

#define STATE_HITMAN_STOP					1
#define STATE_HITMAN_WALK					2
#define STATE_HITMAN_RUN					3
#define STATE_HITMAN_START_END_MONKEY		4
#define STATE_HITMAN_MONKEY					5
#define STATE_HITMAN_JUMP					15
#define STATE_HITMAN_JUMP_2BLOCKS			16
#define STATE_HITMAN_AIM					38
#define STATE_HITMAN_FIRE					39
#define STATE_HITMAN_GASSED					42
#define STATE_HITMAN_DEATH					43

BITE_INFO HitmanGun = { 0, 300, 64, 7 };
byte HitmanJoints[12] = { 15, 14, 13, 6, 5, 12, 7, 4, 10, 11, 19 };

void InitialiseHitman(short itemNum)
{
    ITEM_INFO* item;

    item = &g_Level.Items[itemNum];
    ClearItem(itemNum);
    item->AnimNumber = Objects[item->ObjectNumber].animIndex + 4;
    item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
    item->TargetState = STATE_HITMAN_STOP;
    item->ActiveState = STATE_HITMAN_STOP;
}

static void TriggerHitmanSparks(int x, int y, int z, short xv, short yv, short zv)
{
	int dx = LaraItem->Position.xPos - x;
	int dz = LaraItem->Position.zPos - z;

	if (dx >= -16384 && dx <= 16384 && dz >= -16384 && dz <= 16384)
	{
		SPARKS* spark = &Sparks[GetFreeSpark()];

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

void HitmanControl(short itemNumber)
{
	if (CreatureActive(itemNumber))
	{
		ITEM_INFO* item = &g_Level.Items[itemNumber];
		CreatureInfo* creature = (CreatureInfo*)item->Data;
		OBJECT_INFO* obj = &Objects[item->ObjectNumber];

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

		short roomNumber = item->RoomNumber;
		FLOOR_INFO* floor = GetFloor(x, item->Position.yPos, z, &roomNumber);
		int height1 = GetFloorHeight(floor, x, item->Position.yPos, z);

		x += dx;
		z += dz;

		roomNumber = item->RoomNumber;
		floor = GetFloor(x, item->Position.yPos, z, &roomNumber);
		int height2 = GetFloorHeight(floor, x, item->Position.yPos, z);

		x += dx;
		z += dz;

		roomNumber = item->RoomNumber;
		floor = GetFloor(x, item->Position.yPos, z, &roomNumber);
		int height3 = GetFloorHeight(floor, x, item->Position.yPos, z);

		bool canJump1block;
		if (item->BoxNumber == LaraItem->BoxNumber
			|| item->Position.yPos >= height1 - 384
			|| item->Position.yPos >= height2 + 256
			|| item->Position.yPos <= height2 - 256)
			canJump1block = false;
		else
			canJump1block = true;

		bool canJump2blocks;
		if (item->BoxNumber == LaraItem->BoxNumber
			|| item->Position.yPos >= height1 - 384
			|| item->Position.yPos >= height2 - 384
			|| item->Position.yPos >= height3 + 256
			|| item->Position.yPos <= height3 - 256)
			canJump2blocks = false;
		else
			canJump2blocks = true;

		if (creature->FiredWeapon)
		{
			PHD_VECTOR pos;
			pos.x = HitmanGun.x;
			pos.y = HitmanGun.y;
			pos.z = HitmanGun.z;
			GetJointAbsPosition(item, &pos, HitmanGun.meshNum);
			TriggerDynamicLight(pos.x, pos.y, pos.z, 2 * creature->FiredWeapon + 10, 192, 128, 32);
			creature->FiredWeapon--;
		}

		if (item->AIBits)
			GetAITarget(creature);
		else
			creature->Enemy = LaraItem;

		AI_INFO info, laraInfo;
		CreatureAIInfo(item, &info);

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
			PHD_VECTOR pos;
			pos.x = 0;
			pos.y = 0;
			pos.z = 50;
			GetJointAbsPosition(item, &pos, HitmanJoints[random]);

			TriggerLightningGlow(pos.x, pos.y, pos.z, 48, 32, 32, 64);
			TriggerHitmanSparks(pos.x, pos.y, pos.z, -1, -1, -1);
			TriggerDynamicLight(pos.x, pos.y, pos.z, (GetRandomControl() & 3) + 16, 31, 63, 127);

			SoundEffect(SFX_TR5_HITMAN_ELEC_SHORT, &item->Position, 0);

			if (random == 5 || random == 7 || random == 10)
			{
				PHD_VECTOR pos2;
				pos2.x = 0;
				pos2.y = 0;
				pos2.z = 50;

				switch (random)
				{
				case 5:
					GetJointAbsPosition(item, &pos2, 15);
					break;
				case 7:
					GetJointAbsPosition(item, &pos2, 6);
					if (g_Level.Rooms[item->RoomNumber].flags & ENV_FLAG_WATER && item->HitPoints > 0)
					{
						item->ActiveState = STATE_HITMAN_DEATH;
						item->AnimNumber = obj->animIndex + 69;
						item->HitPoints = 0;
						item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
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
			if (creature->Enemy == LaraItem)
			{
				laraInfo.angle = info.angle;
				laraInfo.distance = info.distance;
			}
			else
			{
				int dx = LaraItem->Position.xPos - item->Position.xPos;
				int dz = LaraItem->Position.zPos - item->Position.zPos;
				laraInfo.angle = phd_atan(dz, dx) - item->Position.yRot;
				laraInfo.distance = pow(dx, 2) + pow(dz, 2);
			}

			GetCreatureMood(item, &info, creature->Enemy != LaraItem);

			if (g_Level.Rooms[item->RoomNumber].flags & ENV_FLAG_NO_LENSFLARE) // Gassed room?
			{
				if (!(GlobalCounter & 7))
					item->HitPoints--;

				creature->Mood = MoodType::Escape;

				if (item->HitPoints <= 0)
				{
					item->ActiveState = STATE_HITMAN_GASSED;
					item->AnimNumber = obj->animIndex + 68;
					item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
				}
			}

			CreatureMood(item, &info, creature->Enemy != LaraItem);
			
			angle = CreatureTurn(item, creature->MaxTurn);
			
			if (laraInfo.distance < SQUARE(2048) 
				&& LaraItem->Velocity> 20
				|| item->HitStatus
				|| TargetVisible(item, &laraInfo))
			{
				if (!(item->AIBits & FOLLOW))
				{
					creature->Enemy = LaraItem;
					AlertAllGuards(itemNumber);
				}
			}

			FLOOR_INFO* floor;
			int height;
			short roomNumber;

			switch (item->ActiveState)
			{
			case STATE_HITMAN_STOP:
				creature->LOT.IsJumping = false;
				joint2 = laraInfo.angle;
				creature->Flags = 0;
				creature->MaxTurn = 0;

				if (info.ahead && item->AIBits != GUARD)
				{
					joint0 = info.angle / 2;
					joint1 = info.xAngle;
				}
				
				if (item->RequiredState)
				{
					item->TargetState = item->RequiredState;
				}
				else
				{
					if (item->AIBits & GUARD)
					{
						joint2 = AIGuard(creature);
						
						if (item->AIBits & PATROL1)
						{
							item->TriggerFlags--;
							if (item->TriggerFlags < 1)
							{
								item->AIBits |= PATROL1;
							}
						}
					}
					else if (Targetable(item, &info))
					{
						if (info.distance < SQUARE(4096) || info.zoneNumber != info.enemyZone)
						{
							item->TargetState = STATE_HITMAN_AIM;
						}
						else if (item->AIBits != MODIFY)
						{
							item->TargetState = STATE_HITMAN_WALK;
						}
					}
					else
					{
						if (item->AIBits & PATROL1)
						{
							item->TargetState = STATE_HITMAN_WALK;
						}
						else
						{
							if (canJump1block || canJump2blocks)
							{
								creature->MaxTurn = 0;
								item->AnimNumber = obj->animIndex + 22;
								item->ActiveState = STATE_HITMAN_JUMP;
								item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
								if (canJump2blocks)
									item->TargetState = STATE_HITMAN_JUMP_2BLOCKS;
								creature->LOT.IsJumping = true;
							}
							else if (!creature->MonkeySwingAhead)
							{
								if (creature->Mood != MoodType::Bored)
								{
									if (info.distance < SQUARE(3072) || item->AIBits & FOLLOW)
										item->TargetState = STATE_HITMAN_WALK;
									else
										item->TargetState = STATE_HITMAN_RUN;
								}
								else
								{
									item->TargetState = STATE_HITMAN_STOP;
								}
							}
							else
							{
								floor = GetFloor(item->Position.xPos, item->Position.yPos, item->Position.zPos, &roomNumber);
								height = GetFloorHeight(floor, item->Position.xPos, item->Position.yPos, item->Position.zPos);
								if (GetCeiling(floor, item->Position.xPos, item->Position.yPos, item->Position.zPos) == height - 1536)
									item->TargetState = STATE_HITMAN_START_END_MONKEY;
								else
									item->TargetState = STATE_HITMAN_WALK;
							}
						}
					}
				}
				break;

			case STATE_HITMAN_WALK:
				creature->LOT.IsJumping = false;
				creature->MaxTurn = ANGLE(5);
				if (Targetable(item, &info)
					&& (info.distance < SQUARE(4096) 
						|| info.zoneNumber != info.enemyZone))
				{
					item->TargetState = STATE_HITMAN_STOP;
					item->RequiredState = STATE_HITMAN_AIM;
				}
				else
				{
					if (canJump1block || canJump2blocks)
					{
						creature->MaxTurn = 0;
						item->AnimNumber = obj->animIndex + 22;
						item->ActiveState = STATE_HITMAN_JUMP;
						item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
						if (canJump2blocks)
							item->TargetState = STATE_HITMAN_JUMP_2BLOCKS;
						creature->LOT.IsJumping = true;
					}
					else if (!creature->MonkeySwingAhead)
					{
						if (info.distance >= SQUARE(1024))
						{
							if (info.distance > SQUARE(3072))
							{
								if (!item->AIBits)
									item->TargetState = STATE_HITMAN_RUN;
							}
						}
						else
						{
							item->TargetState = STATE_HITMAN_STOP;
						}
					}
					else
					{
						item->TargetState = STATE_HITMAN_STOP;
					}
				}
				break;

			case STATE_HITMAN_RUN:
				creature->LOT.IsJumping = false;
				creature->MaxTurn = ANGLE(10);

				if (Targetable(item, &info)
					&& (info.distance < SQUARE(4096) 
						|| info.zoneNumber != info.enemyZone))
				{
					item->TargetState = STATE_HITMAN_STOP;
					item->RequiredState = STATE_HITMAN_AIM;
				}
				else if (canJump1block || canJump2blocks)
				{
					creature->MaxTurn = 0;
					item->AnimNumber = obj->animIndex + 22;
					item->ActiveState = STATE_HITMAN_JUMP;
					item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
					if (canJump2blocks)
						item->TargetState = STATE_HITMAN_JUMP_2BLOCKS;
					creature->LOT.IsJumping = true;
				}
				else
				{
					if (creature->MonkeySwingAhead)
					{
						item->TargetState = STATE_HITMAN_STOP;
					}
					else if (info.distance < SQUARE(3072))
						item->TargetState = STATE_HITMAN_WALK;
				}
				break;

			case STATE_HITMAN_START_END_MONKEY:
				creature->MaxTurn = 0;
				
				if (item->BoxNumber == creature->LOT.TargetBox 
					|| !creature->MonkeySwingAhead)
				{
					floor = GetFloor(item->Position.xPos, item->Position.yPos, item->Position.zPos, &roomNumber);
					height = GetFloorHeight(floor, item->Position.xPos, item->Position.yPos, item->Position.zPos);
					if (GetCeiling(floor, item->Position.xPos, item->Position.yPos, item->Position.zPos) == height - 1536)
						item->TargetState = STATE_HITMAN_STOP;
				}
				else
				{
					item->TargetState = STATE_HITMAN_MONKEY;
				}
				break;

			case STATE_HITMAN_MONKEY:
				creature->LOT.IsMonkeying = true;
				creature->LOT.IsJumping = true;
				creature->MaxTurn = ANGLE(5);
				
				if (item->BoxNumber == creature->LOT.TargetBox
					|| !creature->MonkeySwingAhead)
				{
					floor = GetFloor(item->Position.xPos, item->Position.yPos, item->Position.zPos, &roomNumber);
					height = GetFloorHeight(floor, item->Position.xPos, item->Position.yPos, item->Position.zPos);
					if (GetCeiling(floor, item->Position.xPos, item->Position.yPos, item->Position.zPos) == height - 1536)
						item->TargetState = STATE_HITMAN_START_END_MONKEY;
				}
				break;

			case STATE_HITMAN_AIM:
				joint0 = laraInfo.angle / 2;
				joint2 = laraInfo.angle / 2;
				creature->Flags = 0;
				creature->MaxTurn = 0;

				if (info.ahead)
					joint1 = info.xAngle;

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

				if (Targetable(item, &info) 
					&& (info.distance < SQUARE(4096) 
						|| info.zoneNumber != info.enemyZone))
					item->TargetState = STATE_HITMAN_FIRE;
				else
					item->TargetState = STATE_HITMAN_STOP;
				break;

			case STATE_HITMAN_FIRE:
				joint0 = laraInfo.angle / 2;
				joint2 = laraInfo.angle / 2;

				if (info.ahead)
					joint1 = info.xAngle;

				creature->MaxTurn = 0;

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

				if (item->FrameNumber > g_Level.Anims[item->AnimNumber].frameBase + 6
					&& item->FrameNumber < g_Level.Anims[item->AnimNumber].frameBase + 16
					&& ((byte)item->FrameNumber - (byte)g_Level.Anims[item->AnimNumber].frameBase) & 1)
				{
					creature->FiredWeapon = 1;
					ShotLara(item, &info, &HitmanGun, joint0, 12);
				}
				break;

			default:
				break;

			}
		}
		else if (item->ActiveState == 43 && !Lara.burn)
		{
			PHD_VECTOR pos;
			pos.x = 0;			
			pos.y = 0;
			pos.z = 0;
			GetLaraJointPosition(&pos, LM_LFOOT);
			
			short roomNumberLeft = LaraItem->RoomNumber;
			GetFloor(pos.x, pos.y, pos.z, &roomNumberLeft);
			
			pos.x = 0;
			pos.y = 0;
			pos.z = 0; 
			GetLaraJointPosition(&pos, LM_RFOOT);

			short roomNumberRight = LaraItem->RoomNumber;
			GetFloor(pos.x, pos.y, pos.z, &roomNumberRight);

			ROOM_INFO* roomRight = &g_Level.Rooms[roomNumberRight];
			ROOM_INFO* roomLeft = &g_Level.Rooms[roomNumberLeft];

			short flipNumber = g_Level.Rooms[item->RoomNumber].flipNumber;

			if ((roomRight->flags | roomLeft->flags) & ENV_FLAG_WATER)
			{
				if (roomLeft->flipNumber == flipNumber || roomRight->flipNumber == flipNumber)
				{
					LaraBurn(LaraItem);
					Lara.BurnCount = 48;
					Lara.burnBlue = 1;
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
				
				item->RequiredState = STATE_HITMAN_WALK;

				if (creature->Enemy->Flags & 2)
					item->ItemFlags[3] = (item->Tosspad & 0xFF) - 1;

				if (creature->Enemy->Flags & 8)
				{
					item->RequiredState = STATE_HITMAN_STOP;
					item->TriggerFlags = 300;
					item->AIBits = GUARD | PATROL1;
				}
				
				item->ItemFlags[3]++;
				creature->ReachedGoal = false;
				creature->Enemy = NULL;
			}
		}
		
		if (item->ActiveState >= 15 || item->ActiveState == 5)
		{
			CreatureAnimation(itemNumber, angle, 0);
		}
		else
		{
			switch (CreatureVault(itemNumber, angle, 2, 260) + 4)
			{
			case 0:
				creature->MaxTurn = 0;
				item->AnimNumber = obj->animIndex + 35;
				item->ActiveState = 25;
				item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
				break;

			case 1:
				creature->MaxTurn = 0;
				item->AnimNumber = obj->animIndex + 41;
				item->ActiveState = 24;
				item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
				break;

			case 2:
				creature->MaxTurn = 0;
				item->AnimNumber = obj->animIndex + 42;
				item->ActiveState = 23;
				item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
				break;

			case 6:
				creature->MaxTurn = 0;
				item->AnimNumber = obj->animIndex + 29;
				item->ActiveState = 19;
				item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
				break;

			case 7:
				creature->MaxTurn = 0;
				item->AnimNumber = obj->animIndex + 28;
				item->ActiveState = 18;
				item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
				break;

			case 8:
				creature->MaxTurn = 0;
				item->AnimNumber = obj->animIndex + 27;
				item->ActiveState = 17;
				item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
				break;

			default:
				return;
			}
		}
	}
}