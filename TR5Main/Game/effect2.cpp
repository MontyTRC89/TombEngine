#include "effect2.h"
#include "draw.h"
#include "tomb4fx.h"
#include "traps.h"

#include "..\Scripting\GameFlowScript.h"

//long wibble;
//long SplashCount;
//long KillEverythingFlag;
int NextSpark;

unsigned char TES_extra_tab[] =
{
	0x00, 0x04, 0x07, 0x0A, 0x00, 0x00
};

int DeadlyBounds[6];
SPLASH_STRUCT Splashes[MAX_SPLASH];
RIPPLE_STRUCT Ripples[MAX_RIPPLES];
DYNAMIC Dynamics[MAX_DYNAMICS];
SPLASH_SETUP SplashSetup;
SP_DYNAMIC SparkDynamics[8];
int SmokeWeapon;
int SmokeCountL;
int SmokeCountR;
//int SmokeWindX;
//int SmokeWindZ;
int SplashCount = 0;
SPARKS Sparks[MAX_SPARKS];

extern GameFlow* g_GameFlow;

void DetatchSpark(int num, int type)//32D8C, 3328C (F)
{
	FX_INFO* fx = &Effects[num];
	ITEM_INFO* item = &Items[num];
	SPARKS* sptr = &Sparks[0];

	for (int lp = 0; lp < MAX_SPARKS; lp++, sptr++)
	{
		if (sptr->on && sptr->flags & type && sptr->fxObj == num)
		{
			if (type == 64)
			{
				sptr->x += fx->pos.xPos;
				sptr->y += fx->pos.yPos;
				sptr->z += fx->pos.zPos;

				sptr->flags &= 0xBF;
			}
			else if (type == 128)
			{
				sptr->x += item->pos.xPos;
				sptr->y += item->pos.yPos;
				sptr->z += item->pos.zPos;

				sptr->flags &= 0x7F;
			}
		}
	}
}

int GetFreeSpark()
{
	short sparkNumber = NextSpark;

	for (int i = 0; i < MAX_SPARKS; i++)
	{
		SPARKS* spark = &Sparks[sparkNumber];
		if (!spark->on)
		{
			NextSpark = (sparkNumber + 1) & 0x3FF;

			spark->extras = 0;
			spark->dynamic = -1;
			spark->def = Objects[ID_DEFAULT_SPRITES].meshIndex;

			return sparkNumber;
		}
		else if (sparkNumber == 1023)
		{
			sparkNumber = 0;
		}
		else
		{
			spark++;
			sparkNumber++;
		}
	}

	int life = 4095;
	for (int i = 0; i < MAX_SPARKS; i++)
	{
		SPARKS* spark = &Sparks[i];
		if (spark->life < life
			&& spark->dynamic == -1
			&& !(spark->flags & SP_EXPLOSION))
		{
			sparkNumber = i;
			life = spark->life;
		}
	}

	NextSpark = (sparkNumber + 1) & 0x3FF;

	SPARKS * spark = &Sparks[sparkNumber];
	spark->extras = 0;
	spark->dynamic = -1;
	spark->def = Objects[ID_DEFAULT_SPRITES].meshIndex;

	return sparkNumber;
}

void UpdateSparks()
{
	short* bounds = GetBoundsAccurate(LaraItem);

	DeadlyBounds[0] = LaraItem->pos.xPos + *bounds;
	DeadlyBounds[1] = LaraItem->pos.xPos + bounds[1];
	DeadlyBounds[2] = LaraItem->pos.yPos + bounds[2];
	DeadlyBounds[3] = LaraItem->pos.yPos + bounds[3];
	DeadlyBounds[4] = LaraItem->pos.zPos + bounds[4];
	DeadlyBounds[5] = LaraItem->pos.zPos + bounds[5];

	for (int i = 0; i < MAX_SPARKS; i++)
	{
		SPARKS* spark = &Sparks[i];

		if (spark->on)
		{
			spark->life--;
			if (!spark->life)
			{
				if (spark->dynamic != -1)
					SparkDynamics[spark->dynamic].On = false;
				spark->on = false;
				continue;
			}

			int life = spark->sLife - spark->life;
			if (life < spark->colFadeSpeed)
			{
				int dl = (life << 16) / spark->colFadeSpeed;
				spark->r = spark->sR + (dl * (spark->dR - spark->sR) >> 16);
				spark->g = spark->sG + (dl * (spark->dG - spark->sG) >> 16);
				spark->b = spark->sB + (dl * (spark->dB - spark->sB) >> 16);
			}
			else if (spark->life >= spark->fadeToBlack)
			{
				spark->r = spark->dR;
				spark->g = spark->dG;
				spark->b = spark->dB;
			}
			else
			{
				spark->r = (spark->dR * (((spark->life - spark->fadeToBlack) << 16) / spark->fadeToBlack + 0x10000)) >> 16;
				spark->g = (spark->dG * (((spark->life - spark->fadeToBlack) << 16) / spark->fadeToBlack + 0x10000)) >> 16;
				spark->b = (spark->dB * (((spark->life - spark->fadeToBlack) << 16) / spark->fadeToBlack + 0x10000)) >> 16;

				if (spark->r < 8 && spark->g < 8 && spark->b < 8)
				{
					spark->on = 0;
					continue;
				}
			}

			if (spark->life == spark->colFadeSpeed)
			{
				if (spark->flags & 0x800)
					spark->dSize >>= 2;
			}

			if (spark->flags & 0x10)
				spark->rotAng = (spark->rotAng + spark->rotAdd) & 0xFFF;

			if (spark->sLife - spark->life == spark->extras >> 3
				&& spark->extras & 7)
			{
				int unk;
				if (spark->flags & 0x800)
					unk = 1;
				else
					unk = (spark->flags & 0x2000) >> 12;

				for (int j = 0; j < (spark->extras & 7); j++)
				{
					TriggerExplosionSparks(spark->x,
						spark->y,
						spark->z,
						(spark->extras & 7) - 1,
						spark->dynamic,
						unk,
						(spark->extras & 7));
					spark->dynamic = -1;
				}

				if (unk == 1)
				{
					TriggerExplosionBubble(
						spark->x,
						spark->y,
						spark->z,
						spark->roomNumber);
				}
				spark->extras = 0;
			}

			spark->yVel += spark->gravity;
			if (spark->maxYvel)
			{
				if (spark->yVel > spark->maxYvel)
					spark->yVel = spark->maxYvel;
			}

			if (spark->friction & 0xF)
			{
				spark->xVel -= spark->xVel >> (spark->friction & 0xF);
				spark->zVel -= spark->zVel >> (spark->friction & 0xF);
			}

			if (spark->friction & 0xF0)
				spark->yVel -= spark->yVel >> (spark->friction >> 4);

			spark->x += spark->xVel >> 5;
			spark->y += spark->yVel >> 5;
			spark->z += spark->zVel >> 5;

			if (spark->flags & 0x100)
			{
				spark->x += SmokeWindX >> 1;
				spark->z += SmokeWindZ >> 1;
			}

			int dl = (spark->sLife - spark->life << 16) / spark->sLife;
			int ds = dl * (spark->dSize - spark->sSize);
			spark->size = spark->sSize + (ds & 0xFF);

			if (spark->flags & 1
				&& !Lara.burn
				|| spark->flags & 0x400)
			{
				ds = spark->size << spark->scalar >> 1;

				if (spark->x + ds > DeadlyBounds[0] && spark->x - ds < DeadlyBounds[1])
				{
					if (spark->y + ds > DeadlyBounds[2] && spark->y - ds < DeadlyBounds[3])
					{
						if (spark->z + ds > DeadlyBounds[4] && spark->z - ds < DeadlyBounds[5])
						{
							if (spark->flags & 1)
								LaraBurn();
							else
								LaraItem->hitPoints -= 2;
						}
					}
				}
			}
		}
	}

	for (int i = 0; i < MAX_SPARKS; i++)
	{
		SPARKS* spark = &Sparks[i];

		if (spark->on && spark->dynamic != -1)
		{
			SP_DYNAMIC* dynsp = &SparkDynamics[spark->dynamic];
			if (dynsp->Flags & 3)
			{
				int random = GetRandomControl();

				byte x = spark->x + 16 * (random & 0xF);
				byte y = spark->y + (random & 0xF0);
				byte z = spark->z + ((random >> 4) & 0xF0);

				byte r, g, b;

				int dl = spark->sLife - spark->life - 1;
				if (dl >= 2)
				{
					if (dl >= 4)
					{
						if (dynsp->Falloff)
							dynsp->Falloff--;

						b = ((random >> 4) & 0x1F) + 128;
						g = (random & 0x1F) + 224;
						r = (random >> 8) & 0x3F;
					}
					else
					{
						if (dynsp->Falloff < 28)
							dynsp->Falloff += 6;

						b = -8 * dl + 128;
						g = -8 * dl - (random & 0x1F) + 255;
						r = 32 * (4 - dl);
						if (32 * (4 - dl) < 0)
							r = 0;
					}
				}
				else
				{
					if (dynsp->Falloff < 28)
						dynsp->Falloff += 6;

					g = 255 - 8 * dl - (random & 0x1F);
					b = 255 - 16 * dl - (random & 0x1F);
					r = 255 - (dl << 6) - (random & 0x1F);
				}

				if (spark->flags & 0x2000)
				{
					int falloff;
					if (dynsp->Falloff <= 28)
						falloff = dynsp->Falloff;
					else
						falloff = 31;

					TriggerDynamicLight(x, y, z, falloff, r, g, b);
				}
				else
				{
					int falloff;
					if (dynsp->Falloff <= 28)
						falloff = dynsp->Falloff;
					else
						falloff = 31;

					TriggerDynamicLight(x, y, z, falloff, g, b, r);
				}
			}
		}
	}
}

