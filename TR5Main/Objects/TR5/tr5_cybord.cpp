#include "../oldobjects.h"
#include "../../Game/items.h"
#include "../../Game/sphere.h"
#include "../../Game/Box.h"
#include "../../Game/effect2.h"
#include "../../Game/people.h"
#include "../../Game/draw.h"
#include "../../Game/tomb4fx.h"
#include "../../Game/lara.h"
#include "../../Game/traps.h"

#define STATE_CYBORG_STOP					1

#define ANIMATION_CYBORG_STAY_IDLE			4

BITE_INFO CyborgGun = { 0x00, 0x12C, 0x40, 0x07 };
byte CyborgJoints[12] = { 0x0F, 0x0E, 0x0D, 6, 5, 0x0C, 7, 4, 0x0A, 0x0B, 0x13 };

void InitialiseCyborg(short itemNum)
{
    ITEM_INFO* item;

    item = &Items[itemNum];
    ClearItem(itemNum);
    item->animNumber = Objects[item->objectNumber].animIndex + ANIMATION_CYBORG_STAY_IDLE;
    item->frameNumber = Anims[item->animNumber].frameBase;
    item->goalAnimState = STATE_CYBORG_STOP;
    item->currentAnimState = STATE_CYBORG_STOP;
}

void TriggerCyborgSparks(int x, int y, int z, short xv, short yv, short zv)
{
	int dx = LaraItem->pos.xPos - x;
	int dz = LaraItem->pos.zPos - z;

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
		spark->dG = (rand() & 0x7F) + 64;
		spark->dB = -64 - spark->dG;
		spark->life = 10;
		spark->sLife = 10;
		spark->transType = COLADD;
		spark->friction = 34;
		spark->scalar = 1;
		spark->x = (rand() & 7) + x - 3;
		spark->flags = SP_SCALE;
		spark->y = ((rand() >> 3) & 7) + y - 3;
		spark->z = ((rand() >> 6) & 7) + z - 3;
		spark->xVel = (byte)(rand() >> 2) + xv - 128;
		spark->yVel = (byte)(rand() >> 4) + yv - 128;
		spark->zVel = (byte)(rand() >> 6) + zv - 128;
		spark->sSize = spark->size= ((rand() >> 9) & 3) + 4;
		spark->dSize = ((rand() >> 12) & 1) + 1;
		spark->maxYvel = 0;
		spark->gravity = 0;
	}
}

