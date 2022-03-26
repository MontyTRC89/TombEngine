#include "framework.h"
#include "tr4_enemy_jeep.h"
#include "Game/items.h"
#include "Specific/level.h"
#include "Game/collision/collide_room.h"
#include "Game/control/box.h"
#include "Specific/trmath.h"
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

		grenadeItem->Shade = -15856;
		grenadeItem->ObjectNumber = ID_GRENADE;
		grenadeItem->RoomNumber = item->RoomNumber;

		InitialiseItem(grenadeItemNumber);

		grenadeItem->Position.xRot = item->Position.xRot;
		grenadeItem->Position.yRot = item->Position.yRot + -ANGLE(180);
		grenadeItem->Position.zRot = 0;

		grenadeItem->Position.xPos = item->Position.xPos + 1024 * phd_sin(grenadeItem->Position.yRot);
		grenadeItem->Position.yPos = item->Position.yPos - 768;
		grenadeItem->Position.zPos = item->Position.xPos + 1024 * phd_cos(grenadeItem->Position.yRot);

		SmokeCountL = 32;
		SmokeWeapon = (LaraWeaponType)5; // TODO: 5 is the HK. Did the TEN enum get shuffled around? @Sezz 2022.03.09

		for (int i = 0; i < 5; i++)
		{
			TriggerGunSmoke(item->Position.xPos, item->Position.yPos, item->Position.zPos, 0, 0, 0, 1, (LaraWeaponType)5, 32);
		}

		if (GetRandomControl() & 3)
		{
			grenadeItem->ItemFlags[0] = 1;
		}
		else
		{
			grenadeItem->ItemFlags[0] = 2;
		}

		grenadeItem->Animation.Velocity = 32;
		grenadeItem->Animation.ActiveState = grenadeItem->Position.xRot;
		grenadeItem->Animation.VerticalVelocity = -32 * phd_sin(grenadeItem->Position.xRot);
		grenadeItem->Animation.TargetState = grenadeItem->Position.yRot;
		grenadeItem->Animation.RequiredState = 0;
		grenadeItem->HitPoints = 120;

		AddActiveItem(grenadeItemNumber);
	}
}

void InitialiseEnemyJeep(short itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	item->ItemFlags[0] = -80;

	if (g_Level.NumItems > 0)
	{
		for (int i = 0; i < g_Level.NumItems; i++)
		{
			ITEM_INFO* other = &g_Level.Items[i];

			if (other == item || other->TriggerFlags != item->TriggerFlags)
				continue;

			item->ItemFlags[1] = i;
			other->ItemFlags[0] = -80;
			other->Position.yPos = item->Position.yPos - 1024;
		}
	}
}

