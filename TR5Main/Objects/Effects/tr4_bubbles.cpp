#include "framework.h"
#include "tr4_bubbles.h"
#include "effects\debris.h"
#include "items.h"
#include "traps.h"
#include "animation.h"
#include "effects\tomb4fx.h"
#include "effects\effects.h"
#include "level.h"
#include "lara.h"
#include "control/control.h"
#include "tr4_mutant.h"
#include "collide.h"
#include "Game/effects/lara_burn.h"
#include "item.h"

using namespace TEN::Effects::Fire;

namespace TEN::entities::all
{
	void TriggerSethMissileFlame(short fxNum, short xVel, short yVel, short zVel)
	{
		FX_INFO* fx = &EffectList[fxNum];

		int dx = LaraItem->pos.xPos - fx->pos.xPos;
		int dz = LaraItem->pos.zPos - fx->pos.zPos;

		if (dx >= -16384 && dx <= 16384 && dz >= -16384 && dz <= 16384)
		{
			SPARKS* spark = &Sparks[GetFreeSpark()];

			spark->on = 1;
			spark->sR = 0;
			spark->dR = 0;
			spark->sG = (GetRandomControl() & 0x7F) + 32;
			spark->sB = spark->dG + 64;
			spark->dB = (GetRandomControl() & 0x7F) + 32;
			spark->dG = spark->dB + 64;
			spark->fadeToBlack = 8;
			spark->colFadeSpeed = (GetRandomControl() & 3) + 4;
			spark->transType = TransTypeEnum::COLADD;
			spark->life = spark->sLife = (GetRandomControl() & 3) + 16;
			spark->y = 0;
			spark->x = (GetRandomControl() & 0xF) - 8;
			spark->xVel = xVel;
			spark->yVel = yVel;
			spark->z = (GetRandomControl() & 0xF) - 8;
			spark->zVel = zVel;
			spark->friction = 68;
			spark->flags = 602;
			spark->rotAng = GetRandomControl() & 0xFFF;
			if (GetRandomControl() & 1)
			{
				spark->rotAdd = -32 - (GetRandomControl() & 0x1F);
			}
			else
			{
				spark->rotAdd = (GetRandomControl() & 0x1F) + 32;
			}
			spark->gravity = 0;
			spark->maxYvel = 0;
			spark->fxObj = fxNum;
			if (fx->flag1 == 1)
			{
				spark->scalar = 3;
			}
			else
			{
				spark->scalar = 2;
			}
			spark->sSize = spark->size = (GetRandomControl() & 7) + 64;
			spark->dSize = spark->size / 32;
		}
	}

	void TriggerHarpyFlameFlame(short fxNum, short xVel, short yVel, short zVel)
	{
		FX_INFO* fx = &EffectList[fxNum];

		int dx = LaraItem->pos.xPos - fx->pos.xPos;
		int dz = LaraItem->pos.zPos - fx->pos.zPos;

		if (dx >= -16384 && dx <= 16384 && dz >= -16384 && dz <= 16384)
		{
			SPARKS* spark = &Sparks[GetFreeSpark()];

			spark->on = 1;
			spark->sR = 0;
			spark->sG = (GetRandomControl() & 0x7F) + 32;
			spark->sB = spark->dG + 64;
			spark->dB = 0;
			spark->dG = spark->dR = (GetRandomControl() & 0x7F) + 32;
			spark->fadeToBlack = 8;
			spark->colFadeSpeed = (GetRandomControl() & 3) + 4;
			spark->transType = COLADD;
			spark->life = spark->sLife = (GetRandomControl() & 3) + 16;
			spark->y = 0;
			spark->x = (GetRandomControl() & 0xF) - 8;
			spark->xVel = xVel;
			spark->zVel = zVel;
			spark->z = (GetRandomControl() & 0xF) - 8;
			spark->yVel = yVel;
			spark->friction = 68;
			spark->flags = 602;
			spark->rotAng = GetRandomControl() & 0xFFF;
			if (GetRandomControl() & 1)
			{
				spark->rotAdd = -32 - (GetRandomControl() & 0x1F);
			}
			else
			{
				spark->rotAdd = (GetRandomControl() & 0x1F) + 32;
			}
			spark->gravity = 0;
			spark->maxYvel = 0;
			spark->fxObj = fxNum;
			spark->scalar = 2;
			spark->sSize = spark->size = (GetRandomControl() & 7) + 64;
			spark->dSize = spark->size / 32;
		}
	}

