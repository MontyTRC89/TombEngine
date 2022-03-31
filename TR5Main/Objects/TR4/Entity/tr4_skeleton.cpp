#include "framework.h"
#include "tr4_skeleton.h"
#include "Game/items.h"
#include "Game/control/box.h"
#include "Game/people.h"
#include "Game/effects/effects.h"
#include "Game/effects/debris.h"
#include "Game/control/lot.h"
#include "Game/Lara/lara.h"
#include "Sound/sound.h"
#include "Specific/setup.h"
#include "Game/effects/tomb4fx.h"
#include "Specific/level.h"
#include "Game/animation.h"
#include "Game/itemdata/creature_info.h"
#include "Game/collision/floordata.h"
#include "Game/collision/collide_room.h"

namespace TEN::Entities::TR4
{
	BITE_INFO skeletonBite = { 0, -16, 200, 11 };

	void TriggerRiseEffect(ITEM_INFO* item)
	{
		short fxNum = CreateNewEffect(item->RoomNumber);
		if (fxNum != NO_ITEM)
		{
			FX_INFO* fx = &EffectList[fxNum];

			short roomNumber = item->RoomNumber;
			FLOOR_INFO* floor = GetFloor(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, &roomNumber);

			fx->pos.Position.x = (byte)GetRandomControl() + item->Pose.Position.x - 128;
			fx->pos.Position.y = GetFloorHeight(floor, item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z);
			fx->pos.Position.z = (byte)GetRandomControl() + item->Pose.Position.z - 128;
			fx->roomNumber = item->RoomNumber;
			fx->pos.Orientation.y = 2 * GetRandomControl();
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
			spark->x = fx->pos.Position.x;
			spark->y = fx->pos.Position.y;
			spark->z = fx->pos.Position.z;
			spark->xVel = phd_sin(fx->pos.Orientation.y) * 4096;
			spark->yVel = 0;
			spark->zVel = phd_cos(fx->pos.Orientation.y) * 4096;
			spark->transType = TransTypeEnum::COLADD;
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
		OBJECT_INFO* obj = &Objects[item->ObjectNumber];

		ClearItem(itemNumber);

		switch (item->TriggerFlags)
		{
		case 0:
			item->Animation.TargetState = 0;
			item->Animation.ActiveState = 0;
			item->Animation.AnimNumber = obj->animIndex;
			item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
			break;

		case 1:
			item->Animation.TargetState = STATE_SKELETON_JUMP_RIGHT;
			item->Animation.ActiveState = STATE_SKELETON_JUMP_RIGHT;
			item->Animation.AnimNumber = obj->animIndex + 37;
			item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
			break;

		case 2:
			item->Animation.TargetState = STATE_SKELETON_JUMP_LEFT;
			item->Animation.ActiveState = STATE_SKELETON_JUMP_LEFT;
			item->Animation.AnimNumber = obj->animIndex + 34;
			item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
			break;

		case 3:
			item->Animation.TargetState = STATE_SKELETON_JUMP_LIE_DOWN;
			item->Animation.ActiveState = STATE_SKELETON_JUMP_LIE_DOWN;
			item->Animation.AnimNumber = obj->animIndex;
			item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
			//item->status = ITEM_DEACTIVATED;
			break;

		}
	}

	void SkeletonControl(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;

		ITEM_INFO* item = &g_Level.Items[itemNumber];
		CreatureInfo* creature = (CreatureInfo*)item->Data;
		ITEM_INFO* enemyItem = creature->Enemy;
		bool jumpLeft = false;
		bool jumpRight = false;
		short tilt = 0;
		short angle = 0;
		short joint1 = 0;
		short joint2 = 0;
		short joint3 = 0;
		int distance = 0;
		short rot = 0;

		// Can skeleton jump? Check for a distance of 1 and 2 sectors
		int x = item->Pose.Position.x;
		int y = item->Pose.Position.y;
		int z = item->Pose.Position.z;

		int dx = 870 * phd_sin(item->Pose.Orientation.y);
		int dz = 870 * phd_cos(item->Pose.Orientation.y);

		x += dx;
		z += dz;

		short roomNumber = item->RoomNumber;
		FLOOR_INFO* floor = GetFloor(x, y, z, &roomNumber);
		int height1 = GetFloorHeight(floor, x, y, z);

		x += dx;
		z += dz;

		roomNumber = item->RoomNumber;
		floor = GetFloor(x, y, z, &roomNumber);
		int height2 = GetFloorHeight(floor, x, y, z);

		x += dx;
		z += dz;

		roomNumber = item->RoomNumber;
		floor = GetFloor(x, y, z, &roomNumber);
		int height3 = GetFloorHeight(floor, x, y, z);

		int height = 0;
		bool canJump1sector = true;
		if (enemyItem && item->BoxNumber == LaraItem->BoxNumber && item->MeshBits & 0x200
			|| y >= height1 - 384
			|| y >= height2 + 256
			|| y <= height2 - 256)
		{
			height = height2;
			canJump1sector = false;
		}

		bool canJump2sectors = true;
		if (enemyItem && item->BoxNumber == LaraItem->BoxNumber && item->MeshBits & 0x200
			|| y >= height1 - 384
			|| y >= height - 384
			|| y >= height3 + 256
			|| y <= height3 - 256)
		{
			canJump2sectors = false;
		}

		if (item->AIBits)
			GetAITarget(creature);
		else
			creature->Enemy = LaraItem;

		AI_INFO info;
		AI_INFO laraInfo;
		CreatureAIInfo(item, &info);

		if (item->HitStatus
			&& Lara.Control.Weapon.GunType == LaraWeaponType::Shotgun
			&& info.distance < SQUARE(3584)
			&& item->Animation.ActiveState != 7
			&& item->Animation.ActiveState != 17
			&& item->Animation.ActiveState != STATE_SKELETON_HURT_BY_SHOTGUN1
			&& item->Animation.ActiveState != STATE_SKELETON_HURT_BY_SHOTGUN2
			&& item->Animation.ActiveState != 25)
		{
			if (info.angle >= 12288 || info.angle <= -12288)
			{
				item->Animation.ActiveState = STATE_SKELETON_HURT_BY_SHOTGUN2;
				item->Animation.AnimNumber = Objects[ID_SKELETON].animIndex + 33;
				item->Pose.Orientation.y += info.angle + -32768;
			}
			else
			{
				item->Animation.ActiveState = STATE_SKELETON_HURT_BY_SHOTGUN1;
				item->Animation.AnimNumber = Objects[ID_SKELETON].animIndex + 17;
				item->Pose.Orientation.y += info.angle;
			}

			item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
			creature->LOT.IsJumping = true;
			item->HitPoints = 25;
		}
		else
		{
			if (creature->Enemy == LaraItem)
			{
				laraInfo.distance = info.distance;
				laraInfo.angle = info.angle;
			}
			else
			{
				dx = LaraItem->Pose.Position.x - item->Pose.Position.x;
				dz = LaraItem->Pose.Position.z - item->Pose.Position.z;
				laraInfo.angle = phd_atan(dz, dx) - item->Pose.Orientation.y;
				laraInfo.distance = SQUARE(dx) + SQUARE(dz);
			}

			GetCreatureMood(item, &info, VIOLENT);

			if (!(item->MeshBits & 0x200))
				creature->Mood = MoodType::Escape;

			CreatureMood(item, &info, VIOLENT);

			angle = CreatureTurn(item, creature->MaxTurn);

			ITEM_INFO* tempEnemy = creature->Enemy;
			creature->Enemy = LaraItem;
			if (item->HitStatus
				|| distance < SQUARE(1024)
				|| TargetVisible(item, &laraInfo))
				creature->Alerted = true;
			creature->Enemy = tempEnemy;

			if (item != Lara.TargetEntity || laraInfo.distance <= 870 || angle <= -10240 || angle >= 10240)
			{
				jumpLeft = false;
				jumpRight = false;
			}
			else
			{
				dx = 870 * phd_sin(item->Pose.Orientation.y + ANGLE(45));
				dz = 870 * phd_cos(item->Pose.Orientation.y + ANGLE(45));

				x = item->Pose.Position.x + dx;
				y = item->Pose.Position.y;
				z = item->Pose.Position.z + dz;

				roomNumber = item->RoomNumber;
				floor = GetFloor(x, y, z, &roomNumber);
				int height4 = GetFloorHeight(floor, x, y, z);

				dx = 870 * phd_sin(item->Pose.Orientation.y + 14336);
				dz = 870 * phd_cos(item->Pose.Orientation.y + 14336);

				x = item->Pose.Position.x + dx;
				y = item->Pose.Position.y;
				z = item->Pose.Position.z + dz;

				roomNumber = item->RoomNumber;
				floor = GetFloor(x, y, z, &roomNumber);
				int height5 = GetFloorHeight(floor, x, y, z);

				if (abs(height5 - item->Pose.Position.y) > 256)
					jumpRight = false;
				else
				{
					jumpRight = true;
					if (height4 + 512 >= item->Pose.Position.y)
						jumpRight = false;
				}

				dx = 870 * phd_sin(item->Pose.Orientation.y - 8192);
				dz = 870 * phd_cos(item->Pose.Orientation.y - 8192);

				x = item->Pose.Position.x + dx;
				y = item->Pose.Position.y;
				z = item->Pose.Position.z + dz;

				roomNumber = item->RoomNumber;
				floor = GetFloor(x, y, z, &roomNumber);
				int height6 = GetFloorHeight(floor, x, y, z);

				dx = 870 * phd_sin(item->Pose.Orientation.y - 14336);
				dz = 870 * phd_cos(item->Pose.Orientation.y - 14336);

				x = item->Pose.Position.x + dx;
				y = item->Pose.Position.y;
				z = item->Pose.Position.z + dz;

				roomNumber = item->RoomNumber;
				floor = GetFloor(x, y, z, &roomNumber);
				int height7 = GetFloorHeight(floor, x, y, z);

				if (abs(height7 - item->Pose.Position.y) > 256 || height6 + 512 >= item->Pose.Position.y)
					jumpLeft = false;
				else
					jumpLeft = true;

			}

			switch (item->Animation.ActiveState)
			{
			case STATE_SKELETON_STOP:
				if (!(GetRandomControl() & 0xF))
				{
					item->Animation.TargetState = 2;
				}
				break;

			case 2:
				creature->Flags = 0;
				creature->LOT.IsJumping = false;
				creature->MaxTurn = creature->Mood != MoodType::Escape ? ANGLE(2) : 0;
				if (item->AIBits & GUARD
					|| !(GetRandomControl() & 0x1F) 
					&& (info.distance > SQUARE(1024) 
						|| creature->Mood != MoodType::Attack))
				{
					if (!(GetRandomControl() & 0x3F))
					{
						if (GetRandomControl() & 1)
						{
							item->Animation.TargetState = 3;
						}
						else
						{
							item->Animation.TargetState = 4;
						}
					}
				}
				else
				{
					if (item->AIBits & PATROL1)
					{
						item->Animation.TargetState = 15;
					}
					else if (canJump1sector || canJump2sectors)
					{
						creature->MaxTurn = 0;
						item->Animation.AnimNumber = Objects[ID_SKELETON].animIndex + 40;
						item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
						item->Animation.ActiveState = STATE_SKELETON_JUMP_LEFT;
						if (!canJump2sectors)
						{
							item->Animation.TargetState = STATE_SKELETON_JUMP_1SECTOR;
							creature->LOT.IsJumping = true;
						}
						else
						{
							item->Animation.TargetState = STATE_SKELETON_JUMP_2SECTORS;
							creature->LOT.IsJumping = true;
						}
					}
					else if (jumpLeft)
					{
						item->Animation.AnimNumber = Objects[ID_SKELETON].animIndex + 34;
						item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
						item->Animation.TargetState = STATE_SKELETON_JUMP_LEFT;
						item->Animation.ActiveState = STATE_SKELETON_JUMP_LEFT;
					}
					else if (jumpRight)
					{
						item->Animation.AnimNumber = Objects[ID_SKELETON].animIndex + 37;
						item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
						item->Animation.TargetState = STATE_SKELETON_JUMP_RIGHT;
						item->Animation.ActiveState = STATE_SKELETON_JUMP_RIGHT;
					}
					else
					{
						if (creature->Mood == MoodType::Escape)
						{
							if (Lara.TargetEntity == item 
								|| !info.ahead 
								|| item->HitStatus 
								|| !(item->MeshBits & 0x200))
							{
								item->Animation.TargetState = 15;
								break;
							}
							item->Animation.TargetState = 2;
						}
						else if (creature->Mood == MoodType::Bored ||
							item->AIBits & FOLLOW &&
							(creature->ReachedGoal ||
								laraInfo.distance > SQUARE(2048)))
						{
							if (item->Animation.RequiredState)
							{
								item->Animation.TargetState = item->Animation.RequiredState;
							}
							else if (!(GetRandomControl() & 0x3F))
							{
								item->Animation.TargetState = 15;
							}
						}
						else if (Lara.TargetEntity == item
							&& laraInfo.angle
							&& laraInfo.distance < SQUARE(2048)
							&& GetRandomControl() & 1
							&& (Lara.Control.Weapon.GunType == LaraWeaponType::Shotgun || !(GetRandomControl() & 0xF))
							&& item->MeshBits == -1)
						{
							item->Animation.TargetState = STATE_SKELETON_USE_SHIELD;
						}
						else if (info.bite && info.distance < SQUARE(682))
						{
							if (GetRandomControl() & 3 && LaraItem->HitPoints > 0)
							{
								if (GetRandomControl() & 1)
								{
									item->Animation.TargetState = STATE_SKELETON_ATTACK1;
								}
								else
								{
									item->Animation.TargetState = STATE_SKELETON_ATTACK2;
								}
							}
							else
							{
								item->Animation.TargetState = STATE_SKELETON_ATTACK3;
							}
						}
						else if (item->HitStatus || item->Animation.RequiredState)
						{
							if (GetRandomControl() & 1)
							{
								item->Animation.TargetState = STATE_SKELETON_AVOID_ATTACK1;
								item->Animation.RequiredState = item->Animation.TargetState;
							}
							else
							{
								item->Animation.TargetState = STATE_SKELETON_AVOID_ATTACK2;
								item->Animation.RequiredState = item->Animation.TargetState;
							}
						}
						else
						{
							item->Animation.TargetState = 15;
						}
					}
				}
				break;

			case 15:
				creature->Flags = 0;
				creature->LOT.IsJumping = false;
				creature->MaxTurn = creature->Mood != MoodType::Bored ? 1092 : 364;
				if (item->AIBits & PATROL1)
				{
					item->Animation.TargetState = 15;
				}
				else if (item->HitStatus)
				{
					item->Animation.TargetState = 2;
					if (GetRandomControl() & 1)
					{
						item->Animation.RequiredState = 5;
					}
					else
					{
						item->Animation.RequiredState = 6;
					}
				}
				else
				{
					if (jumpLeft || jumpRight)
					{
						item->Animation.TargetState = 2;
						break;
					}
					if (creature->Mood == MoodType::Escape)
					{
						item->Animation.TargetState = 16;
					}
					else if (creature->Mood != MoodType::Bored)
					{
						if (info.distance >= SQUARE(682))
						{
							if (info.bite && info.distance < SQUARE(1024))
							{
								item->Animation.TargetState = 18;
							}
							else if (canJump1sector || canJump2sectors)
							{
								creature->MaxTurn = 0;
								item->Animation.TargetState = 2;
							}
							else if (!info.ahead || info.distance > SQUARE(2048))
							{
								item->Animation.TargetState = 16;
							}
						}
						else
						{
							item->Animation.TargetState = 2;
						}
					}
					else if (!(GetRandomControl() & 0x3F))
					{
						item->Animation.TargetState = 2;
					}
				}
				break;

			case 16:
				creature->MaxTurn = 1274;
				creature->LOT.IsJumping = false;

				if (item->AIBits & GUARD || canJump1sector || canJump2sectors)
				{
					if (item->MeshBits & 0x200)
					{
						creature->MaxTurn = 0;
						item->Animation.TargetState = 2;
						break;
					}

					creature->LOT.IsJumping = true;
					floor = GetFloor(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, &roomNumber);
					if (GetFloorHeight(floor, item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z) > item->Pose.Position.y + 1024)
					{
						creature->MaxTurn = 0;
						item->Animation.AnimNumber = Objects[ID_SKELETON].animIndex + 44;
						item->Animation.ActiveState = 23;
						item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
						creature->LOT.IsJumping = false;
						item->Animation.Airborne = true;
					}
				}
				else
				{
					if (creature->Mood == MoodType::Escape)
					{
						if (Lara.TargetEntity != item && info.ahead && (item->MeshBits & 0x200))
							item->Animation.TargetState = 2;
					}
					else if (item->AIBits & FOLLOW && (creature->ReachedGoal || laraInfo.distance > SQUARE(2048)))
					{
						item->Animation.TargetState = 2;
					}
					else if (creature->Mood != MoodType::Bored)
					{
						if (info.ahead && info.distance < SQUARE(2048))
						{
							item->Animation.TargetState = 15;
						}
					}
					else
					{
						item->Animation.TargetState = 15;
					}
				}
				break;

			case 10:
				creature->MaxTurn = 0;
				if (abs(info.angle) >= 1092)
				{
					if (info.angle >= 0)
					{
						item->Pose.Orientation.y += 1092;
					}
					else
					{
						item->Pose.Orientation.y -= 1092;
					}
				}
				else
				{
					item->Pose.Orientation.y += info.angle;
				}

				if (!creature->Flags)
				{
					if (item->TouchBits & 0x18000)
					{
						LaraItem->HitPoints -= 80;
						LaraItem->HitStatus = true;
						CreatureEffect2(item, &skeletonBite, 15, -1, DoBloodSplat);
						SoundEffect(SFX_TR4_LARA_THUD, &item->Pose, 0);
						creature->Flags = 1;
					}
				}
				if (!(GetRandomControl() & 0x3F) || LaraItem->HitPoints <= 0)
				{
					item->Animation.TargetState = 11;
				}
				break;

			case STATE_SKELETON_ATTACK1:
			case STATE_SKELETON_ATTACK2:
			case 18:
				creature->MaxTurn = 0;
				if (abs(info.angle) >= 1092)
				{
					if (info.angle >= 0)
					{
						item->Pose.Orientation.y += 1092;
					}
					else
					{
						item->Pose.Orientation.y -= 1092;
					}
				}
				else
				{
					item->Pose.Orientation.y += info.angle;
				}
				if (item->Animation.FrameNumber > g_Level.Anims[item->Animation.AnimNumber].frameBase + 15)
				{
					ROOM_INFO* room = &g_Level.Rooms[item->RoomNumber];
					Vector3Int pos;

					GetJointAbsPosition(item, &pos, 16);

					auto floor = GetCollision(x, y, z, item->RoomNumber).Block;
					if (floor->Stopper)
					{
						for (int i = 0; i < room->mesh.size(); i++)
						{
							MESH_INFO* staticMesh = &room->mesh[i];
							if (abs(pos.x - staticMesh->pos.Position.x) < 1024 && 
								abs(pos.z - staticMesh->pos.Position.z) < 1024 && 
								StaticObjects[staticMesh->staticNumber].shatterType != SHT_NONE)
							{
								ShatterObject(0, staticMesh, -128, LaraItem->RoomNumber, 0);
								SoundEffect(SFX_TR4_HIT_ROCK, &item->Pose, 0);
								staticMesh->flags &= ~StaticMeshFlags::SM_VISIBLE;
								floor->Stopper = false;
								TestTriggers(item, true);
								break;
							}
						}
					}
					if (!creature->Flags)
					{
						if (item->TouchBits & 0x18000)
						{
							LaraItem->HitPoints -= 80;
							LaraItem->HitStatus = true;
							CreatureEffect2(item, &skeletonBite, 10, item->Pose.Orientation.y, DoBloodSplat);

							SoundEffect(SFX_TR4_LARA_THUD, &item->Pose, 0);

							creature->Flags = 1;
						}
					}
				}
				break;

			case STATE_SKELETON_USE_SHIELD:
				if (item->HitStatus)
				{
					if (item->MeshBits == -1 && laraInfo.angle && Lara.Control.Weapon.GunType == LaraWeaponType::Shotgun)
					{
						if (GetRandomControl() & 3)
						{
							item->Animation.TargetState = 17;
						}
						else
						{
							ExplodeItemNode(item, 11, 1, -24);
						}
					}
					else
					{
						item->Animation.TargetState = 2;
					}
				}
				else if (Lara.TargetEntity != item || item->MeshBits != -1 || Lara.Control.Weapon.GunType != LaraWeaponType::Shotgun || !(GetRandomControl() & 0x7F))
				{
					item->Animation.TargetState = 2;
				}
				break;

			case STATE_SKELETON_JUMP_1SECTOR:
				if (item->Animation.AnimNumber == Objects[item->ObjectNumber].animIndex + 43)
				{
					roomNumber = item->RoomNumber;
					floor = GetFloor(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, &roomNumber);
					if (GetFloorHeight(floor, item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z) > item->Pose.Position.y + 1280)
					{
						creature->MaxTurn = 0;
						item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 44;
						item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
						item->Animation.ActiveState = 23;
						creature->LOT.IsJumping = false;
						item->Animation.Airborne = true;
					}
				}
				break;

			case 23:
			case 24:
				roomNumber = item->RoomNumber;
				floor = GetFloor(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, &roomNumber);
				if (GetFloorHeight(floor, item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z) <= item->Pose.Position.y)
				{
					if (item->Active)
					{
						ExplodingDeath(itemNumber, -1, 929);
						KillItem(itemNumber);
						DisableEntityAI(itemNumber);
						//Savegame.Kills++;
					}
				}
				break;

			case 25:
			case 11:
			case STATE_SKELETON_HURT_BY_SHOTGUN1:
			case STATE_SKELETON_HURT_BY_SHOTGUN2:
				if ((item->Animation.ActiveState == STATE_SKELETON_HURT_BY_SHOTGUN1 
					|| item->Animation.ActiveState == STATE_SKELETON_HURT_BY_SHOTGUN2) &&
					item->Animation.FrameNumber < g_Level.Anims[item->Animation.AnimNumber].frameBase + 20)
				{
					item->HitPoints = 25;
					creature->MaxTurn = 0;
					break;
				}
				if (item->Animation.ActiveState == 11)
				{
					creature->MaxTurn = 0;
					break;
				}

				item->HitPoints = 25;
				creature->LOT.IsJumping = false;

				roomNumber = item->RoomNumber;
				floor = GetFloor(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, &roomNumber);
				if (GetFloorHeight(floor, item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z) <= item->Pose.Position.y + 1024)
				{
					if (!(GetRandomControl() & 0x1F))
					{
						item->Animation.TargetState = 14;
					}
				}
				else
				{
					creature->MaxTurn = 0;
					item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 47;
					item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
					item->Animation.ActiveState = 24;
					item->Animation.Airborne = true;
				}
				break;

			case STATE_SKELETON_JUMP_LEFT:
			case STATE_SKELETON_JUMP_RIGHT:
				creature->Alerted = false;
				creature->MaxTurn = 0;
				item->Status = ITEM_ACTIVE;
				break;

			case STATE_SKELETON_UNDER_FLOOR:
				if (item->Animation.FrameNumber - g_Level.Anims[item->Animation.AnimNumber].frameBase < 32)
				{
					TriggerRiseEffect(item);
				}
				break;

			default:
				break;
			}

			CreatureAnimation(itemNumber, angle, 0);
		}
	}
}
