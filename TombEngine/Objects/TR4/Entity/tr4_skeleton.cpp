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
#include "Game/misc.h"

namespace TEN::Entities::TR4
{
	BITE_INFO SkeletonBite = { 0, -16, 200, 11 };

	void TriggerRiseEffect(ItemInfo* item)
	{
		short fxNumber = CreateNewEffect(item->RoomNumber);
		if (fxNumber != NO_ITEM)
		{
			auto* fx = &EffectList[fxNumber];

			fx->pos.Position.x = (byte)GetRandomControl() + item->Pose.Position.x - 128;
			fx->pos.Position.y = GetCollision(item).Position.Floor;
			fx->pos.Position.z = (byte)GetRandomControl() + item->Pose.Position.z - 128;
			fx->roomNumber = item->RoomNumber;
			fx->pos.Orientation.y = 2 * GetRandomControl();
			fx->speed = GetRandomControl() / 2048;
			fx->fallspeed = -(GetRandomControl() / 1024);
			fx->frameNumber = Objects[103].meshIndex;
			fx->objectNumber = ID_BODY_PART;
			fx->shade = 0x4210;
			fx->flag2 = 0x601;

			auto* spark = &Sparks[GetFreeSpark()];
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
				spark->rotAdd = -16 - (GetRandomControl() & 0xF);
			else
				spark->rotAdd = (GetRandomControl() & 0xF) + 16;

			spark->gravity = -4 - (GetRandomControl() & 3);
			spark->scalar = 3;
			spark->maxYvel = -4 - (GetRandomControl() & 3);
			spark->sSize = spark->size = (GetRandomControl() & 0xF) + 8;
			spark->dSize = spark->size * 4;
		}
	}

	void InitialiseSkeleton(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];
		auto* object = &Objects[item->ObjectNumber];

		ClearItem(itemNumber);

		switch (item->TriggerFlags)
		{
		case 0:
			item->Animation.AnimNumber = object->animIndex;
			item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].FrameBase;
			item->Animation.TargetState = 0;
			item->Animation.ActiveState = 0;
			break;

