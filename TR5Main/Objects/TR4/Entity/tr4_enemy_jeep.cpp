#include "framework.h"
#include "tr4_enemy_jeep.h"
#include "Game/items.h"
#include "Specific/level.h"
#include "Game/control/box.h"
#include "Specific/trmath.h"
#include "Game/control/control.h"
#include "Game/Lara/lara.h"
#include "Game/control/lot.h"
#include "Sound/sound.h"
#include "Game/animation.h"
#include "Game/itemdata/creature_info.h"
#include "Specific/setup.h"
#include "Game/control/trigger.h"
#include "Game/effects/effects.h"
#include "Game/effects/tomb4fx.h"

void EnemyJeepLaunchGrenade(ITEM_INFO* item)
{
	short grenadeItemNumber = CreateItem();

	if (grenadeItemNumber != NO_ITEM)
	{
		ITEM_INFO* grenadeItem = &g_Level.Items[grenadeItemNumber];

		grenadeItem->shade = -15856;
		grenadeItem->objectNumber = ID_GRENADE;
		grenadeItem->roomNumber = item->roomNumber;

		InitialiseItem(grenadeItemNumber);

		grenadeItem->pos.xRot = item->pos.xRot;
		grenadeItem->pos.yRot = item->pos.yRot + -ANGLE(180);
		grenadeItem->pos.zRot = 0;

		grenadeItem->pos.xPos = item->pos.xPos + 1024 * phd_sin(grenadeItem->pos.yRot);
		grenadeItem->pos.yPos = item->pos.yPos - 768;
		grenadeItem->pos.zPos = item->pos.xPos + 1024 * phd_cos(grenadeItem->pos.yRot);

		SmokeCountL = 32;
		SmokeWeapon = 5;

		for (int i = 0; i < 5; i++)
		{
			TriggerGunSmoke(item->pos.xPos, item->pos.yPos, item->pos.zPos, 0, 0, 0, 1, 5, 32);
		}

		if (GetRandomControl() & 3)
		{
			grenadeItem->itemFlags[0] = 1;
		}
		else
		{
			grenadeItem->itemFlags[0] = 2;
		}

		grenadeItem->Velocity = 32;
		grenadeItem->activeState = grenadeItem->pos.xRot;
		grenadeItem->VerticalVelocity = -32 * phd_sin(grenadeItem->pos.xRot);
		grenadeItem->targetState = grenadeItem->pos.yRot;
		grenadeItem->requiredState = 0;
		grenadeItem->hitPoints = 120;

		AddActiveItem(grenadeItemNumber);
	}
}

void InitialiseEnemyJeep(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	item->itemFlags[0] = -80;

	if (g_Level.NumItems > 0)
	{
		for (int i = 0; i < g_Level.NumItems; i++)
		{
			ITEM_INFO* other = &g_Level.Items[i];

			if (other == item || other->triggerFlags != item->triggerFlags)
				continue;

			item->itemFlags[1] = i;
			other->itemFlags[0] = -80;
			other->pos.yPos = item->pos.yPos - 1024;
		}
	}
}

