#include "newobjects.h"
#include "..\Global\global.h"
#include "..\Game\Box.h"
#include "..\Game\items.h"
#include "..\Game\lot.h"
#include "..\Game\control.h"
#include "..\Game\effects.h"
#include "..\Game\draw.h"
#include "..\Game\sphere.h"
#include "..\Game\effect2.h"
#include "..\Game\people.h"
#include "..\Game\debris.h"

BITE_INFO skeletonBite = { 0, -16, 200, 11 };

void __cdecl InitialiseSkeleton(__int16 itemNum)
{

}

void __cdecl SkeletonControl(__int16 itemNum)
{
	bool someFlag1 = false;
	bool someFlag2 = false;

	if (!CreatureActive(itemNum))
		return;

	ITEM_INFO* item = &Items[itemNum];
	CREATURE_INFO* creature = (CREATURE_INFO*)item->data;
	ITEM_INFO* enemyItem = creature->enemy;

	__int16 tilt = 0;
	__int16 angle = 0;
	__int16 joint1 = 0;
	__int16 joint2 = 0;
	__int16 joint3 = 0;
	__int32 distance = 0;
	__int16 rot = 0;

	// Can skeleton jump? Check for a distance of 1 and 2 sectors
	__int32 x = item->pos.xPos;
	__int32 y = item->pos.yPos;
	__int32 z = item->pos.zPos;

	__int32 dx = 870 * SIN(item->pos.yRot) >> 14;
	__int32 dz = 870 * COS(item->pos.yRot) >> 14;

	x += dx;
	z += dz;

	__int16 roomNumber = item->roomNumber;
	FLOOR_INFO* floor = GetFloor(x, y, z, &roomNumber);
	__int32 height1 = GetFloorHeight(floor, x, y, z);

	x += dx;
	z += dz;

	roomNumber = item->roomNumber;
	floor = GetFloor(x, y, z, &roomNumber);
	__int32 height2 = GetFloorHeight(floor, x, y, z);

	x += dx;
	z += dz;

	roomNumber = item->roomNumber;
	floor = GetFloor(x, y, z, &roomNumber);
	__int32 height3 = GetFloorHeight(floor, x, y, z);

	__int32 height = 0;
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
	else if (!creature->enemy)
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

		item->frameNumber = Anims[item->animNumber].frameBase;
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
			laraInfo.angle = ATAN(dz, dx) - item->pos.yRot;
			laraInfo.distance = dx * dx + dz * dz;
		}

		GetCreatureMood(item, &info, 1);

		if (!(item->meshBits & 0x200))
			creature->mood = MOOD_TYPE::ESCAPE_MOOD;
		else
			CreatureMood(item, &info, 1);

		angle = CreatureTurn(item, creature->maximumTurn);

		creature->enemy = LaraItem;
		if (item->hitStatus || distance < SQUARE(1024) || TargetVisible(item, &laraInfo))
			creature->alerted = true;

		if (item != Lara.target || laraInfo.distance <= 870 || angle <= -10240 || angle >= 10240)
		{
			someFlag1 = 0;
			someFlag2 = 0;
		}
		else
		{
			dx = 870 * SIN(item->pos.yRot + ANGLE(45)) >> 14;
			dz = 870 * COS(item->pos.yRot + ANGLE(45)) >> 14;

			x = item->pos.xPos + dx;
			y = item->pos.yPos;
			z = item->pos.zPos + dz;

			roomNumber = item->roomNumber;
			floor = GetFloor(x, y, z, &roomNumber);
			__int32 height4 = GetFloorHeight(floor, x, y, z);

			dx = 870 * SIN(item->pos.yRot + 14336) >> 14;
			dz = 870 * COS(item->pos.yRot + 14336) >> 14;

			x = item->pos.xPos + dx;
			y = item->pos.yPos;
			z = item->pos.zPos + dz;

			roomNumber = item->roomNumber;
			floor = GetFloor(x, y, z, &roomNumber);
			__int32 height5 = GetFloorHeight(floor, x, y, z);

			if (abs(height5 - item->pos.yPos) > 256)
				someFlag2 = false;
			else
			{
				someFlag2 = true;
				if (height4 + 512 >= item->pos.yPos)
					someFlag2 = false;
			}

			dx = 870 * SIN(item->pos.yRot - 8192) >> 14;
			dz = 870 * COS(item->pos.yRot - 8192) >> 14;

			x = item->pos.xPos + dx;
			y = item->pos.yPos;
			z = item->pos.zPos + dz;

			roomNumber = item->roomNumber;
			floor = GetFloor(x, y, z, &roomNumber);
			__int32 height6 = GetFloorHeight(floor, x, y, z);

			dx = 870 * SIN(item->pos.yRot - 14336) >> 14;
			dz = 870 * COS(item->pos.yRot - 14336) >> 14;

			x = item->pos.xPos + dx;
			y = item->pos.yPos;
			z = item->pos.zPos + dz;

			roomNumber = item->roomNumber;
			floor = GetFloor(x, y, z, &roomNumber);
			__int32 height7 = GetFloorHeight(floor, x, y, z);

			if (abs(height7 - item->pos.yPos) > 256 || height6 + 512 >= item->pos.yPos)
				someFlag1 = false;
			else
				someFlag1 = true;

		}

		switch (item->currentAnimState)
		{
		case 1:
			if (!(GetRandomControl() & 0xF))
			{
				item->goalAnimState = 2;
			}
			break;

		case 2:
			creature->flags = 0;
			creature->LOT.isJumping = false;
			creature->maximumTurn = creature->mood != MOOD_TYPE::ESCAPE_MOOD ? ANGLE(2) : 0;
			if (item->aiBits & GUARD
				|| !(GetRandomControl() & 0x1F) && (info.distance > SQUARE(1024) || creature->mood != MOOD_TYPE::ATTACK_MOOD))
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
					item->frameNumber = Anims[item->animNumber].frameBase;
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
					item->frameNumber = Anims[item->animNumber].frameBase;
					item->goalAnimState = 19;
					item->currentAnimState = 19;
				}
				else if (someFlag2)
				{
					item->animNumber = Objects[ID_SKELETON].animIndex + 37;
					item->frameNumber = Anims[item->animNumber].frameBase;
					item->goalAnimState = 20;
					item->currentAnimState = 20;
				}
				else
				{
					if (creature->mood == MOOD_TYPE::ESCAPE_MOOD)
					{
						if (Lara.target == item || !info.ahead || item->hitStatus || !(item->meshBits & 0x200))
						{
							item->goalAnimState = 15;
							break;
						}
						item->goalAnimState = 2;
					}
					else if (creature->mood != MOOD_TYPE::BORED_MOOD ||
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
			creature->maximumTurn = creature->mood != MOOD_TYPE::BORED_MOOD ? 1092 : 364;
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
				if (creature->mood == MOOD_TYPE::ESCAPE_MOOD)
				{
					item->goalAnimState = 16;
				}
				else if (creature->mood != MOOD_TYPE::BORED_MOOD)
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
					item->frameNumber = Anims[item->animNumber].frameBase;
					creature->LOT.isJumping = false;
					item->gravityStatus = true;
				}
			}
			else
			{
				if (creature->mood == MOOD_TYPE::ESCAPE_MOOD)
				{
					if (Lara.target != item && info.ahead && (item->meshBits & 0x200))
						item->goalAnimState = 2;
				}
				else if (item->aiBits & FOLLOW && (creature->reachedGoal || laraInfo.distance > SQUARE(2048)))
				{
					item->goalAnimState = 2;
				}
				else if (creature->mood != MOOD_TYPE::BORED_MOOD)
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
					SoundEffect(SFX_LARA_THUD, &item->pos, 0);
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
			if (item->frameNumber > Anims[item->animNumber].frameBase + 15)
			{
				ROOM_INFO* room = &Rooms[item->roomNumber];
				PHD_VECTOR pos;
				
				GetJointAbsPosition(item, &pos, 16);

				FLOOR_INFO* floor = &room->floor[((z - room->z) >> 10) + room->ySize * ((x - room->x) >> 10)];
				if (floor->stopper)
				{
					MESH_INFO* staticMesh = room->mesh;
					if (room->numMeshes > 0)
					{
						for (__int32 i = 0; i < room->numMeshes; i++)
						{
							staticMesh = &room->mesh[i];
							if (abs(pos.x - staticMesh->x) < 1024 && abs(pos.z - staticMesh->z) < 1024 && staticMesh->staticNumber >= 50)
							{
								ShatterObject(0, staticMesh, -128, LaraItem->roomNumber, 0);
								SoundEffect(SFX_TR4_HIT_ROCK_ID347, &item->pos, 0);
								staticMesh->Flags &= ~1;
								floor->stopper = 0;
								GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);
								TestTriggers(TriggerIndex, 1, 0);
							}
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
						
						SoundEffect(SFX_LARA_THUD, &item->pos, 0);
						
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
					item->frameNumber = Anims[item->animNumber].frameBase;
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
					ExplodingDeath(itemNum, -1, 929);
					KillItem(itemNum);
					DisableBaddieAI(itemNum);
					//Savegame.Kills++;
				}
			}
			break;

		case 25:
		case 11:
		case 12:
		case 13:
			if ((item->currentAnimState == 12 || item->currentAnimState == 13) && 
				item->frameNumber < Anims[item->animNumber].frameBase + 20)
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
				item->frameNumber = Anims[item->animNumber].frameBase;
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
			if (item->frameNumber - Anims[item->animNumber].frameBase < 32)
			{
				//WakeUpSkeleton(item);
			}
			break;

		default:
			break;
		}

		CreatureAnimation(itemNum, angle, 0);
	}
}