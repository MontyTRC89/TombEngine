#include "framework.h"
#include "tr4_horseman.h"
#include "Game/items.h"
#include "Game/effects/effects.h"
#include "Specific/setup.h"
#include "Specific/level.h"
#include "Game/control/control.h"
#include "Specific/trmath.h"
#include "Game/Lara/lara.h"
#include "Sound/sound.h"
#include "Game/collision/sphere.h"
#include "Game/control/box.h"
#include "Game/animation.h"

namespace TEN::Entities::TR4
{
	enum HORSEMAN_STATES
	{
		STATE_HORSEMAN_HORSE_RUN = 1,
		STATE_HORSEMAN_HORSE_WALK = 2,
		STATE_HORSEMAN_HORSE_STOP = 3,
		STATE_HORSEMAN_HORSE_REARING = 4,
		STATE_HORSEMAN_GET_ON_HORSE = 5,
	};

	BITE_INFO horseBite1 = { 0, 0, 0, 0x0D };
	BITE_INFO horseBite2 = { 0, 0, 0, 0x11 };
	BITE_INFO horseBite3 = { 0, 0, 0, 0x13 };
	BITE_INFO horsemanBite1 = { 0, 0, 0, 0x06 };
	BITE_INFO horsemanBite2 = { 0, 0, 0, 0x0E };
	BITE_INFO horsemanBite3 = { 0, 0, 0, 0x0A };

	static void HorsemanSparks(PHD_VECTOR* pos, int param1, int num)
	{
		for (int i = 0; i < num; i++)
		{
			SPARKS* spark = &Sparks[GetFreeSpark()];

			int r = GetRandomControl();

			spark->on = 1;
			spark->sG = -128;
			spark->sB = (r & 0xF) + 16;
			spark->sR = 0;
			spark->dG = 96;
			spark->dB = ((r / 16) & 0x1F) + 48;
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
			spark->yVel = (r & 0xFFF) - 2048;
			spark->flags = SP_NONE;
			spark->gravity = (r / 128) & 0x1F;
			spark->maxYvel = 0;
			spark->zVel = phd_cos((r & 0x7FF) + param1 - 1024) * 4096;
			spark->xVel = -phd_sin((r & 0x7FF) + param1 - 1024) * 4096;
		}

		for (int i = 0; i < num; i++)
		{
			SPARKS* spark = &Sparks[GetFreeSpark()];

			int r = GetRandomControl();

			spark->on = 1;
			spark->sG = -128;
			spark->sR = 0;
			spark->dG = 96;
			spark->sB = (r & 0xF) + 16;
			spark->dR = 0;
			spark->colFadeSpeed = 2;
			spark->fadeToBlack = 4;
			spark->dB = ((r / 16) & 0x1F) + 48;
			spark->life = 9;
			spark->sLife = 9;
			spark->transType = TransTypeEnum::COLADD;
			spark->x = pos->x;
			spark->y = pos->y;
			spark->z = pos->z;
			spark->yVel = (r & 0xFFF) - 2048;
			spark->gravity = (r / 128) & 0x1F;
			spark->rotAng = r / 8;
			if (r & 1)
			{
				spark->rotAdd = -16 - (r & 0xF);
			}
			else
			{
				spark->rotAdd = spark->sB;
			}
			spark->scalar = 3;
			spark->friction = 34;
			spark->sSize = spark->size = ((r / 32) & 7) + 4;
			spark->dSize = spark->sSize / 2;
			spark->flags = SP_DEF | SP_ROTATE | SP_SCALE;
			spark->maxYvel = 0;
			spark->xVel = -phd_sin((r & 0x7FF) + param1 - 1024) * 4096;
			spark->zVel = phd_cos((r & 0x7FF) + param1 - 1024) * 4096;
		}
	}

	void InitialiseHorse(short itemNumber)
	{
		ITEM_INFO* item = &g_Level.Items[itemNumber];
		OBJECT_INFO* obj = &Objects[ID_HORSE];

		item->AnimNumber = obj->animIndex + 2;
		item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
		item->TargetState = STATE_HORSEMAN_HORSE_RUN;
		item->ActiveState = STATE_HORSEMAN_HORSE_RUN;
	}

	void InitialiseHorseman(short itemNumber)
	{
		ITEM_INFO* item = &g_Level.Items[itemNumber];
		OBJECT_INFO* obj = &Objects[ID_HORSEMAN];

		ClearItem(itemNumber);

		item->AnimNumber = obj->animIndex + 8;
		item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
		item->TargetState = 9;
		item->ActiveState = 9;
		item->ItemFlags[0] = NO_ITEM; // No horse yet
	}