void TriggerRicochetSpark(GAME_VECTOR* pos, short angle, int num, int unk)
{
	int random;
	SPARKS* spark;

	if (!unk)
	{
		for (int i = 0; i < num; i++)
		{
			spark = &Sparks[GetFreeSpark()];
			random = GetRandomControl();

			spark->on = true;
			spark->sR = -128;
			spark->sG = (random & 0xF) + 16;
			spark->sB = 0;
			spark->dR = 96;
			spark->dG = ((random >> 4) & 0x1F) + 48;
			spark->dB = 0;
			spark->colFadeSpeed = 2;
			spark->fadeToBlack = 4;
			spark->life = 9;
			spark->sLife = 9;
			spark->transType = 2;
			spark->x = pos->x;
			spark->y = pos->y;
			spark->z = pos->z;
			short ang = (((random >> 3) & 0x7FF) + angle - WALL_SIZE) & 0xFFF;
			spark->xVel = -rcossin_tbl[2 * ang] >> 2;
			spark->yVel = (random & 0xFFF) - 2048;
			spark->zVel = rcossin_tbl[2 * ang + 1] >> 2;
			spark->gravity = (random >> 7) & 0x1F;
			spark->friction = 34;
			spark->flags = 0;
			spark->maxYvel = 0;
		}
		
		spark = &Sparks[GetFreeSpark()];
		random = GetRandomControl();

		spark->on = true;
		spark->sR = 48;
		spark->sG = (random & 0xF) + 32;
		spark->sB = 0;
		spark->dR = 0;
		spark->dG = 0;
		spark->dB = 0;
		spark->colFadeSpeed = 4;
		spark->fadeToBlack = 0;
		spark->life = 4;
		spark->sLife = 4;
		spark->transType = 2;
		spark->x = pos->x;
		spark->y = pos->y;
		spark->z = pos->z;
		spark->xVel = 0;
		spark->yVel = 0;
		spark->zVel = 0;
		spark->flags = 26;
		spark->rotAng = (random >> 2) & 0xFFF;
		if (random & 1)
			spark->rotAdd = -64 - ((random >> 1) & 0x3F);
		else
			spark->rotAdd = ((random >> 1) & 0x3F) + 64;
		spark->scalar = 3;
		spark->def = Objects[ID_DEFAULT_SPRITES].meshIndex + 12;
		spark->sSize = spark->size = ((random >> 10) & 7) + 8;
		spark->dSize = 1;
		spark->maxYvel = 0;
		spark->gravity = 0;
	}
	
	if (1 - unk > 0)
	{
		num = (1 - unk);
		for (int i = 0; i < num; i++)
		{
			spark = &Sparks[GetFreeSpark()];
			int random = GetRandomControl();
			
			spark->on = true;
			spark->sR = 0;
			spark->sG = 0;
			spark->sB = 0;
			spark->dR = 40;
			spark->dG = 40;
			spark->dB = 40;
			spark->colFadeSpeed = (random & 3) + 4;
			spark->fadeToBlack = 8;
			spark->life =spark->sLife= ((random >> 2) & 7) + 16;
			spark->x = pos->x;
			spark->y = pos->y;
			spark->z = pos->z;
			if (unk)
			{
				spark->colFadeSpeed >>= 1;
				spark->fadeToBlack = 4;
				spark->sLife >>= 1;
				spark->life >>= 1;
				spark->xVel = (random & 0x1FF) - 256;
				spark->yVel = ((random >> 2) & 0x1FF) - 256;
				spark->zVel = ((random >> 4) & 0x1FF) - 256;
			}
			else
			{
				spark->yVel = 0;
				spark->xVel = 0;
				spark->zVel = 0;
			}
			spark->transType = 2;
			spark->friction = 0;
			spark->flags = 26;
			spark->rotAng = random >> 3;
			if (random & 1)
				spark->rotAdd = -random - (random & 0xF);
			else
				spark->rotAdd = (random & 0xF) + 16;
			spark->scalar = 2;
			spark->gravity = -4 - ((random >> 9) & 3);
			spark->sSize = spark->size= ((random >> 5) & 7) + 4;
			spark->maxYvel = -4 - ((random >> 6) & 3);
			spark->dSize = 4 * spark->size;
		}
	}
}

void TriggerCyborgSpark(int x, int y, int z, short xv, short yv, short zv)
{
	int dx = LaraItem->pos.xPos - x;
	int dz = LaraItem->pos.zPos - z;

	if (dx >= -16384 && dx <= 16384 && dz >= -16384 && dz <= 16384)
	{
		SPARKS* spark = &Sparks[GetFreeSpark()];
		int random = rand();
		
		spark->sR = -1;
		spark->sB = -1;
		spark->sG = -1;
		spark->dR = -1;
		spark->on = true;
		spark->colFadeSpeed = 3;
		spark->fadeToBlack = 5;
		spark->dG = (random & 0x7F) + 64;
		spark->dB = -64 - ((random & 0x7F) + 64);
		spark->life = 10;
		spark->sLife = 10;
		spark->transType = 2;
		spark->friction = 34;
		spark->scalar = 1;
		spark->x = (random & 7) + x - 3;
		spark->y = ((random >> 3) & 7) + y - 3;
		spark->z = ((random >> 6) & 7) + z - 3;
		spark->flags = 2;
		spark->xVel = (random >> 2) + xv - 128;
		spark->yVel = (random >> 4) + yv - 128;
		spark->zVel = (random >> 6) + zv - 128;
		spark->sSize = spark->size = ((random >> 9) & 3) + 4;
		spark->dSize = ((random >> 12) & 1) + 1;
		spark->maxYvel = 0;
		spark->gravity = 0;
	}
}

