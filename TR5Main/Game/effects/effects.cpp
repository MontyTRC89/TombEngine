#include "framework.h"
#include "animation.h"
#include "effects.h"
#include "lara.h"
#include "effects\tomb4fx.h"
#include "traps.h"
#include "Specific\trmath.h"
#include "Sound\sound.h"
#include "setup.h"
#include "level.h"
#include "objectslist.h"
#include "GameFlowScript.h"
#include "spark.h"
#include "explosion.h"
#include "effects\drip.h"
#include "effects\bubble.h"
#include "effects\weather.h"
#include "smoke.h"
#include "Specific\prng.h"
#include "Renderer11.h"
#include "Game/effects/lara_fx.h"
#include "items.h"

using TEN::Renderer::g_Renderer;
using namespace TEN::Effects::Explosion;
using namespace TEN::Effects::Lara;
using namespace TEN::Effects::Spark;
using namespace TEN::Effects::Environment;
using namespace TEN::Math::Random;

FX_INFO EffectList[NUM_EFFECTS];

int NextSpark;
int DeadlyBounds[6];
SPLASH_SETUP SplashSetup;
SPLASH_STRUCT Splashes[MAX_SPLASHES];
RIPPLE_STRUCT Ripples[MAX_RIPPLES];
SPARKS Sparks[MAX_SPARKS];
SP_DYNAMIC SparkDynamics[MAX_SPARKS_DYNAMICS];
int SmokeWeapon;
byte SmokeCountL;
byte SmokeCountR;
int SplashCount = 0;

PHD_VECTOR NodeVectors[MAX_NODE];
NODEOFFSET_INFO NodeOffsets[MAX_NODE] = {
	{ -16, 40, 160, -LM_LHAND, false }, // TR5 offset 0
	{ -16, -8, 160, 0, false }, // TR5 offset 1
	{ 0, 0, 256, 8, false }, // TR5 offset 2
	{ 0, 0, 256, 17, false }, // TR5 offset 3
	{ 0, 0, 256, 26, false }, // TR5 offset 4
	{ 0, 144, 40, 10, false }, // TR5 offset 5
	{ -40, 64, 360, 14, false }, // TR5 offset 6
	{ 0, -600, -40, 0, false }, // TR5 offset 7
	{ 0, 32, 16, 9, false }, // TR5 offset 8
	{ 0, 340, 0, 7, false }, // TR3 offset 0
	{ 0, 0, -96, 10, false }, // TR3 offset 1
	{ 13, 48, 320, 13, false }, // TR3 offset 2
	{ 0, -256, 0, 5, false }, // TR3 offset 3
	{ 0, 64, 0, 10, false }, // TR3 offset 4 // tony left
	{ 0, 64, 0, 13, false }, // TR3 offset 5 // tony right
	{ -32, -16, -192, 13, false }, // TR3 offset 6
	{ -64, 410, 0, 20, false }, // TR3 offset 7
	{ 64, 410, 0, 23, false }, // TR3 offset 8
	{ -160, -8, 16, 5, false }, // TR3 offset 9
	{ -160, -8, 16, 9, false }, // TR3 offset 10
	{ -160, -8, 16, 13, false }, // TR3 offset 11
	{ 0, 0, 0, 0, false }, // TR3 offset 12
	{ 0, 0, 0, 0, false }, // Empty
};

