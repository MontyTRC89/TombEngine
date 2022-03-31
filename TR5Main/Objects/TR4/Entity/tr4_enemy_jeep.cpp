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

		grenadeItem->Pose.Orientation.x = item->Pose.Orientation.x;
		grenadeItem->Pose.Orientation.y = item->Pose.Orientation.y + -ANGLE(180);
		grenadeItem->Pose.Orientation.z = 0;

		grenadeItem->Pose.Position.x = item->Pose.Position.x + 1024 * phd_sin(grenadeItem->Pose.Orientation.y);
		grenadeItem->Pose.Position.y = item->Pose.Position.y - 768;
		grenadeItem->Pose.Position.z = item->Pose.Position.x + 1024 * phd_cos(grenadeItem->Pose.Orientation.y);

		SmokeCountL = 32;
		SmokeWeapon = (LaraWeaponType)5; // TODO: 5 is the HK. Did the TEN enum get shuffled around? @Sezz 2022.03.09

		for (int i = 0; i < 5; i++)
		{
			TriggerGunSmoke(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, 0, 0, 0, 1, (LaraWeaponType)5, 32);
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
		grenadeItem->Animation.ActiveState = grenadeItem->Pose.Orientation.x;
		grenadeItem->Animation.VerticalVelocity = -32 * phd_sin(grenadeItem->Pose.Orientation.x);
		grenadeItem->Animation.TargetState = grenadeItem->Pose.Orientation.y;
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
			other->Pose.Position.y = item->Pose.Position.y - 1024;
		}
	}
}

void EnemyJeepControl(short itemNumber)
{
	if (CreatureActive(itemNumber))
	{
		ITEM_INFO* item = &g_Level.Items[itemNumber];
		CreatureInfo* creature = (CreatureInfo*)item->Data;

		int x = item->Pose.Position.x;
		int y = item->Pose.Position.y;
		int z = item->Pose.Position.z;

		int dx = 682 * phd_sin(item->Pose.Orientation.y);
		int dz = 682 * phd_cos(item->Pose.Orientation.y);

		short roomNumber = item->RoomNumber;
		FLOOR_INFO* floor = GetFloor(x - dz, y, z - dx, &roomNumber);
		int height1 = GetFloorHeight(floor, x - dz, y, z - dx);
		if (abs(item->Pose.Position.y - height1) > 768)
		{
			item->Pose.Orientation.y += ANGLE(2);
			item->Pose.Position.x += (dz / 64);
			item->Pose.Position.z += (dx / 64);
			height1 = y;
		}

		roomNumber = item->RoomNumber;
		floor = GetFloor(x + dz, y, z - dx, &roomNumber);
		int height2 = GetFloorHeight(floor, x + dz, y, z - dx);
		if (abs(item->Pose.Position.y - height2) > 768)
		{
			item->Pose.Orientation.y -= ANGLE(2);
			item->Pose.Position.x -= (dz / 64);
			item->Pose.Position.z += (dx / 64);
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
			dx = LaraItem->Pose.Position.x - item->Pose.Position.x;
			dz = LaraItem->Pose.Position.z - item->Pose.Position.z;
			angle = phd_atan(dz, dx) - item->Pose.Orientation.y;
			if (dx > 32000 || dx < -32000 || dz > 32000 || dz < -32000)
				distance = 0x7FFFFFFF;
			else
				distance = SQUARE(dx) + SQUARE(dz);
		}

		Vector3Int pos;

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
				item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
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
				item->Pose.Position.y += item->ItemFlags[1] / 64;
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
			TestTriggers(target->Pose.Position.x,target->Pose.Position.y,target->Pose.Position.z,target->RoomNumber, true);

			if (Lara.Location < item->ItemFlags[3] && item->Animation.ActiveState != 2 && item->Animation.TargetState != 2)
			{
				item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 1;
				item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
				item->Animation.TargetState = 2;
				item->Animation.ActiveState = 2;

				if (target->Flags & 4)
				{
					item->Pose.Position.x = target->Pose.Position.x;
					item->Pose.Position.y = target->Pose.Position.y;
					item->Pose.Position.z = target->Pose.Position.z;
					item->Pose.Orientation.x = target->Pose.Orientation.x;
					item->Pose.Orientation.y = target->Pose.Orientation.y;
					item->Pose.Orientation.z = target->Pose.Orientation.z;

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
					target->Pose.Position.x = aiObject->x;
					target->Pose.Position.y = aiObject->y;
					target->Pose.Position.z = aiObject->z;
					target->Pose.Orientation.y = aiObject->yRot;
					target->Flags = aiObject->flags;
					target->TriggerFlags = aiObject->triggerFlags;
					target->BoxNumber = aiObject->boxNumber;
					if (!(aiObject->flags & 0x20))
					{
						target->Pose.Position.x += 256 * phd_sin(target->Pose.Orientation.y);
						target->Pose.Position.z += 256 * phd_cos(target->Pose.Orientation.y);
					}
				}
			}
		}

		item->ItemFlags[2]--;
		if (item->ItemFlags[2] < 0)
			item->ItemFlags[2] = 0;

		if (abs(xRot - item->Pose.Orientation.x) < 256)
			item->Pose.Orientation.x = xRot;
		else if (xRot < item->Pose.Orientation.x)
			item->Pose.Orientation.x -= 256;
		else 
			item->Pose.Orientation.x += 256;

		if (abs(zRot - item->Pose.Orientation.z) < 256)
			item->Pose.Orientation.z = zRot;
		else if (zRot < item->Pose.Orientation.z)
			item->Pose.Orientation.z -= 256;
		else
			item->Pose.Orientation.z += 256;

		item->ItemFlags[0] += -2 - xRot / 512;
		if (item->ItemFlags[0] < 0)
			item->ItemFlags[0] = 0;

		dx = item->ItemFlags[0] * phd_sin(-2 - xRot / 512);
		dz = item->ItemFlags[0] * phd_cos(-2 - xRot / 512);

		item->Pose.Position.x += dx / 64;
		item->Pose.Position.z += dz / 64;

		for (int i = 0; i < 4; i++)
		{
			creature->JointRotation[i] -= item->ItemFlags[0];
		}

		if (!creature->ReachedGoal)
			ClampRotation(&item->Pose, info.angle, item->ItemFlags[0] / 16);

		creature->MaxTurn = 0;
		AnimateItem(item);

		floor = GetFloor(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, &roomNumber);
		item->Floor = GetFloorHeight(floor, item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z);
		if (item->RoomNumber != roomNumber)
			ItemNewRoom(itemNumber, roomNumber);

		if (item->Pose.Position.y < item->Floor)
			item->Animation.Airborne = true;
		else
		{
			item->Animation.VerticalVelocity = 0;
			item->Pose.Position.y = item->Floor;
			item->Animation.Airborne = false;
		}

		SoundEffect(SFX_TR4_JEEP_MOVE, &item->Pose, (item->ItemFlags[0] * 1024) + 16777220);
	}
}