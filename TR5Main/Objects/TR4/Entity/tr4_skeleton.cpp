#include "framework.h"
#include "tr4_skeleton.h"
#include "items.h"
#include "box.h"
#include "people.h"
#include "effect.h"
#include "sphere.h"
#include "debris.h"
#include "effect2.h"
#include "lot.h"
#include "lara.h"
#include "sound.h"
#include "setup.h"
#include "tomb4fx.h"
#include "level.h"

#define STATE_SKELETON_UNDER_FLOOR		0
#define STATE_SKELETON_STOP				1
#define STATE_SKELETON_WALK				2
#define STATE_SKELETON_RUN				3
#define STATE_SKELETON_JUMP_LEFT		19
#define STATE_SKELETON_JUMP_RIGHT		20
#define STATE_SKELETON_JUMP_LIE_DOWN	25

BITE_INFO skeletonBite = { 0, -16, 200, 11 };

void WakeUpSkeleton(ITEM_INFO* item)
{
	short fxNum = CreateNewEffect(item->roomNumber);
	if (fxNum != NO_ITEM)
	{
		FX_INFO* fx = &EffectList[fxNum];

		short roomNumber = item->roomNumber;
		FLOOR_INFO* floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);

		fx->pos.xPos = (byte)GetRandomControl() + item->pos.xPos - 128;
		fx->pos.yPos = GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);
		fx->pos.zPos = (byte)GetRandomControl() + item->pos.zPos - 128;
		fx->roomNumber = item->roomNumber;
		fx->pos.yRot = 2 * GetRandomControl();
		fx->speed = GetRandomControl() / 2048;
		fx->fallspeed = -(GetRandomControl() / 1024);
		fx->frameNumber = Objects[103].meshIndex;
		fx->objectNumber = ID_BODY_PART;
		fx->shade = 0x4210;
		fx->flag2 = 0x601;

		SPARKS* spark = &Sparks[GetFreeSpark()];
		spark->on = 1;
		spark->sR = 0;
		spark->sG = 0;
		spark->sB = 0;
		spark->dR = 100;
		spark->dG = 60;
		spark->dB = 30;
		spark->fadeToBlack = 8;
		spark->colFadeSpeed = (GetRandomControl() & 3) + 4;
		spark->life = spark->sLife = (GetRandomControl() & 7) + 16;
		spark->x = fx->pos.xPos;
		spark->y = fx->pos.yPos;
		spark->z = fx->pos.zPos;
		spark->xVel = phd_sin(fx->pos.yRot) * 4096;
		spark->yVel = 0;
		spark->zVel = phd_cos(fx->pos.yRot) * 4096;
		spark->transType = COLADD;
		spark->friction = 68;
		spark->flags = 26;
		spark->rotAng = GetRandomControl() & 0xFFF;
		if (GetRandomControl() & 1)
		{
			spark->rotAdd = -16 - (GetRandomControl() & 0xF);
		}
		else
		{
			spark->rotAdd = (GetRandomControl() & 0xF) + 16;
		}
		spark->gravity = -4 - (GetRandomControl() & 3);
		spark->scalar = 3;
		spark->maxYvel = -4 - (GetRandomControl() & 3);
		spark->sSize = spark->size = (GetRandomControl() & 0xF) + 8;
		spark->dSize = spark->size * 4;
	}
}

void InitialiseSkeleton(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];
	OBJECT_INFO* obj = &Objects[item->objectNumber];

	ClearItem(itemNumber);

	switch (item->triggerFlags)
	{
	case 0:
		item->goalAnimState = 0;
		item->currentAnimState = 0;
		item->animNumber = obj->animIndex;
		item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
		break;

	case 1:
		item->goalAnimState = STATE_SKELETON_JUMP_RIGHT;
		item->currentAnimState = STATE_SKELETON_JUMP_RIGHT;
		item->animNumber = obj->animIndex + 37;
		item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
		break;

	case 2:
		item->goalAnimState = STATE_SKELETON_JUMP_LEFT;
		item->currentAnimState = STATE_SKELETON_JUMP_LEFT;
		item->animNumber = obj->animIndex + 34;
		item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
		break;

	case 3:
		item->goalAnimState = STATE_SKELETON_JUMP_LIE_DOWN;
		item->currentAnimState = STATE_SKELETON_JUMP_LIE_DOWN;
		item->animNumber = obj->animIndex;
		item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
		//item->status = ITEM_DEACTIVATED;
		break;

	}
}

