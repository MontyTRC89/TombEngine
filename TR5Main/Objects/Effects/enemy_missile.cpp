#include "framework.h"
#include "Objects/Effects/enemy_missile.h"

#include "Game/collision/collide_item.h"
#include "Game/collision/collide_room.h"
#include "Game/control/control.h"
#include "Game/effects/debris.h"
#include "Game/effects/effects.h"
#include "Game/effects/lara_fx.h"
#include "Game/effects/tomb4fx.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Objects/TR4/Entity/tr4_mutant.h"
#include "Objects/TR4/Entity/tr4_demigod.h"
#include "Specific/level.h"

using namespace TEN::Effects::Lara;
using namespace TEN::Entities::TR4;

namespace TEN::Entities::Effects
{
	void TriggerSethMissileFlame(short fxNum, short xVel, short yVel, short zVel)
	{
		auto* fx = &EffectList[fxNum];

		int dx = LaraItem->Pose.Position.x - fx->pos.xPos;
		int dz = LaraItem->Pose.Position.z - fx->pos.zPos;

		if (dx >= -SECTOR(16) && dx <= SECTOR(16) &&
			dz >= -SECTOR(16) && dz <= SECTOR(16))
		{
			auto* spark = &Sparks[GetFreeSpark()];

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
				spark->rotAdd = -32 - (GetRandomControl() & 0x1F);
			else
				spark->rotAdd = (GetRandomControl() & 0x1F) + 32;
			
			spark->gravity = 0;
			spark->maxYvel = 0;
			spark->fxObj = fxNum;

			if (fx->flag1 == 1)
				spark->scalar = 3;
			else
				spark->scalar = 2;
			
			spark->sSize = spark->size = (GetRandomControl() & 7) + 64;
			spark->dSize = spark->size / 32;
		}
	}

	void TriggerHarpyFlameFlame(short fxNum, short xVel, short yVel, short zVel)
	{
		auto* fx = &EffectList[fxNum];

		int dx = LaraItem->Pose.Position.x - fx->pos.xPos;
		int dz = LaraItem->Pose.Position.z - fx->pos.zPos;

		if (dx >= -16384 && dx <= 16384 && dz >= -16384 && dz <= 16384)
		{
			auto* spark = &Sparks[GetFreeSpark()];

			spark->on = 1;
			spark->sR = 0;
			spark->sG = (GetRandomControl() & 0x7F) + 32;
			spark->sB = spark->dG + 64;
			spark->dB = 0;
			spark->dG = spark->dR = (GetRandomControl() & 0x7F) + 32;
			spark->fadeToBlack = 8;
			spark->colFadeSpeed = (GetRandomControl() & 3) + 4;
			spark->transType = TransTypeEnum::COLADD;
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
				spark->rotAdd = -32 - (GetRandomControl() & 0x1F);
			else
				spark->rotAdd = (GetRandomControl() & 0x1F) + 32;

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
		ShatterItem.meshIndex = fx->frameNumber;
		ShatterItem.sphere.x = fx->pos.xPos;
		ShatterItem.sphere.y = fx->pos.yPos;
		ShatterItem.sphere.z = fx->pos.zPos;
		ShatterItem.bit = 0;
		ShatterItem.flags = fx->flag2 & 0x400;
		ShatterObject(&ShatterItem, 0, param2, fx->roomNumber, param1);
	}

	void ControlEnemyMissile(short fxNum)
	{
		auto* fx = &EffectList[fxNum];

		short angles[2];
		phd_GetVectorAngles(
			LaraItem->Pose.Position.x - fx->pos.xPos,
			LaraItem->Pose.Position.y - fx->pos.yPos - CLICK(1),
			LaraItem->Pose.Position.z - fx->pos.zPos,
			angles);

		int maxRotation = 0;
		int maxVelocity = 0;

		if (fx->flag1 == 1)
		{
			maxRotation = ANGLE(2.8f);
			maxVelocity = CLICK(1);
		}
		else
		{
			if (fx->flag1 == 6)
			{
				if (fx->counter)
					fx->counter--;

				maxRotation = ANGLE(1.4f);
			}
			else
				maxRotation = ANGLE(4.5f);

			maxVelocity = CLICK(0.75f);
		}

		if (fx->speed < maxVelocity)
		{
			if (fx->flag1 == 6)
				fx->speed++;
			else
				fx->speed += 3;

			int dy = angles[0] - fx->pos.yRot;
			if (abs(dy) > 0x8000)
				dy = -dy;

			int dx = angles[1] - fx->pos.xRot;
			if (abs(dx) > 0x8000)
				dx = -dx;

			dy >>= 3;
			dx >>= 3;

			if (dy < -maxRotation)
				dy = -maxRotation;
			else if (dy > maxRotation)
				dy = maxRotation;

			if (dx < -maxRotation)
				dx = -maxRotation;
			else if (dx > maxRotation)
				dx = maxRotation;

			if (fx->flag1 != 4 && (fx->flag1 != 6 || !fx->counter))
				fx->pos.yRot += dy;
			fx->pos.xRot += dx;
		}

		fx->pos.zRot += 16 * fx->speed;
		if (fx->flag1 == 6)
			fx->pos.zRot += 16 * fx->speed;

		int oldX = fx->pos.xPos;
		int oldY = fx->pos.yPos;
		int oldZ = fx->pos.zPos;

		int speed = (fx->speed * phd_cos(fx->pos.xRot));
		fx->pos.xPos += (speed * phd_sin(fx->pos.yRot));
		fx->pos.yPos += -((fx->speed * phd_sin(fx->pos.xRot))) + fx->fallspeed;
		fx->pos.zPos += (speed * phd_cos(fx->pos.yRot));

		auto probe = GetCollision(fx->pos.xPos, fx->pos.yPos, fx->pos.zPos, fx->roomNumber);

		if (fx->pos.yPos >= probe.Position.Floor || fx->pos.yPos <= probe.Position.Ceiling)
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
						TriggerShockwave(&fx->pos, 32, 160, 64, 128, 64, 0, 16, 0, 0);
					else if (fx->flag1 == 5)
						TriggerShockwave(&fx->pos, 32, 160, 64, 0, 96, 128, 16, 0, 0);
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
							TriggerShockwave(&fx->pos, 32, 160, 64, 0, 128, 128, 16, 0, 0);
					}
				}
				else
					TriggerShockwave(&fx->pos, 32, 160, 64, 64, 128, 0, 16, 0, 0);
			}

			KillEffect(fxNum);
			return;
		}

		if (ItemNearLara(&fx->pos, 200))
		{
			LaraItem->HitStatus = true;
			if (fx->flag1 != 6)
				BubblesShatterFunction(fx, 0, -32);

			KillEffect(fxNum);

			if (fx->flag1 == 1)
			{
				TriggerShockwave(&fx->pos, 48, 240, 64, 64, 128, 0, 24, 0, 0);
				TriggerExplosionSparks(oldX, oldY, oldZ, 3, -2, 2, fx->roomNumber);
				LaraBurn(LaraItem);
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
					LaraBurn(LaraItem);
					break;
				}
			}
			else
				TriggerShockwave(&fx->pos, 24, 88, 48, 64, 128, 0, 16, (((~g_Level.Rooms[fx->roomNumber].flags) / 16) & 2) * 65536, 0);
		}
		else
		{
			if (probe.RoomNumber != fx->roomNumber)
				EffectNewRoom(fxNum, probe.RoomNumber);

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
					TriggerCrocgodMissileFlame(fxNum, 16 * dx, 16 * dy, 16 * dz);
					break;
				}
			}
		}
	}
}