void EnemyJeepControl(short itemNumber)
{
	if (CreatureActive(itemNumber))
	{
		ITEM_INFO* item = &g_Level.Items[itemNumber];
		CreatureInfo* creature = (CreatureInfo*)item->Data;

		int x = item->Position.xPos;
		int y = item->Position.yPos;
		int z = item->Position.zPos;

		int dx = 682 * phd_sin(item->Position.yRot);
		int dz = 682 * phd_cos(item->Position.yRot);

		short roomNumber = item->RoomNumber;
		FLOOR_INFO* floor = GetFloor(x - dz, y, z - dx, &roomNumber);
		int height1 = GetFloorHeight(floor, x - dz, y, z - dx);
		if (abs(item->Position.yPos - height1) > 768)
		{
			item->Position.yRot += ANGLE(2);
			item->Position.xPos += (dz / 64);
			item->Position.zPos += (dx / 64);
			height1 = y;
		}

		roomNumber = item->RoomNumber;
		floor = GetFloor(x + dz, y, z - dx, &roomNumber);
		int height2 = GetFloorHeight(floor, x + dz, y, z - dx);
		if (abs(item->Position.yPos - height2) > 768)
		{
			item->Position.yRot -= ANGLE(2);
			item->Position.xPos -= (dz / 64);
			item->Position.zPos += (dx / 64);
			height2 = y;
		}

		short zRot = phd_atan(1364, height2 - height1);

		roomNumber = item->RoomNumber;
		floor = GetFloor(x + dx, y, z + dz, &roomNumber);
		int height3 = GetFloorHeight(floor, x + dx, y, z + dz);
		if (abs(y - height3) > 768)
		{
			height3 = y;
		}

		roomNumber = item->RoomNumber;
		floor = GetFloor(x - dx, y, z - dz, &roomNumber);
		int height4 = GetFloorHeight(floor, x - dx, y, z - dz);
		if (abs(y - height4) > 768)
		{
			height4 = y;
		}

		short xRot = phd_atan(1364, height4 - height3);

		AI_INFO info;
		CreatureAIInfo(item, &info);

		creature->Enemy = creature->AITarget;
		ITEM_INFO* target = creature->AITarget;
		short angle;
		int distance;
		{
			dx = LaraItem->Position.xPos - item->Position.xPos;
			dz = LaraItem->Position.zPos - item->Position.zPos;
			angle = phd_atan(dz, dx) - item->Position.yRot;
			if (dx > 32000 || dx < -32000 || dz > 32000 || dz < -32000)
				distance = 0x7FFFFFFF;
			else
				distance = SQUARE(dx) + SQUARE(dz);
		}

		PHD_VECTOR pos;

		switch (item->Animation.ActiveState)
		{
		case 0:
		case 2:
			item->ItemFlags[0] -= 128;
			if (item->ItemFlags[0] < 0)
				item->ItemFlags[0] = 0;

			item->MeshBits = -98305;
			
			pos.x = 0;
			pos.y = -144;
			pos.z = -1024;
			GetJointAbsPosition(item, &pos, 11);
			TriggerDynamicLight(pos.x, pos.y, pos.z, 10, 64, 0, 0);
			
			if (item->Animation.RequiredState)
				item->Animation.TargetState = item->Animation.RequiredState;
			else if (info.distance > SQUARE(1024) || Lara.Location >= item->ItemFlags[3])
				item->Animation.TargetState = 1;

			break;

		case 1:
			creature->MaxTurn = item->ItemFlags[0] / 16;
			item->ItemFlags[0] += 37;
			if (item->ItemFlags[0] > 8704)
				item->ItemFlags[0] = 8704;

			item->MeshBits = -147457;

			if (info.angle <= 256)
			{
				if (info.angle < -256)
				{
					item->Animation.TargetState = 3;
				}
			}
			else
			{
				item->Animation.TargetState = 4;
			}

			break;

		case 3:
		case 4:
			item->ItemFlags[0] += 18;
			if (item->ItemFlags[0] > 8704)
				item->ItemFlags[0] = 8704;
			item->Animation.TargetState = 1;

			break;

		case 5:
			if (item->ItemFlags[0] < 1184)
				item->ItemFlags[0] = 1184;

			break;

		default:
			break;
		}

		if (height3 <= item->Floor + 512)
		{
			if (height4 > item->Floor + 512 && item->Animation.ActiveState != 5)
			{
				item->ItemFlags[1] = 0;
				item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 8;
				item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].FrameBase;
				item->Animation.ActiveState = 5;
				item->Animation.TargetState = 1;
			}
		}
		else
		{
			creature->LOT.RequiredBox |= 8;
			if (item->ItemFlags[1] > 0)
			{
				item->ItemFlags[1] -= 8;
				if (item->ItemFlags[1]<0)
					creature->LOT.RequiredBox &= ~8;
				item->Position.yPos += item->ItemFlags[1] / 64;
			}
			else
			{
				item->ItemFlags[1] = 2 * xRot;
				creature->LOT.RequiredBox |= 8u;
			}
			if (creature->LOT.RequiredBox & 8)
			{
				creature->MaxTurn = 0;
				item->Animation.TargetState = 1;
			}
		}

		if (info.distance < SQUARE(1536) || item->ItemFlags[3] == -2)
			creature->ReachedGoal = true;

		if (creature->ReachedGoal)
		{
			TestTriggers(target->Position.xPos,target->Position.yPos,target->Position.zPos,target->RoomNumber, true);

			if (Lara.Location < item->ItemFlags[3] && item->Animation.ActiveState != 2 && item->Animation.TargetState != 2)
			{
				item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 1;
				item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].FrameBase;
				item->Animation.TargetState = 2;
				item->Animation.ActiveState = 2;

				if (target->Flags & 4)
				{
					item->Position.xPos = target->Position.xPos;
					item->Position.yPos = target->Position.yPos;
					item->Position.zPos = target->Position.zPos;
					item->Position.xRot = target->Position.xRot;
					item->Position.yRot = target->Position.yRot;
					item->Position.zRot = target->Position.zRot;

					if (item->RoomNumber != target->RoomNumber)
						ItemNewRoom(itemNumber, target->RoomNumber);
				}
			}

			if (distance > SQUARE(2048) && distance < SQUARE(10240) && !item->ItemFlags[2] && (angle < -20480 || angle > 20480))
			{
				EnemyJeepLaunchGrenade(item);
				item->ItemFlags[2] = 150;
			}

			if (target->Flags == 62)
			{
				item->Status = ITEM_INVISIBLE;
				RemoveActiveItem(itemNumber);
				DisableEntityAI(itemNumber);
			}

			if (Lara.Location >= item->ItemFlags[3] || !(target->Flags & 4))
			{
				creature->ReachedGoal = false;
				item->ItemFlags[3]++;

				creature->Enemy = nullptr;
				AI_OBJECT* aiObject = nullptr;

				for (int i = 0; i < g_Level.AIObjects.size(); i++)
				{ 
					aiObject = &g_Level.AIObjects[i];

					if (g_Level.AIObjects[i].triggerFlags == item->ItemFlags[3] && g_Level.AIObjects[i].roomNumber != NO_ROOM)
					{
						aiObject = &g_Level.AIObjects[i];
						break;
					}
				}

				if (aiObject != nullptr)
				{
					creature->Enemy = nullptr;
					target->ObjectNumber = aiObject->objectNumber;
					target->RoomNumber = aiObject->roomNumber;
					target->Position.xPos = aiObject->x;
					target->Position.yPos = aiObject->y;
					target->Position.zPos = aiObject->z;
					target->Position.yRot = aiObject->yRot;
					target->Flags = aiObject->flags;
					target->TriggerFlags = aiObject->triggerFlags;
					target->BoxNumber = aiObject->boxNumber;
					if (!(aiObject->flags & 0x20))
					{
						target->Position.xPos += 256 * phd_sin(target->Position.yRot);
						target->Position.zPos += 256 * phd_cos(target->Position.yRot);
					}
				}
			}
		}

		item->ItemFlags[2]--;
		if (item->ItemFlags[2] < 0)
			item->ItemFlags[2] = 0;

		if (abs(xRot - item->Position.xRot) < 256)
			item->Position.xRot = xRot;
		else if (xRot < item->Position.xRot)
			item->Position.xRot -= 256;
		else 
			item->Position.xRot += 256;

		if (abs(zRot - item->Position.zRot) < 256)
			item->Position.zRot = zRot;
		else if (zRot < item->Position.zRot)
			item->Position.zRot -= 256;
		else
			item->Position.zRot += 256;

		item->ItemFlags[0] += -2 - xRot / 512;
		if (item->ItemFlags[0] < 0)
			item->ItemFlags[0] = 0;

		dx = item->ItemFlags[0] * phd_sin(-2 - xRot / 512);
		dz = item->ItemFlags[0] * phd_cos(-2 - xRot / 512);

		item->Position.xPos += dx / 64;
		item->Position.zPos += dz / 64;

		for (int i = 0; i < 4; i++)
		{
			creature->JointRotation[i] -= item->ItemFlags[0];
		}

		if (!creature->ReachedGoal)
			ClampRotation(&item->Position, info.angle, item->ItemFlags[0] / 16);

		creature->MaxTurn = 0;
		AnimateItem(item);

		floor = GetFloor(item->Position.xPos, item->Position.yPos, item->Position.zPos, &roomNumber);
		item->Floor = GetFloorHeight(floor, item->Position.xPos, item->Position.yPos, item->Position.zPos);
		if (item->RoomNumber != roomNumber)
			ItemNewRoom(itemNumber, roomNumber);

		if (item->Position.yPos < item->Floor)
			item->Animation.Airborne = true;
		else
		{
			item->Animation.VerticalVelocity = 0;
			item->Position.yPos = item->Floor;
			item->Animation.Airborne = false;
		}

		SoundEffect(SFX_TR4_JEEP_MOVE, &item->Position, (item->ItemFlags[0] * 1024) + 16777220);
	}
}