void EnemyJeepControl(short itemNumber)
{
	if (CreatureActive(itemNumber))
	{
		ITEM_INFO* item = &g_Level.Items[itemNumber];
		CREATURE_INFO* creature = (CREATURE_INFO*)item->data;

		int x = item->pos.xPos;
		int y = item->pos.yPos;
		int z = item->pos.zPos;

		int dx = 682 * phd_sin(item->pos.yRot);
		int dz = 682 * phd_cos(item->pos.yRot);

		short roomNumber = item->roomNumber;
		FLOOR_INFO* floor = GetFloor(x - dz, y, z - dx, &roomNumber);
		int height1 = GetFloorHeight(floor, x - dz, y, z - dx);
		if (abs(item->pos.yPos - height1) > 768)
		{
			item->pos.yRot += ANGLE(2);
			item->pos.xPos += (dz / 64);
			item->pos.zPos += (dx / 64);
			height1 = y;
		}

		roomNumber = item->roomNumber;
		floor = GetFloor(x + dz, y, z - dx, &roomNumber);
		int height2 = GetFloorHeight(floor, x + dz, y, z - dx);
		if (abs(item->pos.yPos - height2) > 768)
		{
			item->pos.yRot -= ANGLE(2);
			item->pos.xPos -= (dz / 64);
			item->pos.zPos += (dx / 64);
			height2 = y;
		}

		short zRot = phd_atan(1364, height2 - height1);

		roomNumber = item->roomNumber;
		floor = GetFloor(x + dx, y, z + dz, &roomNumber);
		int height3 = GetFloorHeight(floor, x + dx, y, z + dz);
		if (abs(y - height3) > 768)
		{
			height3 = y;
		}

		roomNumber = item->roomNumber;
		floor = GetFloor(x - dx, y, z - dz, &roomNumber);
		int height4 = GetFloorHeight(floor, x - dx, y, z - dz);
		if (abs(y - height4) > 768)
		{
			height4 = y;
		}

		short xRot = phd_atan(1364, height4 - height3);

		AI_INFO info;
		CreatureAIInfo(item, &info);

		creature->enemy = creature->aiTarget;
		ITEM_INFO* target = creature->aiTarget;
		short angle;
		int distance;
		{
			dx = LaraItem->pos.xPos - item->pos.xPos;
			dz = LaraItem->pos.zPos - item->pos.zPos;
			angle = phd_atan(dz, dx) - item->pos.yRot;
			if (dx > 32000 || dx < -32000 || dz > 32000 || dz < -32000)
				distance = 0x7FFFFFFF;
			else
				distance = SQUARE(dx) + SQUARE(dz);
		}

		PHD_VECTOR pos;

		switch (item->activeState)
		{
		case 0:
		case 2:
			item->itemFlags[0] -= 128;
			if (item->itemFlags[0] < 0)
				item->itemFlags[0] = 0;

			item->meshBits = -98305;
			
			pos.x = 0;
			pos.y = -144;
			pos.z = -1024;
			GetJointAbsPosition(item, &pos, 11);
			TriggerDynamicLight(pos.x, pos.y, pos.z, 10, 64, 0, 0);
			
			if (item->requiredState)
				item->targetState = item->requiredState;
			else if (info.distance > SQUARE(1024) || Lara.location >= item->itemFlags[3])
				item->targetState = 1;

			break;

		case 1:
			creature->maximumTurn = item->itemFlags[0] / 16;
			item->itemFlags[0] += 37;
			if (item->itemFlags[0] > 8704)
				item->itemFlags[0] = 8704;

			item->meshBits = -147457;

			if (info.angle <= 256)
			{
				if (info.angle < -256)
				{
					item->targetState = 3;
				}
			}
			else
			{
				item->targetState = 4;
			}

			break;

		case 3:
		case 4:
			item->itemFlags[0] += 18;
			if (item->itemFlags[0] > 8704)
				item->itemFlags[0] = 8704;
			item->targetState = 1;

			break;

		case 5:
			if (item->itemFlags[0] < 1184)
				item->itemFlags[0] = 1184;

			break;

		default:
			break;
		}

		if (height3 <= item->floor + 512)
		{
			if (height4 > item->floor + 512 && item->activeState != 5)
			{
				item->itemFlags[1] = 0;
				item->animNumber = Objects[item->objectNumber].animIndex + 8;
				item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
				item->activeState = 5;
				item->targetState = 1;
			}
		}
		else
		{
			creature->LOT.requiredBox |= 8;
			if (item->itemFlags[1] > 0)
			{
				item->itemFlags[1] -= 8;
				if (item->itemFlags[1]<0)
					creature->LOT.requiredBox &= ~8;
				item->pos.yPos += item->itemFlags[1] / 64;
			}
			else
			{
				item->itemFlags[1] = 2 * xRot;
				creature->LOT.requiredBox |= 8u;
			}
			if (creature->LOT.requiredBox & 8)
			{
				creature->maximumTurn = 0;
				item->targetState = 1;
			}
		}

		if (info.distance < SQUARE(1536) || item->itemFlags[3] == -2)
			creature->reachedGoal = true;

		if (creature->reachedGoal)
		{
			TestTriggers(target->pos.xPos,target->pos.yPos,target->pos.zPos,target->roomNumber, true);

			if (Lara.location < item->itemFlags[3] && item->activeState != 2 && item->targetState != 2)
			{
				item->animNumber = Objects[item->objectNumber].animIndex + 1;
				item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
				item->targetState = 2;
				item->activeState = 2;

				if (target->flags & 4)
				{
					item->pos.xPos = target->pos.xPos;
					item->pos.yPos = target->pos.yPos;
					item->pos.zPos = target->pos.zPos;
					item->pos.xRot = target->pos.xRot;
					item->pos.yRot = target->pos.yRot;
					item->pos.zRot = target->pos.zRot;

					if (item->roomNumber != target->roomNumber)
						ItemNewRoom(itemNumber, target->roomNumber);
				}
			}

			if (distance > SQUARE(2048) && distance < SQUARE(10240) && !item->itemFlags[2] && (angle < -20480 || angle > 20480))
			{
				EnemyJeepLaunchGrenade(item);
				item->itemFlags[2] = 150;
			}

			if (target->flags == 62)
			{
				item->status = ITEM_INVISIBLE;
				RemoveActiveItem(itemNumber);
				DisableBaddieAI(itemNumber);
			}

			if (Lara.location >= item->itemFlags[3] || !(target->flags & 4))
			{
				creature->reachedGoal = false;
				item->itemFlags[3]++;

				creature->enemy = nullptr;
				AI_OBJECT* aiObject = nullptr;

				for (int i = 0; i < g_Level.AIObjects.size(); i++)
				{ 
					aiObject = &g_Level.AIObjects[i];

					if (g_Level.AIObjects[i].triggerFlags == item->itemFlags[3] && g_Level.AIObjects[i].roomNumber != NO_ROOM)
					{
						aiObject = &g_Level.AIObjects[i];
						break;
					}
				}

				if (aiObject != nullptr)
				{
					creature->enemy = nullptr;
					target->objectNumber = aiObject->objectNumber;
					target->roomNumber = aiObject->roomNumber;
					target->pos.xPos = aiObject->x;
					target->pos.yPos = aiObject->y;
					target->pos.zPos = aiObject->z;
					target->pos.yRot = aiObject->yRot;
					target->flags = aiObject->flags;
					target->triggerFlags = aiObject->triggerFlags;
					target->boxNumber = aiObject->boxNumber;
					if (!(aiObject->flags & 0x20))
					{
						target->pos.xPos += 256 * phd_sin(target->pos.yRot);
						target->pos.zPos += 256 * phd_cos(target->pos.yRot);
					}
				}
			}
		}

		item->itemFlags[2]--;
		if (item->itemFlags[2] < 0)
			item->itemFlags[2] = 0;

		if (abs(xRot - item->pos.xRot) < 256)
			item->pos.xRot = xRot;
		else if (xRot < item->pos.xRot)
			item->pos.xRot -= 256;
		else 
			item->pos.xRot += 256;

		if (abs(zRot - item->pos.zRot) < 256)
			item->pos.zRot = zRot;
		else if (zRot < item->pos.zRot)
			item->pos.zRot -= 256;
		else
			item->pos.zRot += 256;

		item->itemFlags[0] += -2 - xRot / 512;
		if (item->itemFlags[0] < 0)
			item->itemFlags[0] = 0;

		dx = item->itemFlags[0] * phd_sin(-2 - xRot / 512);
		dz = item->itemFlags[0] * phd_cos(-2 - xRot / 512);

		item->pos.xPos += dx / 64;
		item->pos.zPos += dz / 64;

		for (int i = 0; i < 4; i++)
		{
			creature->jointRotation[i] -= item->itemFlags[0];
		}

		if (!creature->reachedGoal)
			ClampRotation(&item->pos, info.angle, item->itemFlags[0] / 16);

		creature->maximumTurn = 0;
		AnimateItem(item);

		floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
		item->floor = GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);
		if (item->roomNumber != roomNumber)
			ItemNewRoom(itemNumber, roomNumber);

		if (item->pos.yPos < item->floor)
			item->Airborne = true;
		else
		{
			item->VerticalVelocity = 0;
			item->pos.yPos = item->floor;
			item->Airborne = false;
		}

		SoundEffect(SFX_TR4_JEEP_MOVE, &item->pos, (item->itemFlags[0] * 1024) + 16777220);
	}
}