	void TriggerDemigodMissileFlame(short fxNum, short xVel, short yVel, short zVel)
	{
		FX_INFO* fx = &EffectList[fxNum];

		int dx = LaraItem->pos.xPos - fx->pos.xPos;
		int dz = LaraItem->pos.zPos - fx->pos.zPos;

		if (dx >= -16384 && dx <= 16384 && dz >= -16384 && dz <= 16384)
		{
			SPARKS* spark = &Sparks[GetFreeSpark()];

			spark->on = 1;
			if (fx->flag1 == 3 || fx->flag1 == 4)
			{
				spark->sR = 0;
				spark->dR = 0;
				spark->sB = (GetRandomControl() & 0x7F) + 32;
				spark->sG = spark->sB + 64;
				spark->dG = (GetRandomControl() & 0x7F) + 32;
				spark->dB = spark->dG + 64;
			}
			else
			{
				spark->sR = (GetRandomControl() & 0x7F) + 32;
				spark->sG = spark->sR - (GetRandomControl() & 0x1F);
				spark->sB = 0;
				spark->dR = (GetRandomControl() & 0x7F) + 32;
				spark->dB = 0;
				spark->dG = spark->dR - (GetRandomControl() & 0x1F);
			}
			spark->fadeToBlack = 8;
			spark->colFadeSpeed = (GetRandomControl() & 3) + 4;
			spark->transType = COLADD;
			spark->life = spark->sLife = (GetRandomControl() & 3) + 16;
			spark->y = 0;
			spark->x = (GetRandomControl() & 0xF) - 8;
			spark->yVel = yVel;
			spark->zVel = zVel;
			spark->z = (GetRandomControl() & 0xF) - 8;
			spark->xVel = xVel;
			spark->friction = 68;
			spark->flags = 602;
			spark->rotAng = GetRandomControl() & 0xFFF;
			if (GetRandomControl() & 1)
			{
				spark->rotAdd = -32 - (GetRandomControl() & 0x1F);
			}
			else
			{
				spark->rotAdd = (GetRandomControl() & 0x1F) + 32;
			}
			spark->gravity = 0;
			spark->maxYvel = 0;
			spark->fxObj = fxNum;
			spark->scalar = 2;
			spark->sSize = spark->size = (GetRandomControl() & 7) + 64;
			spark->dSize = spark->size / 32;
		}
	}

	void BubblesShatterFunction(FX_INFO* fx, int param1, int param2)
	{
		ShatterItem.yRot = fx->pos.yRot;
		ShatterItem.meshp = &g_Level.Meshes[fx->frameNumber];
		ShatterItem.sphere.x = fx->pos.xPos;
		ShatterItem.sphere.y = fx->pos.yPos;
		ShatterItem.sphere.z = fx->pos.zPos;
		ShatterItem.bit = 0;
		ShatterItem.flags = fx->flag2 & 0x400;
		ShatterObject(&ShatterItem, 0, param2, fx->roomNumber, param1);
	}