	void HorsemanControl(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;

		ITEM_INFO* item = &g_Level.Items[itemNumber];
		CREATURE_INFO* creature = (CREATURE_INFO*)item->Data;

		// Try to find the horse
		if (item->ItemFlags[0] == NO_ITEM)
		{
			for (int i = 0; i < g_Level.NumItems; i++)
			{
				ITEM_INFO* currentItem = &g_Level.Items[i];
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
		ITEM_INFO* horseItem = NULL;
		if (item->ItemFlags[0] != 0)
			horseItem = &g_Level.Items[item->ItemFlags[0]];

		int x = 0;
		int y = 0;
		int z = 0;
		short roomNumber = 0;
		int deltaX = 0;
		int deltaZ = 0;
		FLOOR_INFO* floor;
		short height = 0;
		short height1 = 0;
		short height2 = 0;
		int xRot = 0;
		short angle = 0;

		if (horseItem != NULL)
		{
			roomNumber = item->RoomNumber;

			x = horseItem->Position.xPos + 341 * phd_sin(horseItem->Position.yRot);
			y = horseItem->Position.yPos;
			z = horseItem->Position.zPos + 341 * phd_cos(horseItem->Position.yRot);

			floor = GetFloor(x, y, z, &roomNumber);
			height1 = GetFloorHeight(floor, x, y, z);

			x = horseItem->Position.xPos - 341 * phd_sin(horseItem->Position.yRot);
			y = horseItem->Position.yPos;
			z = horseItem->Position.zPos - 341 * phd_cos(horseItem->Position.yRot);

			floor = GetFloor(x, y, z, &roomNumber);
			height2 = GetFloorHeight(floor, x, y, z);

			xRot = phd_atan(682, height2 - height1);
		}

		if (item->HitPoints <= 0)
		{
			item->HitPoints = 0;
			if (item->ItemFlags[1] == 0)
			{
				if (item->ActiveState != 16)
				{
					item->AnimNumber = Objects[ID_HORSEMAN].animIndex + 21;
					item->ActiveState = 16;
					item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;

					if (item->ItemFlags[0])
					{
						horseItem->AfterDeath = 1;
						item->ItemFlags[0] = 0;
					}
				}
			}
			else
			{
				item->HitPoints = 100;
				item->AIBits = 0;
				item->ItemFlags[1] = 0;
				item->AnimNumber = Objects[ID_HORSEMAN].animIndex + 3;
				item->ActiveState = 8;
				item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;

				creature->enemy = NULL;

				horseItem->TargetState = STATE_HORSEMAN_HORSE_RUN;
			}
		}
		else
		{
			if (item->AIBits)
			{
				GetAITarget(creature);
			}
			else if (creature->hurtByLara)
			{
				creature->enemy = LaraItem;
			}

			AI_INFO info;
			AI_INFO laraInfo;

			CreatureAIInfo(item, &info);

			if (creature->enemy == LaraItem)
			{
				laraInfo.angle = info.angle;
				laraInfo.distance = info.distance;
			}
			else
			{
				deltaX = LaraItem->Position.zPos - item->Position.zPos;
				deltaZ = LaraItem->Position.zPos - item->Position.zPos;

				laraInfo.angle = phd_atan(deltaZ, deltaX) - item->Position.yRot;
				laraInfo.distance = SQUARE(deltaX) + SQUARE(deltaZ);
			}

			short tilt = 0;

			if (item->HitStatus
				&& laraInfo.angle < 12288
				&& laraInfo.angle > -12288
				&& laraInfo.distance < SQUARE(2048))
			{
				if (item->ActiveState != 15)
				{
					if (laraInfo.angle <= 0)
					{
						if (item->ItemFlags[1])
						{
							if (!item->ItemFlags[1])
							{
								if (item->MeshBits & 0x400)
								{
									item->RequiredState = 15;  
								}
							}
						}
						else
						{
							if (laraInfo.angle > 0 || !(item->MeshBits & 0x400))
							{
								if (Lara.Control.WeaponControl.GunType == WEAPON_SHOTGUN)
								{
									item->HitPoints -= 10;
									item->HitStatus = true;
								}
								else if (Lara.Control.WeaponControl.GunType == WEAPON_REVOLVER)
								{
									item->HitPoints -= 20;
									item->HitStatus = true;
								}
								else
								{
									item->HitPoints--;
								}

								SoundEffect(SFX_TR4_HORSEMAN_TAKEHIT, &item->Position, 0);
								SoundEffect(SFX_TR4_HORSE_RICOCHETS, &item->Position, 0);

								PHD_VECTOR pos;
								pos.x = 0;
								pos.y = -128;
								pos.z = 80;

								GetJointAbsPosition(item, &pos, SPHERES_SPACE_WORLD);
								HorsemanSparks(&pos, item->Position.yRot, 7);
							}
							else if (!(GetRandomControl() & 7))
							{
								if (item->ActiveState == 15)
								{
									item->TargetState = 9;
								}
								ExplodeItemNode(item, 10, 1, -24);
							}
						}
					}
				}
			}

			creature->hurtByLara = false;

			GetCreatureMood(item, &info, VIOLENT);
			CreatureMood(item, &info, VIOLENT);

			angle = CreatureTurn(item, creature->maximumTurn);

			switch (item->ActiveState)
			{
			case STATE_HORSEMAN_HORSE_RUN:
				creature->maximumTurn = ANGLE(3);
				horseItem->TargetState = STATE_HORSEMAN_HORSE_WALK;
				if (item->RequiredState)
				{
					item->TargetState = 17;
					horseItem->TargetState = STATE_HORSEMAN_GET_ON_HORSE;
				}
				else if (creature->flags
					|| creature->reachedGoal
					|| item->HitStatus
					&& !GetRandomControl())
				{
					if (laraInfo.distance > SQUARE(4096)
						|| creature->reachedGoal)
					{
						creature->flags = 0;
						creature->enemy = LaraItem;
						if (laraInfo.angle > -8192 && laraInfo.angle < 0x2000)
						{
							item->TargetState = STATE_HORSEMAN_HORSE_STOP;
							horseItem->TargetState = STATE_HORSEMAN_HORSE_RUN;
						}
					}
					else
					{
						item->AIBits = FOLLOW;
						item->ItemFlags[3] = (-(item->ItemFlags[3] != 1)) + 2;
					}
				}

				if (info.distance >= SQUARE(1024))
				{
					if (info.bite)
					{
						if (info.angle >= -ANGLE(10)
							|| info.distance >= SQUARE(1024)
							&& (info.distance >= SQUARE(1365)
								|| info.angle <= -ANGLE(20)))
						{
							if (info.angle > ANGLE(10)
								&& (info.distance < SQUARE(1024)
									|| info.distance < SQUARE(1365) &&
									info.angle < ANGLE(20)))
							{
								creature->maximumTurn = 0;
								item->TargetState = 6;
							}
						}
						else
						{
							creature->maximumTurn = 0;
							item->TargetState = 7;
						}
					}
				}
				else if (info.bite)
				{
					if (info.angle >= -ANGLE(10)
						|| info.angle <= ANGLE(10))
					{
						if (info.bite)
						{
							if (info.angle >= -ANGLE(10)
								|| info.distance >= SQUARE(1024)
								&& (info.distance >= SQUARE(1365)
									|| info.angle <= -ANGLE(20)))
							{
								if (info.angle > ANGLE(10)
									&& (info.distance < SQUARE(1024)
										|| info.distance < SQUARE(1365) &&
										info.angle < ANGLE(20)))
								{
									creature->maximumTurn = 0;
									item->TargetState = 6;
								}
							}
							else
							{
								creature->maximumTurn = 0;
								item->TargetState = 7;
							}
						}
					}
				}

				break;

			case 2:
				creature->maximumTurn = 273;

				if (laraInfo.distance > SQUARE(4096) || creature->reachedGoal || creature->enemy == LaraItem)
				{
					creature->reachedGoal = false;
					creature->flags = 0;
					item->TargetState = STATE_HORSEMAN_HORSE_RUN;
					horseItem->TargetState = STATE_HORSEMAN_HORSE_WALK;
					creature->enemy = LaraItem;
				}

				break;

			case 3:
				creature->maximumTurn = 0;
				horseItem->TargetState = STATE_HORSEMAN_HORSE_RUN;

				if (creature->flags)
				{
					item->AIBits = FOLLOW;
					item->ItemFlags[3] = -(item->ItemFlags[3] != 1) + 2;
				}
				else
				{
					creature->flags = 0;
				}

				if (item->RequiredState)
				{
					item->TargetState = STATE_HORSEMAN_HORSE_RUN;
					horseItem->TargetState = STATE_HORSEMAN_HORSE_WALK;
					horseItem->Flags = 0;
				}
				else if (creature->reachedGoal
					|| !horseItem->Flags
					&& info.distance < SQUARE(1024)
					&& info.bite
					&& info.angle < ANGLE(10)
					&& info.angle > -ANGLE(10))
				{
					item->TargetState = STATE_HORSEMAN_HORSE_REARING;
					if (creature->reachedGoal)
					{
						item->RequiredState = 17;
					}
					horseItem->Flags = 0;
				}
				else
				{
					item->TargetState = STATE_HORSEMAN_HORSE_RUN;
					horseItem->TargetState = STATE_HORSEMAN_HORSE_WALK;
					horseItem->Flags = 0;
				}

				break;

			case 4:
				creature->maximumTurn = 0;

				if (item->FrameNumber == g_Level.Anims[item->AnimNumber].frameBase)
				{
					horseItem->AnimNumber = Objects[ID_HORSE].animIndex + 1;
					horseItem->ActiveState = STATE_HORSEMAN_HORSE_REARING;
					horseItem->FrameNumber = g_Level.Anims[horseItem->AnimNumber].frameBase;
				}

				if (!horseItem->Flags)
				{
					if (horseItem->TouchBits & 0x22000)
					{
						LaraItem->HitPoints -= 150;
						LaraItem->HitStatus = true;

						if (horseItem->TouchBits & 0x2000)
						{
							CreatureEffect2(
								horseItem,
								&horseBite1,
								10,
								-1,
								DoBloodSplat);
						}
						else
						{
							CreatureEffect2(
								horseItem,
								&horseBite2,
								10,
								-1,
								DoBloodSplat);
						}

						horseItem->Flags = 1;
					}
				}

				break;

			case 6:
				if (!creature->flags)
				{
					if (item->TouchBits & 0x60)
					{
						LaraItem->HitPoints -= 250;
						LaraItem->HitStatus = true;

						CreatureEffect2(
							item,
							&horsemanBite1,
							10,
							item->Position.yRot,
							DoBloodSplat);

						creature->flags = 1;
					}
				}
				if (item->HitStatus)
				{
					item->TargetState = 9;
				}

				break;

			case 7:
				if (!creature->flags)
				{
					if (item->TouchBits & 0x4000)
					{
						LaraItem->HitPoints -= 100;
						LaraItem->HitStatus = true;

						CreatureEffect2(
							item,
							&horsemanBite2,
							3,
							item->Position.yRot,
							DoBloodSplat);

						creature->flags = 1;
					}
				}

				break;

			case 9:
				creature->maximumTurn = 0;
				creature->flags = 0;

				if (!item->AIBits || item->ItemFlags[3])
				{
					if (item->RequiredState)
					{
						item->TargetState = item->RequiredState;
					}
					else if (info.bite && info.distance < SQUARE(682))
					{
						item->TargetState = 14;
					}
					else if (info.distance < SQUARE(6144) && info.distance > SQUARE(682))
					{
						item->TargetState = 10;
					}
				}
				else
				{
					item->TargetState = 10;
				}

				break;

			case 10:
				creature->maximumTurn = ANGLE(3);
				creature->flags = 0;

				if (creature->reachedGoal)
				{
					item->AIBits = 0;
					item->ItemFlags[1] = 1;

					item->Position.xPos = horseItem->Position.xPos;
					item->Position.yPos = horseItem->Position.yPos;
					item->Position.zPos = horseItem->Position.zPos;
					item->Position.xRot = horseItem->Position.xRot;
					item->Position.yRot = horseItem->Position.yRot;
					item->Position.zRot = horseItem->Position.zRot;

					creature->reachedGoal = false;
					creature->enemy = NULL;

					item->AnimNumber = Objects[ID_HORSEMAN].animIndex + 14;
					item->ActiveState = STATE_HORSEMAN_GET_ON_HORSE;
					item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;

					creature->maximumTurn = 0;

					break;
				}

				if (item->HitStatus)
				{
					item->TargetState = 9;
				}
				else if (info.bite && info.distance < SQUARE(682))
				{
					if (GetRandomControl() & 1)
					{
						item->TargetState = 12;
					}
					else if (GetRandomControl() & 1)
					{
						item->TargetState = 13;
					}
					else
					{
						item->TargetState = 9;
					}
				}
				else if (info.distance < SQUARE(5120) && info.distance > SQUARE(1365))
				{
					item->TargetState = 11;
				}

				break;

			case 11:
				if (info.distance < SQUARE(1365))
				{
					item->TargetState = 10;
				}

				break;

			case 12:
			case 13:
			case 14:
				creature->maximumTurn = 0;
				if (abs(info.angle) >= ANGLE(3))
				{
					if (info.angle >= 0)
					{
						item->Position.yRot += ANGLE(3);
					}
					else
					{
						item->Position.yRot -= ANGLE(3);
					}
				}
				else
				{
					item->Position.yRot += info.angle;
				}

				if (!creature->flags)
				{
					if (item->TouchBits & 0x4000)
					{
						LaraItem->HitPoints -= 100;
						LaraItem->HitStatus = true;

						CreatureEffect2(
							item,
							&horsemanBite2,
							3,
							item->Position.yRot,
							DoBloodSplat);

						creature->flags = 1;
					}
				}

				break;

			case 15:
				if (Lara.target != item || info.bite && info.distance < SQUARE(682))
				{
					item->TargetState = 9;
				}

				break;

			case 17:
				creature->reachedGoal = false;
				creature->maximumTurn = 546;

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
								&horseBite1,
								10,
								-1,
								DoBloodSplat);
						}

						if (horseItem->TouchBits & 0x20000)
						{
							CreatureEffect2(
								horseItem,
								&horseBite2,
								10,
								-1,
								DoBloodSplat);
						}

						if (horseItem->TouchBits & 0x80000)
						{
							CreatureEffect2(
								horseItem,
								&horseBite3,
								10,
								-1,
								DoBloodSplat);
						}

						horseItem->Flags = 1;
					}
				}

				if (!creature->flags)
				{
					if (item->TouchBits & 0x460)
					{
						LaraItem->HitStatus = true;

						if (item->TouchBits & 0x60)
						{
							CreatureEffect2(
								horseItem,
								&horsemanBite1,
								20,
								-1,
								DoBloodSplat);
							LaraItem->HitPoints -= 250;
						}
						else if (item->TouchBits & 0x400)
						{
							CreatureEffect2(
								horseItem,
								&horsemanBite3,
								10,
								-1,
								DoBloodSplat);
							LaraItem->HitPoints -= 150;
						}

						creature->flags = 1;
					}
				}

				if (item->AnimNumber == Objects[ID_HORSEMAN].animIndex + 29 &&
					item->FrameNumber == g_Level.Anims[item->AnimNumber].frameBase)
				{
					horseItem->AnimNumber = Objects[ID_HORSE].animIndex + 10;
					horseItem->FrameNumber = g_Level.Anims[horseItem->AnimNumber].frameBase;
				}

				if (laraInfo.distance > SQUARE(4096) || creature->reachedGoal)
				{
					creature->reachedGoal = false;
					creature->flags = 0;
					creature->enemy = LaraItem;
				}
				else if (!info.ahead)
				{
					item->TargetState = STATE_HORSEMAN_HORSE_STOP;
					horseItem->TargetState = STATE_HORSEMAN_HORSE_RUN;
				}
				break;

			default:
				break;
			}

			if (horseItem && item->ItemFlags[1])
			{
				if (abs(xRot - item->Position.xRot) < 256)
				{
					item->Position.xRot = xRot;
				}
				else if (xRot <= item->Position.xRot)
				{
					if (xRot < item->Position.xRot)
					{
						item->Position.xRot -= 256;
					}
				}
				else
				{
					item->Position.xRot += 256;
				}

				horseItem->Position.xPos = item->Position.xPos;
				horseItem->Position.yPos = item->Position.yPos;
				horseItem->Position.zPos = item->Position.zPos;
				horseItem->Position.xRot = item->Position.xRot;
				horseItem->Position.yRot = item->Position.yRot;
				horseItem->Position.zRot = item->Position.zRot;

				if (horseItem->RoomNumber != item->RoomNumber)
				{
					ItemNewRoom(item->ItemFlags[0], item->RoomNumber);
				}
				AnimateItem(horseItem);
			}
		}

		Objects[ID_HORSEMAN].radius = item->ItemFlags[1] != 0 ? 409 : 170;
		CreatureAnimation(itemNumber, angle, 0);
	}
}