void TriggerExplosionSparks(int x, int y, int z, int extraTrig, int dynamic, int uw, int roomNumber)
{
	int shift = 0;
	if ((roomNumber & 0x8000) != 0)
	{
		//v7 = -roomNumber;
		roomNumber = -roomNumber;
		shift = 1;
	}
	/*if (v7 == gfMirrorRoom && gfLevelFlags & 0x2000)
		v27 = 1;
	z_bis = z;
	do
	{*/
	SPARKS* spark = &Sparks[GetFreeSpark()];
	spark->on = 1;
	spark->sR = -1;
	if (uw == 1)
	{
		spark->sB = 32;
		spark->dR = -64;
		spark->sG = (GetRandomControl() & 0x3F) + -128;
		spark->dB = 0;
		spark->colFadeSpeed = 7;
		spark->dG = (GetRandomControl() & 0x1F) + 64;
		spark->fadeToBlack = 8;
		spark->transType = 2;
		spark->life = spark->sLife = (GetRandomControl() & 7) + 16;
		spark->roomNumber = roomNumber;
	}
	else
	{
		spark->sB = 0;
		spark->sG = (GetRandomControl() & 0xF) + 32;
		spark->dR = (GetRandomControl() & 0x3F) - 64;
		spark->dB = 32;
		spark->colFadeSpeed = 8;
		spark->dG = (GetRandomControl() & 0x3F) + -128;
		spark->fadeToBlack = 16;
		spark->transType = 2;
		spark->life = spark->sLife = (GetRandomControl() & 7) + 24;
	}
	spark->extras = extraTrig | 8 * (TES_extra_tab[extraTrig] + (GetRandomControl() & 7) + 28);
	spark->dynamic = dynamic;
	if (dynamic == -2)
	{
		int j;
		for (j = 0; j < 8; j++)
		{
			SP_DYNAMIC* spdyn = &SparkDynamics[j];
			if (!spdyn->On)
			{
				spdyn->On = 1;
				spdyn->Falloff = 4;
				if (uw == 1)
					spdyn->Flags = SD_UWEXPLOSION;
				else
					spdyn->Flags = SD_EXPLOSION;
				spark->dynamic = j;
				break;
			}
		}

		if (j == 8)
		{
			spark->dynamic = -1;
		}
	}
	spark->xVel = (GetRandomControl() & 0xFFF) - 2048;
	spark->yVel = (GetRandomControl() & 0xFFF) - 2048;
	spark->zVel = (GetRandomControl() & 0xFFF) - 2048;
	if (dynamic != -2 || uw == 1)
	{
		spark->x = (GetRandomControl() & 0x1F) + x - 16;
		spark->y = (GetRandomControl() & 0x1F) + y - 16;
		spark->z = (GetRandomControl() & 0x1F) + z - 16;
	}
	else
	{
		spark->x = (GetRandomControl() & 0x1FF) + x - 256;
		spark->y = (GetRandomControl() & 0x1FF) + y - 256;
		spark->z = (GetRandomControl() & 0x1FF) + z - 256;
	}
	if (uw == 1)
		spark->friction = 17;
	else
		spark->friction = 51;
	if (GetRandomControl() & 1)
	{
		if (uw == 1)
			spark->flags = SP_SCALE | SP_DEF | SP_ROTATE | SP_EXPDEF | SP_UNDERWEXP;
		else
			spark->flags = SP_SCALE | SP_DEF | SP_ROTATE | SP_EXPDEF;
		spark->rotAng = GetRandomControl() & 0xFFF;
		spark->rotAdd = GetRandomControl() + -128;
	}
	else if (uw == 1)
	{
		spark->flags = SP_SCALE | SP_DEF | SP_EXPDEF | SP_UNDERWEXP;
	}
	else
	{
		spark->flags = SP_SCALE | SP_DEF | SP_EXPDEF;
	}
	spark->scalar = 3;
	spark->gravity = 0;
	int size = (GetRandomControl() & 0xF) + 40;
	spark->maxYvel = 0;
	spark->sSize = size << shift;
	spark->size = size << shift;
	spark->dSize = size << (shift + 1);

	if (uw == 2)
	{
		spark->sG = spark->sR;
		spark->sB = spark->sG;
		spark->flags |= SP_PLASMAEXP;
		spark->sR = spark->sB;
		spark->dR = spark->dB;
		spark->dG = spark->dR;
		spark->dB = spark->dG;
	}
	else if (extraTrig)
	{
		TriggerExplosionSmoke(x, y, z, uw);
	}
	else
	{
		TriggerExplosionSmokeEnd(x, y, z, uw);
	}
	//} while (!v24);
}

void TriggerExplosionSmokeEnd(int x, int y, int z, int uw)
{
	SPARKS* spark = &Sparks[GetFreeSpark()];
	spark->on = true;
	if (uw)
	{
		spark->sR = 0;
		spark->sG = 0;
		spark->sB = 0;
		spark->dR = 192;
		spark->dG = 192;
		spark->dB = 208;
	}
	else
	{
		spark->dR = 64;
		spark->sR = 144;
		spark->sG = 144;
		spark->sB = 144;
		spark->dG = 64;
		spark->dB = 64;
	}
	spark->colFadeSpeed = 8;
	spark->fadeToBlack = 64;
	spark->life = spark->sLife= (GetRandomControl() & 0x1F) + 96;
	if (uw)
		spark->transType = COLADD;
	else
		spark->transType = COLSUB;
	spark->x = (GetRandomControl() & 0x1F) + x - 16;
	spark->y = (GetRandomControl() & 0x1F) + y - 16;
	spark->z = (GetRandomControl() & 0x1F) + z - 16;
	spark->xVel = ((GetRandomControl() & 0xFFF) - 2048) >> 2;
	spark->yVel = GetRandomControl() - 128;
	spark->zVel = ((GetRandomControl() & 0xFFF) - 2048) >> 2;
	if (uw)
	{
		spark->friction = 20;
		spark->yVel >>= 4;
		spark->y += 32;
	}
	else
	{
		spark->friction = 6;
	}
	spark->flags = SP_SCALE | SP_DEF | SP_ROTATE | SP_EXPDEF;
	spark->rotAng = GetRandomControl() & 0xFFF;
	if (GetRandomControl() & 1)
		spark->rotAdd = -16 - (GetRandomControl() & 0xF);
	else
		spark->rotAdd = (GetRandomControl() & 0xF) + 16;
	spark->scalar = 3;
	if (uw)
	{
		spark->maxYvel = 0;
		spark->gravity = 0;
	}
	else
	{
		spark->gravity = -3 - (GetRandomControl() & 3);
		spark->maxYvel = -4 - (GetRandomControl() & 3);
	}
	spark->dSize = (GetRandomControl() & 0x1F) + 128;
	spark->sSize = spark->dSize >> 2;
	spark->size = spark->dSize >> 2;
}

void TriggerExplosionSmoke(int x, int y, int z, int uw)
{
	int dx = LaraItem->pos.xPos - x;
	int dz = LaraItem->pos.zPos - z;
	
	if (dx >= -16384 && dx <= 16384 && dz >= -16384 && dz <= 16384)
	{
		SPARKS* spark = &Sparks[GetFreeSpark()];
		spark->sR = -112;
		spark->sG = -112;
		spark->sB = -112;
		spark->on = 1;
		spark->dR = 64;
		spark->dG = 64;
		spark->dB = 64;
		spark->colFadeSpeed = 2;
		spark->fadeToBlack = 8;
		spark->transType = COLSUB;
		spark->life = spark->sLife = (GetRandomControl() & 3) + 10;
		spark->x = (GetRandomControl() & 0x1FF) + x - 256;
		spark->y = (GetRandomControl() & 0x1FF) + y - 256;
		spark->z = (GetRandomControl() & 0x1FF) + z - 256;
		spark->xVel = ((GetRandomControl() & 0xFFF) - 2048) >> 2;
		spark->yVel = GetRandomControl() - 128;
		spark->zVel = ((GetRandomControl() & 0xFFF) - 2048) >> 2;
		if (uw)
			spark->friction = 2;
		else
			spark->friction = 6;
		spark->flags = SP_SCALE | SP_DEF | SP_ROTATE | SP_EXPDEF;
		spark->rotAng = GetRandomControl() & 0xFFF;
		spark->scalar = 1;
		spark->rotAdd = (GetRandomControl() & 0xF) + 16;
		spark->gravity = -3 - (GetRandomControl() & 3);
		spark->maxYvel = -4 - (GetRandomControl() & 3);
		spark->dSize = (GetRandomControl() & 0x1F) + 128;
		spark->sSize = spark->dSize >> 2;
		spark->size = spark->dSize >> 2;
	}
}

