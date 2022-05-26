#include "framework.h"
#include "tr4_horseman.h"
#include "Game/items.h"
#include "Game/effects/effects.h"
#include "Specific/setup.h"
#include "Specific/level.h"
#include "Game/collision/collide_room.h"
#include "Game/control/control.h"
#include "Specific/trmath.h"
#include "Game/Lara/lara.h"
#include "Game/misc.h"
#include "Sound/sound.h"
#include "Game/collision/sphere.h"
#include "Game/control/box.h"
#include "Game/animation.h"

namespace TEN::Entities::TR4
{
	BITE_INFO HorseBite1 = { 0, 0, 0, 13 };
	BITE_INFO HorseBite2 = { 0, 0, 0, 17 };
	BITE_INFO HorseBite3 = { 0, 0, 0, 19 };
	BITE_INFO HorsemanBite1 = { 0, 0, 0, 6 };
	BITE_INFO HorsemanBite2 = { 0, 0, 0, 14 };
	BITE_INFO HorsemanBite3 = { 0, 0, 0, 10 };

	enum HorsemanState
	{
		HORSEMAN_STATE_HORSE_RUN = 1,
		HORSEMAN_STATE_HORSE_WALK = 2,
		HORSEMAN_STATE_HORSE_IDLE = 3,
		HORSEMAN_STATE_HORSE_REARING = 4,
		HORSEMAN_STATE_MOUNT_HORSE = 5,
	};

	// TODO
	enum HorsemanAnim
	{

	};

	static void HorsemanSparks(Vector3Int* pos, int param1, int num)
	{
		for (int i = 0; i < num; i++)
		{
			auto* spark = &Sparks[GetFreeSpark()];

			int random = GetRandomControl();

			spark->on = 1;
			spark->sG = -128;
			spark->sB = (random & 0xF) + 16;
			spark->sR = 0;
			spark->dG = 96;
			spark->dB = ((random / 16) & 0x1F) + 48;
			spark->dR = 0;
			spark->colFadeSpeed = 2;
			spark->fadeToBlack = 4;
			spark->life = 9;
			spark->sLife = 9;
			spark->transType = TransTypeEnum::COLADD;
			spark->x = pos->x;
			spark->y = pos->y;
			spark->z = pos->z;
			spark->friction = 34;
			spark->yVel = (random & 0xFFF) - 2048;
			spark->flags = SP_NONE;
			spark->gravity = (random / 128) & 0x1F;
			spark->maxYvel = 0;
			spark->zVel = phd_cos((random & 0x7FF) + param1 - 1024) * 4096;
			spark->xVel = -phd_sin((random & 0x7FF) + param1 - 1024) * 4096;
		}

		for (int i = 0; i < num; i++)
		{
			auto* spark = &Sparks[GetFreeSpark()];

			int random = GetRandomControl();

			spark->on = 1;
			spark->sG = -128;
			spark->sR = 0;
			spark->dG = 96;
			spark->sB = (random & 0xF) + 16;
			spark->dR = 0;
			spark->colFadeSpeed = 2;
			spark->fadeToBlack = 4;
			spark->dB = ((random / 16) & 0x1F) + 48;
			spark->life = 9;
			spark->sLife = 9;
			spark->transType = TransTypeEnum::COLADD;
			spark->x = pos->x;
			spark->y = pos->y;
			spark->z = pos->z;
			spark->yVel = (random & 0xFFF) - 2048;
			spark->gravity = (random / 128) & 0x1F;
			spark->rotAng = random / 8;

			if (random & 1)
				spark->rotAdd = -16 - (random & 0xF);
			else
				spark->rotAdd = spark->sB;
			
			spark->scalar = 3;
			spark->friction = 34;
			spark->sSize = spark->size = ((random / 32) & 7) + 4;
			spark->dSize = spark->sSize / 2;
			spark->flags = SP_DEF | SP_ROTATE | SP_SCALE;
			spark->maxYvel = 0;
			spark->xVel = -phd_sin((random & 0x7FF) + param1 - 1024) * 4096;
			spark->zVel = phd_cos((random & 0x7FF) + param1 - 1024) * 4096;
		}
	}

