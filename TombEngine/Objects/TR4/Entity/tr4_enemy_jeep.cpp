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
#include "Game/misc.h"

namespace TEN::Entities::TR4
{
	void EnemyJeepLaunchGrenade(ItemInfo* item)
	{
		short grenadeItemNumber = CreateItem();

		if (grenadeItemNumber != NO_ITEM)
		{
			auto* grenadeItem = &g_Level.Items[grenadeItemNumber];

			grenadeItem->Color = Vector4(0.5f, 0.5f, 0.5f, 1.0f);
			grenadeItem->ObjectNumber = ID_GRENADE;
			grenadeItem->RoomNumber = item->RoomNumber;

			InitialiseItem(grenadeItemNumber);

			grenadeItem->Pose.Orientation.x = item->Pose.Orientation.x;
			grenadeItem->Pose.Orientation.y = item->Pose.Orientation.y - ANGLE(180.0f);
			grenadeItem->Pose.Orientation.z = 0;

			grenadeItem->Pose.Position.x = item->Pose.Position.x + SECTOR(1) * phd_sin(grenadeItem->Pose.Orientation.y);
			grenadeItem->Pose.Position.y = item->Pose.Position.y - CLICK(3);
			grenadeItem->Pose.Position.z = item->Pose.Position.x + SECTOR(1) * phd_cos(grenadeItem->Pose.Orientation.y);

			for (int i = 0; i < 5; i++)
				TriggerGunSmoke(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, 0, 0, 0, 1, LaraWeaponType::GrenadeLauncher, 32);

			if (GetRandomControl() & 3)
				grenadeItem->ItemFlags[0] = 1;
			else
				grenadeItem->ItemFlags[0] = 2;

			grenadeItem->Animation.ActiveState = grenadeItem->Pose.Orientation.x;
			grenadeItem->Animation.TargetState = grenadeItem->Pose.Orientation.y;
			grenadeItem->Animation.RequiredState = 0;
			grenadeItem->Animation.Velocity = 32;
			grenadeItem->Animation.VerticalVelocity = -32 * phd_sin(grenadeItem->Pose.Orientation.x);
			grenadeItem->HitPoints = 120;

			AddActiveItem(grenadeItemNumber);
		}
	}

	void InitialiseEnemyJeep(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		item->ItemFlags[0] = -80;

		if (g_Level.NumItems > 0)
		{
			for (int i = 0; i < g_Level.NumItems; i++)
			{
				auto* other = &g_Level.Items[i];

				if (other == item || other->TriggerFlags != item->TriggerFlags)
					continue;

				item->ItemFlags[1] = i;
				other->ItemFlags[0] = -80;
				other->Pose.Position.y = item->Pose.Position.y - SECTOR(1);
			}
		}
	}