void TriggerFireFlame(int x, int y, int z, int fxObj, int type)
{
	int dx = LaraItem->pos.xPos - x;
	int dz = LaraItem->pos.zPos - z;

	if (dx < -16384 || dx > 16384 || dz < -16384 || dz > 16384)
		return;

	SPARKS * spark = &Sparks[GetFreeSpark()];
	spark->on = true;
	if (type == 2)
	{
		spark->sR = (GetRandomControl() & 0x1F) + 48;
		spark->sG = (GetRandomControl() & 0x1F) + 48;
		spark->sB = (GetRandomControl() & 0x3F) - 64;
	}
	else
	{
		if (type == 254)
		{
			spark->sR = 0;
			spark->sB = 0;
			spark->sG = 0;
			spark->dR = (GetRandomControl() & 0xF) + 32;
			spark->dB = (GetRandomControl() & 0xF) + 32;
			spark->dG = (GetRandomControl() & 0xF) + 32;
		}
		else
		{
			spark->sR = -1;
			spark->sB = 48;
			spark->sG = (GetRandomControl() & 0x1F) + 48;
			if (Lara.burnBlue == 1)
			{
				spark->sR = 48;
				spark->sB = spark->sR;
			}
			else if (Lara.burnBlue == 2)
			{
				spark->sB = spark->sG >> 1;
				spark->sG = -1;
				spark->sR = 0;
			}
		}
	}

	if (type != 254)
	{
		spark->dR = (GetRandomControl() & 0x3F) - 64;
		spark->dB = 32;
		spark->dG = (GetRandomControl() & 0x3F) + -128;
		if (Lara.burnBlue == 1)
		{
			spark->dR = 32;
			spark->dB = spark->dR;
		}
		else if (Lara.burnBlue == 2)
		{
			spark->dB = spark->dG;
			spark->dG = spark->dR;
			spark->dR = 0;
		}
	}

	if (fxObj == -1)
	{
		if (type == 2 || type == 255 || type == 254)
		{
			spark->fadeToBlack = 6;
			spark->colFadeSpeed = (GetRandomControl() & 3) + 5;
			spark->sLife = spark->life = (type < 254 ? 0 : 8) + (GetRandomControl() & 3) + 16;
		}
		else
		{
			spark->fadeToBlack = 8;
			spark->colFadeSpeed = (GetRandomControl() & 3) + 20;
			spark->sLife = spark->life = (GetRandomControl() & 7) + 40;
		}
	}
	else
	{
		spark->fadeToBlack = 16;
		spark->colFadeSpeed = (GetRandomControl() & 3) + 8;
		spark->sLife = spark->life = (GetRandomControl() & 3) + 28;
	}
	spark->transType = COLADD;
	if (fxObj == -1)
	{
		if (type && type != 1)
		{
			if (type < 254)
			{
				spark->y = y;
				spark->x = (GetRandomControl() & 0xF) + x - 8;
				spark->z = (GetRandomControl() & 0xF) + z - 8;
			}
			else
			{
				spark->x = (GetRandomControl() & 0x3F) + x - 32;
				spark->y = y;
				spark->z = (GetRandomControl() & 0x3F) + z - 32;
			}
		}
		else
		{
			spark->y = y;
			spark->x = (GetRandomControl() & 0x1F) + x - 16;
			spark->z = (GetRandomControl() & 0x1F) + z - 16;
		}
	}
	else
	{
		spark->y = 0;
		spark->x = (GetRandomControl() & 0x1F) - 16;
		spark->z = (GetRandomControl() & 0x1F) - 16;
	}

	if (type == 2)
	{
		spark->xVel = (GetRandomControl() & 0x1F) - 16;
		spark->yVel = -1024 - (GetRandomControl() & 0x1FF);
		spark->friction = 68;
		spark->zVel = (GetRandomControl() & 0x1F) - 16;
	}
	else
	{
		spark->xVel = GetRandomControl() - 128;
		spark->yVel = -16 - (GetRandomControl() & 0xF);
		spark->zVel = GetRandomControl() - 128;
		if (type == 1)
			spark->friction = 51;
		else
			spark->friction = 5;
	}
	if (fxObj == -1)
	{
		spark->gravity = -16 - (GetRandomControl() & 0x1F);
		spark->maxYvel = -16 - (GetRandomControl() & 7);
		spark->flags = 538;
		if (type == 254)
			spark->gravity >>= 1;
	}
	else
	{
		spark->flags = 602;
		spark->fxObj = fxObj;
		spark->gravity = -32 - (GetRandomControl() & 0x3F);
		spark->maxYvel = -24 - (GetRandomControl() & 7);
	}
	spark->rotAng = GetRandomControl() & 0xFFF;
	spark->scalar = 2;
	spark->rotAdd = (GetRandomControl() & 0x1F) - 16;
	if (type)
	{
		if (type == 1)
		{
			spark->sSize = spark->size = (GetRandomControl() & 0x1F) + 64;
		}
		else if (type < 254)
		{
			spark->maxYvel = 0;
			spark->gravity = 0;
			spark->sSize = spark->size = (GetRandomControl() & 0x1F) + 32;
		}
		else
		{
			spark->sSize = spark->size = (GetRandomControl() & 0xF) + 48;
		}
	}
	else
	{
		spark->sSize = spark->size = (GetRandomControl() & 0x1F) + 128;
	}

	if (type == 2)
	{
		spark->dSize = spark->size >> 2;
	}
	else
	{
		spark->dSize = spark->size >> 4;
		if (type == 7)
		{
			spark->colFadeSpeed >>= 2;
			spark->fadeToBlack >>= 2;
			spark->life = spark->life >> 2;
			spark->sLife = spark->life >> 2;
		}
	}
}

void TriggerSuperJetFlame(ITEM_INFO* item, int yvel, int deadly)//32EAC, 333AC (F)
{
	long dx = LaraItem->pos.xPos - item->pos.xPos;
	long dz = LaraItem->pos.zPos - item->pos.zPos;

	if (dx >= -16384 && dx <= 16384 && dz >= -16384 && dz <= 16384)
	{
		int size = (GetRandomControl() & 0x1FF) - yvel;
		SPARKS* sptr = &Sparks[GetFreeSpark()];

		if (size < 512)
			size = 512;

		sptr->on = 1;
		sptr->sR = sptr->sG = (GetRandomControl() & 0x1F) + 48;
		sptr->sB = (GetRandomControl() & 0x3F) - 64;
		sptr->dR = (GetRandomControl() & 0x3F) - 64;
		sptr->dG = (GetRandomControl() & 0x3F) - 128;
		sptr->dB = 32;
		sptr->colFadeSpeed = 8;
		sptr->fadeToBlack = 8;
		sptr->transType = 2;
		sptr->life = sptr->sLife = (size >> 9) + (GetRandomControl() & 7) + 16;
		sptr->x = (GetRandomControl() & 0x1F) + item->pos.xPos - 16;
		sptr->y = (GetRandomControl() & 0x1F) + item->pos.yPos - 16;
		sptr->z = (GetRandomControl() & 0x1F) + item->pos.zPos - 16;
		sptr->friction = 51;
		sptr->maxYvel = 0;
		sptr->flags = 538;
		if (deadly)
			sptr->flags = 539;
		sptr->scalar = 2;
		sptr->dSize = (GetRandomControl() & 0xF) + (size >> 6) + 16;
		sptr->sSize = sptr->size = sptr->dSize >> 1;

		if ((-(item->triggerFlags & 0xFF) & 7) == 1)
		{
			sptr->gravity = -16 - (GetRandomControl() & 0x1F);
			sptr->xVel = (GetRandomControl() & 0xFF) - 128;
			sptr->yVel = -size;
			sptr->zVel = (GetRandomControl() & 0xFF) - 128;
			sptr->dSize += sptr->dSize >> 2;
			return;
		}

		sptr->y -= 64;
		sptr->gravity = -((size >> 9) + GetRandomControl() % (size >> 8));
		sptr->yVel = (GetRandomControl() & 0xFF) - 128;
		sptr->xVel = (GetRandomControl() & 0xFF) - 128;
		sptr->zVel = (GetRandomControl() & 0xFF) - 128;

		if (item->pos.yRot == 0)
		{
			sptr->zVel = -(size - (size >> 2));
		}
		else if (item->pos.yRot == ANGLE(90))
		{
			sptr->xVel = -(size - (size >> 2));
		}
		else if (item->pos.yRot == ANGLE(-180))
		{
			sptr->zVel = size - (size >> 2);
		}
		else
		{
			sptr->xVel = size - (size >> 2);
		}
	}
}

