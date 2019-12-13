#include "../newobjects.h"
#include "../../Game/effect2.h"
#include "../../Game/debris.h"
#include "../../Game/items.h"
#include "../../Game/traps.h"
#include "../../Game/draw.h"

void BubblesEffect1(short fxNum, short xVel, short yVel, short zVel)
{
	FX_INFO* fx = &Effects[fxNum];

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
		spark->transType = 2;
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
		spark->dSize = spark->size >> 5;
	}
}

void BubblesEffect2(short fxNum, short xVel, short yVel, short zVel)
{
	FX_INFO* fx = &Effects[fxNum];

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
		spark->transType = 2;
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
		spark->dSize = spark->size >> 5;
	}
}

void BubblesEffect3(short fxNum, short xVel, short yVel, short zVel)
{
	FX_INFO* fx = &Effects[fxNum];

	int dx = LaraItem->pos.xPos - fx->pos.xPos;
	int dz = LaraItem->pos.zPos - fx->pos.zPos;

	if (dx >= -16384 && dx <= 16384 && dz >= -16384 && dz <= 16384)
	{
		SPARKS* spark = &Sparks[GetFreeSpark()];

		spark->on = 1;
		spark->sB = 0;
		spark->sR = (GetRandomControl() & 0x3F) + -128;
		spark->sG = spark->sG >> 1;
		spark->dB = 0;
		spark->dR = (GetRandomControl() & 0x3F) + -128;
		spark->dG = spark->dG >> 1;
		spark->fadeToBlack = 8;
		spark->colFadeSpeed = (GetRandomControl() & 3) + 8;
		spark->transType = 2;
		spark->dynamic = -1;
		spark->life = spark->sLife = (GetRandomControl() & 7) + 32;
		spark->y = 0;
		spark->x = (GetRandomControl() & 0xF) - 8;
		spark->z = (GetRandomControl() & 0xF) - 8;
		spark->x += fx->pos.xPos;
		spark->y += fx->pos.yPos;
		spark->z += fx->pos.zPos;
		spark->xVel = xVel;
		spark->yVel = yVel;
		spark->zVel = zVel;
		spark->friction = 34;
		spark->flags = 538;
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
		spark->sSize = spark->size = (GetRandomControl() & 0xF) + 128;
		spark->dSize = spark->size >> 2;
	}
}

void BubblesEffect4(short fxNum, short xVel, short yVel, short zVel)
{
	FX_INFO* fx = &Effects[fxNum];

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
		spark->transType = 2;
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
		spark->dSize = spark->size >> 5;
	}
}

int BubblesShatterFunction(FX_INFO* fx, int param1, int param2)
{
	ShatterItem.yRot = fx->pos.yRot;
	ShatterItem.meshp = Meshes[fx->frameNumber];
	ShatterItem.sphere.x = fx->pos.xPos;
	ShatterItem.sphere.y = fx->pos.yPos;
	ShatterItem.sphere.z = fx->pos.zPos;
	ShatterItem.bit = 0;
	ShatterItem.flags = fx->flag2 & 0x400;
	ShatterObject(&ShatterItem, 0, param2, fx->roomNumber, param1);

	return 1;
}