	void InitialiseHorse(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];
		auto* object = &Objects[ID_HORSE];

		item->Animation.AnimNumber = object->animIndex + 2;
		item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
		item->Animation.TargetState = HORSEMAN_STATE_HORSE_RUN;
		item->Animation.ActiveState = HORSEMAN_STATE_HORSE_RUN;
	}

	void InitialiseHorseman(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];
		auto* object = &Objects[ID_HORSEMAN];

		ClearItem(itemNumber);

		item->Animation.AnimNumber = object->animIndex + 8;
		item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
		item->Animation.TargetState = 9;
		item->Animation.ActiveState = 9;
		item->ItemFlags[0] = NO_ITEM; // No horse yet
	}

	void HorsemanControl(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;

		auto* item = &g_Level.Items[itemNumber];
		auto* creature = GetCreatureInfo(item);

		// Try to find the horse
		if (item->ItemFlags[0] == NO_ITEM)
		{
			for (int i = 0; i < g_Level.NumItems; i++)
			{
				auto* currentItem = &g_Level.Items[i];

				if (currentItem->ObjectNumber == ID_HORSE && item->TriggerFlags == currentItem->TriggerFlags)
				{
					item->ItemFlags[0] = i;
					currentItem->Flags |= 0x20;
				}
			}
		}

		// If no horse was found, then set it to 0 so it won't be searched anymore in the future
		if (item->ItemFlags[0] == NO_ITEM)
			item->ItemFlags[0] = 0;

		// The horse
		ItemInfo* horseItem = NULL;
		if (item->ItemFlags[0] != 0)
			horseItem = &g_Level.Items[item->ItemFlags[0]];

		int xRot;

		if (horseItem != NULL)
		{
			int x = horseItem->Pose.Position.x + 341 * phd_sin(horseItem->Pose.Orientation.y);
			int y = horseItem->Pose.Position.y;
			int z = horseItem->Pose.Position.z + 341 * phd_cos(horseItem->Pose.Orientation.y);

			auto probe = GetCollision(x, y, z, item->RoomNumber);
			int height1 = probe.Position.Floor;

			x = horseItem->Pose.Position.x - 341 * phd_sin(horseItem->Pose.Orientation.y);
			y = horseItem->Pose.Position.y;
			z = horseItem->Pose.Position.z - 341 * phd_cos(horseItem->Pose.Orientation.y);

			int height2 = GetCollision(x, y, z, probe.RoomNumber).Position.Floor;

			xRot = phd_atan(682, height2 - height1);
		}

		short angle;

		if (item->HitPoints <= 0)
		{
			item->HitPoints = 0;
			if (item->ItemFlags[1] == 0)
			{
				if (item->Animation.ActiveState != 16)
				{
					item->Animation.AnimNumber = Objects[ID_HORSEMAN].animIndex + 21;
					item->Animation.ActiveState = 16;
					item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;

					if (item->ItemFlags[0])
					{
						//TODO: horseItem->AfterDeath = 1;
						item->ItemFlags[0] = 0;
					}
				}
			}
			else
			{
				item->HitPoints = 100;
				item->AIBits = 0;
				item->ItemFlags[1] = 0;
				item->Animation.AnimNumber = Objects[ID_HORSEMAN].animIndex + 3;
				item->Animation.ActiveState = 8;
				item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;

				creature->Enemy = NULL;
				horseItem->Animation.TargetState = HORSEMAN_STATE_HORSE_RUN;
			}
		}
		else
		{
			if (item->AIBits)
				GetAITarget(creature);
			else if (creature->HurtByLara)
				creature->Enemy = LaraItem;

			AI_INFO AI;
			CreatureAIInfo(item, &AI);

			AI_INFO laraAI;
			if (creature->Enemy == LaraItem)
			{
				laraAI.angle = AI.angle;
				laraAI.distance = AI.distance;
			}
			else
			{
				int deltaX = LaraItem->Pose.Position.z - item->Pose.Position.z;
				int deltaZ = LaraItem->Pose.Position.z - item->Pose.Position.z;

				laraAI.angle = phd_atan(deltaZ, deltaX) - item->Pose.Orientation.y;
				laraAI.distance = pow(deltaX, 2) + pow(deltaZ, 2);
			}

			short tilt = 0;

			if (item->HitStatus &&
				laraAI.angle < ANGLE(67.5f) &&
				laraAI.angle > -ANGLE(67.5f) &&
				laraAI.distance < pow(SECTOR(2), 2))
			{
				if (item->Animation.ActiveState != 15)
				{
					if (laraAI.angle <= 0)
					{
						if (item->ItemFlags[1])
						{
							if (!item->ItemFlags[1])
							{
								if (item->MeshBits & 0x400)
									item->Animation.RequiredState = 15;  
							}
						}
						else
						{
							if (laraAI.angle > 0 || !(item->MeshBits & 0x400))
							{
								if (Lara.Control.Weapon.GunType == LaraWeaponType::Shotgun)
								{
									item->HitPoints -= 10;
									item->HitStatus = true;
								}
								else if (Lara.Control.Weapon.GunType == LaraWeaponType::Revolver)
								{
									item->HitPoints -= 20;
									item->HitStatus = true;
								}
								else
									item->HitPoints--;

								SoundEffect(SFX_TR4_HORSEMAN_TAKEHIT, &item->Pose, 0);
								SoundEffect(SFX_TR4_HORSE_RICOCHETS, &item->Pose, 0);

								Vector3Int pos = { 0, -128, 80 };
								GetJointAbsPosition(item, &pos, SPHERES_SPACE_WORLD);
								HorsemanSparks(&pos, item->Pose.Orientation.y, 7);
							}
							else if (!(GetRandomControl() & 7))
							{
								if (item->Animation.ActiveState == 15)
									item->Animation.TargetState = 9;
								
								ExplodeItemNode(item, 10, 1, -24);
							}
						}
					}
				}
			}

			creature->HurtByLara = false;

			GetCreatureMood(item, &AI, VIOLENT);
			CreatureMood(item, &AI, VIOLENT);

			angle = CreatureTurn(item, creature->MaxTurn);

			switch (item->Animation.ActiveState)
			{
			case HORSEMAN_STATE_HORSE_RUN:
				creature->MaxTurn = ANGLE(3.0f);
				horseItem->Animation.TargetState = HORSEMAN_STATE_HORSE_WALK;
				if (item->Animation.RequiredState)
				{
					item->Animation.TargetState = 17;
					horseItem->Animation.TargetState = HORSEMAN_STATE_MOUNT_HORSE;
				}
				else if (creature->Flags ||
					creature->ReachedGoal ||
					item->HitStatus &&
					!GetRandomControl())
				{
					if (laraAI.distance > pow(SECTOR(4), 2) ||
						creature->ReachedGoal)
					{
						creature->Flags = 0;
						creature->Enemy = LaraItem;

						if (laraAI.angle > -8192 && laraAI.angle < 0x2000)
						{
							item->Animation.TargetState = HORSEMAN_STATE_HORSE_IDLE;
							horseItem->Animation.TargetState = HORSEMAN_STATE_HORSE_RUN;
						}
					}
					else
					{
						item->AIBits = FOLLOW;
						item->ItemFlags[3] = (-(item->ItemFlags[3] != 1)) + 2;
					}
				}

				if (AI.distance >= pow(SECTOR(1), 2))
				{
					if (AI.bite)
					{
						if (AI.angle >= -ANGLE(10.0f) ||
							AI.distance >= pow(SECTOR(1), 2) &&
							(AI.distance >= pow(1365, 2) ||
								AI.angle <= -ANGLE(20.0f)))
						{
							if (AI.angle > ANGLE(10.0f) &&
								(AI.distance < pow(SECTOR(1), 2) ||
									AI.distance < pow(1365, 2) &&
									AI.angle < ANGLE(20.0f)))
							{
								creature->MaxTurn = 0;
								item->Animation.TargetState = 6;
							}
						}
						else
						{
							creature->MaxTurn = 0;
							item->Animation.TargetState = 7;
						}
					}
				}
				else if (AI.bite)
				{
					if (AI.angle >= -ANGLE(10.0f) ||
						AI.angle <= ANGLE(10.0f))
					{
						if (AI.bite)
						{
							if (AI.angle >= -ANGLE(10.0f) ||
								AI.distance >= pow(SECTOR(1), 2) &&
								(AI.distance >= pow(1365, 2) ||
									AI.angle <= -ANGLE(20.0f)))
							{
								if (AI.angle > ANGLE(10.0f) &&
									(AI.distance < pow(SECTOR(1), 2) ||
										AI.distance < pow(1365, 2) &&
										AI.angle < ANGLE(20.0f)))
								{
									creature->MaxTurn = 0;
									item->Animation.TargetState = 6;
								}
							}
							else
							{
								creature->MaxTurn = 0;
								item->Animation.TargetState = 7;
							}
						}
					}
				}

				break;

			case 2:
				creature->MaxTurn = 273;

				if (laraAI.distance > pow(SECTOR(4), 2) || creature->ReachedGoal || creature->Enemy == LaraItem)
				{
					item->Animation.TargetState = HORSEMAN_STATE_HORSE_RUN;
					horseItem->Animation.TargetState = HORSEMAN_STATE_HORSE_WALK;
					creature->ReachedGoal = false;
					creature->Enemy = LaraItem;
					creature->Flags = 0;
				}

				break;

			case 3:
				creature->MaxTurn = 0;
				horseItem->Animation.TargetState = HORSEMAN_STATE_HORSE_RUN;

				if (creature->Flags)
				{
					item->AIBits = FOLLOW;
					item->ItemFlags[3] = -(item->ItemFlags[3] != 1) + 2;
				}
				else
					creature->Flags = 0;

				if (item->Animation.RequiredState)
				{
					item->Animation.TargetState = HORSEMAN_STATE_HORSE_RUN;
					horseItem->Animation.TargetState = HORSEMAN_STATE_HORSE_WALK;
					horseItem->Flags = 0;
				}
				else if (creature->ReachedGoal ||
					!horseItem->Flags &&
					AI.distance < pow(SECTOR(1), 2) &&
					AI.bite &&
					AI.angle < ANGLE(10.0f) &&
					AI.angle > -ANGLE(10.0f))
				{
					item->Animation.TargetState = HORSEMAN_STATE_HORSE_REARING;

					if (creature->ReachedGoal)
						item->Animation.RequiredState = 17;
					
					horseItem->Flags = 0;
				}
				else
				{
					item->Animation.TargetState = HORSEMAN_STATE_HORSE_RUN;
					horseItem->Animation.TargetState = HORSEMAN_STATE_HORSE_WALK;
					horseItem->Flags = 0;
				}

				break;

			case 4:
				creature->MaxTurn = 0;

				if (item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameBase)
				{
					horseItem->Animation.AnimNumber = Objects[ID_HORSE].animIndex + 1;
					horseItem->Animation.ActiveState = HORSEMAN_STATE_HORSE_REARING;
					horseItem->Animation.FrameNumber = g_Level.Anims[horseItem->Animation.AnimNumber].frameBase;
				}

				if (!horseItem->Flags)
				{
					if (horseItem->TouchBits & 0x22000)
					{
						if (horseItem->TouchBits & 0x2000)
						{
							CreatureEffect2(
								horseItem,
								&HorseBite1,
								10,
								-1,
								DoBloodSplat);
						}
						else
						{
							CreatureEffect2(
								horseItem,
								&HorseBite2,
								10,
								-1,
								DoBloodSplat);
						}

						horseItem->Flags = 1;

						LaraItem->HitPoints -= 150;
						LaraItem->HitStatus = true;
					}
				}

				break;

			case 6:
				if (!creature->Flags)
				{
					if (item->TouchBits & 0x60)
					{

						CreatureEffect2(
							item,
							&HorsemanBite1,
							10,
							item->Pose.Orientation.y,
							DoBloodSplat);

						creature->Flags = 1;

						LaraItem->HitPoints -= 250;
						LaraItem->HitStatus = true;
					}
				}
				if (item->HitStatus)
					item->Animation.TargetState = 9;

				break;

			case 7:
				if (!creature->Flags)
				{
					if (item->TouchBits & 0x4000)
					{

						CreatureEffect2(
							item,
							&HorsemanBite2,
							3,
							item->Pose.Orientation.y,
							DoBloodSplat);

						creature->Flags = 1;

						LaraItem->HitPoints -= 100;
						LaraItem->HitStatus = true;
					}
				}

				break;

			case 9:
				creature->MaxTurn = 0;
				creature->Flags = 0;

				if (!item->AIBits || item->ItemFlags[3])
				{
					if (item->Animation.RequiredState)
						item->Animation.TargetState = item->Animation.RequiredState;
					else if (AI.bite && AI.distance < pow(682,2))
						item->Animation.TargetState = 14;
					else if (AI.distance < pow(SECTOR(6), 2) && AI.distance > pow(682, 2))
						item->Animation.TargetState = 10;
				}
				else
					item->Animation.TargetState = 10;

				break;

			case 10:
				creature->MaxTurn = ANGLE(3.0f);
				creature->Flags = 0;

				if (creature->ReachedGoal)
				{
					item->AIBits = 0;
					item->ItemFlags[1] = 1;

					item->Pose.Position.x = horseItem->Pose.Position.x;
					item->Pose.Position.y = horseItem->Pose.Position.y;
					item->Pose.Position.z = horseItem->Pose.Position.z;
					item->Pose.Orientation.x = horseItem->Pose.Orientation.x;
					item->Pose.Orientation.y = horseItem->Pose.Orientation.y;
					item->Pose.Orientation.z = horseItem->Pose.Orientation.z;

					creature->ReachedGoal = false;
					creature->Enemy = NULL;

					item->Animation.AnimNumber = Objects[ID_HORSEMAN].animIndex + 14;
					item->Animation.ActiveState = HORSEMAN_STATE_MOUNT_HORSE;
					item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;

					creature->MaxTurn = 0;
					break;
				}

				if (item->HitStatus)
				{
					item->Animation.TargetState = 9;
				}
				else if (AI.bite && AI.distance < pow(682, 2))
				{
					if (GetRandomControl() & 1)
						item->Animation.TargetState = 12;
					else if (GetRandomControl() & 1)
						item->Animation.TargetState = 13;
					else
						item->Animation.TargetState = 9;
				}
				else if (AI.distance < pow(SECTOR(5), 2) && AI.distance > pow(1365, 2))
					item->Animation.TargetState = 11;

				break;

			case 11:
				if (AI.distance < pow(1365, 2))
					item->Animation.TargetState = 10;

				break;

			case 12:
			case 13:
			case 14:
				creature->MaxTurn = 0;
				if (abs(AI.angle) >= ANGLE(3.0f))
				{
					if (AI.angle >= 0)
						item->Pose.Orientation.y += ANGLE(3.0f);
					else
						item->Pose.Orientation.y -= ANGLE(3.0f);
				}
				else
					item->Pose.Orientation.y += AI.angle;

				if (!creature->Flags)
				{
					if (item->TouchBits & 0x4000)
					{
						LaraItem->HitPoints -= 100;
						LaraItem->HitStatus = true;

						CreatureEffect2(
							item,
							&HorsemanBite2,
							3,
							item->Pose.Orientation.y,
							DoBloodSplat);

						creature->Flags = 1;
					}
				}

				break;

			case 15:
				if (Lara.TargetEntity != item || AI.bite && AI.distance < pow(682, 2))
					item->Animation.TargetState = 9;

				break;

			case 17:
				creature->ReachedGoal = false;
				creature->MaxTurn = 546;

				if (!horseItem->Flags)
				{
					if (horseItem->TouchBits & 0xA2000)
					{
						LaraItem->HitPoints -= 150;
						LaraItem->HitStatus = true;

						if (horseItem->TouchBits & 0x2000)
						{
							CreatureEffect2(
								horseItem,
								&HorseBite1,
								10,
								-1,
								DoBloodSplat);
						}

						if (horseItem->TouchBits & 0x20000)
						{
							CreatureEffect2(
								horseItem,
								&HorseBite2,
								10,
								-1,
								DoBloodSplat);
						}

						if (horseItem->TouchBits & 0x80000)
						{
							CreatureEffect2(
								horseItem,
								&HorseBite3,
								10,
								-1,
								DoBloodSplat);
						}

						horseItem->Flags = 1;
					}
				}

				if (!creature->Flags)
				{
					if (item->TouchBits & 0x460)
					{
						LaraItem->HitStatus = true;

						if (item->TouchBits & 0x60)
						{
							CreatureEffect2(
								horseItem,
								&HorsemanBite1,
								20,
								-1,
								DoBloodSplat);
							LaraItem->HitPoints -= 250;
						}
						else if (item->TouchBits & 0x400)
						{
							CreatureEffect2(
								horseItem,
								&HorsemanBite3,
								10,
								-1,
								DoBloodSplat);
							LaraItem->HitPoints -= 150;
						}

						creature->Flags = 1;
					}
				}

				if (item->Animation.AnimNumber == Objects[ID_HORSEMAN].animIndex + 29 &&
					item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameBase)
				{
					horseItem->Animation.AnimNumber = Objects[ID_HORSE].animIndex + 10;
					horseItem->Animation.FrameNumber = g_Level.Anims[horseItem->Animation.AnimNumber].frameBase;
				}

				if (laraAI.distance > pow(SECTOR(4), 2) || creature->ReachedGoal)
				{
					creature->ReachedGoal = false;
					creature->Flags = 0;
					creature->Enemy = LaraItem;
				}
				else if (!AI.ahead)
				{
					item->Animation.TargetState = HORSEMAN_STATE_HORSE_IDLE;
					horseItem->Animation.TargetState = HORSEMAN_STATE_HORSE_RUN;
				}

				break;

			default:
				break;
			}

			if (horseItem && item->ItemFlags[1])
			{
				if (abs(xRot - item->Pose.Orientation.x) < ANGLE(1.4f))
					item->Pose.Orientation.x = xRot;
				else if (xRot <= item->Pose.Orientation.x)
				{
					if (xRot < item->Pose.Orientation.x)
						item->Pose.Orientation.x -= ANGLE(1.4f);
				}
				else
					item->Pose.Orientation.x += ANGLE(1.4f);

				horseItem->Pose.Position.x = item->Pose.Position.x;
				horseItem->Pose.Position.y = item->Pose.Position.y;
				horseItem->Pose.Position.z = item->Pose.Position.z;
				horseItem->Pose.Orientation.x = item->Pose.Orientation.x;
				horseItem->Pose.Orientation.y = item->Pose.Orientation.y;
				horseItem->Pose.Orientation.z = item->Pose.Orientation.z;

				if (horseItem->RoomNumber != item->RoomNumber)
					ItemNewRoom(item->ItemFlags[0], item->RoomNumber);
				
				AnimateItem(horseItem);
			}
		}

		Objects[ID_HORSEMAN].radius = item->ItemFlags[1] != 0 ? 409 : 170;
		CreatureAnimation(itemNumber, angle, 0);
	}
}