void SetupSplash(SPLASH_SETUP* setup)
{
	for (int i = 0; i < MAX_SPLASH; i++)
	{
		SPLASH_STRUCT* splash = &Splashes[i];

		if (!(splash->flags & 1))
		{
			splash->flags = 1;
			splash->x = setup->x;
			splash->y = setup->y;
			splash->z = setup->z;
			splash->life = 62;
			splash->innerRad = setup->innerRad;
			splash->innerSize = setup->innerSize;
			splash->innerRadVel = setup->innerRadVel;
			splash->innerYVel = setup->innerYVel;
			splash->innerY = setup->innerYVel >> 2;
			splash->middleRad = setup->middleRad;
			splash->middleSize = setup->middleSize;
			splash->middleRadVel = setup->middleRadVel;
			splash->middleYVel = setup->middleYVel;
			splash->middleY = setup->middleYVel >> 2;
			splash->outerRad = setup->outerRad;
			splash->outerSize = setup->outerSize;
			splash->outerRadVel = setup->outerRadVel;

			break;
		}
	}

	SoundEffect(SFX_LARA_SPLASH, (PHD_3DPOS*)setup, 0);
}

void UpdateSplashes()
{
	for (int i = 0; i < MAX_SPLASH; i++)
	{
		SPLASH_STRUCT* splash = &Splashes[i];

		if (splash->flags & 1)
		{
			splash->innerRad += splash->innerRadVel >> 5;
			splash->innerSize += splash->innerRadVel >> 6;
			splash->innerRadVel -= (splash->innerRadVel >> 6);

			splash->middleRad += splash->middleRadVel >> 5;
			splash->middleSize += splash->middleRadVel >> 6;
			splash->middleRadVel -= splash->middleRadVel >> 6;

			splash->outerRad += splash->outerRadVel >> 5;
			splash->outerSize += splash->outerRadVel >> 6;
			splash->outerRadVel -= splash->outerRadVel >> 6;

			splash->innerY += splash->innerYVel >> 4;
			splash->innerYVel += 1024;

			if (splash->innerYVel > 16384)
				splash->innerYVel = 16384;

			if (splash->innerY < 0)
			{
				if (splash->innerY < -28672)
					splash->innerY = -28672;
			}
			else
			{
				splash->innerY = 0;
				splash->flags |= 4;
				splash->life -= 2;

				if (splash->life == 0)
					splash->flags = 0;
			}

			splash->middleY += splash->middleYVel >> 4;
			splash->middleYVel += 896;

			if (splash->middleYVel > 16384)
				splash->middleYVel = 16384;

			if (splash->middleY < 0)
			{
				if (splash->middleY < -28672)
					splash->middleY = -28672;
			}
			else
			{
				splash->middleY = 0;
				splash->flags |= 8;
			}
		}
	}

	for (int i = 0; i < MAX_RIPPLES; i++)
	{
		RIPPLE_STRUCT* ripple = &Ripples[i];

		if (ripple->flags & 1)
		{
			if (ripple->size < 252)
			{
				if (ripple->flags & 2)
					ripple->size += 2;
				else
					ripple->size += 4;

				if (ripple->init)
				{
					if (ripple->init < ripple->life)
					{
						if (ripple->flags & 2)
							ripple->init += 8;
						else
							ripple->init += 4;

						if (ripple->init >= ripple->life)
							ripple->init = 0;
					}
				}
				else
				{
					ripple->life -= 3;
					if (ripple->life > 250)
						ripple->flags = 0;
				}
			}
		}
	}
}

void SetupRipple(int x, int y, int z, char size, char flags)
{
	RIPPLE_STRUCT* ripple;
	int i;

	for (i = 0; i < MAX_RIPPLES; i++)
	{
		ripple = &Ripples[i];
		if (!(ripple->flags & 1))
			break;
	}

	if (i == MAX_RIPPLES)
		return;

	ripple->flags = flags | 1;
	ripple->size = size;
	ripple->life = (GetRandomControl() & 0xF) + 48;
	ripple->init = 1;
	ripple->x = x;
	ripple->y = y;
	ripple->z = z;
	if (flags & 0x40)
	{
		ripple->x += (GetRandomControl() & 0x7F) - 64;
		ripple->z += (GetRandomControl() & 0x7F) - 64;
	}
}

void TriggerUnderwaterBlood(int x, int y, int z, int sizeme) 
{
	for (int i = 0; i < MAX_RIPPLES; i++)
	{
		RIPPLE_STRUCT* ripple = &Ripples[i];
		if (!(ripple->flags & 1))
		{
			ripple->flags = 0x31;
			ripple->init = 1;
			ripple->life = (GetRandomControl() & 7) - 16;
			ripple->size = sizeme;
			ripple->x = (GetRandomControl() & 0x3F) + x - 32;
			ripple->y = y;
			ripple->z = (GetRandomControl() & 0x3F) + z - 32;
			return;
		}
	}
}

void TriggerWaterfallMist(int x, int y, int z, int angle)
{
	SPARKS* spark;
	int dh = 0;
	int ang1 = angle;
	int ang2 = angle;
	int dl;

	// CHECK THIS LOOP CONDITIONS
	for (ang1 = angle; ; ang1 *= 2)
	{
		spark = &Sparks[GetFreeSpark()];
		spark->on = 1;
		spark->sR = 64;
		spark->sG = 64;
		spark->sB = 64;
		spark->dR = 64;
		spark->dG = 64;
		spark->dB = 64;
		spark->colFadeSpeed = 1;
		spark->transType = 2;
		spark->life = spark->sLife = (GetRandomControl() & 3) + 6;
		spark->fadeToBlack = spark->life - 4;
		dl = ((dh + (GlobalCounter << 6)) % 1536) + (GetRandomControl() & 0x3F) - 32;
		spark->x = dl * SIN(ang1) >> W2V_SHIFT + (GetRandomControl() & 0xF) + x - 8;
		spark->y = (GetRandomControl() & 0xF) + y - 8;
		spark->z = dl * COS(ang1) >> W2V_SHIFT + (GetRandomControl() & 0xF) + z - 8;
		spark->xVel = 0;
		spark->zVel = 0;
		spark->friction = 0;
		spark->flags = 538;
		spark->yVel = (GetRandomControl() & 0x7F) + 128;
		spark->rotAng = GetRandomControl() & 0xFFF;
		spark->scalar = 3;
		spark->maxYvel = 0;
		spark->rotAdd = (GetRandomControl() & 0x1F) - 16;
		spark->gravity = -spark->yVel >> 2;
		spark->sSize = spark->size = (GetRandomControl() & 3) + 16;
		spark->dSize = 2 * spark->size;

		dh += 256;
		if (dh > 1536)
			break;
	}

	spark = &Sparks[GetFreeSpark()];
	spark->on = 1;
	spark->sR = 96;
	spark->sG = 96;
	spark->sB = 96;
	spark->dR = 96;
	spark->dG = 96;
	spark->dB = 96;
	spark->colFadeSpeed = 1;
	spark->transType = 2;
	spark->life = spark->sLife = (GetRandomControl() & 3) + 6;
	spark->fadeToBlack = spark->life - 1;
	dl = GetRandomControl() % 1408 + 64;
	spark->x = dl * SIN(ang1) >> W2V_SHIFT + (GetRandomControl() & 0x1F) + x - 16;
	spark->y = (GetRandomControl() & 0xF) + y - 8;
	spark->xVel = 0;
	spark->zVel = 0;
	spark->z = dl * COS(ang1) >> W2V_SHIFT + (GetRandomControl() & 0x1F) + z - 16;
	spark->friction = 0;
	spark->flags = 10;
	spark->yVel = GetRandomControl() & 0x100 + (GetRandomControl() & 0x7F) + 128;
	spark->scalar = 2;
	spark->def = Objects[ID_DEFAULT_SPRITES].meshIndex + 17;
	spark->gravity = 0;
	spark->maxYvel = 0;
	spark->sSize = spark->size = (GetRandomControl() & 7) + 8;
	spark->dSize = spark->size * 2;
}