void DetatchSpark(int num, SpriteEnumFlag type)
{
	FX_INFO* fx;
	ITEM_INFO* item;
	SPARKS* sptr;
	int lp;
	
	sptr = &Sparks[0];
	for (lp = 0; lp < MAX_SPARKS; lp++, sptr++)
	{
		if (sptr->on && (sptr->flags & type) && sptr->fxObj == num)
		{
			switch (type)
			{
				case SP_FX:
					if (sptr->flags & SP_DAMAGE)
					{
						sptr->on = false;
					}
					else
					{
						fx = &EffectList[num];
						sptr->x += fx->pos.xPos;
						sptr->y += fx->pos.yPos;
						sptr->z += fx->pos.zPos;
						sptr->flags &= ~SP_FX;
					}
					break;
				case SP_ITEM:
					if (sptr->flags & SP_DAMAGE)
					{
						sptr->on = false;
					}
					else
					{
						item = &g_Level.Items[num];
						sptr->x += item->pos.xPos;
						sptr->y += item->pos.yPos;
						sptr->z += item->pos.zPos;
						sptr->flags &= ~SP_ITEM;
					}
					break;
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
	BOUNDING_BOX* bounds = GetBoundsAccurate(LaraItem);

	DeadlyBounds[0] = LaraItem->pos.xPos + bounds->X1;
	DeadlyBounds[1] = LaraItem->pos.xPos + bounds->X2;
	DeadlyBounds[2] = LaraItem->pos.yPos + bounds->Y1;
	DeadlyBounds[3] = LaraItem->pos.yPos + bounds->Y2;
	DeadlyBounds[4] = LaraItem->pos.zPos + bounds->Z1;
	DeadlyBounds[5] = LaraItem->pos.zPos + bounds->Z2;

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
				if (spark->flags & SP_UNDERWEXP)
					spark->dSize /= 4;
			}

			if (spark->flags & SP_ROTATE)
				spark->rotAng = (spark->rotAng + spark->rotAdd) & 0xFFF;

			if (spark->sLife - spark->life == spark->extras >> 3
				&& spark->extras & 7)
			{
				int unk;
				if (spark->flags & SP_UNDERWEXP)
					unk = 1;
				else
					unk = (spark->flags & SP_PLASMAEXP) >> 12;

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

			if (spark->flags & SP_WIND)
			{
				spark->x += Weather.Wind().x;
				spark->z += Weather.Wind().z;
			}

			int dl = (spark->sLife - spark->life << 16) / spark->sLife;
			int ds = dl * (spark->dSize - spark->sSize);
			//spark->size = spark->sSize + (ds & 0xFF);
			float alpha = (spark->sLife - spark->life) / (float)spark->sLife;
			spark->size = lerp(spark->sSize, spark->dSize, alpha);

			if (spark->flags & SP_FIRE && !Lara.burn || spark->flags & SP_DAMAGE)
			{
				ds = spark->size * (spark->scalar / 2.0);

				if (spark->x + ds > DeadlyBounds[0] && spark->x - ds < DeadlyBounds[1])
				{
					if (spark->y + ds > DeadlyBounds[2] && spark->y - ds < DeadlyBounds[3])
					{
						if (spark->z + ds > DeadlyBounds[4] && spark->z - ds < DeadlyBounds[5])
						{
							if (spark->flags & SP_FIRE)
								LaraBurn(LaraItem);
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
	TriggerRicochetSpark(pos, angle, num);
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
		spark->transType = TransTypeEnum::COLADD;
		spark->friction = 34;
		spark->scalar = 1;
		spark->x = (random & 7) + x - 3;
		spark->y = ((random >> 3) & 7) + y - 3;
		spark->z = ((random >> 6) & 7) + z - 3;
		spark->flags = SP_SCALE;
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
	TriggerExplosion(Vector3(x, y, z), 512, true, false, true, roomNumber);
}

void TriggerExplosionBubbles(int x, int y, int z, short roomNumber)
{
	int dx = LaraItem->pos.xPos - x;
	int dz = LaraItem->pos.zPos - z;

	if (dx >= -ANGLE(90) && dx <= ANGLE(90) && dz >= -ANGLE(90) && dz <= ANGLE(90))
	{
		SPARKS* spark = &Sparks[GetFreeSpark()];

		spark->sR = -128;
		spark->dR = -128;
		spark->dG = -128;
		spark->dB = -128;
		spark->on = 1;
		spark->life = 24;
		spark->sLife = 24;
		spark->sG = 64;
		spark->sB = 0;
		spark->colFadeSpeed = 8;
		spark->fadeToBlack = 12;
		spark->transType = TransTypeEnum::COLADD;
		spark->x = x;
		spark->y = y;
		spark->z = z;
		spark->xVel = 0;
		spark->yVel = 0;
		spark->zVel = 0;
		spark->friction = 0;
		spark->flags = SP_UNDERWEXP | SP_DEF | SP_SCALE; 
		spark->def = Objects[ID_DEFAULT_SPRITES].meshIndex + 13;
		spark->scalar = 3;
		spark->gravity = 0;
		spark->maxYvel = 0;
		int size = (GetRandomControl() & 7) + 63;
		spark->sSize = size >> 1;
		spark->size = size >> 1;
		spark->dSize = 2 * size;

		for (int i = 0; i < 8; i++)
		{
			PHD_VECTOR pos;
			pos.x = (GetRandomControl() & 0x1FF) + x - 256;
			pos.y = (GetRandomControl() & 0x7F) + y - 64;
			pos.z = (GetRandomControl() & 0x1FF) + z - 256;
			CreateBubble(&pos, roomNumber, 6, 15, 0, 0, 0, 0);
		}
	}
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
		spark->transType = TransTypeEnum::COLADD;
	else
		spark->transType = TransTypeEnum::COLSUB;
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
	spark->sSize = spark->dSize / 4;
	spark->size = spark->dSize / 4;
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
		spark->transType = TransTypeEnum::COLSUB;
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
		spark->sSize = spark->dSize / 4;
		spark->size = spark->dSize / 4;
	}
}

/*void TriggerFireFlame(int x, int y, int z, int fxObj, int type)
{
	int dx = LaraItem->pos.xPos - x;
	int dz = LaraItem->pos.zPos - z;

	if (dx < -16384 || dx > 16384 || dz < -16384 || dz > 16384)
		return;

	SPARKS* spark = &Sparks[GetFreeSpark()];
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
	spark->transType = TransTypeEnum::COLADD;
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
		spark->xVel = (byte)GetRandomControl() - 128;
		spark->yVel = -16 - ((byte)GetRandomControl() & 0xF);
		spark->zVel = (byte)GetRandomControl() - 128;
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
		spark->dSize = spark->size / 4;
	}
	else
	{
		spark->sSize = GenerateFloat(128, 156);
		spark->dSize = spark->sSize / 16;
		if (type == 7)
		{
			spark->colFadeSpeed >>= 2;
			spark->fadeToBlack >>= 2;
			spark->life = spark->life >> 2;
			spark->sLife = spark->life >> 2;
		}
	}
}*/

void TriggerSuperJetFlame(ITEM_INFO* item, int yvel, int deadly)
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
		sptr->transType = TransTypeEnum::COLADD;
		sptr->life = sptr->sLife = (size >> 9) + (GetRandomControl() & 7) + 16;
		sptr->x = (GetRandomControl() & 0x1F) + item->pos.xPos - 16;
		sptr->y = (GetRandomControl() & 0x1F) + item->pos.yPos - 16;
		sptr->z = (GetRandomControl() & 0x1F) + item->pos.zPos - 16;
		sptr->friction = 51;
		sptr->maxYvel = 0;
		sptr->flags = SP_EXPDEF | SP_ROTATE | SP_DEF | SP_SCALE;
		if (deadly)
			sptr->flags = SP_EXPDEF | SP_ROTATE | SP_DEF | SP_SCALE | SP_FIRE;
		sptr->scalar = 2;
		sptr->dSize = (GetRandomControl() & 0xF) + (size >> 6) + 16;
		sptr->sSize = sptr->size = sptr->dSize / 2;

		if ((-(item->triggerFlags & 0xFF) & 7) == 1)
		{
			sptr->gravity = -16 - (GetRandomControl() & 0x1F);
			sptr->xVel = (GetRandomControl() & 0xFF) - 128;
			sptr->yVel = -size;
			sptr->zVel = (GetRandomControl() & 0xFF) - 128;
			sptr->dSize += sptr->dSize / 4;
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

void SetupSplash(const SPLASH_SETUP* const setup,int room)
{
	constexpr size_t NUM_SPLASHES = 3;
	int numSplashesSetup = 0;
	float splashVelocity;
	for (int i = 0; i < MAX_SPLASHES; i++)
	{
		SPLASH_STRUCT& splash = Splashes[i];
		if (!splash.isActive)
		{
			if (numSplashesSetup == 0) {
				float splashPower =  fmin(256, setup->splashPower);
				splash.isActive = true;
				splash.x = setup->x;
				splash.y = setup->y;
				splash.z = setup->z;
				splash.life = 62;
				splash.isRipple = false;
				splash.innerRad = setup->innerRadius;
				splashVelocity = splashPower / 16;
				splash.innerRadVel = splashVelocity;
				splash.heightSpeed = splashPower * 1.2f;
				splash.height = 0;
				splash.heightVel = -16;
				splash.outerRad = setup->innerRadius / 3;
				splash.outerRadVel = splashVelocity*1.5f;
				splash.spriteSequenceStart = 8; //Splash Texture
				numSplashesSetup++;
			}
			else {
				float thickness = GenerateFloat(64,128);
				splash.isActive = true;
				splash.x = setup->x;
				splash.y = setup->y;
				splash.z = setup->z;
				splash.isRipple = true;
				float vel;
				if (numSplashesSetup == 2) {
					vel = (splashVelocity / 16) + GenerateFloat(2, 4);
				}
				else {
					vel = (splashVelocity / 7) + GenerateFloat(3, 7);
				}
				
				float innerRadius = 0;
				splash.innerRad = innerRadius;
				splash.innerRadVel = vel*1.3f;
				splash.outerRad = innerRadius+thickness;
				splash.outerRadVel = vel*2.3f;
				splash.heightSpeed = 128;
				splash.height = 0;
				splash.heightVel = -16;
				float t = vel / (splashVelocity / 2) + 16;
				t = fmax(0, fmin(t, 1));
				splash.life = lerp(48, 70, t);
				splash.spriteSequenceStart = 4; //Splash Texture
				splash.spriteSequenceEnd = 7; //Splash Texture
				splash.animationSpeed = fmin(0.6f,(1 / splash.outerRadVel)*2);

				numSplashesSetup++;
			}
			if (numSplashesSetup == NUM_SPLASHES) {
				break;
			}
			continue;
		}
	}
	TEN::Effects::Drip::SpawnSplashDrips(Vector3(setup->x, setup->y-15, setup->z),32,room);
	PHD_3DPOS soundPosition;
	soundPosition.xPos = setup->x;
	soundPosition.yPos = setup->y;
	soundPosition.zPos = setup->z;
	soundPosition.yRot = 0;
	soundPosition.xRot = 0;
	soundPosition.zRot = 0;

	SoundEffect(SFX_TR4_LARA_SPLASH, &soundPosition, 0);
}

void UpdateSplashes()
{
	for (int i = 0; i < MAX_SPLASHES; i++)
	{
		SPLASH_STRUCT& splash = Splashes[i];
		if (splash.isActive) {
			splash.life--;
			if (splash.life <= 0) {
				splash.isActive = false;
			}
			splash.heightSpeed += splash.heightVel;
			splash.height += splash.heightSpeed;
			if (splash.height < 0) {
				splash.height = 0;
				if (!splash.isRipple) {
					splash.isActive = false;
				}
			}
			splash.innerRad += splash.innerRadVel;
			splash.outerRad += splash.outerRadVel;
			splash.animationPhase += splash.animationSpeed;
			short sequenceLength = splash.spriteSequenceEnd - splash.spriteSequenceStart;
			if (splash.animationPhase > sequenceLength) {
				splash.animationPhase = fmod(splash.animationPhase, sequenceLength);
			}
		}
		
		
	}

	for (int i = 0; i < MAX_RIPPLES; i++)
	{
		RIPPLE_STRUCT* ripple = &Ripples[i];

		if (ripple->active)
		{
			if (ripple->lifeTime > ripple->life) {
				ripple->active = false;
				continue;
			}
			//normalized Lifetime
			float n = ripple->lifeTime / ripple->life;
			n = fmin(n, 1.0f);
			n = fmax(0.0f, n);
			constexpr float peakTime = 0.2f;
			constexpr float expIn = 1.5f;
			constexpr float expOut = 2.0f;
			if (n <= peakTime) {
				//we ascend our color
				float alpha = pow((n / peakTime), expIn);
				ripple->currentColor = Vector4::Lerp(Vector4::Zero, ripple->initialColor, alpha);
			}
			else {
				//we descend
				float alphaTerm = 1.0f - ((n - peakTime) / 1 - peakTime);
				float alpha = pow(alphaTerm, expOut);
				//float alpha = alphaTerm;
				ripple->currentColor = Vector4::Lerp(Vector4::Zero, ripple->initialColor, alpha);
				
			}
			ripple->size += ripple->sizeRate;
			ripple->lifeTime += ripple->lifeRate;
		}
	}
}

void SetupRipple(int x, int y, int z, float size, char flags, unsigned int spriteID, float rotation)
{
	RIPPLE_STRUCT* ripple;
	int i;

	for (i = 0; i < MAX_RIPPLES; i++)
	{
		ripple = &Ripples[i];
		if (!(ripple->active)) {
			ripple->active = true;
			ripple->size = size;
			ripple->lifeTime = 0;
			ripple->SpriteID = spriteID;
			if (flags & RIPPLE_FLAG_SHORT_LIFE) {
				ripple->life = (rand() & 16) + 16;

			}
			else {
				ripple->life = (rand() & 16) + 48;
			}
			ripple->worldPos = { (float)x,(float)y,(float)z };
			ripple->currentColor = Vector4(0, 0, 0, 0);
			ripple->rotation = rotation;

			if (flags & RIPPLE_FLAG_BLOOD ) {
				ripple->initialColor = Vector4(1, 0, 0, 1);
				ripple->lifeRate = 0.9f;
				ripple->sizeRate = 8.0f;
				ripple->isBillboard = true;
			}
			else {
				ripple->initialColor = Vector4(0.5, 0.5, 0.5, 1);
				ripple->lifeRate = 1.0f;
				ripple->sizeRate = 4.0f;
				ripple->isBillboard = false;
			}
			if (flags & RIPPLE_FLAG_LOW_OPACITY)
				ripple->initialColor *= 0.6f;
			if (flags & RIPPLE_FLAG_RAND_POS)
			{
				ripple->worldPos.x += GenerateFloat(-32, 32);
				ripple->worldPos.z += GenerateFloat(-32, 32);
			}
			if (flags & RIPPLE_FLAG_RAND_ROT)
			{
				ripple->rotation += GenerateFloat(-PI, PI);
			}
			break;
		}
	}
}

short DoBloodSplat(int x, int y, int z, short a4, short a5, short roomNumber)
{
	short roomNum = roomNumber;
	GetFloor(x, y, z, &roomNum);
	if (g_Level.Rooms[roomNum].flags & ENV_FLAG_WATER)
		TriggerUnderwaterBlood(x, y, z, a4);
	else
		TriggerBlood(x, y, z, a5 >> 4, a4);
	return 0;
}

void DoLotsOfBlood(int x, int y, int z, int speed, short direction, short roomNumber, int count)
{
	for (int i = 0; i < count; i++)
	{
		DoBloodSplat(x + 256 - (GetRandomControl() * 512 / 0x8000),
			y + 256 - (GetRandomControl() * 512 / 0x8000),
			z + 256 - (GetRandomControl() * 512 / 0x8000),
			speed, direction, roomNumber);
	}
}

void TriggerLaraBlood()
{
	int i;
	int node = 1;

	for (i = 0; i < LARA_MESHES::LM_HEAD; i++)
	{
		if (node & LaraItem->touchBits)
		{
			PHD_VECTOR vec;
			vec.x = (GetRandomControl() & 31) - 16;
			vec.y = (GetRandomControl() & 31) - 16;
			vec.z = (GetRandomControl() & 31) - 16;

			GetLaraJointPosition(&vec, (LARA_MESHES)i);
			DoBloodSplat(vec.x, vec.y, vec.z, (GetRandomControl() & 7) + 8, 2 * GetRandomControl(), LaraItem->roomNumber);
		}

		node <<= 1;
	}
}

void TriggerUnderwaterBlood(int x, int y, int z, int sizeme) 
{
	SetupRipple(x, y, z, sizeme, RIPPLE_FLAG_BLOOD | RIPPLE_FLAG_RAND_POS | RIPPLE_FLAG_RAND_ROT, Objects[ID_DEFAULT_SPRITES].meshIndex + SPR_BLOOD);
}

void Richochet(PHD_3DPOS* pos)
{
	short angle = mGetAngle(pos->zPos, pos->xPos, LaraItem->pos.zPos, LaraItem->pos.xPos);
	GAME_VECTOR target;
	target.x = pos->xPos;
	target.y = pos->yPos;
	target.z = pos->zPos;
	TriggerRicochetSpark(&target, angle / 16, 3, 0);
	SoundEffect(SFX_TR4_LARA_RICOCHET, pos, 0);
}

void ControlWaterfallMist(short itemNumber) // ControlWaterfallMist
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];
	int x, z;

	x = item->pos.xPos - phd_sin(item->pos.yRot + ANGLE(180)) * 512 + phd_sin(item->pos.yRot - ANGLE(90)) * 256;
	z = item->pos.zPos - phd_cos(item->pos.yRot + ANGLE(180)) * 512 + phd_cos(item->pos.yRot - ANGLE(90)) * 256;

	TriggerWaterfallMist(x, item->pos.yPos, z, item->pos.yRot + ANGLE(180));
	SoundEffect(SFX_TR4_WATERFALL_LOOP, &item->pos, 0);
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
		spark->transType = TransTypeEnum::COLADD;
		spark->life = spark->sLife = (GetRandomControl() & 3) + 6;
		spark->fadeToBlack = spark->life - 4;
		dl = ((dh + (GlobalCounter << 6)) % 1536) + (GetRandomControl() & 0x3F) - 32;
		spark->x = dl * phd_sin(ang1) + (GetRandomControl() & 0xF) + x - 8;
		spark->y = (GetRandomControl() & 0xF) + y - 8;
		spark->z = dl * phd_cos(ang1) + (GetRandomControl() & 0xF) + z - 8;
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
	spark->transType = TransTypeEnum::COLADD;
	spark->life = spark->sLife = (GetRandomControl() & 3) + 6;
	spark->fadeToBlack = spark->life - 1;
	dl = GetRandomControl() % 1408 + 64;
	spark->x = dl * phd_sin(ang1) + (GetRandomControl() & 0x1F) + x - 16;
	spark->y = (GetRandomControl() & 0xF) + y - 8;
	spark->xVel = 0;
	spark->zVel = 0;
	spark->z = dl * phd_cos(ang1) + (GetRandomControl() & 0x1F) + z - 16;
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

void KillAllCurrentItems(short itemNumber)
{
	// TODO: Reimplement this functionality
}

void TriggerDynamicLight(int x, int y, int z, short falloff, byte r, byte g, byte b)
{
	g_Renderer.addDynamicLight(x, y, z, falloff, r, g, b);
}

void WadeSplash(ITEM_INFO* item, int wh, int wd)
{
	short roomNumber = item->roomNumber;
	GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);

	ROOM_INFO* room = &g_Level.Rooms[roomNumber];
	if (!(room->flags & ENV_FLAG_WATER))
		return;

	short roomNumber2 = item->roomNumber;
	GetFloor(item->pos.xPos, room->y - 128, item->pos.zPos, &roomNumber2);

	ROOM_INFO* room2 = &g_Level.Rooms[roomNumber2];

	if (room2->flags & ENV_FLAG_WATER)
		return;

	ANIM_FRAME* frame = GetBestFrame(item);
	if (item->pos.yPos + frame->boundingBox.Y1 > wh)
		return;

	if (item->pos.yPos + frame->boundingBox.Y2 < wh)
		return;

	if (item->fallspeed <= 0 || wd >= 474 || SplashCount != 0)
	{
		if (!(Wibble & 0xF))
		{
			if (!(GetRandomControl() & 0xF) || item->currentAnimState != LS_STOP)
			{
				if (item->currentAnimState == LS_STOP)
				{
					SetupRipple(item->pos.xPos, wh - 1, item->pos.zPos, (GetRandomControl() & 0xF) + 112, RIPPLE_FLAG_RAND_ROT | RIPPLE_FLAG_RAND_POS, Objects[ID_DEFAULT_SPRITES].meshIndex + SPR_RIPPLES);
				}
				else
				{
					SetupRipple(item->pos.xPos, wh - 1, item->pos.zPos, (GetRandomControl() & 0xF) + 112, RIPPLE_FLAG_RAND_ROT | RIPPLE_FLAG_RAND_POS, Objects[ID_DEFAULT_SPRITES].meshIndex + SPR_RIPPLES);
				}
			}
		}
	}
	else
	{
		SplashSetup.y = wh - 1;
		SplashSetup.x = item->pos.xPos;
		SplashSetup.z = item->pos.zPos;
		SplashSetup.innerRadius = 16;
		SplashSetup.splashPower = item->speed;
		SetupSplash(&SplashSetup, roomNumber);
		SplashCount = 16;
	}
}

void Splash(ITEM_INFO* item)
{
	short roomNumber = item->roomNumber;
	GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);

	ROOM_INFO* room = &g_Level.Rooms[roomNumber];
	if (room->flags & ENV_FLAG_WATER)
	{
		int wh = GetWaterHeight(item->pos.xPos, item->pos.yPos, item->pos.zPos, roomNumber);
		SplashSetup.y = wh - 1;
		SplashSetup.x = item->pos.xPos;
		SplashSetup.z = item->pos.zPos;
		SplashSetup.splashPower = item->fallspeed;
		SplashSetup.innerRadius = 64;
		SetupSplash(&SplashSetup,roomNumber);
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
	sptr->transType = TransTypeEnum::COLADD;
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

void TriggerRocketFire(int x, int y, int z)
{
	SPARKS* sptr = &Sparks[GetFreeSpark()];

	sptr->on = true;

	sptr->sR = sptr->sG = (GetRandomControl() & 0x1F) + 48;
	sptr->sB = (GetRandomControl() & 0x3F) - 64;
	sptr->dR = (GetRandomControl() & 0x3F) - 64;
	sptr->dG = (GetRandomControl() & 0x3F) - 128;
	sptr->dB = 32;

	sptr->colFadeSpeed = 4 + (GetRandomControl() & 3);
	sptr->fadeToBlack = 12;
	sptr->sLife = sptr->life = (GetRandomControl() & 3) + 20;
	sptr->transType = TransTypeEnum::COLADD;
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
	sptr->scalar = 1;
	sptr->gravity = -(GetRandomControl() & 3) - 4;
	sptr->maxYvel = -(GetRandomControl() & 3) - 4;

	int size = (GetRandomControl() & 7) + 128;
	sptr->size = sptr->sSize = size >> 2;
	sptr->dSize = size;
}


void TriggerRocketSmoke(int x, int y, int z, int bodyPart)
{
	/*SPARKS* sptr = &Sparks[GetFreeSpark()];

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
	sptr->transType = TransTypeEnum::COLADD;
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
	sptr->scalar = 15;
	sptr->gravity = -(GetRandomControl() & 3) - 4;
	sptr->maxYvel = -(GetRandomControl() & 3) - 4;

	int size = (GetRandomControl() & 7) + 32;
	sptr->size = sptr->sSize = size >> 2;*/
	TEN::Effects::Smoke::TriggerRocketSmoke(x, y, z, 0);
}

void TriggerFlashSmoke(int x, int y, int z, short roomNumber)
{
	ROOM_INFO* room = &g_Level.Rooms[roomNumber];

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
	spark->transType = TransTypeEnum::COLADD;
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

void TriggerFireFlame(int x, int y, int z, int fxObj, int type)
{
	int dx = LaraItem->pos.xPos - x;
	int dz = LaraItem->pos.zPos - z;

	if (dx >= -16384 && dx <= 16384 && dz >= -16384 && dz <= 16384)
	{
		SPARKS* spark = &Sparks[GetFreeSpark()];

		spark->on = true;

		if (type == 2)
		{
			spark->sR = spark->sG = (GetRandomControl() & 0x1F) + 48;
			spark->sB = (GetRandomControl() & 0x3F) - 64;
		}
		else
		{
			if (type == -2)
			{
				spark->sR = 48;
				spark->sG = 255;
				spark->sB = (GetRandomControl() & 0x1F) + 48;

				spark->dR = 32;
				spark->dG = (GetRandomControl() & 0x3F) - 64;
				spark->dB = (GetRandomControl() & 0x3F) - 128;
			}
			else
			{
				spark->sR = 255;
				spark->sB = 48;
				spark->sG = (GetRandomControl() & 0x1F) + 48;
			}
		}

		if (type != -2)
		{
			spark->dR = (GetRandomControl() & 0x3F) - 64;
			spark->dG = (GetRandomControl() & 0x3F) + -128;
			spark->dB = 32;
		}

		if (fxObj == -1)
		{
			if (type == 2 || type == -2 || type == -1)
			{
				spark->fadeToBlack = 6;
				spark->colFadeSpeed = (GetRandomControl() & 3) + 5;
				spark->life = spark->sLife = (type < -2 ? 0 : 8) + (GetRandomControl() & 3) + 16;
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
			spark->life = spark->sLife = (GetRandomControl() & 3) + 18;
		}

		spark->transType = TransTypeEnum::COLADD;

		if (fxObj != -1)
		{
			spark->x = (GetRandomControl() & 0x1F) - 16;
			spark->y = 0;
			spark->z = (GetRandomControl() & 0x1F) - 16;
		}
		else if (type && type != 1)
		{
			if (type < 254)
			{
				spark->x = (GetRandomControl() & 0xF) + x - 8;
				spark->y = y;
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
			spark->x = (GetRandomControl() & 0x1F) + x - 16;
			spark->y = y;
			spark->z = (GetRandomControl() & 0x1F) + z - 16;
		}

		if (type == 2)
		{
			spark->xVel = (GetRandomControl() & 0x1F) - 16;
			spark->yVel = -1024 - (GetRandomControl() & 0x1FF);
			spark->zVel = (GetRandomControl() & 0x1F) - 16;
			spark->friction = 68;
		}
		else
		{
			spark->xVel = (GetRandomControl() & 0xFF) - 128;
			spark->yVel = -16 - (GetRandomControl() & 0xF);
			spark->zVel = (GetRandomControl() & 0xFF) - 128;

			if (type == 1)
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
			if (fxObj == -1)
			{
				spark->gravity = -16 - (GetRandomControl() & 0x1F);
				spark->maxYvel = -16 - (GetRandomControl() & 7);
				spark->flags = 538;
			}
			else
			{
				spark->flags = 602;
				spark->fxObj = fxObj;
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
			if (fxObj == -1)
			{
				spark->flags = SP_EXPDEF | SP_DEF | SP_SCALE;
				spark->gravity = -16 - (GetRandomControl() & 0x1F);
				spark->maxYvel = -16 - (GetRandomControl() & 7);
			}
			else
			{
				spark->flags = SP_EXPDEF | SP_DEF | SP_SCALE | SP_FX;
				spark->fxObj = fxObj;
				spark->gravity = -32 - (GetRandomControl() & 0x3F);
				spark->maxYvel = -24 - (GetRandomControl() & 7);
			}
		}

		spark->scalar = 2;

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
				spark->dSize = spark->size / 16;
				if (type == 7)
				{
					spark->colFadeSpeed >>= 2;
					spark->fadeToBlack = spark->fadeToBlack >> 2;
					spark->life = spark->life >> 2;
					spark->sLife = spark->life >> 2;
				}
				spark->sSize = spark->size = (GetRandomControl() & 0xF) + 48;
			}
		}
		else
		{
			spark->sSize = spark->size = (GetRandomControl() & 0x1F) + 128;
		}

		if (type == 2)
		{
			spark->dSize = (spark->size / 4.0f);
		}
		else
		{
			spark->dSize = (spark->size / 16.0f);

			if (type == 7)
			{
				spark->colFadeSpeed >>= 2;
				spark->fadeToBlack = spark->fadeToBlack >> 2;
				spark->life = spark->life >> 2;
				spark->sLife = spark->life >> 2;
			}
		}
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
		spark->transType = TransTypeEnum::COLADD;
		spark->friction = 34;
		spark->scalar = 1;
		spark->z = ((r >> 6) & 7) + z - 3;
		spark->flags = 2;
		spark->xVel = (byte)(r >> 2) + xv - 128;
		spark->yVel = (byte)(r >> 4) + yv - 128;
		spark->zVel = (byte)(r >> 6) + zv - 128;
		spark->sSize = ((r >> 9) & 3) + 4;
		spark->size = ((r >> 9) & 3) + 4;
		spark->dSize = ((r >> 9) & 1) + 1;
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
			spark->transType = TransTypeEnum::COLADD;
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
