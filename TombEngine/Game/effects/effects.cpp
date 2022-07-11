#include "framework.h"
#include "Game/effects/effects.h"

#include "Game/animation.h"
#include "Game/collision/collide_room.h"
#include "Game/effects/lara_fx.h"
#include "Game/effects/drip.h"
#include "Game/effects/bubble.h"
#include "Game/effects/explosion.h"
#include "Game/effects/smoke.h"
#include "Game/effects/spark.h"
#include "Game/effects/tomb4fx.h"
#include "Game/effects/weather.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Objects/objectslist.h"
#include "Sound/sound.h"
#include "Specific/level.h"
#include "Specific/setup.h"
#include "Specific/prng.h"
#include "Specific/trmath.h"
#include "Flow/ScriptInterfaceFlowHandler.h"
#include "Renderer/Renderer11.h"

using TEN::Renderer::g_Renderer;
using namespace TEN::Effects::Explosion;
using namespace TEN::Effects::Lara;
using namespace TEN::Effects::Spark;
using namespace TEN::Effects::Environment;
using namespace TEN::Math::Random;

// New particle class
Particle Particles[MAX_PARTICLES];
ParticleDynamic ParticleDynamics[MAX_PARTICLE_DYNAMICS];

FX_INFO EffectList[NUM_EFFECTS];

int DeadlyBounds[6];
SPLASH_SETUP SplashSetup;
SPLASH_STRUCT Splashes[MAX_SPLASHES];
RIPPLE_STRUCT Ripples[MAX_RIPPLES];
int SplashCount = 0;