	void EnemyJeepControl(short itemNumber)
	{
		if (CreatureActive(itemNumber))
		{
			auto* item = &g_Level.Items[itemNumber];
			auto* creature = GetCreatureInfo(item);

			int x = item->Pose.Position.x;
			int y = item->Pose.Position.y;
			int z = item->Pose.Position.z;

			int dx = 682 * phd_sin(item->Pose.Orientation.y);
			int dz = 682 * phd_cos(item->Pose.Orientation.y);

			int height1 = GetCollision(x - dz, y, z - dx, item->RoomNumber).Position.Floor;
			if (abs(item->Pose.Position.y - height1) > CLICK(3))
			{
				item->Pose.Position.x += dz / 64;
				item->Pose.Position.z += dx / 64;
				item->Pose.Orientation.y += ANGLE(2.0f);
				height1 = y;
			}

			int height2 = GetCollision(x + dz, y, z - dx, item->RoomNumber).Position.Floor;
			if (abs(item->Pose.Position.y - height2) > CLICK(3))
			{
				item->Pose.Orientation.y -= ANGLE(2.0f);
				item->Pose.Position.x -= dz / 64;
				item->Pose.Position.z += dx / 64;
				height2 = y;
			}

			short zRot = phd_atan(1364, height2 - height1);

			int height3 = GetCollision(x + dx, y, z + dz, item->RoomNumber).Position.Floor;
			if (abs(y - height3) > CLICK(3))
				height3 = y;

			int height4 = GetCollision(x - dx, y, z - dz, item->RoomNumber).Position.Floor;
			if (abs(y - height4) > CLICK(3))
				height4 = y;

			short xRot = phd_atan(1364, height4 - height3);

			AI_INFO AI;
			CreatureAIInfo(item, &AI);

			creature->Enemy = creature->AITarget;

			auto* target = creature->AITarget;

			dx = LaraItem->Pose.Position.x - item->Pose.Position.x;
			dz = LaraItem->Pose.Position.z - item->Pose.Position.z;
			short angle = phd_atan(dz, dx) - item->Pose.Orientation.y;

			int distance;
			if (dx > SECTOR(31.25f) || dx < -SECTOR(31.25f) ||
				dz > SECTOR(31.25f) || dz < -SECTOR(31.25f))
			{
				distance = INT_MAX;
			}
			else
				distance = pow(dx, 2) + pow(dz, 2);

			Vector3Int pos;

			switch (item->Animation.ActiveState)
			{
			case 0:
			case 2:
				item->ItemFlags[0] -= 128;
				item->MeshBits = -98305;

				pos = Vector3Int(0, -144, -1024);
				GetJointAbsPosition(item, &pos, 11);

				TriggerDynamicLight(pos.x, pos.y, pos.z, 10, 64, 0, 0);

				if (item->ItemFlags[0] < 0)
					item->ItemFlags[0] = 0;

				if (item->Animation.RequiredState)
					item->Animation.TargetState = item->Animation.RequiredState;
				else if (AI.distance > pow(SECTOR(1), 2) || Lara.Location >= item->ItemFlags[3])
					item->Animation.TargetState = 1;

				break;

			case 1:
				item->ItemFlags[0] += 37;
				item->MeshBits = 0xFFFDBFFF;
				creature->MaxTurn = item->ItemFlags[0] / 16;

				if (item->ItemFlags[0] > 8704)
					item->ItemFlags[0] = 8704;

				if (AI.angle <= ANGLE(1.4f))
				{
					if (AI.angle < -ANGLE(1.4f))
						item->Animation.TargetState = 3;
				}
				else
					item->Animation.TargetState = 4;

				break;

			case 3:
			case 4:
				item->Animation.TargetState = 1;
				item->ItemFlags[0] += 18;

				if (item->ItemFlags[0] > 8704)
					item->ItemFlags[0] = 8704;

				break;

			case 5:
				if (item->ItemFlags[0] < 1184)
					item->ItemFlags[0] = 1184;

				break;

			default:
				break;
			}

			if (height3 <= (item->Floor + CLICK(2)))
			{
				if (height4 > (item->Floor + CLICK(2)) && item->Animation.ActiveState != 5)
				{
					item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 8;
					item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
					item->Animation.ActiveState = 5;
					item->Animation.TargetState = 1;
					item->ItemFlags[1] = 0;
				}
			}
			else
			{
				creature->LOT.RequiredBox |= 8;

				if (item->ItemFlags[1] > 0)
				{
					item->ItemFlags[1] -= 8;
					item->Pose.Position.y += item->ItemFlags[1] / 64;

					if (item->ItemFlags[1] < 0)
						creature->LOT.RequiredBox &= ~8;
				}
				else
				{
					item->ItemFlags[1] = 2 * xRot;
					creature->LOT.RequiredBox |= 8u;
				}

				if (creature->LOT.RequiredBox & 8)
				{
					item->Animation.TargetState = 1;
					creature->MaxTurn = 0;
				}
			}

			if (AI.distance < pow(SECTOR(1.5f), 2) || item->ItemFlags[3] == -2)
				creature->ReachedGoal = true;

			if (creature->ReachedGoal)
			{
				TestTriggers(target->Pose.Position.x, target->Pose.Position.y, target->Pose.Position.z, target->RoomNumber, true);

				if (Lara.Location < item->ItemFlags[3] && item->Animation.ActiveState != 2 && item->Animation.TargetState != 2)
				{
					item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 1;
					item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
					item->Animation.TargetState = 2;
					item->Animation.ActiveState = 2;

					if (target->Flags & 4)
					{
						item->Pose = target->Pose;

						if (item->RoomNumber != target->RoomNumber)
							ItemNewRoom(itemNumber, target->RoomNumber);
					}
				}

				if (distance > pow(SECTOR(2), 2) &&
					distance < pow(SECTOR(10), 2) &&
					!item->ItemFlags[2] &&
					(angle < -ANGLE(112.5f) || angle > ANGLE(112.5f)))
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
							target->Pose.Position.x += CLICK(1) * phd_sin(target->Pose.Orientation.y);
							target->Pose.Position.z += CLICK(1) * phd_cos(target->Pose.Orientation.y);
						}
					}
				}
			}

			item->ItemFlags[2]--;
			if (item->ItemFlags[2] < 0)
				item->ItemFlags[2] = 0;

			if (abs(xRot - item->Pose.Orientation.x) < ANGLE(1.4f))
				item->Pose.Orientation.x = xRot;
			else if (xRot < item->Pose.Orientation.x)
				item->Pose.Orientation.x -= ANGLE(1.4f);
			else
				item->Pose.Orientation.x += ANGLE(1.4f);

			if (abs(zRot - item->Pose.Orientation.z) < ANGLE(1.4f))
				item->Pose.Orientation.z = zRot;
			else if (zRot < item->Pose.Orientation.z)
				item->Pose.Orientation.z -= ANGLE(1.4f);
			else
				item->Pose.Orientation.z += ANGLE(1.4f);

			item->ItemFlags[0] += -2 - xRot / 512;
			if (item->ItemFlags[0] < 0)
				item->ItemFlags[0] = 0;

			dx = item->ItemFlags[0] * phd_sin(-2 - xRot / 512);
			dz = item->ItemFlags[0] * phd_cos(-2 - xRot / 512);

			item->Pose.Position.x += dx / 64;
			item->Pose.Position.z += dz / 64;

			for (int i = 0; i < 4; i++)
				creature->JointRotation[i] -= item->ItemFlags[0];

			if (!creature->ReachedGoal)
				ClampRotation(&item->Pose, AI.angle, item->ItemFlags[0] / 16);

			creature->MaxTurn = 0;
			AnimateItem(item);

			auto probe = GetCollision(item);
			item->Floor = probe.Position.Floor;
			if (item->RoomNumber != probe.RoomNumber)
				ItemNewRoom(itemNumber, probe.RoomNumber);

			if (item->Pose.Position.y < item->Floor)
				item->Animation.IsAirborne = true;
			else
			{
				item->Pose.Position.y = item->Floor;
				item->Animation.IsAirborne = false;
				item->Animation.VerticalVelocity = 0;
			}

			SoundEffect(SFX_TR4_VEHICLE_JEEP_MOVING, &item->Pose, SoundEnvironment::Land, 1.0f + (float)item->ItemFlags[0] / SECTOR(8)); // TODO: Check actual sound!
		}
	}
}