void TriggerDartSmoke(int x, int y, int z, int xv, int zv, int hit)
{
	int dx = LaraItem->pos.xPos - x;
	int dz = LaraItem->pos.zPos - z;
	
	if (dx < -16384 || dx > 16384 || dz < -16384 || dz > 16384)
		return;

	SPARKS* spark = &Sparks[GetFreeSpark()];
	spark->on = 1;
	spark->sR = 16;
	spark->sG = 8;
	spark->sB = 4;
	spark->dR = 64;
	spark->dG = 48;
	spark->dB = 32;
	spark->colFadeSpeed = 8;
	spark->fadeToBlack = 4;
	spark->transType = 2;
	spark->life = spark->sLife = (GetRandomControl() & 3) + 32;
	spark->x = (GetRandomControl() & 0x1F) + x - 16;
	spark->y = (GetRandomControl() & 0x1F) + y - 16;
	spark->z = (GetRandomControl() & 0x1F) + z - 16;
	if (hit)
	{
		spark->xVel = GetRandomControl() - xv - 128;
		spark->yVel = -4 - (GetRandomControl() & 3);
		spark->zVel = GetRandomControl() - zv - 128;
	}
	else
	{
		if (xv)
			spark->xVel = -xv;
		else
			spark->xVel = GetRandomControl() - 128;
		spark->yVel = -4 - (GetRandomControl() & 3);
		if (!zv)
		{
			spark->zVel = GetRandomControl();
		}
		else
			spark->zVel = -zv;
	}

	spark->friction = 3;
	if (GetRandomControl() & 1)
	{
		spark->flags = 538;
		spark->rotAng = GetRandomControl() & 0xFFF;
		if (GetRandomControl() & 1)
			spark->rotAdd = -16 - (GetRandomControl() & 0xF);
		else
			spark->rotAdd = (GetRandomControl() & 0xF) + 16;
	}
	else
	{
		spark->flags = 522;
	}
	spark->scalar = 1;
	int size = (GetRandomControl() & 0x3F) + 72;
	if (hit)
	{
		spark->maxYvel = 0;
		spark->sSize = spark->size = spark->dSize = size >> 3;
		spark->gravity = 0;
	}
	else
	{
		spark->sSize = spark->size = size >> 4;
		spark->gravity = -4 - (GetRandomControl() & 3);
		spark->dSize = size;
		spark->maxYvel = -4 - (GetRandomControl() & 3);
	}
}

void KillAllCurrentItems(short itemNumber)
{
	KillEverythingFlag = 1;
}

void TriggerDynamicLight(int x, int y, int z, short falloff, byte r, byte g, byte b)
{
	g_Renderer->AddDynamicLight(x, y, z, falloff, r, g, b);
}

// Really needed?
void ClearDynamicLights()
{
	for (int i = 0; i < 32; i++)
	{
		Dynamics[i].on = false;
	}
}

void WadeSplash(ITEM_INFO* item, int wh, int wd)
{
	short roomNumber = item->roomNumber;
	GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);

	ROOM_INFO* room = &Rooms[roomNumber];
	if (room->flags & ENV_FLAG_WATER)
	{
		short roomNumber2 = item->roomNumber;
		GetFloor(item->pos.xPos, room->y - 128, item->pos.zPos, &roomNumber2);

		ROOM_INFO* room2 = &Rooms[roomNumber2];
		if (!(room2->flags & ENV_FLAG_WATER))
		{
			short* frame = GetBestFrame(item);
			if (item->pos.yPos + frame[2] <= wh)
			{
				if (item->pos.yPos + frame[3] >= wh)
				{
					if (item->fallspeed <= 0 || wd >= 474 || SplashCount != 0)
					{
						if (!(Wibble & 0xF))
						{
							if (!(GetRandomControl() & 0xF) || item->currentAnimState != STATE_LARA_STOP)
							{
								if (item->currentAnimState == STATE_LARA_STOP)
								{
									SetupRipple(item->pos.xPos, wh, item->pos.zPos, (GetRandomControl() & 0xF) + 112, 16);
								}
								else
								{
									SetupRipple(item->pos.xPos, wh, item->pos.zPos, (GetRandomControl() & 0xF) + 112, 18);
								}
							}
						}
					}
					else
					{
						SplashSetup.y = wh;
						SplashSetup.x = item->pos.xPos;
						SplashSetup.z = item->pos.zPos;
						SplashSetup.innerRad = 16;
						SplashSetup.innerSize = 12;
						SplashSetup.innerRadVel = 160;
						SplashSetup.innerYVel = -72 * item->fallspeed;
						SplashSetup.middleRad = 24;
						SplashSetup.middleSize = 24;
						SplashSetup.middleRadVel = 224;
						SplashSetup.middleYVel = -36 * item->fallspeed;
						SplashSetup.outerRad = 32;
						SplashSetup.outerSize = 32;
						SplashSetup.outerRadVel = 272;
						SetupSplash(&SplashSetup);
						SplashCount = 16;
					}
				}
			}
		}
	}
}

void Splash(ITEM_INFO* item)
{
	short roomNumber = item->roomNumber;
	GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);

	ROOM_INFO* room = &Rooms[roomNumber];
	if (room->flags & ENV_FLAG_WATER)
	{
		int wh = GetWaterHeight(item->pos.xPos, item->pos.yPos, item->pos.zPos, roomNumber);
		SplashSetup.y = wh;
		SplashSetup.x = item->pos.xPos;
		SplashSetup.z = item->pos.zPos;
		SplashSetup.innerRad = 32;
		SplashSetup.innerSize = 8;
		SplashSetup.innerRadVel = 320;
		SplashSetup.innerYVel = -40 * item->fallspeed;
		SplashSetup.middleRad = 48;
		SplashSetup.middleSize = 32;
		SplashSetup.middleRadVel = 480;
		SplashSetup.middleYVel = -20 * item->fallspeed;
		SplashSetup.outerRad = 32;
		SplashSetup.outerSize = 128;
		SplashSetup.outerRadVel = 544;
		SetupSplash(&SplashSetup);
	}
}

void TriggerRocketFlame(int x, int y, int z, int xv, int yv, int zv, int itemNumber)
{
	SPARKS* sptr = &Sparks[GetFreeSpark()];

	sptr->on = true;
	sptr->sR = 48 + (GetRandomControl() & 31);
	sptr->sG = sptr->sR;
	sptr->sB = 192 + (GetRandomControl() & 63);

	sptr->dR = 192 + (GetRandomControl() & 63);
	sptr->dG = 128 + (GetRandomControl() & 63);
	sptr->dB = 32;

	sptr->colFadeSpeed = 12 + (GetRandomControl() & 3);
	sptr->fadeToBlack = 12;
	sptr->sLife = sptr->life = (GetRandomControl() & 3) + 28;
	sptr->transType = 2;
	sptr->extras = 0;
	sptr->dynamic = -1;

	sptr->x = x + ((GetRandomControl() & 31) - 16);
	sptr->y = y;
	sptr->z = z + ((GetRandomControl() & 31) - 16);

	sptr->xVel = xv;
	sptr->yVel = yv;
	sptr->zVel = zv;
	sptr->friction = 3 | (3 << 4);

	if (GetRandomControl() & 1)
	{
		sptr->flags = SP_SCALE | SP_DEF | SP_ROTATE | SP_ITEM | SP_EXPDEF;
		sptr->fxObj = itemNumber;
		sptr->rotAng = GetRandomControl() & 4095;
		if (GetRandomControl() & 1)
			sptr->rotAdd = -(GetRandomControl() & 15) - 16;
		else
			sptr->rotAdd = (GetRandomControl() & 15) + 16;
	}
	else
	{
		sptr->flags = SP_SCALE | SP_DEF | SP_ITEM | SP_EXPDEF;
		sptr->fxObj = itemNumber;
	}
	
	sptr->gravity = 0;
	sptr->maxYvel = 0;

	// TODO: right sprite
	sptr->def = Objects[ID_DEFAULT_SPRITES].meshIndex;
	sptr->scalar = 2;

	int size = (GetRandomControl() & 7) + 32;
	sptr->size = sptr->sSize = size;
}