Vector3Int NodeVectors[MAX_NODE];
NODEOFFSET_INFO NodeOffsets[MAX_NODE] =
{
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

void DetatchSpark(int number, SpriteEnumFlag type)
{
	auto* sptr = &Particles[0];

	for (int lp = 0; lp < MAX_PARTICLES; lp++, sptr++)
	{
		if (sptr->on && (sptr->flags & type) && sptr->fxObj == number)
		{
			switch (type)
			{
				case SP_FX:
					if (sptr->flags & SP_DAMAGE)
						sptr->on = false;
					else
					{
						auto* fx = &EffectList[number];

						sptr->x += fx->pos.Position.x;
						sptr->y += fx->pos.Position.y;
						sptr->z += fx->pos.Position.z;
						sptr->flags &= ~SP_FX;
					}

					break;

				case SP_ITEM:
					if (sptr->flags & SP_DAMAGE)
						sptr->on = false;
					else
					{
						auto* item = &g_Level.Items[number];

						sptr->x += item->Pose.Position.x;
						sptr->y += item->Pose.Position.y;
						sptr->z += item->Pose.Position.z;
						sptr->flags &= ~SP_ITEM;
					}

					break;
			}
		}
	}
}

Particle* GetFreeParticle()
{
	int result = -1;

	// Get first free available spark

	for (int i = 0; i < MAX_PARTICLES; i++)
	{
		auto* particle = &Particles[i];

		if (!particle->on)
		{
			result = i;
			break;
		}
	}

	// No free sparks left, hijack existing one with less possible life

	int life = INT_MAX;
	if (result == -1)
	{
		for (int i = 0; i < MAX_PARTICLES; i++)
		{
			auto* particle = &Particles[i];

			if (particle->life < life && particle->dynamic == -1 && !(particle->flags & SP_EXPLOSION))
			{
				result = i;
				life = particle->life;
			}
		}
	}

	auto* spark = &Particles[result];

	spark->extras = 0;
	spark->dynamic = -1;
	spark->spriteIndex = Objects[ID_DEFAULT_SPRITES].meshIndex;
	spark->blendMode = BLEND_MODES::BLENDMODE_ADDITIVE;

	return spark;
}

void UpdateSparks()
{
	auto* bounds = GetBoundsAccurate(LaraItem);
	DeadlyBounds[0] = LaraItem->Pose.Position.x + bounds->X1;
	DeadlyBounds[1] = LaraItem->Pose.Position.x + bounds->X2;
	DeadlyBounds[2] = LaraItem->Pose.Position.y + bounds->Y1;
	DeadlyBounds[3] = LaraItem->Pose.Position.y + bounds->Y2;
	DeadlyBounds[4] = LaraItem->Pose.Position.z + bounds->Z1;
	DeadlyBounds[5] = LaraItem->Pose.Position.z + bounds->Z2;

	for (int i = 0; i < MAX_PARTICLES; i++)
	{
		auto* spark = &Particles[i];

		if (spark->on)
		{
			spark->life--;

			if (!spark->life)
			{
				if (spark->dynamic != -1)
					ParticleDynamics[spark->dynamic].On = false;

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
				spark->rotAng = (spark->rotAng + spark->rotAdd) & 0x0FFF;

			if (spark->sLife - spark->life == spark->extras >> 3 &&
				spark->extras & 7)
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
			float alpha = (spark->sLife - spark->life) / (float)spark->sLife;
			spark->size = Lerp(spark->sSize, spark->dSize, alpha);

			if ((spark->flags & SP_FIRE && !Lara.Burn) || 
				(spark->flags & SP_DAMAGE) || 
				(spark->flags & SP_POISON))
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

							if (spark->flags & SP_DAMAGE)
								DoDamage(LaraItem, 2);

							if (spark->flags & SP_POISON)
								Lara.PoisonPotency += 5;
						}
					}
				}
			}
		}
	}

	for (int i = 0; i < MAX_PARTICLES; i++)
	{
		auto* spark = &Particles[i];

		if (spark->on && spark->dynamic != -1)
		{
			auto* dynsp = &ParticleDynamics[spark->dynamic];
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

void TriggerRicochetSpark(GameVector* pos, short angle, int num, int unk)
{
	TriggerRicochetSpark(pos, angle, num);
}

void TriggerCyborgSpark(int x, int y, int z, short xv, short yv, short zv)
{
	int dx = LaraItem->Pose.Position.x - x;
	int dz = LaraItem->Pose.Position.z - z;

	if (dx >= -16384 && dx <= 16384 && dz >= -16384 && dz <= 16384)
	{
		auto* spark = GetFreeParticle();

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
		spark->blendMode = BLEND_MODES::BLENDMODE_ADDITIVE;
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
	int dx = LaraItem->Pose.Position.x - x;
	int dz = LaraItem->Pose.Position.z - z;

	if (dx >= -ANGLE(90.0f) && dx <= ANGLE(90.0f) && dz >= -ANGLE(90.0f) && dz <= ANGLE(90.0f))
	{
		auto* spark = GetFreeParticle();

		spark->sR = 128;
		spark->dR = 128;
		spark->dG = 128;
		spark->dB = 128;
		spark->on = 1;
		spark->life = 24;
		spark->sLife = 24;
		spark->sG = 64;
		spark->sB = 0;
		spark->colFadeSpeed = 8;
		spark->fadeToBlack = 12;
		spark->blendMode = BLEND_MODES::BLENDMODE_ADDITIVE;
		spark->x = x;
		spark->y = y;
		spark->z = z;
		spark->xVel = 0;
		spark->yVel = 0;
		spark->zVel = 0;
		spark->friction = 0;
		spark->flags = SP_UNDERWEXP | SP_DEF | SP_SCALE; 
		spark->spriteIndex = Objects[ID_DEFAULT_SPRITES].meshIndex + 13;
		spark->scalar = 3;
		spark->gravity = 0;
		spark->maxYvel = 0;

		int size = (GetRandomControl() & 7) + 63;
		spark->sSize = size >> 1;
		spark->size = size >> 1;
		spark->dSize = 2 * size;

		for (int i = 0; i < 8; i++)
		{
			Vector3Int pos;
			pos.x = (GetRandomControl() & 0x1FF) + x - 256;
			pos.y = (GetRandomControl() & 0x7F) + y - 64;
			pos.z = (GetRandomControl() & 0x1FF) + z - 256;
			CreateBubble(&pos, roomNumber, 6, 15, 0, 0, 0, 0);
		}
	}
}

void TriggerExplosionSmokeEnd(int x, int y, int z, int uw)
{
	auto* spark = GetFreeParticle();

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
		spark->blendMode = BLEND_MODES::BLENDMODE_ADDITIVE;
	else
		spark->blendMode = BLEND_MODES::BLENDMODE_SUBTRACTIVE;

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
		spark->friction = 6;
	
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
	int dx = LaraItem->Pose.Position.x - x;
	int dz = LaraItem->Pose.Position.z - z;
	
	if (dx >= -16384 && dx <= 16384 && dz >= -16384 && dz <= 16384)
	{
		auto* spark = GetFreeParticle();

		spark->sR = 144;
		spark->sG = 144;
		spark->sB = 144;
		spark->on = 1;
		spark->dR = 64;
		spark->dG = 64;
		spark->dB = 64;
		spark->colFadeSpeed = 2;
		spark->fadeToBlack = 8;
		spark->blendMode = BLEND_MODES::BLENDMODE_SUBTRACTIVE;
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
	int dx = LaraItem->pos.Position.x - x;
	int dz = LaraItem->pos.Position.z - z;

	if (dx < -16384 || dx > 16384 || dz < -16384 || dz > 16384)
		return;

	auto* spark = GetFreeParticle();

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

	spark->blendMode = BLEND_MODES::BLENDMODE_ADDITIVE;

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
			spark->sSize = spark->size = (GetRandomControl() & 0x1F) + 64;
		else if (type < 254)
		{
			spark->maxYvel = 0;
			spark->gravity = 0;
			spark->sSize = spark->size = (GetRandomControl() & 0x1F) + 32;
		}
		else
			spark->sSize = spark->size = (GetRandomControl() & 0xF) + 48;
	}
	else
		spark->sSize = spark->size = (GetRandomControl() & 0x1F) + 128;

	if (type == 2)
		spark->dSize = spark->size / 4;
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

void TriggerSuperJetFlame(ItemInfo* item, int yvel, int deadly)
{
	long dx = LaraItem->Pose.Position.x - item->Pose.Position.x;
	long dz = LaraItem->Pose.Position.z - item->Pose.Position.z;

	if (dx >= -16384 && dx <= 16384 && dz >= -16384 && dz <= 16384)
	{
		int size = (GetRandomControl() & 0x1FF) - yvel;
		auto* sptr = GetFreeParticle();

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
		sptr->blendMode = BLEND_MODES::BLENDMODE_ADDITIVE;
		sptr->life = sptr->sLife = (size >> 9) + (GetRandomControl() & 7) + 16;
		sptr->x = (GetRandomControl() & 0x1F) + item->Pose.Position.x - 16;
		sptr->y = (GetRandomControl() & 0x1F) + item->Pose.Position.y - 16;
		sptr->z = (GetRandomControl() & 0x1F) + item->Pose.Position.z - 16;
		sptr->friction = 51;
		sptr->maxYvel = 0;
		sptr->flags = SP_EXPDEF | SP_ROTATE | SP_DEF | SP_SCALE;

		if (deadly)
			sptr->flags = SP_EXPDEF | SP_ROTATE | SP_DEF | SP_SCALE | SP_FIRE;

		sptr->scalar = 2;
		sptr->dSize = (GetRandomControl() & 0xF) + (size >> 6) + 16;
		sptr->sSize = sptr->size = sptr->dSize / 2;

		if ((-(item->TriggerFlags & 0xFF) & 7) == 1)
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

		if (item->Pose.Orientation.y == 0)
			sptr->zVel = -(size - (size >> 2));
		else if (item->Pose.Orientation.y == ANGLE(90.0f))
			sptr->xVel = -(size - (size >> 2));
		else if (item->Pose.Orientation.y == -ANGLE(180.0f))
			sptr->zVel = size - (size >> 2);
		else
			sptr->xVel = size - (size >> 2);
	}
}

void SetupSplash(const SPLASH_SETUP* const setup, int room)
{
	constexpr size_t NUM_SPLASHES = 3;
	int numSplashesSetup = 0;
	float splashVelocity;

	for (int i = 0; i < MAX_SPLASHES; i++)
	{
		SPLASH_STRUCT& splash = Splashes[i];
		if (!splash.isActive)
		{
			if (numSplashesSetup == 0)
			{
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
			else
			{
				float thickness = GenerateFloat(64,128);
				splash.isActive = true;
				splash.x = setup->x;
				splash.y = setup->y;
				splash.z = setup->z;
				splash.isRipple = true;
				float vel;

				if (numSplashesSetup == 2)
					vel = (splashVelocity / 16) + GenerateFloat(2, 4);
				else
					vel = (splashVelocity / 7) + GenerateFloat(3, 7);
				
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
				splash.life = Lerp(48.0f, 70.0f, t);
				splash.spriteSequenceStart = 4; //Splash Texture
				splash.spriteSequenceEnd = 7; //Splash Texture
				splash.animationSpeed = fmin(0.6f,(1 / splash.outerRadVel)*2);

				numSplashesSetup++;
			}
			if (numSplashesSetup == NUM_SPLASHES)
				break;
			
			continue;
		}
	}

	TEN::Effects::Drip::SpawnSplashDrips(Vector3(setup->x, setup->y-15, setup->z),32,room);
	PHD_3DPOS soundPosition;
	soundPosition.Position.x = setup->x;
	soundPosition.Position.y = setup->y;
	soundPosition.Position.z = setup->z;
	soundPosition.Orientation.y = 0;
	soundPosition.Orientation.x = 0;
	soundPosition.Orientation.z = 0;

	SoundEffect(SFX_TR4_LARA_SPLASH, &soundPosition);
}

void UpdateSplashes()
{
	if (SplashCount)
		SplashCount--;

	for (int i = 0; i < MAX_SPLASHES; i++)
	{
		SPLASH_STRUCT& splash = Splashes[i];
		if (splash.isActive)
		{
			splash.life--;
			if (splash.life <= 0)
				splash.isActive = false;
			
			splash.heightSpeed += splash.heightVel;
			splash.height += splash.heightSpeed;

			if (splash.height < 0)
			{
				splash.height = 0;
				if (!splash.isRipple)
					splash.isActive = false;
			}
			
			splash.innerRad += splash.innerRadVel;
			splash.outerRad += splash.outerRadVel;
			splash.animationPhase += splash.animationSpeed;
			short sequenceLength = splash.spriteSequenceEnd - splash.spriteSequenceStart;

			if (splash.animationPhase > sequenceLength)
				splash.animationPhase = fmod(splash.animationPhase, sequenceLength);
		}
	}

	for (int i = 0; i < MAX_RIPPLES; i++)
	{
		auto* ripple = &Ripples[i];

		if (ripple->flags & RIPPLE_FLAG_ACTIVE)
		{
			if (ripple->size < 252)
			{
				if (ripple->flags & RIPPLE_FLAG_SHORT_INIT)
					ripple->size += 2;
				else
					ripple->size += 4;
			}

			if (!ripple->init)
			{
				ripple->life -= 3;
				if (ripple->life > 250)
					ripple->flags = 0;
			}
			else if (ripple->init < ripple->life)
			{
				if (ripple->flags & RIPPLE_FLAG_SHORT_INIT)
					ripple->init += 8;
				else
					ripple->init += 4;
				if (ripple->init >= ripple->life)
					ripple->init = 0;
			}
		}
	}
}

short DoBloodSplat(int x, int y, int z, short speed, short direction, short roomNumber)
{
	short probedRoomNumber = GetCollision(x, y, z, roomNumber).RoomNumber;
	if (TestEnvironment(ENV_FLAG_WATER, probedRoomNumber))
		TriggerUnderwaterBlood(x, y, z, speed);
	else
		TriggerBlood(x, y, z, direction >> 4, speed);

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
	int node = 1;

	for (int i = 0; i < LARA_MESHES::LM_HEAD; i++)
	{
		if (node & LaraItem->TouchBits)
		{
			Vector3Int vec;
			vec.x = (GetRandomControl() & 31) - 16;
			vec.y = (GetRandomControl() & 31) - 16;
			vec.z = (GetRandomControl() & 31) - 16;

			GetLaraJointPosition(&vec, (LARA_MESHES)i);
			DoBloodSplat(vec.x, vec.y, vec.z, (GetRandomControl() & 7) + 8, 2 * GetRandomControl(), LaraItem->RoomNumber);
		}

		node <<= 1;
	}
}

void TriggerUnderwaterBlood(int x, int y, int z, int size) 
{
	for (int i = 0; i < MAX_RIPPLES; i++)
	{
		auto* ripple = &Ripples[i];

		if (!(ripple->flags & RIPPLE_FLAG_ACTIVE))
		{
			ripple->flags = RIPPLE_FLAG_ACTIVE | RIPPLE_FLAG_LOW_OPACITY | RIPPLE_FLAG_BLOOD;

			ripple->life = 240 + (GetRandomControl() & 7);
			ripple->init = 1;
			ripple->size = size;
			ripple->x = x + (GetRandomControl() & 63) - 32;
			ripple->y = y;
			ripple->z = z + (GetRandomControl() & 63) - 32;
			
			return;
		}
	}
}

void Richochet(PHD_3DPOS* pos)
{
	short angle = mGetAngle(pos->Position.z, pos->Position.x, LaraItem->Pose.Position.z, LaraItem->Pose.Position.x);
	GameVector target;
	target.x = pos->Position.x;
	target.y = pos->Position.y;
	target.z = pos->Position.z;
	TriggerRicochetSpark(&target, angle / 16, 3, 0);
	SoundEffect(SFX_TR4_WEAPON_RICOCHET, pos);
}

void ControlWaterfallMist(short itemNumber) // ControlWaterfallMist
{
	auto* item = &g_Level.Items[itemNumber];

	int x = item->Pose.Position.x - phd_sin(item->Pose.Orientation.y + ANGLE(180.0f)) * CLICK(2) + phd_sin(item->Pose.Orientation.y - ANGLE(90.0f)) * CLICK(1);
	int z = item->Pose.Position.z - phd_cos(item->Pose.Orientation.y + ANGLE(180.0f)) * CLICK(2) + phd_cos(item->Pose.Orientation.y - ANGLE(90.0f)) * CLICK(1);

	TriggerWaterfallMist(x, item->Pose.Position.y, z, item->Pose.Orientation.y + ANGLE(180.0f));
	SoundEffect(SFX_TR4_WATERFALL_LOOP, &item->Pose);
}

void TriggerWaterfallMist(int x, int y, int z, int angle)
{
	int dh = 0;
	int ang1 = angle;
	int ang2 = angle;
	int dl;

	// CHECK THIS LOOP CONDITIONS
	for (ang1 = angle; ; ang1 *= 2)
	{
		auto* spark = GetFreeParticle();
		spark->on = 1;
		spark->sR = 64;
		spark->sG = 64;
		spark->sB = 64;
		spark->dR = 64;
		spark->dG = 64;
		spark->dB = 64;
		spark->colFadeSpeed = 1;
		spark->blendMode = BLEND_MODES::BLENDMODE_ADDITIVE;
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

	auto* spark = GetFreeParticle();
	spark->on = 1;
	spark->sR = 96;
	spark->sG = 96;
	spark->sB = 96;
	spark->dR = 96;
	spark->dG = 96;
	spark->dB = 96;
	spark->colFadeSpeed = 1;
	spark->blendMode = BLEND_MODES::BLENDMODE_ADDITIVE;
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
	spark->spriteIndex = Objects[ID_DEFAULT_SPRITES].meshIndex + 17;
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
	g_Renderer.AddDynamicLight(x, y, z, falloff, r, g, b);
}

void SetupRipple(int x, int y, int z, int size, int flags)
{
	for (int i = 0; i < MAX_RIPPLES; i++)
	{
		auto* ripple = &Ripples[i];

		if (!(ripple->flags & RIPPLE_FLAG_ACTIVE))
		{
			ripple->flags = RIPPLE_FLAG_ACTIVE | flags;
			ripple->life = 48 + (GetRandomControl() & 15);
			ripple->init = 1;
			ripple->size = size;
			ripple->x = x;
			ripple->y = y;
			ripple->z = z;
			if (flags & RIPPLE_FLAG_NO_RAND)
			{
				ripple->x += (GetRandomControl() & 127) - 64;
				ripple->z += (GetRandomControl() & 127) - 64;
			}

			return;
		}
	}
}

void WadeSplash(ItemInfo* item, int wh, int wd)
{
	short roomNumber = item->RoomNumber;
	GetFloor(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, &roomNumber);

	auto* room = &g_Level.Rooms[roomNumber];
	if (!TestEnvironment(ENV_FLAG_WATER, room))
		return;

	short roomNumber2 = item->RoomNumber;
	GetFloor(item->Pose.Position.x, room->y - 128, item->Pose.Position.z, &roomNumber2);

	auto* room2 = &g_Level.Rooms[roomNumber2];

	if (TestEnvironment(ENV_FLAG_WATER, room2))
		return;

	auto* frame = GetBestFrame(item);
	if (item->Pose.Position.y + frame->boundingBox.Y1 > wh)
		return;

	if (item->Pose.Position.y + frame->boundingBox.Y2 < wh)
		return;

	if (item->Animation.VerticalVelocity <= 0 || wd >= 474 || SplashCount != 0)
	{
		if (!(Wibble & 0xF))
		{
			if (!(GetRandomControl() & 0xF) || item->Animation.ActiveState != LS_IDLE)
			{
				if (item->Animation.ActiveState != LS_IDLE)
					SetupRipple(item->Pose.Position.x, wh - 1, item->Pose.Position.z, 112 + (GetRandomControl() & 15), RIPPLE_FLAG_SHORT_INIT | RIPPLE_FLAG_LOW_OPACITY);
				else
					SetupRipple(item->Pose.Position.x, wh - 1, item->Pose.Position.z, 112 + (GetRandomControl() & 15), RIPPLE_FLAG_LOW_OPACITY);	
			}
		}
	}
	else
	{
		SplashSetup.y = wh - 1;
		SplashSetup.x = item->Pose.Position.x;
		SplashSetup.z = item->Pose.Position.z;
		SplashSetup.innerRadius = 16;
		SplashSetup.splashPower = item->Animation.Velocity;
		SetupSplash(&SplashSetup, roomNumber);
		SplashCount = 16;
	}
}

void Splash(ItemInfo* item)
{
	short roomNumber = item->RoomNumber;
	GetFloor(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, &roomNumber);

	auto* room = &g_Level.Rooms[roomNumber];
	if (TestEnvironment(ENV_FLAG_WATER, room))
	{
		int waterHeight = GetWaterHeight(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, roomNumber);
		SplashSetup.y = waterHeight - 1;
		SplashSetup.x = item->Pose.Position.x;
		SplashSetup.z = item->Pose.Position.z;
		SplashSetup.splashPower = item->Animation.VerticalVelocity;
		SplashSetup.innerRadius = 64;
		SetupSplash(&SplashSetup, roomNumber);
	}
}

void TriggerRocketFlame(int x, int y, int z, int xv, int yv, int zv, int itemNumber)
{
	auto* sptr = GetFreeParticle();

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
	sptr->blendMode = BLEND_MODES::BLENDMODE_ADDITIVE;
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
	sptr->spriteIndex = Objects[ID_DEFAULT_SPRITES].meshIndex;
	sptr->scalar = 2;

	int size = (GetRandomControl() & 7) + 32;
	sptr->size = sptr->sSize = size;
}

void TriggerRocketFire(int x, int y, int z)
{
	auto* sptr = GetFreeParticle();

	sptr->on = true;

	sptr->sR = sptr->sG = (GetRandomControl() & 0x1F) + 48;
	sptr->sB = (GetRandomControl() & 0x3F) - 64;
	sptr->dR = (GetRandomControl() & 0x3F) - 64;
	sptr->dG = (GetRandomControl() & 0x3F) - 128;
	sptr->dB = 32;

	sptr->colFadeSpeed = 4 + (GetRandomControl() & 3);
	sptr->fadeToBlack = 12;
	sptr->sLife = sptr->life = (GetRandomControl() & 3) + 20;
	sptr->blendMode = BLEND_MODES::BLENDMODE_ADDITIVE;
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
	sptr->spriteIndex = Objects[ID_DEFAULT_SPRITES].meshIndex;
	sptr->scalar = 1;
	sptr->gravity = -(GetRandomControl() & 3) - 4;
	sptr->maxYvel = -(GetRandomControl() & 3) - 4;

	int size = (GetRandomControl() & 7) + 128;
	sptr->size = sptr->sSize = size >> 2;
	sptr->dSize = size;
}


void TriggerRocketSmoke(int x, int y, int z, int bodyPart)
{
	/*auto* sptr = GetFreeParticle();

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
	sptr->blendMode = BLEND_MODES::BLENDMODE_ADDITIVE;
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
	auto* room = &g_Level.Rooms[roomNumber];

	bool mirror = (roomNumber == g_GameFlow->GetLevel(CurrentLevel)->GetMirrorRoom());

	bool water = false;
	if (TestEnvironment(ENV_FLAG_WATER, room))
	{
		TriggerExplosionBubble(x, y, z, roomNumber);
		water = true;
	}

	auto* spark = &SmokeSparks[GetFreeSmokeSpark()];
	spark->on = true;
	spark->sShade = 0;
	spark->dShade = -128;
	spark->colFadeSpeed = 4;
	spark->fadeToBlack = 16;
	spark->blendMode = BLEND_MODES::BLENDMODE_ADDITIVE;
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

	if (TestEnvironment(ENV_FLAG_WIND, room))
		spark->flags = 272;
	else
		spark->flags = 16;

	spark->rotAng = GetRandomControl() & 0xFFF;
	if (GetRandomControl() & 1)
		spark->rotAdd = -16 - (GetRandomControl() & 0xF);
	else
		spark->rotAdd = (GetRandomControl() & 0xF) + 16;
	
	spark->maxYvel = 0;
	spark->gravity = 0;
	spark->sSize = spark->size = (GetRandomControl() & 0x1F) + 64;
	spark->dSize = 2 * (spark->sSize + 4);
	spark->mirror = mirror;
}

void TriggerFireFlame(int x, int y, int z, int fxObj, int type)
{
	int dx = LaraItem->Pose.Position.x - x;
	int dz = LaraItem->Pose.Position.z - z;

	if (dx >= -16384 && dx <= 16384 && dz >= -16384 && dz <= 16384)
	{
		auto* spark = GetFreeParticle();

		spark->on = true;

		if (type == 2)
		{
			spark->sR = spark->sG = (GetRandomControl() & 0x1F) + 48;
			spark->sB = (GetRandomControl() & 0x3F) - 64;
		}
		else
		{
			if (type == 254)
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

		spark->blendMode = BLEND_MODES::BLENDMODE_ADDITIVE;

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
				spark->friction = 51;
			else
				spark->friction = 5;
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
				spark->rotAdd = -16 - (GetRandomControl() & 0xF);
			else
				spark->rotAdd = (GetRandomControl() & 0xF) + 16;
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
				spark->sSize = spark->size = (GetRandomControl() & 0x1F) + 64;
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
			spark->sSize = spark->size = (GetRandomControl() & 0x1F) + 128;

		if (type == 2)
			spark->dSize = (spark->size / 4.0f);
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
	int dx = LaraItem->Pose.Position.x - x;
	int dz = LaraItem->Pose.Position.z - z;

	if (dx >= -16384 && dx <= 16384 && dz >= -16384 && dz <= 16384)
	{
		int r = rand();

		auto* spark = GetFreeParticle();

		spark->dG = (r & 0x7F) + 64;
		spark->dB = -64 - (r & 0x7F) + 64;
		spark->life = 10;
		spark->sLife = 10;
		spark->sR = 255;
		spark->sG = 255;
		spark->sB = 255;
		spark->dR = 255;
		spark->x = (r & 7) + x - 3;
		spark->on = 1;
		spark->colFadeSpeed = 3;
		spark->fadeToBlack = 5;
		spark->y = ((r >> 3) & 7) + y - 3;
		spark->blendMode = BLEND_MODES::BLENDMODE_ADDITIVE;
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
			spark = GetFreeParticle();
			spark->on = 1;
			spark->sR = spark->dR >> 1;
			spark->sG = spark->dG >> 1;
			spark->fadeToBlack = 4;
			spark->blendMode = BLEND_MODES::BLENDMODE_ADDITIVE;
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
					spark->rotAdd = -16 - (r & 0xF);
				else
					spark->rotAdd = (r & 0xF) + 16;
			}
			else
				spark->flags = 522;

			spark->gravity = -8 - (r >> 3 & 3);
			spark->scalar = 2;
			spark->maxYvel = -4 - (r >> 6 & 3);
			spark->sSize = ((r >> 8) & 0xF) + 24 >> 3;
			spark->size = ((r >> 8) & 0xF) + 24 >> 3;
			spark->dSize = ((r >> 8) & 0xF) + 24;
		}
	}
}
