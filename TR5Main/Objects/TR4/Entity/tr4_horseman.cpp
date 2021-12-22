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

		item->animNumber = obj->animIndex + 2;
		item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
		item->goalAnimState = STATE_HORSEMAN_HORSE_RUN;
		item->currentAnimState = STATE_HORSEMAN_HORSE_RUN;
	}

	void InitialiseHorseman(short itemNumber)
	{
		ITEM_INFO* item = &g_Level.Items[itemNumber];
		OBJECT_INFO* obj = &Objects[ID_HORSEMAN];

		ClearItem(itemNumber);

		item->animNumber = obj->animIndex + 8;
		item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
		item->goalAnimState = 9;
		item->currentAnimState = 9;
		item->itemFlags[0] = NO_ITEM; // No horse yet
	}

	void HorsemanControl(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;

		ITEM_INFO* item = &g_Level.Items[itemNumber];
		CREATURE_INFO* creature = (CREATURE_INFO*)item->data;

		// Try to find the horse
		if (item->itemFlags[0] == NO_ITEM)
		{
			for (int i = 0; i < g_Level.NumItems; i++)
			{
				ITEM_INFO* currentItem = &g_Level.Items[i];
				if (currentItem->objectNumber == ID_HORSE && item->triggerFlags == currentItem->triggerFlags)
				{
					item->itemFlags[0] = i;
					currentItem->flags |= 0x20;
				}
			}
		}

		// If no horse was found, then set it to 0 so it won't be searched anymore in the future
		if (item->itemFlags[0] == NO_ITEM)
			item->itemFlags[0] = 0;

		// The horse
		ITEM_INFO* horseItem = NULL;
		if (item->itemFlags[0] != 0)
			horseItem = &g_Level.Items[item->itemFlags[0]];

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
			roomNumber = item->roomNumber;

			x = horseItem->pos.xPos + 341 * phd_sin(horseItem->pos.yRot);
			y = horseItem->pos.yPos;
			z = horseItem->pos.zPos + 341 * phd_cos(horseItem->pos.yRot);

			floor = GetFloor(x, y, z, &roomNumber);
			height1 = GetFloorHeight(floor, x, y, z);

			x = horseItem->pos.xPos - 341 * phd_sin(horseItem->pos.yRot);
			y = horseItem->pos.yPos;
			z = horseItem->pos.zPos - 341 * phd_cos(horseItem->pos.yRot);

			floor = GetFloor(x, y, z, &roomNumber);
			height2 = GetFloorHeight(floor, x, y, z);

			xRot = phd_atan(682, height2 - height1);
		}

		if (item->hitPoints <= 0)
		{
			item->hitPoints = 0;
			if (item->itemFlags[1] == 0)
			{
				if (item->currentAnimState != 16)
				{
					item->animNumber = Objects[ID_HORSEMAN].animIndex + 21;
					item->currentAnimState = 16;
					item->frameNumber = g_Level.Anims[item->animNumber].frameBase;

					if (item->itemFlags[0])
					{
						horseItem->afterDeath = 1;
						item->itemFlags[0] = 0;
					}
				}
			}
			else
			{
				item->hitPoints = 100;
				item->aiBits = 0;
				item->itemFlags[1] = 0;
				item->animNumber = Objects[ID_HORSEMAN].animIndex + 3;
				item->currentAnimState = 8;
				item->frameNumber = g_Level.Anims[item->animNumber].frameBase;

				creature->enemy = NULL;

				horseItem->goalAnimState = STATE_HORSEMAN_HORSE_RUN;
			}
		}
		else
		{
			if (item->aiBits)
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
				deltaX = LaraItem->pos.zPos - item->pos.zPos;
				deltaZ = LaraItem->pos.zPos - item->pos.zPos;

				laraInfo.angle = phd_atan(deltaZ, deltaX) - item->pos.yRot;
				laraInfo.distance = SQUARE(deltaX) + SQUARE(deltaZ);
			}

			short tilt = 0;

			if (item->hitStatus
				&& laraInfo.angle < 12288
				&& laraInfo.angle > -12288
				&& laraInfo.distance < SQUARE(2048))
			{
				if (item->currentAnimState != 15)
				{
					if (laraInfo.angle <= 0)
					{
						if (item->itemFlags[1])
						{
							if (!item->itemFlags[1])
							{
								if (item->meshBits & 0x400)
								{
									item->requiredAnimState = 15;  
								}
							}
						}
						else
						{
							if (laraInfo.angle > 0 || !(item->meshBits & 0x400))
							{
								if (Lara.gunType == WEAPON_SHOTGUN)
								{
									item->hitPoints -= 10;
									item->hitStatus = true;
								}
								else if (Lara.gunType == WEAPON_REVOLVER)
								{
									item->hitPoints -= 20;
									item->hitStatus = true;
								}
								else
								{
									item->hitPoints--;
								}

								SoundEffect(SFX_TR4_HORSEMAN_TAKEHIT, &item->pos, 0);
								SoundEffect(SFX_TR4_HORSE_RICOCHETS, &item->pos, 0);

								PHD_VECTOR pos;
								pos.x = 0;
								pos.y = -128;
								pos.z = 80;

								GetJointAbsPosition(item, &pos, SPHERES_SPACE_WORLD);
								HorsemanSparks(&pos, item->pos.yRot, 7);
							}
							else if (!(GetRandomControl() & 7))
							{
								if (item->currentAnimState == 15)
								{
									item->goalAnimState = 9;
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

			switch (item->currentAnimState)
			{
			case STATE_HORSEMAN_HORSE_RUN:
				creature->maximumTurn = ANGLE(3);
				horseItem->goalAnimState = STATE_HORSEMAN_HORSE_WALK;
				if (item->requiredAnimState)
				{
					item->goalAnimState = 17;
					horseItem->goalAnimState = STATE_HORSEMAN_GET_ON_HORSE;
				}
				else if (creature->flags
					|| creature->reachedGoal
					|| item->hitStatus
					&& !GetRandomControl())
				{
					if (laraInfo.distance > SQUARE(4096)
						|| creature->reachedGoal)
					{
						creature->flags = 0;
						creature->enemy = LaraItem;
						if (laraInfo.angle > -8192 && laraInfo.angle < 0x2000)
						{
							item->goalAnimState = STATE_HORSEMAN_HORSE_STOP;
							horseItem->goalAnimState = STATE_HORSEMAN_HORSE_RUN;
						}
					}
					else
					{
						item->aiBits = FOLLOW;
						item->itemFlags[3] = (-(item->itemFlags[3] != 1)) + 2;
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
								item->goalAnimState = 6;
							}
						}
						else
						{
							creature->maximumTurn = 0;
							item->goalAnimState = 7;
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
									item->goalAnimState = 6;
								}
							}
							else
							{
								creature->maximumTurn = 0;
								item->goalAnimState = 7;
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
					item->goalAnimState = STATE_HORSEMAN_HORSE_RUN;
					horseItem->goalAnimState = STATE_HORSEMAN_HORSE_WALK;
					creature->enemy = LaraItem;
				}

				break;

			case 3:
				creature->maximumTurn = 0;
				horseItem->goalAnimState = STATE_HORSEMAN_HORSE_RUN;

				if (creature->flags)
				{
					item->aiBits = FOLLOW;
					item->itemFlags[3] = -(item->itemFlags[3] != 1) + 2;
				}
				else
				{
					creature->flags = 0;
				}

				if (item->requiredAnimState)
				{
					item->goalAnimState = STATE_HORSEMAN_HORSE_RUN;
					horseItem->goalAnimState = STATE_HORSEMAN_HORSE_WALK;
					horseItem->flags = 0;
				}
				else if (creature->reachedGoal
					|| !horseItem->flags
					&& info.distance < SQUARE(1024)
					&& info.bite
					&& info.angle < ANGLE(10)
					&& info.angle > -ANGLE(10))
				{
					item->goalAnimState = STATE_HORSEMAN_HORSE_REARING;
					if (creature->reachedGoal)
					{
						item->requiredAnimState = 17;
					}
					horseItem->flags = 0;
				}
				else
				{
					item->goalAnimState = STATE_HORSEMAN_HORSE_RUN;
					horseItem->goalAnimState = STATE_HORSEMAN_HORSE_WALK;
					horseItem->flags = 0;
				}

				break;

			case 4:
				creature->maximumTurn = 0;

				if (item->frameNumber == g_Level.Anims[item->animNumber].frameBase)
				{
					horseItem->animNumber = Objects[ID_HORSE].animIndex + 1;
					horseItem->currentAnimState = STATE_HORSEMAN_HORSE_REARING;
					horseItem->frameNumber = g_Level.Anims[horseItem->animNumber].frameBase;
				}

				if (!horseItem->flags)
				{
					if (horseItem->touchBits & 0x22000)
					{
						LaraItem->hitPoints -= 150;
						LaraItem->hitStatus = true;

						if (horseItem->touchBits & 0x2000)
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

						horseItem->flags = 1;
					}
				}

				break;

			case 6:
				if (!creature->flags)
				{
					if (item->touchBits & 0x60)
					{
						LaraItem->hitPoints -= 250;
						LaraItem->hitStatus = true;

						CreatureEffect2(
							item,
							&horsemanBite1,
							10,
							item->pos.yRot,
							DoBloodSplat);

						creature->flags = 1;
					}
				}
				if (item->hitStatus)
				{
					item->goalAnimState = 9;
				}

				break;

			case 7:
				if (!creature->flags)
				{
					if (item->touchBits & 0x4000)
					{
						LaraItem->hitPoints -= 100;
						LaraItem->hitStatus = true;

						CreatureEffect2(
							item,
							&horsemanBite2,
							3,
							item->pos.yRot,
							DoBloodSplat);

						creature->flags = 1;
					}
				}

				break;

			case 9:
				creature->maximumTurn = 0;
				creature->flags = 0;

				if (!item->aiBits || item->itemFlags[3])
				{
					if (item->requiredAnimState)
					{
						item->goalAnimState = item->requiredAnimState;
					}
					else if (info.bite && info.distance < SQUARE(682))
					{
						item->goalAnimState = 14;
					}
					else if (info.distance < SQUARE(6144) && info.distance > SQUARE(682))
					{
						item->goalAnimState = 10;
					}
				}
				else
				{
					item->goalAnimState = 10;
				}

				break;

			case 10:
				creature->maximumTurn = ANGLE(3);
				creature->flags = 0;

				if (creature->reachedGoal)
				{
					item->aiBits = 0;
					item->itemFlags[1] = 1;

					item->pos.xPos = horseItem->pos.xPos;
					item->pos.yPos = horseItem->pos.yPos;
					item->pos.zPos = horseItem->pos.zPos;
					item->pos.xRot = horseItem->pos.xRot;
					item->pos.yRot = horseItem->pos.yRot;
					item->pos.zRot = horseItem->pos.zRot;

					creature->reachedGoal = false;
					creature->enemy = NULL;

					item->animNumber = Objects[ID_HORSEMAN].animIndex + 14;
					item->currentAnimState = STATE_HORSEMAN_GET_ON_HORSE;
					item->frameNumber = g_Level.Anims[item->animNumber].frameBase;

					creature->maximumTurn = 0;

					break;
				}

				if (item->hitStatus)
				{
					item->goalAnimState = 9;
				}
				else if (info.bite && info.distance < SQUARE(682))
				{
					if (GetRandomControl() & 1)
					{
						item->goalAnimState = 12;
					}
					else if (GetRandomControl() & 1)
					{
						item->goalAnimState = 13;
					}
					else
					{
						item->goalAnimState = 9;
					}
				}
				else if (info.distance < SQUARE(5120) && info.distance > SQUARE(1365))
				{
					item->goalAnimState = 11;
				}

				break;

			case 11:
				if (info.distance < SQUARE(1365))
				{
					item->goalAnimState = 10;
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
						item->pos.yRot += ANGLE(3);
					}
					else
					{
						item->pos.yRot -= ANGLE(3);
					}
				}
				else
				{
					item->pos.yRot += info.angle;
				}

				if (!creature->flags)
				{
					if (item->touchBits & 0x4000)
					{
						LaraItem->hitPoints -= 100;
						LaraItem->hitStatus = true;

						CreatureEffect2(
							item,
							&horsemanBite2,
							3,
							item->pos.yRot,
							DoBloodSplat);

						creature->flags = 1;
					}
				}

				break;

			case 15:
				if (Lara.target != item || info.bite && info.distance < SQUARE(682))
				{
					item->goalAnimState = 9;
				}

				break;

			case 17:
				creature->reachedGoal = false;
				creature->maximumTurn = 546;

				if (!horseItem->flags)
				{
					if (horseItem->touchBits & 0xA2000)
					{
						LaraItem->hitPoints -= 150;
						LaraItem->hitStatus = true;

						if (horseItem->touchBits & 0x2000)
						{
							CreatureEffect2(
								horseItem,
								&horseBite1,
								10,
								-1,
								DoBloodSplat);
						}

						if (horseItem->touchBits & 0x20000)
						{
							CreatureEffect2(
								horseItem,
								&horseBite2,
								10,
								-1,
								DoBloodSplat);
						}

						if (horseItem->touchBits & 0x80000)
						{
							CreatureEffect2(
								horseItem,
								&horseBite3,
								10,
								-1,
								DoBloodSplat);
						}

						horseItem->flags = 1;
					}
				}

				if (!creature->flags)
				{
					if (item->touchBits & 0x460)
					{
						LaraItem->hitStatus = true;

						if (item->touchBits & 0x60)
						{
							CreatureEffect2(
								horseItem,
								&horsemanBite1,
								20,
								-1,
								DoBloodSplat);
							LaraItem->hitPoints -= 250;
						}
						else if (item->touchBits & 0x400)
						{
							CreatureEffect2(
								horseItem,
								&horsemanBite3,
								10,
								-1,
								DoBloodSplat);
							LaraItem->hitPoints -= 150;
						}

						creature->flags = 1;
					}
				}

				if (item->animNumber == Objects[ID_HORSEMAN].animIndex + 29 &&
					item->frameNumber == g_Level.Anims[item->animNumber].frameBase)
				{
					horseItem->animNumber = Objects[ID_HORSE].animIndex + 10;
					horseItem->frameNumber = g_Level.Anims[horseItem->animNumber].frameBase;
				}

				if (laraInfo.distance > SQUARE(4096) || creature->reachedGoal)
				{
					creature->reachedGoal = false;
					creature->flags = 0;
					creature->enemy = LaraItem;
				}
				else if (!info.ahead)
				{
					item->goalAnimState = STATE_HORSEMAN_HORSE_STOP;
					horseItem->goalAnimState = STATE_HORSEMAN_HORSE_RUN;
				}
				break;

			default:
				break;
			}

			if (horseItem && item->itemFlags[1])
			{
				if (abs(xRot - item->pos.xRot) < 256)
				{
					item->pos.xRot = xRot;
				}
				else if (xRot <= item->pos.xRot)
				{
					if (xRot < item->pos.xRot)
					{
						item->pos.xRot -= 256;
					}
				}
				else
				{
					item->pos.xRot += 256;
				}

				horseItem->pos.xPos = item->pos.xPos;
				horseItem->pos.yPos = item->pos.yPos;
				horseItem->pos.zPos = item->pos.zPos;
				horseItem->pos.xRot = item->pos.xRot;
				horseItem->pos.yRot = item->pos.yRot;
				horseItem->pos.zRot = item->pos.zRot;

				if (horseItem->roomNumber != item->roomNumber)
				{
					ItemNewRoom(item->itemFlags[0], item->roomNumber);
				}
				AnimateItem(horseItem);
			}
		}

		Objects[ID_HORSEMAN].radius = item->itemFlags[1] != 0 ? 409 : 170;
		CreatureAnimation(itemNumber, angle, 0);
	}
}