void TriggerRocketSmoke(int x, int y, int z, int bodyPart)
{
	SPARKS* sptr = &Sparks[GetFreeSpark()];

	sptr->on = true;
	sptr->sR = 0;
	sptr->sG = 0;
	sptr->sB = 0;

	sptr->dR = 64 + bodyPart;
	sptr->dG = 64 + bodyPart;
	sptr->dB = 64 + bodyPart;

	sptr->colFadeSpeed = 4 + (GetRandomControl() & 3);
	sptr->fadeToBlack = 12;
	sptr->sLife = sptr->life = (GetRandomControl() & 3) + 20;
	sptr->transType = 2;
	sptr->extras = 0;
	sptr->dynamic = -1;

	sptr->x = x + ((GetRandomControl() & 15) - 8);
	sptr->y = y + ((GetRandomControl() & 15) - 8);
	sptr->z = z + ((GetRandomControl() & 15) - 8);
	sptr->xVel = ((GetRandomControl() & 255) - 128);
	sptr->yVel = -(GetRandomControl() & 3) - 4;
	sptr->zVel = ((GetRandomControl() & 255) - 128);
	sptr->friction = 4;

	if (GetRandomControl() & 1)
	{
		sptr->flags = SP_SCALE | SP_DEF | SP_ROTATE | SP_EXPDEF;
		sptr->rotAng = GetRandomControl() & 4095;
		if (GetRandomControl() & 1)
			sptr->rotAdd = -(GetRandomControl() & 15) - 16;
		else
			sptr->rotAdd = (GetRandomControl() & 15) + 16;
	}
	else
		sptr->flags = SP_SCALE | SP_DEF | SP_EXPDEF;

	// TODO: right sprite
	sptr->def = Objects[ID_DEFAULT_SPRITES].meshIndex;
	sptr->scalar = 3;
	sptr->gravity = -(GetRandomControl() & 3) - 4;
	sptr->maxYvel = -(GetRandomControl() & 3) - 4;

	int size = (GetRandomControl() & 7) + 32;
	sptr->size = sptr->sSize = size >> 2;
}

void GrenadeExplosionEffects(int x, int y, int z, short roomNumber)
{
	ROOM_INFO* room = &Rooms[roomNumber];

	bool mirror = (roomNumber == g_GameFlow->GetLevel(CurrentLevel)->Mirror.Room);

	bool water = false;
	if (room->flags & ENV_FLAG_WATER)
	{
		TriggerExplosionBubble(x, y, z, roomNumber);
		water = true;
	}

	SMOKE_SPARKS* spark = &SmokeSparks[GetFreeSmokeSpark()];
	spark->on = true;
	spark->sShade = 0;
	spark->dShade = -128;
	spark->colFadeSpeed = 4;
	spark->fadeToBlack = 16;
	spark->transType = 2;
	spark->life = spark->sLife = (GetRandomControl() & 0xF) + 64;
	spark->x = (GetRandomControl() & 0x1F) + x - 16;
	spark->y = (GetRandomControl() & 0x1F) + y - 16;
	spark->z = (GetRandomControl() & 0x1F) + z - 16;

	if (water)
	{
		spark->xVel = spark->yVel = GetRandomControl() & 0x3FF - 512;
		spark->zVel = (GetRandomControl() & 0x3FF) - 512;
		spark->friction = 68;
	}
	else
	{
		spark->xVel = 2 * (GetRandomControl() & 0x3FF) - 1024;
		spark->yVel = -512 - (GetRandomControl() & 0x3FF);
		spark->zVel = 2 * (GetRandomControl() & 0x3FF) - 1024;
		spark->friction = 85;
	}

	if (room->flags & ENV_FLAG_WIND)
		spark->flags = 272;
	else
		spark->flags = 16;

	spark->rotAng = GetRandomControl() & 0xFFF;
	if (GetRandomControl() & 1)
	{
		spark->rotAdd = -16 - (GetRandomControl() & 0xF);
	}
	else
	{
		spark->rotAdd = (GetRandomControl() & 0xF) + 16;
	}
	spark->maxYvel = 0;
	spark->gravity = 0;
	spark->sSize = spark->size = (GetRandomControl() & 0x1F) + 64;
	spark->dSize = 2 * (spark->sSize + 4);
	spark->mirror = mirror;
}