void BubblesControl(short fxNum)
{
	FX_INFO* fx = &Effects[fxNum];

	short angles[2];
	phd_GetVectorAngles(
		LaraItem->pos.xPos - fx->pos.xPos,
		LaraItem->pos.yPos - fx->pos.yPos - 256,
		LaraItem->pos.zPos - fx->pos.zPos,
		angles);

	int unk1 = 0; // v44
	int unk2 = 0; // v3

	if (fx->flag1 == 1)
	{
		unk1 = 512;
		unk2 = 256;
	}
	else
	{
		if (fx->flag1 == 6)
		{
			if (fx->counter)
			{
				fx->counter--;
			}
			unk1 = 256;
		}
		else
		{
			unk1 = 768;
		}
		unk2 = 192;
	}

	if (fx->speed < unk2)
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
		if (abs(dy) > ANGLE(180))
		{
			dy = -dy;
		}

		int dx = angles[1] - fx->pos.xRot;
		if (abs(dx) > ANGLE(180))
		{
			dx = -dx;
		}

		dy >>= 3;
		dx >>= 3;

		if (dy < -unk1)
			dy = -unk1;
		else if (dy > unk1)
			dy = unk1;

		if (dx < -unk1)
			dx = -unk1;
		else if (dx > unk1)
			dx = unk1;

		if (fx->flag1 != 4 && (fx->flag1 != 6 || !fx->counter))
		{
			fx->pos.yRot += dy;
		}
		fx->pos.xRot += dx;
	}

	fx->pos.zRot += 16 * fx->speed;
	if (fx->flag1 == 6)
	{
		fx->pos.zRot += 16 * fx->speed;
	}

	int oldX = fx->pos.xPos;
	int oldY = fx->pos.yPos;
	int oldZ = fx->pos.zPos;

	int c = fx->speed * COS(fx->pos.xRot) >> W2V_SHIFT;  
	fx->pos.xPos += c * SIN(fx->pos.yRot) >> W2V_SHIFT; 
	fx->pos.yPos += fx->speed * SIN(-fx->pos.xRot) >> W2V_SHIFT;  
	fx->pos.zPos += c * COS(fx->pos.yRot) >> W2V_SHIFT;
	
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
		{
			BubblesShatterFunction(fx, 0, -32);
		}
		
		if (fx->flag1 == 1)
		{
			TriggerShockwave(
				&fx->pos,
				10485792,
				64,
				402686016,
				(((~Rooms[fx->roomNumber].flags) >> 4) & 2) << 16, 0);

			TriggerExplosionSparks(oldX, oldY, oldZ, 3, -2, 2, fx->roomNumber);
		}
		else
		{
			int shockwaveValue = 0;

			if (fx->flag1)
			{
				if (fx->flag1 == 3 || fx->flag1 == 4)
				{
					shockwaveValue = 268451968;
				}
				else if (fx->flag1 == 5)
				{
					shockwaveValue = 276848640;
				}
				else
				{
					if (fx->flag1 != 2)
					{
						if (fx->flag1 == 6)
						{
							TriggerExplosionSparks(oldX, oldY, oldZ, 3, -2, 0, fx->roomNumber);
							TriggerShockwave(&fx->pos, 15728688, 64, 411066368, 0x20000, 0);
							fx->pos.yPos -= 128;
							TriggerShockwave(&fx->pos, 15728688, 48, 276852736, 0x20000, 0);
							fx->pos.yPos += 256;
							TriggerShockwave(&fx->pos, 15728688, 48, 276852736, 0x20000, 0);
						}
					}
					else
					{
						shockwaveValue = 276856832;
					}
				}
			}
			else
			{
				shockwaveValue = 268468288;
			}

			TriggerShockwave(&fx->pos, 10485792, 64, shockwaveValue, 0, 0);
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
			TriggerShockwave((PHD_3DPOS*)fx, 15728688, 64, 402686016, 0, 0);
			TriggerExplosionSparks(oldX, oldY, oldZ, 3, -2, 2, fx->roomNumber);
			LaraBurn();
			//Lara.gassed = true; BYTE1(Lara_Flags) |= 2u;			
		}
		else if (fx->flag1)
		{
			switch (fx->flag1)
			{
			case 3:
			case 4:
				TriggerShockwave((PHD_3DPOS*)fx, 10485792, 64, 268451968, 0x10000, 0);
				break;
			case 5:
				TriggerShockwave((PHD_3DPOS*)fx, 10485792, 64, 276848640, 0x20000, 0);
				break;
			case 2:
				TriggerShockwave((PHD_3DPOS*)fx, 10485792, 64, 276856832, 0x20000, 0);
				break;
			case 6:
				TriggerExplosionSparks(oldX, oldY, oldZ, 3, -2, 0, fx->roomNumber);
				TriggerShockwave((PHD_3DPOS*)fx, 15728688, 64, 411066368, 0, 0);
				fx->pos.yPos -= 128;
				TriggerShockwave((PHD_3DPOS*)fx, 15728688, 48, 276852736, 0, 0);
				fx->pos.yPos += 256;
				TriggerShockwave((PHD_3DPOS*)fx, 15728688, 48, 276852736, 0, 0);
				LaraBurn();
				break;
			}
		}
		else
		{
			TriggerShockwave(
				(PHD_3DPOS*)fx,
				0x00580018,
				48,
				268468288,
				(((~Rooms[fx->roomNumber].flags) >> 4) & 2) << 16, 0);
		}
	}
	else
	{
		if (roomNumber != fx->roomNumber)
		{
			EffectNewRoom(fxNum, roomNumber);
		}

		int dx = oldX - fx->pos.xPos;
		int dy = oldY - fx->pos.yPos;
		int dz = oldZ - fx->pos.zPos;

		if (Wibble & 4 || fx->flag1 == 1 || fx->flag1 == 5 || fx->flag1 == 2)
		{
			if (fx->flag1)
			{
				if (fx->flag1 == 1)
				{
					BubblesEffect1(fxNum, 32 * dx, 32 * dy, 32 * dz);
				}
				else if (fx->flag1 < 3 || fx->flag1 > 5)
				{
					if (fx->flag1 == 2)
					{
						BubblesEffect2(fxNum, 16 * dx, 16 * dy, 16 * dz);
					}
					else if (fx->flag1 == 6)
					{
						BubblesEffect3(fxNum, 16 * dx, 16 * dy, 16 * dz);
					}
				}
				else
				{
					BubblesEffect4(fxNum, 16 * dx, 16 * dy, 16 * dz);
				}
			}
			else
			{
				BubblesEffect1(fxNum, 16 * dx, 16 * dy, 16 * dz);
			}
		}
	}
}