		case 1:
			item->Animation.AnimNumber = object->animIndex + 37;
			item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].FrameBase;
			item->Animation.TargetState = SKELETON_STATE_JUMP_RIGHT;
			item->Animation.ActiveState = SKELETON_STATE_JUMP_RIGHT;
			break;

		case 2:
			item->Animation.AnimNumber = object->animIndex + 34;
			item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].FrameBase;
			item->Animation.TargetState = SKELETON_STATE_JUMP_LEFT;
			item->Animation.ActiveState = SKELETON_STATE_JUMP_LEFT;
			break;

		case 3:
			item->Animation.AnimNumber = object->animIndex;
			item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].FrameBase;
			item->Animation.TargetState = SKELETON_STATE_JUMP_LIE_DOWN;
			item->Animation.ActiveState = SKELETON_STATE_JUMP_LIE_DOWN;
			//item->status = ITEM_DEACTIVATED;
			break;

		}
	}

	void SkeletonControl(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;

		auto* item = &g_Level.Items[itemNumber];
		auto* creature = GetCreatureInfo(item);
		auto* enemyItem = creature->Enemy;

		bool jumpLeft = false;
		bool jumpRight = false;

		int distance = 0;
		short tilt = 0;
		short angle = 0;
		short joint1 = 0;
		short joint2 = 0;
		short joint3 = 0;
		short rot = 0;

		// Can skeleton jump? Check for a distance of 1 and 2 sectors.
		int x = item->Pose.Position.x;
		int y = item->Pose.Position.y;
		int z = item->Pose.Position.z;

		int dx = 870 * phd_sin(item->Pose.Orientation.y);
		int dz = 870 * phd_cos(item->Pose.Orientation.y);

		x += dx;
		z += dz;
		int height1 = GetCollision(x, y, z, item->RoomNumber).Position.Floor;

		x += dx;
		z += dz;
		int height2 = GetCollision(x, y, z, item->RoomNumber).Position.Floor;

		x += dx;
		z += dz;
		int height3 = GetCollision(x, y, z, item->RoomNumber).Position.Floor;

		int height = 0;
		bool canJump1sector = true;
		if (enemyItem && item->BoxNumber == LaraItem->BoxNumber && item->MeshBits & 0x200 ||
			y >= height1 - CLICK(1.5f) ||
			y >= height2 + CLICK(2) ||
			y <= height2 - CLICK(2))
		{
			height = height2;
			canJump1sector = false;
		}

		bool canJump2sectors = true;
		if (enemyItem && item->BoxNumber == LaraItem->BoxNumber && item->MeshBits & 0x200 ||
			y >= height1 - CLICK(1.5f) ||
			y >= height - CLICK(1.5f) ||
			y >= height3 + CLICK(2) ||
			y <= height3 - CLICK(2))
		{
			canJump2sectors = false;
		}

		if (item->AIBits)
			GetAITarget(creature);
		else
			creature->Enemy = LaraItem;

		AI_INFO AI;
		CreatureAIInfo(item, &AI);

		if (item->HitStatus &&
			Lara.Control.Weapon.GunType == LaraWeaponType::Shotgun &&
			AI.distance < pow(SECTOR(3.5f), 2) &&
			item->Animation.ActiveState != 7 &&
			item->Animation.ActiveState != 17 &&
			item->Animation.ActiveState != SKELETON_STATE_HURT_BY_SHOTGUN_1 &&
			item->Animation.ActiveState != SKELETON_STATE_HURT_BY_SHOTGUN_2 &&
			item->Animation.ActiveState != 25)
		{
			if (AI.angle >= ANGLE(67.5f) || AI.angle <= -ANGLE(67.5f))
			{
				item->Animation.ActiveState = SKELETON_STATE_HURT_BY_SHOTGUN_2;
				item->Animation.AnimNumber = Objects[ID_SKELETON].animIndex + 33;
				item->Pose.Orientation.y += AI.angle + -32768;
			}
			else
			{
				item->Animation.ActiveState = SKELETON_STATE_HURT_BY_SHOTGUN_1;
				item->Animation.AnimNumber = Objects[ID_SKELETON].animIndex + 17;
				item->Pose.Orientation.y += AI.angle;
			}

			item->HitPoints = 25;
			item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].FrameBase;
			creature->LOT.IsJumping = true;
		}
		else
		{
			AI_INFO laraAI;
			if (creature->Enemy == LaraItem)
			{
				laraAI.distance = AI.distance;
				laraAI.angle = AI.angle;
			}
			else
			{
				dx = LaraItem->Pose.Position.x - item->Pose.Position.x;
				dz = LaraItem->Pose.Position.z - item->Pose.Position.z;
				laraAI.angle = phd_atan(dz, dx) - item->Pose.Orientation.y;
				laraAI.distance = pow(dx, 2) + pow(dz, 2);
			}

			GetCreatureMood(item, &AI, VIOLENT);

			if (!(item->MeshBits & 0x200))
				creature->Mood = MoodType::Escape;

			CreatureMood(item, &AI, VIOLENT);

			angle = CreatureTurn(item, creature->MaxTurn);

			auto* tempEnemy = creature->Enemy;

			creature->Enemy = LaraItem;

			if (item->HitStatus ||
				distance < pow(SECTOR(1), 2) ||
				TargetVisible(item, &laraAI))
			{
				creature->Alerted = true;
			}

			creature->Enemy = tempEnemy;

			if (item != Lara.TargetEntity || laraAI.distance <= 870 || angle <= -ANGLE(56.25f) || angle >= ANGLE(56.25f))
			{
				jumpLeft = false;
				jumpRight = false;
			}
			else
			{
				dx = 870 * phd_sin(item->Pose.Orientation.y + ANGLE(45.0f));
				dz = 870 * phd_cos(item->Pose.Orientation.y + ANGLE(45.0f));

				x = item->Pose.Position.x + dx;
				y = item->Pose.Position.y;
				z = item->Pose.Position.z + dz;
				int height4 = GetCollision(x, y, z, item->RoomNumber).Position.Floor;

				dx = 870 * phd_sin(item->Pose.Orientation.y + ANGLE(78.75f));
				dz = 870 * phd_cos(item->Pose.Orientation.y + ANGLE(78.75f));

				x = item->Pose.Position.x + dx;
				y = item->Pose.Position.y;
				z = item->Pose.Position.z + dz;
				int height5 = GetCollision(x, y, z, item->RoomNumber).Position.Floor;

				if (abs(height5 - item->Pose.Position.y) > CLICK(1))
					jumpRight = false;
				else
				{
					jumpRight = true;

					if ((height4 + CLICK(2)) >= item->Pose.Position.y)
						jumpRight = false;
				}

				dx = 870 * phd_sin(item->Pose.Orientation.y - ANGLE(45.0f));
				dz = 870 * phd_cos(item->Pose.Orientation.y - ANGLE(45.0f));

				x = item->Pose.Position.x + dx;
				y = item->Pose.Position.y;
				z = item->Pose.Position.z + dz;
				int height6 = GetCollision(x, y, z, item->RoomNumber).Position.Floor;

				dx = 870 * phd_sin(item->Pose.Orientation.y - ANGLE(78.75f));
				dz = 870 * phd_cos(item->Pose.Orientation.y - ANGLE(78.75f));

				x = item->Pose.Position.x + dx;
				y = item->Pose.Position.y;
				z = item->Pose.Position.z + dz;
				int height7 = GetCollision(x, y, z, item->RoomNumber).Position.Floor;

				if (abs(height7 - item->Pose.Position.y) > CLICK(1) || height6 + CLICK(2) >= item->Pose.Position.y)
					jumpLeft = false;
				else
					jumpLeft = true;

			}

			switch (item->Animation.ActiveState)
			{
			case SKELETON_STATE_IDLE:
				if (!(GetRandomControl() & 0xF))
					item->Animation.TargetState = 2;
				
				break;

			case 2:
				creature->MaxTurn = (creature->Mood != MoodType::Escape) ? ANGLE(2.0f) : 0;
				creature->LOT.IsJumping = false;
				creature->Flags = 0;

				if (item->AIBits & GUARD ||
					!(GetRandomControl() & 0x1F) &&
					(AI.distance > pow(SECTOR(1), 2) ||
						creature->Mood != MoodType::Attack))
				{
					if (!(GetRandomControl() & 0x3F))
					{
						if (GetRandomControl() & 1)
							item->Animation.TargetState = 3;
						else
							item->Animation.TargetState = 4;
					}
				}
				else
				{
					if (item->AIBits & PATROL1)
						item->Animation.TargetState = 15;
					else if (canJump1sector || canJump2sectors)
					{
						item->Animation.AnimNumber = Objects[ID_SKELETON].animIndex + 40;
						item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].FrameBase;
						item->Animation.ActiveState = SKELETON_STATE_JUMP_LEFT;
						creature->MaxTurn = 0;

						if (!canJump2sectors)
						{
							item->Animation.TargetState = SKELETON_STATE_JUMP_1_BLOCK;
							creature->LOT.IsJumping = true;
						}
						else
						{
							item->Animation.TargetState = SKELETON_STATE_JUMP_2_BLOCKS;
							creature->LOT.IsJumping = true;
						}
					}
					else if (jumpLeft)
					{
						item->Animation.AnimNumber = Objects[ID_SKELETON].animIndex + 34;
						item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].FrameBase;
						item->Animation.TargetState = SKELETON_STATE_JUMP_LEFT;
						item->Animation.ActiveState = SKELETON_STATE_JUMP_LEFT;
					}
					else if (jumpRight)
					{
						item->Animation.AnimNumber = Objects[ID_SKELETON].animIndex + 37;
						item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].FrameBase;
						item->Animation.TargetState = SKELETON_STATE_JUMP_RIGHT;
						item->Animation.ActiveState = SKELETON_STATE_JUMP_RIGHT;
					}
					else
					{
						if (creature->Mood == MoodType::Escape)
						{
							if (Lara.TargetEntity == item ||
								!AI.ahead ||
								item->HitStatus ||
								!(item->MeshBits & 0x200))
							{
								item->Animation.TargetState = 15;
								break;
							}

							item->Animation.TargetState = 2;
						}
						else if (creature->Mood == MoodType::Bored ||
							item->AIBits & FOLLOW &&
							(creature->ReachedGoal ||
								laraAI.distance > pow(SECTOR(2), 2)))
						{
							if (item->Animation.RequiredState)
								item->Animation.TargetState = item->Animation.RequiredState;
							else if (!(GetRandomControl() & 0x3F))
								item->Animation.TargetState = 15;
						}
						else if (Lara.TargetEntity == item &&
							laraAI.angle &&
							laraAI.distance < pow(SECTOR(2), 2) &&
							GetRandomControl() & 1 &&
							(Lara.Control.Weapon.GunType == LaraWeaponType::Shotgun || !(GetRandomControl() & 0xF)) &&
							item->MeshBits == -1)
						{
							item->Animation.TargetState = SKELETON_STATE_USE_SHIELD;
						}
						else if (AI.bite && AI.distance < pow(682, 2))
						{
							if (GetRandomControl() & 3 && LaraItem->HitPoints > 0)
							{
								if (GetRandomControl() & 1)
									item->Animation.TargetState = SKELETON_STATE_ATTACK_1;
								else
									item->Animation.TargetState = SKELETON_STATE_ATTACK_2;
							}
							else
								item->Animation.TargetState = SKELETON_STATE_ATTACK_3;
						}
						else if (item->HitStatus || item->Animation.RequiredState)
						{
							if (GetRandomControl() & 1)
							{
								item->Animation.TargetState = SKELETON_STATE_AVOID_ATTACK_1;
								item->Animation.RequiredState = item->Animation.TargetState;
							}
							else
							{
								item->Animation.TargetState = SKELETON_STATE_AVOID_ATTACK_2;
								item->Animation.RequiredState = item->Animation.TargetState;
							}
						}
						else
							item->Animation.TargetState = 15;
					}
				}

				break;

			case 15:
				creature->MaxTurn = (creature->Mood != MoodType::Bored) ? 1092 : 364;
				creature->LOT.IsJumping = false;
				creature->Flags = 0;

				if (item->AIBits & PATROL1)
					item->Animation.TargetState = 15;
				else if (item->HitStatus)
				{
					item->Animation.TargetState = 2;
					if (GetRandomControl() & 1)
						item->Animation.RequiredState = 5;
					else
						item->Animation.RequiredState = 6;
				}
				else
				{
					if (jumpLeft || jumpRight)
					{
						item->Animation.TargetState = 2;
						break;
					}
					if (creature->Mood == MoodType::Escape)
						item->Animation.TargetState = 16;
					else if (creature->Mood != MoodType::Bored)
					{
						if (AI.distance >= pow(682, 2))
						{
							if (AI.bite && AI.distance < pow(SECTOR(1), 2))
								item->Animation.TargetState = 18;
							else if (canJump1sector || canJump2sectors)
							{
								item->Animation.TargetState = 2;
								creature->MaxTurn = 0;
							}
							else if (!AI.ahead || AI.distance > pow(SECTOR(2), 2))
								item->Animation.TargetState = 16;
						}
						else
							item->Animation.TargetState = 2;
					}
					else if (!(GetRandomControl() & 0x3F))
						item->Animation.TargetState = 2;
				}

				break;

			case 16:
				creature->MaxTurn = ANGLE(7.0f);
				creature->LOT.IsJumping = false;

				if (item->AIBits & GUARD || canJump1sector || canJump2sectors)
				{
					if (item->MeshBits & 0x200)
					{
						item->Animation.TargetState = 2;
						creature->MaxTurn = 0;
						break;
					}

					creature->LOT.IsJumping = true;

					if (GetCollision(item).Position.Floor > item->Pose.Position.y + SECTOR(1))
					{
						item->Animation.AnimNumber = Objects[ID_SKELETON].animIndex + 44;
						item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].FrameBase;
						item->Animation.ActiveState = 23;
						item->Animation.Airborne = true;
						creature->MaxTurn = 0;
						creature->LOT.IsJumping = false;
					}
				}
				else
				{
					if (creature->Mood == MoodType::Escape)
					{
						if (Lara.TargetEntity != item && AI.ahead && (item->MeshBits & 0x200))
							item->Animation.TargetState = 2;
					}
					else if (item->AIBits & FOLLOW && (creature->ReachedGoal || laraAI.distance > pow(SECTOR(2), 2)))
						item->Animation.TargetState = 2;
					else if (creature->Mood != MoodType::Bored)
					{
						if (AI.ahead && AI.distance < pow(SECTOR(2), 2))
							item->Animation.TargetState = 15;
					}
					else
						item->Animation.TargetState = 15;
				}

				break;

			case 10:
				creature->MaxTurn = 0;

				if (abs(AI.angle) >= 1092)
				{
					if (AI.angle >= 0)
						item->Pose.Orientation.y += ANGLE(6.0f);
					else
						item->Pose.Orientation.y -= ANGLE(6.0f);
				}
				else
					item->Pose.Orientation.y += AI.angle;

				if (!creature->Flags)
				{
					if (item->TouchBits & 0x18000)
					{
						CreatureEffect2(item, &SkeletonBite, 15, -1, DoBloodSplat);
						SoundEffect(SFX_TR4_LARA_THUD, &item->Pose);
						creature->Flags = 1;

						LaraItem->HitPoints -= 80;
						LaraItem->HitStatus = true;
					}
				}
				if (!(GetRandomControl() & 0x3F) || LaraItem->HitPoints <= 0)
					item->Animation.TargetState = 11;
				
				break;

			case SKELETON_STATE_ATTACK_1:
			case SKELETON_STATE_ATTACK_2:
			case 18:
				creature->MaxTurn = 0;

				if (abs(AI.angle) >= 1092)
				{
					if (AI.angle >= 0)
						item->Pose.Orientation.y += ANGLE(6.0f);
					else
						item->Pose.Orientation.y -= ANGLE(6.0f);
				}
				else
					item->Pose.Orientation.y += AI.angle;
				if (item->Animation.FrameNumber > g_Level.Anims[item->Animation.AnimNumber].FrameBase + 15)
				{
					auto* room = &g_Level.Rooms[item->RoomNumber];

					Vector3Int pos;
					GetJointAbsPosition(item, &pos, 16);

					auto floor = GetCollision(x, y, z, item->RoomNumber).Block;
					if (floor->Stopper)
					{
						for (int i = 0; i < room->mesh.size(); i++)
						{
							auto* staticMesh = &room->mesh[i];

							if (abs(pos.x - staticMesh->pos.Position.x) < SECTOR(1) && 
								abs(pos.z - staticMesh->pos.Position.z) < SECTOR(1) &&
								StaticObjects[staticMesh->staticNumber].shatterType != SHT_NONE)
							{
								ShatterObject(0, staticMesh, -128, LaraItem->RoomNumber, 0);
								SoundEffect(SFX_TR4_HIT_ROCK, &item->Pose);
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
							CreatureEffect2(item, &SkeletonBite, 10, item->Pose.Orientation.y, DoBloodSplat);
							SoundEffect(SFX_TR4_LARA_THUD, &item->Pose);
							creature->Flags = 1;

							LaraItem->HitPoints -= 80;
							LaraItem->HitStatus = true;
						}
					}
				}
				break;

			case SKELETON_STATE_USE_SHIELD:
				if (item->HitStatus)
				{
					if (item->MeshBits == -1 && laraAI.angle && Lara.Control.Weapon.GunType == LaraWeaponType::Shotgun)
					{
						if (GetRandomControl() & 3)
							item->Animation.TargetState = 17;
						else
							ExplodeItemNode(item, 11, 1, -24);
					}
					else
						item->Animation.TargetState = 2;
				}
				else if (Lara.TargetEntity != item || item->MeshBits != -1 || Lara.Control.Weapon.GunType != LaraWeaponType::Shotgun || !(GetRandomControl() & 0x7F))
					item->Animation.TargetState = 2;
				
				break;

			case SKELETON_STATE_JUMP_1_BLOCK:
				if (item->Animation.AnimNumber == Objects[item->ObjectNumber].animIndex + 43)
				{
					if (GetCollision(item).Position.Floor > (item->Pose.Position.y + CLICK(5)))
					{
						item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 44;
						item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].FrameBase;
						item->Animation.ActiveState = 23;
						item->Animation.Airborne = true;
						creature->MaxTurn = 0;
						creature->LOT.IsJumping = false;
					}
				}

				break;

			case 23:
			case 24:
				if (GetCollision(item).Position.Floor <= item->Pose.Position.y)
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
			case SKELETON_STATE_HURT_BY_SHOTGUN_1:
			case SKELETON_STATE_HURT_BY_SHOTGUN_2:
				if ((item->Animation.ActiveState == SKELETON_STATE_HURT_BY_SHOTGUN_1 ||
					item->Animation.ActiveState == SKELETON_STATE_HURT_BY_SHOTGUN_2) &&
					item->Animation.FrameNumber < g_Level.Anims[item->Animation.AnimNumber].FrameBase + 20)
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

				if (GetCollision(item).Position.Floor <= (item->Pose.Position.y + SECTOR(1)))
				{
					if (!(GetRandomControl() & 0x1F))
						item->Animation.TargetState = 14;
				}
				else
				{
					item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 47;
					item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].FrameBase;
					item->Animation.ActiveState = 24;
					item->Animation.Airborne = true;
					creature->MaxTurn = 0;
				}

				break;

			case SKELETON_STATE_JUMP_LEFT:
			case SKELETON_STATE_JUMP_RIGHT:
				item->Status = ITEM_ACTIVE;
				creature->MaxTurn = 0;
				creature->Alerted = false;

				break;

			case SKELETON_STATE_BENEATH_FLOOR:
				if (item->Animation.FrameNumber - g_Level.Anims[item->Animation.AnimNumber].FrameBase < 32)
					TriggerRiseEffect(item);
				
				break;

			default:
				break;
			}

			CreatureAnimation(itemNumber, angle, 0);
		}
	}
}