void ControlCyborg(short itemNumber)
{
	if (CreatureActive(itemNumber))
	{
		ITEM_INFO* item = &Items[itemNumber];
		CREATURE_INFO* creature = (CREATURE_INFO*)item->data;
		OBJECT_INFO* obj = &Objects[item->objectNumber];

		short angle = 0;
		short joint2 = 0;
		short joint1 = 0;
		short joint0 = 0;

		int x = item->pos.xPos;
		int z = item->pos.zPos;

		int dx = 808 * SIN(item->pos.yRot) >> W2V_SHIFT;
		int dz = 808 * COS(item->pos.yRot) >> W2V_SHIFT;

		x += dx;
		z += dz;

		short roomNumber = item->roomNumber;
		FLOOR_INFO* floor = GetFloor(x, item->pos.yPos, z, &roomNumber);
		int height1 = GetFloorHeight(floor, x, item->pos.yPos, z);

		x += dx;
		z += dz;

		roomNumber = item->roomNumber;
		floor = GetFloor(x, item->pos.yPos, z, &roomNumber);
		int height2 = GetFloorHeight(floor, x, item->pos.yPos, z);

		x += dx;
		z += dz;

		roomNumber = item->roomNumber;
		floor = GetFloor(x, item->pos.yPos, z, &roomNumber);
		int height3 = GetFloorHeight(floor, x, item->pos.yPos, z);

		bool canJump1block;
		if (item->boxNumber == LaraItem->boxNumber
			|| item->pos.yPos >= height1 - 384
			|| item->pos.yPos >= height2 + 256
			|| item->pos.yPos <= height2 - 256)
			canJump1block = false;
		else
			canJump1block = true;

		bool canJump2blocks;
		if (item->boxNumber == LaraItem->boxNumber
			|| item->pos.yPos >= height1 - 384
			|| item->pos.yPos >= height2 - 384
			|| item->pos.yPos >= height3 + 256
			|| item->pos.yPos <= height3 - 256)
			canJump2blocks = false;
		else
			canJump2blocks = true;

		if (item->firedWeapon)
		{
			PHD_VECTOR pos;
			pos.x = CyborgGun.x;
			pos.y = CyborgGun.y;
			pos.z = CyborgGun.z;
			GetJointAbsPosition(item, &pos, CyborgGun.meshNum);
			TriggerDynamicLight(pos.x, pos.y, pos.z, 2 * item->firedWeapon + 10, 192, 128, 32);
			item->firedWeapon--;
		}

		if (item->aiBits)
			GetAITarget(creature);
		else
			creature->enemy = LaraItem;

		AI_INFO info, laraInfo;
		CreatureAIInfo(item, &info);

		if (item->hitStatus)
		{
			if (!(GetRandomControl() & 7))
			{
				if (item->itemFlags[0] < 11)
				{
					item->swapMeshFlags |= 1 << CyborgJoints[item->itemFlags[0]];
					item->itemFlags[0]++;
				}
			}
		}

		byte random = (byte)GetRandomControl();
		if (Rooms[item->roomNumber].flags & ENV_FLAG_WATER)
			random &= 0x1Fu;
		if (random < item->itemFlags[0])
		{
			PHD_VECTOR pos;
			pos.x = 0;
			pos.y = 0;
			pos.z = 50;
			GetJointAbsPosition(item, &pos, CyborgJoints[random]);

			TriggerLightningGlow(pos.x, pos.y, pos.z, 807411776);
			TriggerCyborgSparks(pos.x, pos.y, pos.z, -1, -1, -1);
			TriggerDynamicLight(pos.x, pos.y, pos.z, (GetRandomControl() & 3) + 16, 31, 63, 127);

			SoundEffect(SFX_HITMAN_ELEC_SHORT, &item->pos, 0);

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
					if (Rooms[item->roomNumber].flags & ENV_FLAG_WATER && item->hitPoints > 0)
					{
						item->currentAnimState = 43;
						item->animNumber = obj->animIndex + 69;
						item->hitPoints = 0;
						item->frameNumber = Anims[item->animNumber].frameBase;
						DropBaddyPickups(item);
					}
					break;
				case 10:
					GetJointAbsPosition(item, &pos2, 12);
					break;
				}
				
				//TriggerEnergyArc((PHD_VECTOR*)& src, (PHD_VECTOR*)& src.x_rot, (GetRandomControl() & 7) + 8, 404701055, 13, 64, 3);
			}
		}

		if (item->hitPoints > 0)
		{
			if (creature->enemy == LaraItem)
			{
				laraInfo.angle = info.angle;
				laraInfo.distance = info.distance;
			}
			else
			{
				int dx = LaraItem->pos.xPos - item->pos.xPos;
				int dz = LaraItem->pos.zPos - item->pos.zPos;
				laraInfo.angle = ATAN(dz, dx) - item->pos.yRot;
				laraInfo.distance = SQUARE(dx) + SQUARE(dz);
			}

			GetCreatureMood(item, &info, creature->enemy != LaraItem);

			if (Rooms[item->roomNumber].flags & ENV_FLAG_NO_LENSFLARE) // Gassed room?
			{
				if (!(GlobalCounter & 7))
					item->hitPoints--;

				creature->mood = ESCAPE_MOOD;

				if (item->hitPoints <= 0)
				{
					item->currentAnimState = 42;
					item->animNumber = obj->animIndex + 68;
					item->frameNumber = Anims[item->animNumber].frameBase;
				}
			}

			CreatureMood(item, &info, creature->enemy != LaraItem);
			
			angle = CreatureTurn(item, creature->maximumTurn);
			
			if (laraInfo.distance < SQUARE(2048) 
				&& LaraItem->speed > 20
				|| item->hitStatus
				|| TargetVisible(item, &laraInfo))
			{
				if (!(item->aiBits & FOLLOW))
				{
					creature->enemy = LaraItem;
					AlertAllGuards(itemNumber);
				}
			}

			FLOOR_INFO* floor;
			int height;
			short roomNumber;

			switch (item->currentAnimState)
			{
			case 1:
				creature->LOT.isJumping = false;
				joint2 = laraInfo.angle;
				creature->flags = 0;
				creature->maximumTurn = 0;

				if (info.ahead && (item->aiBits) != GUARD)
				{
					joint0 = info.angle >> 1;
					joint1 = info.xAngle;
				}
				
				if (item->requiredAnimState)
				{
					item->goalAnimState = item->requiredAnimState;
				}
				else
				{
					if (item->aiBits & GUARD)
					{
						joint2 = AIGuard(creature);
						
						if (item->aiBits & PATROL1)
						{
							item->triggerFlags--;
							if (item->triggerFlags < 1)
							{
								item->aiBits |= PATROL1;
							}
						}
					}
					else if (Targetable(item, &info))
					{
						if (info.distance < SQUARE(4096) || info.zoneNumber != info.enemyZone)
						{
							item->goalAnimState = 38;
						}
						else if (item->aiBits != MODIFY)
						{
							item->goalAnimState = 2;
						}
					}
					else
					{
						if (item->aiBits & PATROL1)
						{
							item->goalAnimState = 2;
						}
						else
						{
							if (canJump1block || canJump2blocks)
							{
								creature->maximumTurn = 0;
								item->animNumber = obj->animIndex + 22;
								item->currentAnimState = 15;
								item->frameNumber = Anims[item->animNumber].frameBase;
								if (canJump2blocks)
									item->goalAnimState = 16;
								creature->LOT.isJumping = true;
							}
							else if (!creature->monkeyAhead)
							{
								if (creature->mood)
								{
									if (info.distance < SQUARE(3072) || item->aiBits & FOLLOW)
										item->goalAnimState = 2;
									else
										item->goalAnimState = 3;
								}
								else
								{
									item->goalAnimState = 1;
								}
							}
							else
							{
								floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
								height = GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);
								if (GetCeiling(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos) == height - 1536)
									goto LABEL_145;
								item->goalAnimState = 2;
							}
						}
					}
				}
				break;

			case 2:
				creature->LOT.isJumping = false;
				creature->maximumTurn = ANGLE(5);
				if (Targetable(item, &info)
					&& (info.distance < SQUARE(4096) 
						|| info.zoneNumber != info.enemyZone))
				{
					item->goalAnimState = 1;
					item->requiredAnimState = 38;
				}
				else
				{
					if (canJump1block || canJump2blocks)
					{
						creature->maximumTurn = 0;
						item->animNumber = obj->animIndex + 22;
						item->currentAnimState = 15;
						item->frameNumber = Anims[item->animNumber].frameBase;
						if (canJump2blocks)
							item->goalAnimState = 16;
						creature->LOT.isJumping = true;
					}
					else if (!creature->monkeyAhead)
					{
						if (info.distance >= SQUARE(1024))
						{
							if (info.distance > SQUARE(3072))
							{
								if (!item->aiBits)
									item->goalAnimState = 3;
							}
						}
						else
						{
							item->goalAnimState = 1;
						}
					}
					else
					{
						item->goalAnimState = 1;
					}
				}
				break;

			case 3:
				creature->LOT.isJumping = false;
				creature->maximumTurn = ANGLE(10);

				if (Targetable(item, &info)
					&& (info.distance < SQUARE(4096) 
						|| info.zoneNumber != info.enemyZone))
				{
					item->goalAnimState = 1;
					item->requiredAnimState = 38;
				}
				else if (canJump1block || canJump2blocks)
				{
					creature->maximumTurn = 0;
					item->animNumber = obj->animIndex + 22;
					item->currentAnimState = 15;
					item->frameNumber = Anims[item->animNumber].frameBase;
					if (canJump2blocks)
						item->goalAnimState = 16;
					creature->LOT.isJumping = true;
				}
				else
				{
					if (creature->monkeyAhead)
					{
						item->goalAnimState = 1;
					}
					else if (info.distance < SQUARE(3072))
						item->goalAnimState = 2;
				}
				break;

			case 4:
				creature->maximumTurn = 0;
				
				if (item->boxNumber == creature->LOT.targetBox 
					|| !creature->monkeyAhead)
				{
					floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
					height = GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);
					if (GetCeiling(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos) == height - 1536)
						item->goalAnimState = 1;
				}
				else
				{
					item->goalAnimState = 5;
				}
				break;

			case 5:
				creature->LOT.isMonkeying = true;
				creature->LOT.isJumping = true;
				creature->maximumTurn = ANGLE(5);
				
				if (item->boxNumber == creature->LOT.targetBox
					|| !creature->monkeyAhead)
				{
					floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
					height = GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);
					if (GetCeiling(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos) == height - 1536)
						LABEL_145:
					item->goalAnimState = 4;
				}
				break;

			case 38:
				joint0 = laraInfo.angle >> 1;
				joint2 = laraInfo.angle >> 1;
				creature->flags = 0;
				creature->maximumTurn = 0;

				if (info.ahead)
					joint1 = info.xAngle;

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

				if (Targetable(item, &info) 
					&& (info.distance < SQUARE(4096) 
						|| info.zoneNumber != info.enemyZone))
					item->goalAnimState = 39;
				else
					item->goalAnimState = 1;
				break;

			case 39:
				joint0 = laraInfo.angle >> 1;
				joint2 = laraInfo.angle >> 1;

				if (info.ahead)
					joint1 = info.xAngle;

				creature->maximumTurn = 0;

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

				if (item->frameNumber > Anims[item->animNumber].frameBase + 6
					&& item->frameNumber < Anims[item->animNumber].frameBase + 16
					&& ((byte)item->frameNumber - (byte)Anims[item->animNumber].frameBase) & 1)
				{
					item->firedWeapon = 1;
					ShotLara(item, &info, &CyborgGun, joint0, 12);
				}
				break;

			default:
				break;

			}
		}
		else if (item->currentAnimState == 43 && !Lara.burn)
		{
			PHD_VECTOR pos;
			pos.x = 0;			
			pos.y = 0;
			pos.z = 0;
			GetLaraJointPosition(&pos, LJ_LFOOT);
			
			short roomNumberLeft = LaraItem->roomNumber;
			GetFloor(pos.x, pos.y, pos.z, &roomNumberLeft);
			
			pos.x = 0;
			pos.y = 0;
			pos.z = 0; 
			GetLaraJointPosition(&pos, LJ_RFOOT);

			short roomNumberRight = LaraItem->roomNumber;
			GetFloor(pos.x, pos.y, pos.z, &roomNumberRight);

			ROOM_INFO* roomRight = &Rooms[roomNumberRight];
			ROOM_INFO* roomLeft = &Rooms[roomNumberLeft];

			short flipNumber = Rooms[item->roomNumber].flipNumber;

			if ((roomRight->flags | roomLeft->flags) & ENV_FLAG_WATER)
			{
				if (roomLeft->flipNumber == flipNumber || roomRight->flipNumber == flipNumber)
				{
					LaraBurn();
					Lara.BurnCount = 48;
					Lara.burnBlue = 1;
					LaraItem->hitPoints = 0;
				}
			}
		}

		CreatureJoint(item, 0, joint0);
		CreatureJoint(item, 1, joint1);
		CreatureJoint(item, 2, joint2);

		if (creature->reachedGoal)
		{
			if (creature->enemy)
			{
				roomNumber = creature->enemy->roomNumber;
				FLOOR_INFO* floor = GetFloor(
					creature->enemy->pos.xPos,
					creature->enemy->pos.yPos,
					creature->enemy->pos.zPos,
					&roomNumber);
				GetFloorHeight(floor, creature->enemy->pos.xPos, creature->enemy->pos.yPos, creature->enemy->pos.zPos);
				TestTriggers(TriggerIndex, 1, 0);
				
				item->requiredAnimState = 2;

				if (creature->enemy->flags & 2)
					item->itemFlags[3] = (item->TOSSPAD & 0xFF) - 1;

				if (creature->enemy->flags & 8)
				{
					item->requiredAnimState = 1;
					item->triggerFlags = 300;
					item->aiBits = GUARD | PATROL1;
				}
				
				item->itemFlags[3]++;
				creature->reachedGoal = false;
				creature->enemy = NULL;
			}
		}
		
		if (item->currentAnimState >= 15 || item->currentAnimState == 5)
		{
			CreatureAnimation(itemNumber, angle, 0);
		}
		else
		{
			switch (CreatureVault(itemNumber, angle, 2, 260) + 4)
			{
			case 0:
				creature->maximumTurn = 0;
				item->animNumber = obj->animIndex + 35;
				item->currentAnimState = 25;
				item->frameNumber = Anims[item->animNumber].frameBase;
				break;

			case 1:
				creature->maximumTurn = 0;
				item->animNumber = obj->animIndex + 41;
				item->currentAnimState = 24;
				item->frameNumber = Anims[item->animNumber].frameBase;
				break;

			case 2:
				creature->maximumTurn = 0;
				item->animNumber = obj->animIndex + 42;
				item->currentAnimState = 23;
				item->frameNumber = Anims[item->animNumber].frameBase;
				break;

			case 6:
				creature->maximumTurn = 0;
				item->animNumber = obj->animIndex + 29;
				item->currentAnimState = 19;
				item->frameNumber = Anims[item->animNumber].frameBase;
				break;

			case 7:
				creature->maximumTurn = 0;
				item->animNumber = obj->animIndex + 28;
				item->currentAnimState = 18;
				item->frameNumber = Anims[item->animNumber].frameBase;
				break;

			case 8:
				creature->maximumTurn = 0;
				item->animNumber = obj->animIndex + 27;
				item->currentAnimState = 17;
				item->frameNumber = Anims[item->animNumber].frameBase;
				break;

			default:
				return;
			}
		}
	}
}