	void ControlEnemyMissile(short fxNum)
	{
		FX_INFO* fx = &EffectList[fxNum];

		short angles[2];
		phd_GetVectorAngles(
			LaraItem->pos.xPos - fx->pos.xPos,
			LaraItem->pos.yPos - fx->pos.yPos - STEP_SIZE,
			LaraItem->pos.zPos - fx->pos.zPos,
			angles);

		int maxRotation = 0;
		int maxSpeed = 0;

		if (fx->flag1 == 1)
		{
			maxRotation = 512;
			maxSpeed = 256;
		}
		else
		{
			if (fx->flag1 == 6)
			{
				if (fx->counter)
				{
					fx->counter--;
				}
				maxRotation = 256;
			}
			else
			{
				maxRotation = 768;
			}
			maxSpeed = 192;
		}

		if (fx->speed < maxSpeed)
		{
			if (fx->flag1 == 6)
			{
				fx->speed++;
			}
			else
			{
				fx->speed += 3;
			}

			int dy = angles[0] - fx->pos.yRot;
			if (abs(dy) <= ANGLE(180.0f))
			{
				dy = -dy;
			}

			int dx = angles[1] - fx->pos.xRot;
			if (abs(dx) <= ANGLE(180.0f))
				dx = -dx;

			dy /= 8;
			dx /= 8;

			if (dy < -maxRotation)
				dy = -maxRotation;
			else if (dy > maxRotation)
				dy = maxRotation;

			if (dx < -maxRotation)
				dx = -maxRotation;
			else if (dx > maxRotation)
				dx = maxRotation;

			if (fx->flag1 != 4 && (fx->flag1 != 6 || !fx->counter))
			{
				fx->pos.yRot += dy;
			}
			fx->pos.xRot += dx;
		}

		fx->pos.zRot += 16 * fx->speed;
		if (fx->flag1 == 6)
			fx->pos.zRot += 16 * fx->speed;

		int oldX = fx->pos.xPos;
		int oldY = fx->pos.yPos;
		int oldZ = fx->pos.zPos;

		int speed = (fx->speed * phd_cos(fx->pos.xRot));
		fx->pos.zPos += (speed * phd_cos(fx->pos.yRot));
		fx->pos.xPos += (speed * phd_sin(fx->pos.yRot));
		fx->pos.yPos += -((fx->speed * phd_sin(fx->pos.xRot))) + fx->fallspeed;

		short roomNumber = fx->roomNumber;
		FLOOR_INFO* floor = GetFloor(fx->pos.xPos, fx->pos.yPos, fx->pos.zPos, &roomNumber);
		int floorHeight = GetFloorHeight(floor, fx->pos.xPos, fx->pos.yPos, fx->pos.zPos);
		int ceilingHeight = GetCeiling(floor, fx->pos.xPos, fx->pos.yPos, fx->pos.zPos);

		if (fx->pos.yPos >= floorHeight || fx->pos.yPos <= ceilingHeight)
		{
			fx->pos.xPos = oldX;
			fx->pos.yPos = oldY;
			fx->pos.zPos = oldZ;

			if (fx->flag1 != 6)
				BubblesShatterFunction(fx, 0, -32);

			if (fx->flag1 == 1)
			{
				TriggerShockwave(&fx->pos, 32, 160, 64, 64, 128, 00, 24, (((~g_Level.Rooms[fx->roomNumber].flags) / 16) & 2) * 65536, 0);
				TriggerExplosionSparks(oldX, oldY, oldZ, 3, -2, 2, fx->roomNumber);
			}
			else
			{
				if (fx->flag1)
				{
					if (fx->flag1 == 3 || fx->flag1 == 4)
					{
						TriggerShockwave(&fx->pos, 32, 160, 64, 128, 64, 0, 16, 0, 0);
					}
					else if (fx->flag1 == 5)
					{
						TriggerShockwave(&fx->pos, 32, 160, 64, 0, 96, 128, 16, 0, 0);
					}
					else
					{
						if (fx->flag1 != 2)
						{
							if (fx->flag1 == 6)
							{
								TriggerExplosionSparks(oldX, oldY, oldZ, 3, -2, 0, fx->roomNumber);
								TriggerShockwave(&fx->pos, 48, 240, 64, 0, 96, 128, 24, 0, 2);
								fx->pos.yPos -= 128;
								TriggerShockwave(&fx->pos, 48, 240, 48, 0, 112, 128, 16, 0, 2);
								fx->pos.yPos += 256;
								TriggerShockwave(&fx->pos, 48, 240, 48, 0, 112, 128, 16, 0, 2);
							}

						}
						else
						{
							TriggerShockwave(&fx->pos, 32, 160, 64, 0, 128, 128, 16, 0, 0);
						}
					}
				}
				else
				{
					TriggerShockwave(&fx->pos, 32, 160, 64, 64, 128, 0, 16, 0, 0);
				}
			}

			KillEffect(fxNum);
			return;
		}

		if (ItemNearLara(&fx->pos, 200))
		{
			LaraItem->hitStatus = true;
			if (fx->flag1 != 6)
			{
				BubblesShatterFunction(fx, 0, -32);
			}

			KillEffect(fxNum);

			if (fx->flag1 == 1)
			{
				TriggerShockwave(&fx->pos, 48, 240, 64, 64, 128, 0, 24, 0, 0);
				TriggerExplosionSparks(oldX, oldY, oldZ, 3, -2, 2, fx->roomNumber);
				LaraBurn();
			}
			else if (fx->flag1)
			{
				switch (fx->flag1)
				{
				case 3:
				case 4:
					TriggerShockwave(&fx->pos, 32, 160, 64, 128, 64, 0, 16, 0, 1);
					break;
				case 5:
					TriggerShockwave(&fx->pos, 32, 160, 64, 0, 96, 128, 16, 0, 2);
					break;
				case 2:
					TriggerShockwave(&fx->pos, 32, 160, 64, 0, 128, 128, 16, 0, 2);
					break;
				case 6:
					TriggerExplosionSparks(oldX, oldY, oldZ, 3, -2, 0, fx->roomNumber);
					TriggerShockwave(&fx->pos, 48, 240, 64, 0, 96, 128, 24, 0, 0);
					fx->pos.yPos -= 128;
					TriggerShockwave(&fx->pos, 48, 240, 48, 0, 112, 128, 16, 0, 0);
					fx->pos.yPos += 256;
					TriggerShockwave(&fx->pos, 48, 240, 48, 0, 112, 128, 16, 0, 0);
					LaraBurn();
					break;
				}
			}
			else
			{
				TriggerShockwave(&fx->pos, 24, 88, 48, 64, 128, 0, 16, (((~g_Level.Rooms[fx->roomNumber].flags) / 16) & 2) * 65536, 0);
			}
		}
		else
		{
			if (roomNumber != fx->roomNumber)
				EffectNewRoom(fxNum, roomNumber);

			int dx = oldX - fx->pos.xPos;
			int dy = oldY - fx->pos.yPos;
			int dz = oldZ - fx->pos.zPos;

			if (Wibble & 4)
			{
				switch (fx->flag1)
				{
				default:
				case 1:
					TriggerSethMissileFlame(fxNum, 32 * dx, 32 * dy, 32 * dz);
					break;
				case 2:
					TriggerHarpyFlameFlame(fxNum, 16 * dx, 16 * dy, 16 * dz);
					break;
				case 3:
				case 4:
				case 5:
					TriggerDemigodMissileFlame(fxNum, 16 * dx, 16 * dy, 16 * dz);
					break;
				case 6:
					TEN::Entities::TR4::TriggerCrocgodMissileFlame(fxNum, 16 * dx, 16 * dy, 16 * dz);
					break;
				}
			}
		}
	}
}