void SkeletonControl(short itemNumber)
{
	if (!CreatureActive(itemNumber))
		return;

	ITEM_INFO* item = &g_Level.Items[itemNumber];
	CREATURE_INFO* creature = (CREATURE_INFO*)item->data;
	ITEM_INFO* enemyItem = creature->enemy;
	bool someFlag1 = false;
	bool someFlag2 = false;
	short tilt = 0;
	short angle = 0;
	short joint1 = 0;
	short joint2 = 0;
	short joint3 = 0;
	int distance = 0;
	short rot = 0;

	// Can skeleton jump? Check for a distance of 1 and 2 sectors
	int x = item->pos.xPos;
	int y = item->pos.yPos;
	int z = item->pos.zPos;

	int dx = 870 * phd_sin(item->pos.yRot);
	int dz = 870 * phd_cos(item->pos.yRot);

	x += dx;
	z += dz;

	short roomNumber = item->roomNumber;
	FLOOR_INFO* floor = GetFloor(x, y, z, &roomNumber);
	int height1 = GetFloorHeight(floor, x, y, z);

	x += dx;
	z += dz;

	roomNumber = item->roomNumber;
	floor = GetFloor(x, y, z, &roomNumber);
	int height2 = GetFloorHeight(floor, x, y, z);

	x += dx;
	z += dz;

	roomNumber = item->roomNumber;
	floor = GetFloor(x, y, z, &roomNumber);
	int height3 = GetFloorHeight(floor, x, y, z);

	int height = 0;
	bool canJump1sector = true;
	if (enemyItem && item->boxNumber == LaraItem->boxNumber && item->meshBits & 0x200
		|| y >= height1 - 384
		|| y >= height2 + 256
		|| y <= height2 - 256)
	{
		height = height2;
		canJump1sector = false;
	}

	bool canJump2sectors = true;
	if (enemyItem && item->boxNumber == LaraItem->boxNumber && item->meshBits & 0x200
		|| y >= height1 - 384
		|| y >= height - 384
		|| y >= height3 + 256
		|| y <= height3 - 256)
	{
		canJump2sectors = false;
	}

	if (item->aiBits)
		GetAITarget(creature);
	else
		creature->enemy = LaraItem;

	AI_INFO info;
	AI_INFO laraInfo;
	CreatureAIInfo(item, &info);

	if (item->hitStatus
		&& Lara.gunType == WEAPON_SHOTGUN
		&& info.distance < SQUARE(3584)
		&& item->currentAnimState != 7
		&& item->currentAnimState != 17
		&& item->currentAnimState != 12
		&& item->currentAnimState != 13
		&& item->currentAnimState != 25)
	{
		if (info.angle >= 12288 || info.angle <= -12288)
		{
			item->currentAnimState = 13;
			item->animNumber = Objects[ID_SKELETON].animIndex + 33;
			item->pos.yRot += info.angle + -32768;
		}
		else
		{
			item->currentAnimState = 12;
			item->animNumber = Objects[ID_SKELETON].animIndex + 17;
			item->pos.yRot += info.angle;
		}

		item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
		creature->LOT.isJumping = true;
		item->hitPoints = 25;
	}
	else
	{
		if (creature->enemy == LaraItem)
		{
			laraInfo.distance = info.distance;
			laraInfo.angle = info.angle;
		}
		else
		{
			dx = LaraItem->pos.xPos - item->pos.xPos;
			dz = LaraItem->pos.zPos - item->pos.zPos;
			laraInfo.angle = phd_atan(dz, dx) - item->pos.yRot;
			laraInfo.distance = SQUARE(dx) + SQUARE(dz);
		}

		GetCreatureMood(item, &info, VIOLENT);

		if (!(item->meshBits & 0x200))
			creature->mood = ESCAPE_MOOD;
		
		CreatureMood(item, &info, VIOLENT);

		angle = CreatureTurn(item, creature->maximumTurn);

		ITEM_INFO* tempEnemy = creature->enemy;
		creature->enemy = LaraItem;
		if (item->hitStatus 
			|| distance < SQUARE(1024) 
			|| TargetVisible(item, &laraInfo))
			creature->alerted = true;
		creature->enemy = tempEnemy;

		if (item != Lara.target || laraInfo.distance <= 870 || angle <= -10240 || angle >= 10240)
		{
			someFlag1 = false;
			someFlag2 = false;
		}
		else
		{
			dx = 870 * phd_sin(item->pos.yRot + ANGLE(45));
			dz = 870 * phd_cos(item->pos.yRot + ANGLE(45));

			x = item->pos.xPos + dx;
			y = item->pos.yPos;
			z = item->pos.zPos + dz;

			roomNumber = item->roomNumber;
			floor = GetFloor(x, y, z, &roomNumber);
			int height4 = GetFloorHeight(floor, x, y, z);

			dx = 870 * phd_sin(item->pos.yRot + 14336);
			dz = 870 * phd_cos(item->pos.yRot + 14336);

			x = item->pos.xPos + dx;
			y = item->pos.yPos;
			z = item->pos.zPos + dz;

			roomNumber = item->roomNumber;
			floor = GetFloor(x, y, z, &roomNumber);
			int height5 = GetFloorHeight(floor, x, y, z);

			if (abs(height5 - item->pos.yPos) > 256)
				someFlag2 = false;
			else
			{
				someFlag2 = true;
				if (height4 + 512 >= item->pos.yPos)
					someFlag2 = false;
			}

			dx = 870 * phd_sin(item->pos.yRot - 8192);
			dz = 870 * phd_cos(item->pos.yRot - 8192);

			x = item->pos.xPos + dx;
			y = item->pos.yPos;
			z = item->pos.zPos + dz;

			roomNumber = item->roomNumber;
			floor = GetFloor(x, y, z, &roomNumber);
			int height6 = GetFloorHeight(floor, x, y, z);

			dx = 870 * phd_sin(item->pos.yRot - 14336);
			dz = 870 * phd_cos(item->pos.yRot - 14336);

			x = item->pos.xPos + dx;
			y = item->pos.yPos;
			z = item->pos.zPos + dz;

			roomNumber = item->roomNumber;
			floor = GetFloor(x, y, z, &roomNumber);
			int height7 = GetFloorHeight(floor, x, y, z);

			if (abs(height7 - item->pos.yPos) > 256 || height6 + 512 >= item->pos.yPos)
				someFlag1 = false;
			else
				someFlag1 = true;

		}

		switch (item->currentAnimState)
		{
		case STATE_SKELETON_STOP:
			if (!(GetRandomControl() & 0xF))
			{
				item->goalAnimState = 2;
			}
			break;

		case 2:
			creature->flags = 0;
			creature->LOT.isJumping = false;
			creature->maximumTurn = creature->mood != ESCAPE_MOOD ? ANGLE(2) : 0;
			if (item->aiBits & GUARD
				|| !(GetRandomControl() & 0x1F) && (info.distance > SQUARE(1024) || creature->mood != ATTACK_MOOD))
			{
				if (!(GetRandomControl() & 0x3F))
				{
					if (GetRandomControl() & 1)
					{
						item->goalAnimState = 3;
					}
					else
					{
						item->goalAnimState = 4;
					}
				}
			}
			else
			{
				if (item->aiBits & PATROL1)
				{
					item->goalAnimState = 15;
				}
				else if (canJump1sector || canJump2sectors)
				{
					creature->maximumTurn = 0;
					item->animNumber = Objects[ID_SKELETON].animIndex + 40;
					item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
					item->currentAnimState = 21;
					if (!canJump2sectors)
					{
						item->goalAnimState = 21;
						creature->LOT.isJumping = true;
					}
					else
					{
						item->goalAnimState = 22;
						creature->LOT.isJumping = true;
					}
				}
				else if (someFlag1)
				{
					item->animNumber = Objects[ID_SKELETON].animIndex + 34;
					item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
					item->goalAnimState = 19;
					item->currentAnimState = 19;
				}
				else if (someFlag2)
				{
					item->animNumber = Objects[ID_SKELETON].animIndex + 37;
					item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
					item->goalAnimState = 20;
					item->currentAnimState = 20;
				}
				else
				{
					if (creature->mood == ESCAPE_MOOD)
					{
						if (Lara.target == item || !info.ahead || item->hitStatus || !(item->meshBits & 0x200))
						{
							item->goalAnimState = 15;
							break;
						}
						item->goalAnimState = 2;
					}
					else if (creature->mood != BORED_MOOD ||
						item->aiBits & FOLLOW &&
						(creature->reachedGoal ||
							laraInfo.distance > SQUARE(2048)))
					{
						if (item->requiredAnimState)
						{
							item->goalAnimState = item->requiredAnimState;
						}
						else if (!(GetRandomControl() & 0x3F))
						{
							item->goalAnimState = 15;
						}
					}
					else if (Lara.target == item
						&& laraInfo.angle
						&& laraInfo.distance < SQUARE(2048)
						&& GetRandomControl() & 1
						&& (Lara.gunType == WEAPON_SHOTGUN || !(GetRandomControl() & 0xF))
						&& item->meshBits == -1)
					{
						item->goalAnimState = 7;
					}
					else if (info.bite && info.distance < SQUARE(682))
					{
						if (GetRandomControl() & 3 && LaraItem->hitPoints > 0)
						{
							if (GetRandomControl() & 1)
							{
								item->goalAnimState = 8;
							}
							else
							{
								item->goalAnimState = 9;
							}
						}
						else
						{
							item->goalAnimState = 10;
						}
					}
					else if (item->hitStatus || item->requiredAnimState)
					{
						if (GetRandomControl() & 1)
						{
							item->goalAnimState = 5;
							item->requiredAnimState = item->goalAnimState;
						}
						else
						{
							item->goalAnimState = 6;
							item->requiredAnimState = item->goalAnimState;
						}
					}
					else
					{
						item->goalAnimState = 15;
					}
				}
			}
			break;

		case 15:
			creature->flags = 0;
			creature->LOT.isJumping = false;
			creature->maximumTurn = creature->mood != BORED_MOOD ? 1092 : 364;
			if (item->aiBits & PATROL1)
			{
				item->goalAnimState = 15;
			}
			else if (item->hitStatus)
			{
				item->goalAnimState = 2;
				if (GetRandomControl() & 1)
				{
					item->requiredAnimState = 5;
				}
				else
				{
					item->requiredAnimState = 6;
				}
			}
			else
			{
				if (someFlag1 || someFlag2)
				{
					item->goalAnimState = 2;
					break;
				}
				if (creature->mood == ESCAPE_MOOD)
				{
					item->goalAnimState = 16;
				}
				else if (creature->mood != BORED_MOOD)
				{
					if (info.distance >= SQUARE(682))
					{
						if (info.bite && info.distance < SQUARE(1024))
						{
							item->goalAnimState = 18;
						}
						else if (canJump1sector || canJump2sectors)
						{
							creature->maximumTurn = 0;
							item->goalAnimState = 2;
						}
						else if (!info.ahead || info.distance > SQUARE(2048))
						{
							item->goalAnimState = 16;
						}
					}
					else
					{
						item->goalAnimState = 2;
					}
				}
				else if (!(GetRandomControl() & 0x3F))
				{
					item->goalAnimState = 2;
				}
			}
			break;

		case 16:
			creature->maximumTurn = 1274;
			creature->LOT.isJumping = false;

			if (item->aiBits & GUARD || canJump1sector || canJump2sectors)
			{
				if (item->meshBits & 0x200)
				{
					creature->maximumTurn = 0;
					item->goalAnimState = 2;
					break;
				}

				creature->LOT.isJumping = true;
				floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
				if (GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos) > item->pos.yPos + 1024)
				{
					creature->maximumTurn = 0;
					item->animNumber = Objects[ID_SKELETON].animIndex + 44;
					item->currentAnimState = 23;
					item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
					creature->LOT.isJumping = false;
					item->gravityStatus = true;
				}
			}
			else
			{
				if (creature->mood == ESCAPE_MOOD)
				{
					if (Lara.target != item && info.ahead && (item->meshBits & 0x200))
						item->goalAnimState = 2;
				}
				else if (item->aiBits & FOLLOW && (creature->reachedGoal || laraInfo.distance > SQUARE(2048)))
				{
					item->goalAnimState = 2;
				}
				else if (creature->mood != BORED_MOOD)
				{
					if (info.ahead && info.distance < SQUARE(2048))
					{
						item->goalAnimState = 15;
					}
				}
				else
				{
					item->goalAnimState = 15;
				}
			}
			break;

		case 10:
			creature->maximumTurn = 0;
			if (abs(info.angle) >= 1092)
			{
				if (info.angle >= 0)
				{
					item->pos.yRot += 1092;
				}
				else
				{
					item->pos.yRot -= 1092;
				}
			}
			else
			{
				item->pos.yRot += info.angle;
			}

			if (!creature->flags)
			{
				if (item->touchBits & 0x18000)
				{
					LaraItem->hitPoints -= 80;
					LaraItem->hitStatus = true;
					CreatureEffect2(item, &skeletonBite, 15, -1, DoBloodSplat);
					SoundEffect(SFX_TR4_LARA_THUD, &item->pos, 0);
					creature->flags = 1;
				}
			}
			if (!(GetRandomControl() & 0x3F) || LaraItem->hitPoints <= 0)
			{
				item->goalAnimState = 11;
			}
			break;

		case 8:
		case 9:
		case 18:
			creature->maximumTurn = 0;
			if (abs(info.angle) >= 1092)
			{
				if (info.angle >= 0)
				{
					item->pos.yRot += 1092;
				}
				else
				{
					item->pos.yRot -= 1092;
				}
			}
			else
			{
				item->pos.yRot += info.angle;
			}
			if (item->frameNumber > g_Level.Anims[item->animNumber].frameBase + 15)
			{
				ROOM_INFO* room = &g_Level.Rooms[item->roomNumber];
				PHD_VECTOR pos;
				
				GetJointAbsPosition(item, &pos, 16);

				FLOOR_INFO* floor = &room->floor[((z - room->z) >> 10) + room->ySize * ((x - room->x) >> 10)];
				if (floor->stopper)
				{
					for (int i = 0; i < room->mesh.size(); i++)
					{
						MESH_INFO* staticMesh = &room->mesh[i];
						if (abs(pos.x - staticMesh->x) < 1024 && abs(pos.z - staticMesh->z) < 1024 && staticMesh->staticNumber >= 50)
						{
							ShatterObject(0, staticMesh, -128, LaraItem->roomNumber, 0);
							SoundEffect(SFX_TR4_HIT_ROCK, &item->pos, 0);
							staticMesh->flags &= ~1;
							floor->stopper = 0;
							GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);
							TestTriggers(TriggerIndex, 1, 0);
						}
					}
				}
				if (!creature->flags)
				{
					if (item->touchBits & 0x18000)
					{
						LaraItem->hitPoints -= 80;
						LaraItem->hitStatus = true;
						CreatureEffect2(item, &skeletonBite, 10, item->pos.yRot, DoBloodSplat);
						
						SoundEffect(SFX_TR4_LARA_THUD, &item->pos, 0);
						
						creature->flags = 1;
					}
				}
			}
			break;

		case 7:
			if (item->hitStatus)
			{
				if (item->meshBits == -1 && laraInfo.angle && Lara.gunType == WEAPON_SHOTGUN)
				{
					if (GetRandomControl() & 3)
					{
						item->goalAnimState = 17;
					}
					else
					{
						ExplodeItemNode(item, 11, 1, -24);
					}
				}
				else
				{
				LABEL_153:
					item->goalAnimState = 2;
				}
			}
			else if (Lara.target != item || item->meshBits != -1 || Lara.gunType != WEAPON_SHOTGUN || !(GetRandomControl() & 0x7F))
			{
				item->goalAnimState = 2;
			}
			break;

		case 21:
			if (item->animNumber == Objects[item->objectNumber].animIndex + 43)
			{
				roomNumber = item->roomNumber;
				floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
				if (GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos) > item->pos.yPos + 1280)
				{
					creature->maximumTurn = 0;
					item->animNumber = Objects[item->objectNumber].animIndex + 44;
					item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
					item->currentAnimState = 23;
					creature->LOT.isJumping = false;
					item->gravityStatus = true;
				}
			}
			break;

		case 23:
		case 24:
			roomNumber = item->roomNumber;
			floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
			if (GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos) <= item->pos.yPos)
			{
				if (item->active)
				{
					ExplodingDeath(itemNumber, -1, 929);
					KillItem(itemNumber);
					DisableBaddieAI(itemNumber);
					//Savegame.Kills++;
				}
			}
			break;

		case 25:
		case 11:
		case 12:
		case 13:
			if ((item->currentAnimState == 12 || item->currentAnimState == 13) && 
				item->frameNumber < g_Level.Anims[item->animNumber].frameBase + 20)
			{
				item->hitPoints = 25;
				creature->maximumTurn = 0;
				break;
			}
			if (item->currentAnimState == 11)
			{
				creature->maximumTurn = 0;
				break;
			}

			item->hitPoints = 25;
			creature->LOT.isJumping = false;
			
			roomNumber = item->roomNumber;
			floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
			if (GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos) <= item->pos.yPos + 1024)
			{
				if (!(GetRandomControl() & 0x1F))
				{
					item->goalAnimState = 14;
				}
			}
			else
			{
				creature->maximumTurn = 0;
				item->animNumber = Objects[item->objectNumber].animIndex + 47;
				item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
				item->currentAnimState = 24;
				item->gravityStatus = true;
			}
			break;

		case 19:
		case 20:
			creature->alerted = false;
			creature->maximumTurn = 0;
			item->status = ITEM_ACTIVE;
			break;

		case 0:
			if (item->frameNumber - g_Level.Anims[item->animNumber].frameBase < 32)
			{
				WakeUpSkeleton(item);
			}
			break;

		default:
			break;
		}

		CreatureAnimation(itemNumber, angle, 0);
	}
}