void GrenadeLauncherSpecialEffect1(int x, int y, int z, int flag1, int flag2)
{
	int dx = LaraItem->pos.xPos - x;
	int dz = LaraItem->pos.zPos - z;

	if (dx >= -ANGLE(90) && dx <= ANGLE(90) && dz >= -ANGLE(90) && dz <= ANGLE(90))
	{
		SPARKS* spark = &Sparks[GetFreeSpark()];

		spark->on = true;

		if (flag2 == 2)
		{
			spark->sR = spark->sG = (GetRandomControl() & 0x1F) + 48;
			spark->sB = (GetRandomControl() & 0x3F) - 64;
		}
		else
		{
			if (flag2 == -2)
			{
				spark->sR = 48;
				spark->sG = 255;
				spark->sB = (GetRandomControl() & 0x1F) + 48;

				spark->dR = 32;
				spark->dG = (GetRandomControl() & 0x3F) - 64;
				spark->dB = (GetRandomControl() & 0x3F) + -128;
			}
			else
			{
				spark->sR = 255;
				spark->sB = 48;
				spark->sG = (GetRandomControl() & 0x1F) + 48;
			}
		}

		if (flag2 != -2)
		{
			spark->dR = (GetRandomControl() & 0x3F) - 64;
			spark->dG = (GetRandomControl() & 0x3F) + -128;
			spark->dB = 32;
		}

		if (flag1 == -1)
		{
			if (flag2 == 2 || flag2 == -2 || flag2 == -1)
			{
				spark->fadeToBlack = 6;
				spark->colFadeSpeed = (GetRandomControl() & 3) + 5;
				spark->life = spark->sLife = (flag2 < -2 ? 0 : 8) + (GetRandomControl() & 3) + 16;
			}
			else
			{
				spark->fadeToBlack = 8;
				spark->colFadeSpeed = (GetRandomControl() & 3) + 20;
				spark->life = spark->sLife = (GetRandomControl() & 7) + 40;
			}
		}
		else
		{
			spark->fadeToBlack = 16;
			spark->colFadeSpeed = (GetRandomControl() & 3) + 8;
			spark->life = spark->sLife = (GetRandomControl() & 3) + 28;
		}

		spark->transType = 2;

		if (flag1 != -1)
		{
			spark->y = 0;
			spark->x = (GetRandomControl() & 0x1F) - 16;
			spark->z = (GetRandomControl() & 0x1F) - 16;

		//LABEL_POS_1:

			if (flag2 && flag2 != 1)
			{
				if (flag2 < -2)
				{
					spark->y = y;
					spark->x = (GetRandomControl() & 0xF) + x - 8;
					spark->z = (GetRandomControl() & 0xF) + z - 8;
				}
				else
				{
					spark->x = (GetRandomControl() & 0x3F) + x - 32;
					spark->y = y;
					spark->z = (GetRandomControl() & 0x3F) + z - 32;
				}
			}
			else
			{
				spark->y = y;
				spark->x = (GetRandomControl() & 0x1F) + x - 16;
				spark->z = (GetRandomControl() & 0x1F) + z - 16;
			}

			if (flag2 == 2)
			{
				spark->xVel = (GetRandomControl() & 0x1F) - 16;
				spark->yVel = -1024 - (GetRandomControl() & 0x1FF);
				spark->friction = 68;
				spark->zVel = (GetRandomControl() & 0x1F) - 16;
			}
			else
			{
				spark->xVel = GetRandomControl() - 128;
				spark->yVel = -16 - (GetRandomControl() & 0xF);
				spark->zVel = GetRandomControl() - 128;
				if (flag2 == 1)
				{
					spark->friction = 51;
				}
				else
				{
					spark->friction = 5;
				}
			}

			if (GetRandomControl() & 1)
			{
				if (flag1 == -1)
				{
					spark->gravity = -16 - (GetRandomControl() & 0x1F);
					spark->maxYvel = -16 - (GetRandomControl() & 7);
					spark->flags = 538;
				}
				else
				{
					spark->flags = 602;
					spark->fxObj = flag1;
					spark->gravity = -32 - (GetRandomControl() & 0x3F);
					spark->maxYvel = -24 - (GetRandomControl() & 7);
				}

				spark->rotAng = GetRandomControl() & 0xFFF;

				if (GetRandomControl() & 1)
				{
					spark->rotAdd = -16 - (GetRandomControl() & 0xF);
				}
				else
				{
					spark->rotAdd = (GetRandomControl() & 0xF) + 16;
				}
			}
			else
			{
				if (flag1 == -1)
				{
					spark->flags = 522;
					spark->gravity = -16 - (GetRandomControl() & 0x1F);
					spark->maxYvel = -16 - (GetRandomControl() & 7);
				}
				else
				{
					spark->flags = 586;
					spark->fxObj = flag1;
					spark->gravity = -32 - (GetRandomControl() & 0x3F);
					spark->maxYvel = -24 - (GetRandomControl() & 7);
				}
			}

			spark->scalar = 2;

			if (flag2)
			{
				if (flag2 == 1)
				{
					spark->sSize = spark->size = (GetRandomControl() & 0x1F) + 64;
				}
				else if (flag2 < 254)
				{
					spark->maxYvel = 0;
					spark->gravity = 0;
					spark->sSize = spark->size = (GetRandomControl() & 0x1F) + 32;
				}
				else
				{
					spark->sSize = spark->size = (GetRandomControl() & 0xF) + 48;
				}
			}
			else
			{
				spark->sSize = spark->size = (GetRandomControl() & 0x1F) + 128;
			}

			if (flag2 == 2)
			{
				spark->dSize = spark->size >> 2;
			}
			else
			{
				spark->dSize = spark->size >> 4;
				if (flag2 == 7)
				{
					spark->colFadeSpeed >>= 2;
					spark->fadeToBlack = spark->fadeToBlack >> 2;
					spark->life = spark->life >> 2;
					spark->sLife = spark->life >> 2;
				}
			}

			return;
		}

		/*if (flag2 && flag2 != 1)
		{
			if (flag2 < -2)
			{
				spark->y = y;
				spark->x = (GetRandomControl() & 0xF) + x - 8;
				spark->z = (GetRandomControl() & 0xF) + z - 8;
			}
			else
			{
				spark->x = (GetRandomControl() & 0x3F) + x - 32;
				spark->y = y;
				spark->z = (GetRandomControl() & 0x3F) + z - 32;
			}
		}
		else
		{
			spark->y = y;
			spark->x = (GetRandomControl() & 0x1F) + x - 16;
			spark->z = (GetRandomControl() & 0x1F) + z - 16;
		}

		goto LABEL_POS_1;*/
	}
}

void TriggerMetalSparks(int x, int y, int z, int xv, int yv, int zv, int additional)
{
	int dx = LaraItem->pos.xPos - x;
	int dz = LaraItem->pos.zPos - z;

	if (dx >= -16384 && dx <= 16384 && dz >= -16384 && dz <= 16384)
	{
		int r = rand();

		SPARKS* spark = &Sparks[GetFreeSpark()];

		spark->dG = (r & 0x7F) + 64;
		spark->dB = -64 - (r & 0x7F) + 64;
		spark->life = 10;
		spark->sLife = 10;
		spark->sR = -1;
		spark->sG = -1;
		spark->sB = -1;
		spark->dR = -1;
		spark->x = (r & 7) + x - 3;
		spark->on = 1;
		spark->colFadeSpeed = 3;
		spark->fadeToBlack = 5;
		spark->y = ((r >> 3) & 7) + y - 3;
		spark->transType = 2;
		spark->friction = 34;
		spark->scalar = 1;
		spark->z = ((r >> 6) & 7) + z - 3;
		spark->flags = 2;
		spark->xVel = (byte)(r >> 2) + xv - 128;
		spark->yVel = (byte)(r >> 4) + yv - 128;
		spark->zVel = (byte)(r >> 6) + zv - 128;
		spark->sSize = ((r >> 9) & 3) + 4;
		spark->size = ((r >> 9) & 3) + 4;
		spark->dSize = ((r >> 12) & 1) + 1;
		spark->maxYvel = 0;
		spark->gravity = 0;

		if (additional)
		{
			r = rand();
			spark = &Sparks[GetFreeSpark()];
			spark->on = 1;
			spark->sR = spark->dR >> 1;
			spark->sG = spark->dG >> 1;
			spark->fadeToBlack = 4;
			spark->transType = 2;
			spark->colFadeSpeed = (r & 3) + 8;
			spark->sB = spark->dB >> 1;
			spark->dR = 32;
			spark->dG = 32;
			spark->dB = 32;
			spark->yVel = yv;
			spark->life = ((r >> 3) & 7) + 13;
			spark->sLife = ((r >> 3) & 7) + 13;
			spark->friction = 4;
			spark->x = x + (xv >> 5);
			spark->y = y + (yv >> 5);
			spark->z = z + (zv >> 5);
			spark->xVel = (r & 0x3F) + xv - 32;
			spark->zVel = ((r >> 6) & 0x3F) + zv - 32;
			if (r & 1)
			{
				spark->flags = 538;
				spark->rotAng = r >> 3;
				if (r & 2)
				{
					spark->rotAdd = -16 - (r & 0xF);
				}
				else
				{
					spark->rotAdd = (r & 0xF) + 16;
				}
			}
			else
			{
				spark->flags = 522;
			}
			spark->gravity = -8 - (r >> 3 & 3);
			spark->scalar = 2;
			spark->maxYvel = -4 - (r >> 6 & 3);
			spark->sSize = ((r >> 8) & 0xF) + 24 >> 3;
			spark->size = ((r >> 8) & 0xF) + 24 >> 3;
			spark->dSize = ((r >> 8) & 0xF) + 24;
		}
	}
}

void KillEverything()//338AC(<), 33DAC(<) (F)
{
	KillEverythingFlag = 0;
}

void Inject_Effect2()
{
	INJECT(0x00431240, TriggerDynamicLight);
	INJECT(0x004820A0, TriggerGunSmoke);
	INJECT(0x0042E6A0, DetatchSpark);
	INJECT(0x0042E790, GetFreeSpark);
	INJECT(0x0042E8B0, UpdateSparks);
	INJECT(0x0042F060, TriggerRicochetSpark);
	INJECT(0x0042F460, TriggerCyborgSpark);
	INJECT(0x0042F610, TriggerExplosionSparks);
	INJECT(0x0042FA10, TriggerExplosionSmokeEnd);
	INJECT(0x0042FC20, TriggerExplosionSmoke);
	INJECT(0x0042FE20, TriggerFireFlame);
	INJECT(0x00430350, TriggerSuperJetFlame);
	INJECT(0x00430620, SetupSplash);
	INJECT(0x00430710, UpdateSplashes);
	INJECT(0x00430910, SetupRipple);
	INJECT(0x004309B0, TriggerUnderwaterBlood);
	INJECT(0x00430A40, TriggerWaterfallMist);
	INJECT(0x00430D90, TriggerDartSmoke);
	INJECT(0x00431030, KillAllCurrentItems);
	INJECT(0x00431240, TriggerDynamicLight);
	INJECT(0x00431530, ClearDynamicLights);
	INJECT(0x00432A30, WadeSplash);
	INJECT(0x00432900, Splash);
	INJECT(0x00430620, SetupSplash);
}