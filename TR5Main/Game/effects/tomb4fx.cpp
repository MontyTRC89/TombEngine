#include "framework.h"
#include "effects/tomb4fx.h"
#include "lara.h"
#include "effects/effects.h"
#include "animation.h"
#include "setup.h"
#include "level.h"
#include "Sound/sound.h"
#include "effects/bubble.h"
#include "Specific/trmath.h"
#include "GameFlowScript.h"
#include "smoke.h"
#include "drip.h"
#include "effects.h"
#include "Renderer11.h"
#include "effects/weather.h"
#include "items.h"
#include "Game/floordata.h"
#include "Specific/prng.h"

using std::vector;
using TEN::Renderer::g_Renderer;
using namespace TEN::Floordata;
using namespace TEN::Math::Random;
using namespace TEN::Effects::Environment;

char LaserSightActive = 0;
char LaserSightCol = 0;
int NextGunshell = 0;

int LaserSightX;
int LaserSightY;
int LaserSightZ;

int NextFireSpark = 1;
int NextSmokeSpark = 0;
int NextBubble = 0;
int NextDrip = 0;
int NextBlood = 0;
int NextGunShell = 0;

GUNFLASH_STRUCT Gunflashes[MAX_GUNFLASH]; 
FIRE_SPARKS FireSparks[MAX_SPARKS_FIRE]; 
SMOKE_SPARKS SmokeSparks[MAX_SPARKS_SMOKE]; 
GUNSHELL_STRUCT Gunshells[MAX_GUNSHELL]; 
BLOOD_STRUCT Blood[MAX_SPARKS_BLOOD]; 
DRIP_STRUCT Drips[MAX_DRIPS]; 
SHOCKWAVE_STRUCT ShockWaves[MAX_SHOCKWAVE]; 
FIRE_LIST Fires[MAX_FIRE_LIST];

int GetFreeFireSpark()
{
	int sparkNum = NextFireSpark;
	int minIndex = 0;
	int minLife = 4095;
	int i = 0;

	FIRE_SPARKS* spark = &FireSparks[NextFireSpark];
	while (spark->on)
	{
		if (spark->life < minLife)
		{
			minIndex = sparkNum;
			minLife = spark->life;
		}
		if (sparkNum == MAX_SPARKS_FIRE - 1)
		{
			spark = &FireSparks[1];
			sparkNum = 1;
		}
		else
		{
			sparkNum++;
			spark++;
		}

		if (++i >= MAX_SPARKS_FIRE)
		{
			NextFireSpark = minIndex + 1;
			if (NextFireSpark >= MAX_SPARKS_FIRE)
				NextFireSpark = 1;
			return minIndex;
		}
	}

	NextFireSpark = sparkNum + 1;
	if (sparkNum + 1 >= MAX_SPARKS_FIRE)
		NextFireSpark = 1;

	return sparkNum;
}

void TriggerGlobalStaticFlame()
{
	FIRE_SPARKS* spark = &FireSparks[0];

	spark->on = true;
	spark->dR = spark->sR = (GetRandomControl() & 0x3F) - 64;
	spark->dB = 64;
	spark->sB = 64;
	spark->dG = (GetRandomControl() & 0x3F) + 96;
	spark->sG = (GetRandomControl() & 0x3F) + 96;
	spark->colFadeSpeed = 1;
	spark->fadeToBlack = 0;
	spark->life = 8;
	spark->sLife = 8;
	spark->y = 0;
	spark->x = (GetRandomControl() & 7) - 4;
	spark->maxYvel = 0;
	spark->gravity = 0;
	spark->z = (GetRandomControl() & 7) - 4;
	spark->friction = 0;
	spark->xVel = 0;
	spark->yVel = 0;
	spark->zVel = 0;
	spark->flags = SP_NONE;
	spark->dSize = spark->sSize = spark->size = (GetRandomControl() & 0x1F) + -128;
}

void TriggerGlobalFireSmoke()
{
	FIRE_SPARKS* spark = &FireSparks[GetFreeFireSpark()];

	spark->on = 1;
	spark->sR = 0;
	spark->sG = 0;
	spark->sB = 0;
	spark->dR = 32;
	spark->dG = 32;
	spark->dB = 32;
	spark->fadeToBlack = 16;
	spark->colFadeSpeed = (GetRandomControl() & 7) + 32;
	spark->life = spark->sLife = (GetRandomControl() & 0xF) + 57;
	spark->x = (GetRandomControl() & 0xF) - 8;
	spark->y = -256 - (GetRandomControl() & 0x7F);
	spark->z = (GetRandomControl() & 0xF) - 8;
	spark->xVel = (GetRandomControl() & 0xFF) - 128;
	spark->yVel = -16 - (GetRandomControl() & 0xF);
	spark->zVel = (GetRandomControl() & 0xFF) - 128;
	spark->friction = 4;

	if (GetRandomControl() & 1)
	{
		spark->flags = 16;
		spark->rotAng = GetRandomControl() & 0xFFF;
		if (GetRandomControl() & 1)
			spark->rotAdd = -16 - (GetRandomControl() & 0xF);
		else
			spark->rotAdd = (GetRandomControl() & 0xF) + 16;
	}
	else
	{
		spark->flags = SP_NONE;
	}

	spark->gravity = -16 - (GetRandomControl() & 0xF);
	spark->maxYvel = -8 - (GetRandomControl() & 7);
	spark->dSize = spark->sSize = spark->size = (GetRandomControl() & 0x7F) + 128;
}

void TriggerGlobalFireFlame()
{
	FIRE_SPARKS* spark = &FireSparks[GetFreeFireSpark()];

	spark->on = true;
	spark->sR = 255;
	spark->sB = 48;
	spark->sG = (GetRandomControl() & 0x1F) + 48;
	spark->dR = (GetRandomControl() & 0x3F) - 64;
	spark->dB = 32;
	spark->dG = (GetRandomControl() & 0x3F) + -128;
	spark->fadeToBlack = 8;
	spark->colFadeSpeed = (GetRandomControl() & 3) + 8;
	spark->life = spark->sLife = (GetRandomControl() & 7) + 32;
	spark->y = 0;
	spark->x = 4 * (GetRandomControl() & 0x1F) - 64;
	spark->z = 4 * (GetRandomControl() & 0x1F) - 64;
	spark->xVel = 2 * (GetRandomControl() & 0xFF) - 256;
	spark->yVel = -16 - (GetRandomControl() & 0xF);
	spark->zVel = 2 * (GetRandomControl() & 0xFF) - 256;
	spark->friction = 5;
	spark->gravity = -32 - (GetRandomControl() & 0x1F);
	spark->maxYvel = -16 - (GetRandomControl() & 7);

	if (GetRandomControl() & 1)
	{
		spark->flags = 16;
		spark->rotAng = GetRandomControl() & 0xFFF;
		if (GetRandomControl() & 1)
			spark->rotAdd = -16 - (GetRandomControl() & 0xF);
		else
			spark->rotAdd = (GetRandomControl() & 0xF) + 16;
	}
	else
	{
		spark->flags = SP_NONE;
	}

	spark->sSize = spark->size = (GetRandomControl() & 0x1F) + 128;
	spark->dSize = spark->size;
}

void keep_those_fires_burning()
{
	TriggerGlobalStaticFlame();
	if (!(Wibble & 0xF))
	{
		TriggerGlobalFireFlame();
		if (!(Wibble & 0x1F))
			TriggerGlobalFireSmoke();
	}
}

void AddFire(int x, int y, int z, char size, short roomNum, short on)
{
	FIRE_LIST* fptr = &Fires[0];
	int i = 0;
	while (fptr->on)
	{
		fptr++;
		if (++i >= MAX_FIRE_LIST)
			return;
	}

	if (on)
		fptr->on = on;
	else
		fptr->on = 1;

	fptr->x = x;
	fptr->y = y;
	fptr->z = z;
	fptr->roomNumber = roomNum;

	switch (size)
	{
	case SP_NORMALFIRE:
		fptr->size = 1;
		break;
	case SP_SMALLFIRE:
		fptr->size = 2;
		break;
	case SP_BIGFIRE:
		fptr->size = 3;
		break;
	}
}

void ClearFires()
{
	for (int i = 0; i < MAX_FIRE_LIST; i++)
		Fires[i].on = false;
}

void UpdateFireSparks()
{
	keep_those_fires_burning();

	for (int i = 0; i < MAX_SPARKS_FIRE; i++)
	{
		FIRE_SPARKS* spark = &FireSparks[i];

		if (spark->on)
		{
			spark->life--;

			if (!spark->life)
			{
				spark->on = false;
				continue;
			}

			if (spark->sLife - spark->life < spark->colFadeSpeed)
			{
				int dl = ((spark->sLife - spark->life) << 16) / spark->colFadeSpeed;

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
				int dl = ((spark->life - spark->fadeToBlack) << 16) / spark->fadeToBlack + 0x10000;

				spark->r = dl * spark->dR >> 16;
				spark->g = dl * spark->dG >> 16;
				spark->b = dl * spark->dB >> 16;

				if (spark->r < 8 && spark->g < 8 && spark->b < 8)
				{
					spark->on = false;
					continue;
				}
			}

			if (spark->flags & SP_ROTATE)
				spark->rotAng = (spark->rotAng + spark->rotAdd) & 0xFFF;
			float alpha = fmin(1, fmax(0, 1 - (spark->life / (float)spark->sLife)));
			int sprite = lerp(Objects[ID_FIRE_SPRITES].meshIndex, Objects[ID_FIRE_SPRITES].meshIndex+ (-Objects[ID_FIRE_SPRITES].nmeshes) - 1, alpha);
			spark->def = sprite;

			int dl = ((spark->sLife - spark->life) << 16) / spark->sLife;
			spark->yVel += spark->gravity;
			if (spark->maxYvel)
			{
				if ((spark->yVel < 0 && spark->yVel < (spark->maxYvel << 5)) ||
					(spark->yVel > 0 && spark->yVel > (spark->maxYvel << 5)))
					spark->yVel = spark->maxYvel << 5;
			}

			if (spark->friction)
			{
				spark->xVel -= spark->xVel >> spark->friction;
				spark->zVel -= spark->zVel >> spark->friction;
			}

			spark->x += spark->xVel / 48;
			spark->y += spark->yVel / 48;
			spark->z += spark->zVel / 48;

			spark->size = spark->sSize + ((dl * (spark->dSize - spark->sSize)) / 65536);
		}
	}
}

int GetFreeSmokeSpark() 
{
	SMOKE_SPARKS* spark = &SmokeSparks[NextSmokeSpark];
	int sparkNum = NextSmokeSpark;
	short minLife = 4095;
	short minIndex = 0;
	short count = 0;
	while (spark->on)
	{
		if (spark->life < minLife)
		{
			minIndex = sparkNum;
			minLife = spark->life;
		}

		if (sparkNum == MAX_SPARKS_SMOKE - 1)
		{
			spark = &SmokeSparks[0];
			sparkNum = 0;
		}
		else
		{
			sparkNum++;
			spark++;
		}

		if (++count >= MAX_SPARKS_SMOKE)
		{
			NextSmokeSpark = (minIndex + 1) % MAX_SPARKS_SMOKE;
			return minIndex;
		}
	}

	NextSmokeSpark = (sparkNum + 1) % MAX_SPARKS_SMOKE;

	return sparkNum;
}

void UpdateSmoke()
{
	for (int i = 0; i < MAX_SPARKS_SMOKE; i++)
	{
		SMOKE_SPARKS* spark = &SmokeSparks[i];

		if (spark->on)
		{
			spark->life -= 2;

			if (spark->life <= 0)
			{
				spark->on = false;
				continue;
			}

			if (spark->sLife - spark->life >= spark->colFadeSpeed)
			{
				if (spark->life >= spark->fadeToBlack)
				{
					spark->shade = spark->dShade;
				}
				else
				{
					spark->shade = spark->dShade * (((spark->life - spark->fadeToBlack) << 16) / spark->fadeToBlack + 0x10000) >> 16;
					if (spark->shade < 8)
					{
						spark->on = false;
						continue;
					}
				}
			}
			else
			{
				spark->shade = spark->sShade + ((spark->dShade - spark->sShade) * (((spark->sLife - spark->life) << 16) / spark->colFadeSpeed) >> 16);
			}

			if (spark->shade >= 24)
			{
				if (spark->shade >= 80)
					spark->def = Objects[ID_DEFAULT_SPRITES].meshIndex + SPR_FIRE0;
				else
					spark->def = Objects[ID_DEFAULT_SPRITES].meshIndex + SPR_FIRE1;
			}
			else
			{
				spark->def = Objects[ID_DEFAULT_SPRITES].meshIndex + SPR_FIRE2;
			}

			if (spark->flags & SP_ROTATE)
				spark->rotAng = (spark->rotAng + spark->rotAdd) & 0xFFF;

			int dl = ((spark->sLife - spark->life) << 16) / spark->sLife;

			spark->yVel += spark->gravity;
			
			if (spark->maxYvel != 0)
			{
				if (spark->yVel < 0) 
				{
					if (spark->yVel < spark->maxYvel) 
					{
						spark->yVel = spark->maxYvel;
					}
				}
				else 
				{
					if (spark->yVel > spark->maxYvel) 
					{
						spark->yVel = spark->maxYvel;
					}
				}
			}
			
			if (spark->friction & 0xF)
			{
				spark->xVel -= spark->xVel >> (spark->friction & 0xF);
				spark->zVel -= spark->zVel >> (spark->friction & 0xF);
			}

			if (spark->friction & 0xF0)
			{
				spark->yVel -= spark->yVel >> (spark->friction >> 4);
			}

			spark->x += spark->xVel >> 5;
			spark->y += spark->yVel >> 5;
			spark->z += spark->zVel >> 5;

			if (spark->flags & SP_WIND)
			{
				spark->x += Weather.Wind().x;
				spark->z += Weather.Wind().z;
			}

			spark->size = spark->sSize + (dl * (spark->dSize - spark->sSize) >> 16);
		}
	}
}

byte TriggerGunSmoke_SubFunction(int weaponType)
{
	switch (weaponType)
	{
	case WEAPON_HK:
	case WEAPON_ROCKET_LAUNCHER:
	case WEAPON_GRENADE_LAUNCHER:
		return 24; //(12) Rocket and Grenade value for TriggerGunSmoke in TR3 have the value 12 ! (the HK is not included there)

	// other weapon
	default:
		return 0;
	}
}

void TriggerGunSmoke(int x, int y, int z, short xv, short yv, short zv, byte initial, int weaponType, byte count)
{
	/*
	SMOKE_SPARKS* spark;
	
	spark = &SmokeSparks[GetFreeSmokeSpark()];
	spark->on = true;
	spark->sShade = 0;
	spark->dShade = (count << 2);
	spark->colFadeSpeed = 4;
	spark->fadeToBlack = 32 - (initial << 4);
	spark->life = (GetRandomControl() & 3) + 40;
	spark->sLife = spark->life;

	if (weaponType == WEAPON_PISTOLS || weaponType == WEAPON_REVOLVER || weaponType == WEAPON_UZI)
	{
		if (spark->dShade > 64)
			spark->dShade = 64;
	}

	spark->transType = TransTypeEnum::COLADD;
	spark->x = x + (GetRandomControl() & 31) - 16;
	spark->y = y + (GetRandomControl() & 31) - 16;
	spark->z = z + (GetRandomControl() & 31) - 16;

	if (initial)
	{
		spark->xVel = ((GetRandomControl() & 1023) - 512) + xv;
		spark->yVel = ((GetRandomControl() & 1023) - 512) + yv;
		spark->zVel = ((GetRandomControl() & 1023) - 512) + zv;
	}
	else
	{
		float f = (frand() * 6) - 3;
		spark->xVel = (frand() * 6) - 3;
		spark->yVel = (frand() * 6) - 3;
		spark->zVel = (frand() * 6) - 3;
	}

	spark->friction = 4;

	if (GetRandomControl() & 1)
	{
		if (g_Level.Rooms[LaraItem->roomNumber].flags & ENV_FLAG_WIND)
			spark->flags = SP_ROTATE | SP_WIND;
		else
			spark->flags = SP_ROTATE;

		spark->rotAng = GetRandomControl() & 0xFFF;

		if (GetRandomControl() & 1)
			spark->rotAdd = -(GetRandomControl() & 0x0F) - 16;
		else
			spark->rotAdd = (GetRandomControl() & 0x0F) + 16;
	}
	else if (g_Level.Rooms[LaraItem->roomNumber].flags & ENV_FLAG_WIND)
	{
		spark->flags = SP_WIND;
	}
	else
	{
		spark->flags = SP_NONE;
	}
	float gravity = frand() * 1.25f;
	spark->gravity = gravity;
	spark->maxYvel = frand() * 16;

	byte size = ((GetRandomControl() & 0x0F) + 24); // -TriggerGunSmoke_SubFunction(weaponType);

	if (initial)
	{
		spark->sSize = size >> 1;
		spark->size = size >> 1;
		spark->dSize = (size << 1) + 8;
	}
	else
	{
		spark->sSize = size >> 2;
		spark->size = size >> 2;
		spark->dSize = size;
	}

	/*if (gfLevelFlags & 0x20 && LaraItem->roomNumber == gfMirrorRoom) // 0x20 = GF_MIRROR_ENABLED
	{
		spark->mirror = 1;
	}
	else
	{
		spark->mirror = 0;
	}*/
	TEN::Effects::Smoke::TriggerGunSmokeParticles(x, y, z, xv, yv, zv, initial, weaponType, count);
	
}

void TriggerShatterSmoke(int x, int y, int z)
{
	SMOKE_SPARKS* spark = &SmokeSparks[GetFreeSmokeSpark()];
	
	spark->on = true;
	spark->sShade = 0;
	spark->colFadeSpeed = 4;
	spark->dShade = (GetRandomControl() & 0x1F) + 64;
	spark->fadeToBlack = 24 - (GetRandomControl() & 7);
	spark->transType = TransTypeEnum::COLADD;
	spark->life = spark->sLife = (GetRandomControl() & 7) + 48;
	spark->x = (GetRandomControl() & 0x1F) + x - 16;
	spark->y = (GetRandomControl() & 0x1F) + y - 16;
	spark->z = (GetRandomControl() & 0x1F) + z - 16;
	spark->xVel = 2 * (GetRandomControl() & 0x1FF) - 512;
	spark->yVel = 2 * (GetRandomControl() & 0x1FF) - 512;
	spark->zVel = 2 * (GetRandomControl() & 0x1FF) - 512;
	spark->friction = 7;
	
	if (GetRandomControl() & 1)
	{
		spark->flags = SP_ROTATE;
		spark->rotAng = GetRandomControl() & 0xFFF;
		if (GetRandomControl() & 1)
			spark->rotAdd = -64 - (GetRandomControl() & 0x3F);
		else
			spark->rotAdd = (GetRandomControl() & 0x3F) + 64;
	}
	else if (g_Level.Rooms[LaraItem->roomNumber].flags & ENV_FLAG_WIND)
	{
		spark->flags = SP_WIND;
	}
	else
	{
		spark->flags = SP_NONE;
	}

	spark->gravity = -4 - (GetRandomControl() & 3);
	spark->maxYvel = -4 - (GetRandomControl() & 3);
	spark->dSize = (GetRandomControl() & 0x3F) + 64;
	spark->sSize = spark->dSize >> 3;
	spark->size = spark->dSize >> 3;
}

int GetFreeBlood()
{
	BLOOD_STRUCT* blood = &Blood[NextBlood];
	int bloodNum = NextBlood;
	int minLife = 4095;
	int minIndex = 0;
	int count = 0;

	while (blood->on)
	{
		if (blood->life < minLife)
		{
			minIndex = bloodNum;
			minLife = blood->life;
		}

		if (bloodNum == MAX_SPARKS_BLOOD - 1)
		{
			blood = &Blood[0];
			bloodNum = 0;
		}
		else
		{
			blood++;
			bloodNum++;
		}

		if (++count >= MAX_SPARKS_BLOOD)
		{
			NextBlood = (minIndex + 1) & 31;
			return minIndex;
		}
	}

	NextBlood = (bloodNum + 1) & 31;
	return bloodNum;
}

void TriggerBlood(int x, int y, int z, int unk, int num)
{
	for (int i = 0; i < num; i++)
	{
		BLOOD_STRUCT* blood = &Blood[GetFreeBlood()];
		blood->on = 1;
		blood->sShade = 0;
		blood->colFadeSpeed = 4;
		blood->fadeToBlack = 8;
		blood->dShade = (GetRandomControl() & 0x3F) + 48;
		blood->life = blood->sLife = (GetRandomControl() & 7) + 24;
		blood->x = (GetRandomControl() & 0x1F) + x - 16;
		blood->y = (GetRandomControl() & 0x1F) + y - 16;
		blood->z = (GetRandomControl() & 0x1F) + z - 16;
		int a = (unk == -1 ? GetRandomControl() : (GetRandomControl() & 0x1F) + unk - 16) & 0xFFF;
		int b = GetRandomControl() & 0xF;
		blood->zVel = b * phd_cos(a << 4) * 32;
		blood->xVel = -b * phd_sin(a << 4) * 32;
		blood->friction = 4;
		blood->yVel = -((GetRandomControl() & 0xFF) + 128);
		blood->rotAng = GetRandomControl() & 0xFFF;
		blood->rotAdd = (GetRandomControl() & 0x3F) + 64;
		if (GetRandomControl() & 1)
			blood->rotAdd = -blood->rotAdd;
		blood->gravity = (GetRandomControl() & 0x1F) + 31;
		int size = (GetRandomControl() & 7) + 8;
		blood->sSize = blood->size = size;
		blood->dSize = size >> 2;
	}
}

void UpdateBlood()
{
	for (int i = 0; i < MAX_SPARKS_BLOOD; i++)
	{
		BLOOD_STRUCT* blood = &Blood[i];

		if (blood->on)
		{
			blood->life--;

			if (blood->life <= 0)
			{
				blood->on = false;
				continue;
			}

			if (blood->sLife - blood->life >= blood->colFadeSpeed)
			{
				if (blood->life >= blood->fadeToBlack)
				{
					blood->shade = blood->dShade;
				}
				else
				{
					blood->shade = blood->dShade * (((blood->life - blood->fadeToBlack) << 16) / blood->fadeToBlack + 0x10000) >> 16;
					if (blood->shade < 8)
					{
						blood->on = false;
						continue;
					}
				}
			}
			else
			{
				blood->shade = blood->sShade + ((blood->dShade - blood->sShade) * (((blood->sLife - blood->life) << 16) / blood->colFadeSpeed) >> 16);
			}
			
			blood->rotAng = (blood->rotAng + blood->rotAdd) & 0xFFF;
			blood->yVel += blood->gravity;
						
			if (blood->friction & 0xF)
			{
				blood->xVel -= blood->xVel >> (blood->friction & 0xF);
				blood->zVel -= blood->zVel >> (blood->friction & 0xF);
			}

			int dl = ((blood->sLife - blood->life) << 16) / blood->sLife;

			blood->x += blood->xVel >> 5;
			blood->y += blood->yVel >> 5;
			blood->z += blood->zVel >> 5;

			blood->size = blood->sSize + (dl * (blood->dSize - blood->sSize) >> 16);
		}
	}
}

int GetFreeGunshell()
{
	int gsNum = NextGunShell;
	int minLife = 4095;
	int minIndex = 0;
	int i = 0;

	while (true)
	{
		GUNSHELL_STRUCT* gs = &Gunshells[NextGunShell];

		if (!gs->counter)
			break;

		if (gs->counter < minLife)
		{
			minLife = gs->counter;
			minIndex = gsNum;
		}
		if (gsNum == MAX_GUNSHELL - 1)
		{
			gs = &Gunshells[0];
			gsNum = 0;
		}
		else
		{
			gsNum++;
			gs++;
		}

		if (++i >= MAX_GUNSHELL)
		{
			NextGunShell = minIndex + 1;
			if (minIndex + 1 >= MAX_GUNSHELL)
				NextGunShell = 0;
			return minIndex;
		}
	}

	NextGunShell = gsNum + 1;
	if (gsNum + 1 >= MAX_GUNSHELL)
		NextGunShell = 0;

	return gsNum;
}

void TriggerGunShell(short hand, short objNum, LARA_WEAPON_TYPE weaponType)
{
	PHD_VECTOR pos;

	if (hand)
	{
		switch (weaponType)
		{
		case WEAPON_PISTOLS:
			pos.x = 8;
			pos.y = 48;
			pos.z = 40;
			break;
		case WEAPON_UZI:
			pos.x = 8;
			pos.y = 35;
			pos.z = 48;
			break;
		case WEAPON_SHOTGUN:
			pos.x = 16;
			pos.y = 114;
			pos.z = 32;
			break;
		case WEAPON_HK:
			pos.x = 16;
			pos.y = 114;
			pos.z = 96;
			break;
		default:
			break;
		}

		GetLaraJointPosition(&pos, LM_RHAND);
	}
	else
	{
		if (weaponType == WEAPON_PISTOLS)
		{
			pos.x = -12;
			pos.y = 48;
			pos.z = 40;

			GetLaraJointPosition(&pos, LM_LHAND);
		}
		else if (weaponType == WEAPON_UZI)
		{
			pos.x = -16;
			pos.y = 35;
			pos.z = 48;

			GetLaraJointPosition(&pos, LM_LHAND);
		}		
	}

	GUNSHELL_STRUCT* gshell = &Gunshells[GetFreeGunshell()];

	gshell->pos.xPos = pos.x;
	gshell->pos.yPos = pos.y;
	gshell->pos.zPos = pos.z;
	gshell->pos.xRot = 0;
	gshell->pos.yRot = 0;
	gshell->pos.zRot = GetRandomControl();
	gshell->roomNumber = LaraItem->roomNumber;
	gshell->speed = (GetRandomControl() & 0x1F) + 16;
	gshell->fallspeed = -48 - (GetRandomControl() & 7);
	gshell->objectNumber = objNum;
	gshell->counter = (GetRandomControl() & 0x1F) + 60;
	if (hand)
	{
		if (weaponType == WEAPON_SHOTGUN)
		{
			gshell->dirXrot = Lara.leftArm.yRot
				+ Lara.torsoYrot
				+ LaraItem->pos.yRot
				- (GetRandomControl() & 0xFFF)
				+ 10240;
			gshell->pos.yRot += Lara.leftArm.yRot 
				+ Lara.torsoYrot 
				+ LaraItem->pos.yRot;
			if (gshell->speed < 24)
				gshell->speed += 24;
		}
		else
		{
			gshell->dirXrot = Lara.leftArm.yRot 
				+ LaraItem->pos.yRot 
				- (GetRandomControl() & 0xFFF) 
				+ 18432;
		}
	}
	else
	{
		gshell->dirXrot = Lara.leftArm.yRot 
			+ LaraItem->pos.yRot 
			+ (GetRandomControl() & 0xFFF) 
			- 18432;
	}

	if (LaraItem->meshBits)
	{
		if (weaponType == WEAPON_SHOTGUN)
			TriggerGunSmoke(pos.x, pos.y, pos.z, 0, 0, 0, 0, WEAPON_SHOTGUN, 24);
		else
			TriggerGunSmoke(pos.x, pos.y, pos.z, 0, 0, 0, 0, weaponType, 16);
	}
}

void UpdateGunShells()
{
	for (int i = 0; i < MAX_GUNSHELL; i++)
	{
		GUNSHELL_STRUCT* gs = &Gunshells[i];

		if (gs->counter)
		{
			int oldX = gs->pos.xPos;
			int oldY = gs->pos.yPos;
			int oldZ = gs->pos.zPos;

			gs->counter--;

			short oldRoomNumber = gs->roomNumber;

			if (g_Level.Rooms[gs->roomNumber].flags & ENV_FLAG_WATER)
			{
				gs->fallspeed++;

				if (gs->fallspeed <= 8)
				{
					if (gs->fallspeed < 0)
						gs->fallspeed >>= 1;
				}
				else
				{
					gs->fallspeed = 8;
				}
				gs->speed -= gs->speed >> 1;
			}
			else
			{
				gs->fallspeed += 6;
			}

			gs->pos.xRot += (gs->speed >> 1 + 7) * ANGLE(1);
			gs->pos.yRot += gs->speed * ANGLE(1);
			gs->pos.zRot += ANGLE(23);

			gs->pos.xPos += gs->speed * phd_sin(gs->dirXrot);
			gs->pos.yPos += gs->fallspeed;
			gs->pos.zPos += gs->speed * phd_cos(gs->dirXrot);

			FLOOR_INFO* floor = GetFloor(gs->pos.xPos, gs->pos.yPos, gs->pos.zPos, &gs->roomNumber);
			if (g_Level.Rooms[gs->roomNumber].flags & ENV_FLAG_WATER
				&& !(g_Level.Rooms[oldRoomNumber].flags & ENV_FLAG_WATER))
			{

				TEN::Effects::Drip::SpawnGunshellDrips(Vector3(gs->pos.xPos, g_Level.Rooms[gs->roomNumber].maxceiling, gs->pos.zPos), gs->roomNumber);
				//AddWaterSparks(gs->pos.xPos, g_Level.Rooms[gs->roomNumber].maxceiling, gs->pos.zPos, 8);
				SetupRipple(gs->pos.xPos, g_Level.Rooms[gs->roomNumber].maxceiling, gs->pos.zPos, (GetRandomControl() & 3) + 8, 2, Objects[ID_DEFAULT_SPRITES].meshIndex + SPR_RIPPLES);
				gs->fallspeed >>= 5;
				continue;
			}

			int ceiling = GetCeiling(floor, gs->pos.xPos, gs->pos.yPos, gs->pos.zPos);
			if (gs->pos.yPos < ceiling)
			{
				SoundEffect(SFX_TR4_LARA_SHOTGUN_SHELL, &gs->pos, 0);
				gs->speed -= 4;

				if (gs->speed < 8)
				{
					gs->counter = 0;
					continue;
				}

				gs->pos.yPos = ceiling;
				gs->fallspeed = -gs->fallspeed;
			}

			int height = GetFloorHeight(floor, gs->pos.xPos, gs->pos.yPos, gs->pos.zPos);
			if (gs->pos.yPos >= height)
			{
				SoundEffect(SFX_TR4_LARA_SHOTGUN_SHELL, &gs->pos, 0);
				gs->speed -= 8;
				if (gs->speed >= 8)
				{
					if (oldY <= height)
					{
						gs->fallspeed = -gs->fallspeed >> 1;
					}
					else
					{
						gs->dirXrot += -ANGLE(180);
						gs->pos.xPos = oldX;
						gs->pos.zPos = oldZ;
					}
					gs->pos.yPos = oldY;
				}
				else
				{
					gs->counter = 0;
				}
			}
		}
	}
}

void AddWaterSparks(int x, int y, int z, int num)
{
	for (int i = 0; i < num; i++)
	{
		SPARKS* spark = &Sparks[GetFreeSpark()];

		spark->on = 1;
		spark->sR = 127;
		spark->sG = 127;
		spark->sB = 127;
		spark->dR = 48;
		spark->dG = 48;
		spark->dB = 48;
		spark->colFadeSpeed = 4;
		spark->fadeToBlack = 8;
		spark->life = 10;
		spark->sLife = 10;
		spark->sSize = 8;
		spark->dSize = 32;
		spark->scalar = 1;
		spark->transType = TransTypeEnum::COLADD;	
		int random = GetRandomControl() & 0xFFF;
		spark->xVel = -phd_sin(random << 4) * 128;
		spark->yVel = -GenerateInt(128, 256);
		spark->zVel = phd_cos(random << 4) * 128;
		spark->friction = 5;
		spark->flags = SP_NONE;
		spark->x = x + (spark->xVel >> 3);
		spark->y = y - (spark->yVel >> 5);
		spark->z = z + (spark->zVel >> 3);
		spark->maxYvel = 0;
		spark->gravity = (GetRandomControl() & 0xF);
	}
}

void LaraBubbles(ITEM_INFO* item)
{
	PHD_VECTOR pos;
	int num, i;

	SoundEffect(SFX_TR4_LARA_BUBBLES, &item->pos, 1);

	pos.x = 0;

	auto level = g_GameFlow->GetLevel(CurrentLevel);

	if (level->LaraType == LaraType::Divesuit)
	{
		pos.y = -192;
		pos.z = -160;

		GetLaraJointPosition(&pos, LM_TORSO);
	}
	else
	{
		pos.y = -4;
		pos.z = 64;

		GetLaraJointPosition(&pos, LM_HEAD);
	}

	num = (GetRandomControl() & 1) + 2;

	for (i = 0; i < num; i++)
	{
		CreateBubble(&pos, item->roomNumber, 8, 7, 0, 0, 0, 0);
	}
}


int GetFreeDrip()
{
	DRIP_STRUCT* drip = &Drips[NextDrip];
	int dripNum = NextDrip;
	short minLife = 4095;
	short minIndex = 0;
	short count = 0;	

	while (drip->on)
	{
		if (drip->life < minLife)
		{
			minIndex = dripNum;
			minLife = drip->life;
		}

		if (dripNum == MAX_DRIPS - 1)
		{
			drip = &Drips[0];
			dripNum = 0;
		}
		else
		{
			dripNum++;
			drip++;
		}

		if (++count >= MAX_DRIPS)
		{
			NextDrip = (minIndex + 1) % MAX_DRIPS;
			return minIndex;
		}
	}

	NextDrip = (dripNum + 1) % MAX_DRIPS;

	return dripNum;
}

void UpdateDrips()
{
	for (int i = 0; i < MAX_DRIPS; i++)
	{
		DRIP_STRUCT* drip = &Drips[i];

		if (drip->on)
		{
			drip->life--;
			if (!drip->life)
			{
				drip->on = false;
				continue;
			}

			if (drip->life < 16)
			{
				drip->r -= drip->r >> 3;
				drip->g -= drip->g >> 3;
				drip->b -= drip->b >> 3;
			}

			drip->yVel += drip->gravity;
			
			if (g_Level.Rooms[drip->roomNumber].flags & ENV_FLAG_WIND)
			{
				drip->x += Weather.Wind().x;
				drip->z += Weather.Wind().z;
			}

			drip->y += drip->yVel >> 5;
			
			FLOOR_INFO* floor = GetFloor(drip->x, drip->y, drip->z, &drip->roomNumber);
			if (g_Level.Rooms[drip->roomNumber].flags & ENV_FLAG_WATER)
				drip->on = false;

			int height = GetFloorHeight(floor, drip->x, drip->y, drip->z);
			if (drip->y > height)
			{
				if (i % 2 == 0)
					AddWaterSparks(drip->x, drip->y, drip->z, 6);
				drip->on = false;
			}
		}
	}
}

void TriggerLaraDrips(ITEM_INFO* item)
{
	auto pos = PHD_VECTOR();
	
	if (!(Wibble & 0xF))
	{
		for (int i = 0; i < NUM_LARA_MESHES; i++)
		{
			GetLaraJointPosition(&pos, (LARA_MESHES)i);
			auto room = GetRoom(item->location, pos.x, pos.y, pos.z).roomNumber;

			if (g_Level.Rooms[room].flags & ENV_FLAG_WATER)
				Lara.wet[i] = UCHAR_MAX;

			if (Lara.wet[i] 
				&& !LaraNodeUnderwater[i] 
				&& (GetRandomControl() & 0x1FF) < Lara.wet[i])
			{

				pos.x = (GetRandomControl() & 0x1F) - 16;
				pos.y = (GetRandomControl() & 0xF) + 16;
				pos.z = (GetRandomControl() & 0x1F) - 16;

				DRIP_STRUCT* dptr = &Drips[GetFreeDrip()];
				GetLaraJointPosition(&pos, i);
				dptr->x = pos.x;
				dptr->y = pos.y;
				dptr->z = pos.z;
				dptr->on = 1;
				dptr->r = (GetRandomControl() & 7) + 64;
				dptr->g = (GetRandomControl() & 7) + 96;
				dptr->b = (GetRandomControl() & 7) + 128;
				dptr->yVel = (GetRandomControl() & 0x1F) + 32;
				dptr->gravity = (GetRandomControl() & 0x1F) + 32;
				dptr->life = (GetRandomControl() & 0x1F) + 8;
				dptr->roomNumber = LaraItem->roomNumber;

				if (Lara.wet[i] >= 4)
					Lara.wet[i] -= 4;
				else
					Lara.wet[i] = 0;

			}
		}
	}
}

int ExplodingDeath(short itemNumber, int meshBits, short flags)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];
	OBJECT_INFO* obj = &Objects[item->objectNumber];

	ANIM_FRAME* frame = GetBestFrame(item);
	
	Matrix world = Matrix::CreateFromYawPitchRoll(
		TO_RAD(item->pos.yRot),
		TO_RAD(item->pos.xRot),
		TO_RAD(item->pos.zRot)
	);

	int bit = 1;

	if ((bit & meshBits) && (bit & item->meshBits))
	{
		if ((GetRandomControl() & 3) == 0)
		{
			short fxNumber = CreateNewEffect(item->roomNumber);
			if (fxNumber != NO_ITEM)
			{
				FX_INFO* fx = &EffectList[fxNumber];

				Matrix boneMatrix;
				g_Renderer.getBoneMatrix(itemNumber, 0, &boneMatrix);
				boneMatrix = world * boneMatrix;

				fx->pos.xPos = boneMatrix.Translation().x + item->pos.xPos;
				fx->pos.yPos = boneMatrix.Translation().y + item->pos.yPos;
				fx->pos.zPos = boneMatrix.Translation().z + item->pos.zPos;

				fx->roomNumber = item->roomNumber;
				fx->pos.xRot = 0;
				fx->pos.yRot = GetRandomControl() * 2;

				if (!(flags & 0x10))
				{
					if (flags & 0x20)
						fx->speed = GetRandomControl() >> 12;
					else
						fx->speed = GetRandomControl() >> 8;
				}

				if (flags & 0x40)
					fx->fallspeed = 0;
				else
				{
					if ((flags & 0x80) == 0)
						fx->fallspeed = -(GetRandomControl() >> 8);
					else
						fx->fallspeed = -(GetRandomControl() >> 12);
				}
				fx->objectNumber = ID_BODY_PART;
				fx->frameNumber = obj->meshIndex;
				fx->shade = 16912;
				fx->flag2 = flags;
			}

			item->meshBits -= bit;
		}
	}

	for (int i = 1; i < obj->nmeshes; i++)
	{
		Matrix boneMatrix;
		g_Renderer.getBoneMatrix(itemNumber, i, &boneMatrix);
		boneMatrix = world * boneMatrix;

		bit <<= 1;
		if ((bit & meshBits) && (bit & item->meshBits))
		{
			if ((GetRandomControl() & 3) == 0 && (flags & 0x100))
			{
				short fxNumber = CreateNewEffect(item->roomNumber);
				if (fxNumber != NO_ITEM)
				{
					FX_INFO* fx = &EffectList[fxNumber];

					fx->pos.xPos = boneMatrix.Translation().x + item->pos.xPos;
					fx->pos.yPos = boneMatrix.Translation().y + item->pos.yPos;
					fx->pos.zPos = boneMatrix.Translation().z + item->pos.zPos;

					fx->roomNumber = item->roomNumber;
					fx->pos.xRot = 0;
					fx->pos.yRot = GetRandomControl() * 2;

					if (!(flags & 0x10))
					{
						if (flags & 0x20)
							fx->speed = GetRandomControl() >> 12;
						else
							fx->speed = GetRandomControl() >> 8;
					}

					if (flags & 0x40)
						fx->fallspeed = 0;
					else
					{
						if ((flags & 0x80) == 0)
							fx->fallspeed = -(GetRandomControl() >> 8);
						else
							fx->fallspeed = -(GetRandomControl() >> 12);
					}

					fx->objectNumber = ID_BODY_PART;
					fx->shade = 16912;
					fx->flag2 = flags;
					fx->frameNumber = obj->meshIndex + i;
				}

				item->meshBits -= bit;
			}
		}
	}

	return item->meshBits == 0;
}

int GetFreeShockwave()
{
	for (int i = 0; i < MAX_SHOCKWAVE; i++)
	{
		if (!ShockWaves[i].life)
			return i;
	}

	return -1;
}

void TriggerShockwave(PHD_3DPOS* pos, short innerRad, short outerRad, int speed, char r, char g, char b, char life, short angle, short flags)
{
	int s = GetFreeShockwave();
	SHOCKWAVE_STRUCT* sptr;

	if (s != -1)
	{
		sptr = &ShockWaves[s];

		sptr->x = pos->xPos;
		sptr->y = pos->yPos;
		sptr->z = pos->zPos;
		sptr->innerRad = innerRad;
		sptr->outerRad = outerRad;
		sptr->xRot = angle;
		sptr->flags = flags;
		sptr->speed = speed;
		sptr->r = r;
		sptr->g = g;
		sptr->b = b;
		sptr->life = life;
		
		SoundEffect(SFX_TR5_IMP_STONE_HIT, pos, 0);
	}
}

void TriggerShockwaveHitEffect(int x, int y, int z, byte r, byte g, byte b, short rot, int vel)
{
	int dx = LaraItem->pos.xPos - x;
	int dz = LaraItem->pos.zPos - z;

	if (dx >= -16384 && dx <= 16384 && dz >= -16384 && dz <= 16384)
	{
		SPARKS* spark = &Sparks[GetFreeSpark()];
		spark->dB = b;
		spark->on = true;
		spark->sR = 0;
		spark->sG = 0;
		spark->sB = 0;
		spark->dG = g;
		spark->dR = r;
		spark->colFadeSpeed = 4;
		spark->fadeToBlack = 8;
		spark->transType = TransTypeEnum::COLADD;
		spark->life = spark->sLife = (GetRandomControl() & 3) + 16;

		int speed = (GetRandomControl() & 0xF) + vel;
		spark->xVel = speed * 16 * phd_sin(rot);
		spark->yVel = -512 - (GetRandomControl() & 0x1FF);
		spark->zVel = speed * 16 * phd_cos(rot);

		short angle;
		if (GetRandomControl() & 1)
			angle = rot + ANGLE(90);
		else
			angle = rot - ANGLE(90);

		int shift = (GetRandomControl() & 0x1FF) - 256;
		x += shift * phd_sin(angle);
		z += shift * phd_cos(angle);

		spark->x = (GetRandomControl() & 0x1F) + x - 16;
		spark->y = (GetRandomControl() & 0x1F) + y - 16;
		spark->z = (GetRandomControl() & 0x1F) + z - 16;

		spark->friction = 3;
		spark->flags = SP_EXPDEF | SP_ROTATE | SP_DEF | SP_SCALE;
		spark->rotAng = GetRandomControl() & 0xFFF;
		if (GetRandomControl() & 1)
			spark->rotAdd = -16 - (GetRandomControl() & 0xF);
		else
			spark->rotAdd = (GetRandomControl() & 0xF) + 16;

		spark->scalar = 1;
		spark->def = Objects[ID_DEFAULT_SPRITES].meshIndex + SPR_UNDERWATERDUST;
		spark->maxYvel = 0;
		spark->gravity = (GetRandomControl() & 0x3F) + 64;
		spark->sSize = spark->size = (GetRandomControl() & 0x1F) + 32;
		spark->dSize = spark->size / 4;
	}
}

void UpdateShockwaves()
{
	for (int i = 0; i < MAX_SHOCKWAVE; i++)
	{
		SHOCKWAVE_STRUCT* sw = &ShockWaves[i];

		if (sw->life)
		{
			sw->life--;

			if (sw->life)
			{
				sw->outerRad += sw->speed;
				sw->speed -= (sw->speed >> 4);

				if (LaraItem->hitPoints > 0)
				{
					if (sw->flags & 3)
					{
						ANIM_FRAME* frame = GetBestFrame(LaraItem);

						int dx = LaraItem->pos.xPos - sw->x;
						int dz = LaraItem->pos.zPos - sw->z;
						int distance = sqrt(SQUARE(dx) + SQUARE(dz));
						
						if (sw->y <= LaraItem->pos.yPos + frame->boundingBox.Y1
							|| sw->y >= LaraItem->pos.yPos + frame->boundingBox.Y2 + 256
							|| distance <= sw->innerRad
							|| distance >= sw->outerRad)
						{
							sw->temp = 0;
						}
						else
						{
							short angle = phd_atan(dz, dx);
							TriggerShockwaveHitEffect(LaraItem->pos.xPos,
								sw->y,
								LaraItem->pos.zPos,
								sw->r, sw->g, sw->b,
								angle,
								sw->speed);
							LaraItem->hitPoints -= sw->speed >> (((sw->flags >> 1) & 1) + 2);
						}
					}
				}
			}
		}
	}
}

void TriggerExplosionBubble(int x, int y, int z, short roomNum)
{
	int dx = LaraItem->pos.xPos - x;
	int dz = LaraItem->pos.zPos - z;

	if (dx >= -16384 && dx <= 16384 && dz >= -16384 && dz <= 16384)
	{
		SPARKS* spark = &Sparks[GetFreeSpark()];
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
		spark->transType = TransTypeEnum::COLADD;
		spark->x = x;
		spark->y = y;
		spark->z = z;
		spark->xVel = 0;
		spark->yVel = 0;
		spark->zVel = 0;
		spark->friction = 0;
		spark->flags = 2058;
		spark->scalar = 3;
		spark->gravity = 0;
		spark->def = Objects[ID_DEFAULT_SPRITES].meshIndex + 13;
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
			CreateBubble(&pos, roomNum, 6, 15, BUBBLE_FLAG_CLUMP | BUBBLE_FLAG_BIG_SIZE | BUBBLE_FLAG_HIGH_AMPLITUDE, 0, 0, 0);
		}
	}
}

/*void TriggerExplosionSmokeEnd(int x, int y, int z, int unk)
{
	SPARKS* spark = &Sparks[GetFreeSpark()];
	
	spark->on = 1;
	if (unk)
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
	spark->life = spark->sLife = (GetRandomControl() & 0x1F) + 96;
	if (unk)
		spark->transType = TransTypeEnum::COLADD;
	else
		spark->transType = 3;
	spark->x = (GetRandomControl() & 0x1F) + x - 16;
	spark->y = (GetRandomControl() & 0x1F) + y - 16;
	spark->z = (GetRandomControl() & 0x1F) + z - 16;
	spark->xVel = ((GetRandomControl() & 0xFFF) - 2048) >> 2;
	spark->yVel = (GetRandomControl() & 0xFF) - 128;
	spark->zVel = ((GetRandomControl() & 0xFFF) - 2048) >> 2;
	if (unk)
	{
		spark->friction = 20;
		spark->yVel >>= 4;
		spark->y += 32;
	}
	else
	{
		spark->friction = 6;
	}
	spark->flags = 538;
	spark->rotAng = GetRandomControl() & 0xFFF;
	if (GetRandomControl() & 1)
		spark->rotAdd = -((GetRandomControl() & 0xF) + 16);
	else
		spark->rotAdd = (GetRandomControl() & 0xF) + 16;
	spark->scalar = 3;
	if (unk)
	{
		spark->maxYvel = 0;
		spark->gravity = 0;
	}
	else
	{
		spark->gravity = -3 - (GetRandomControl() & 3);
		spark->maxYvel = -4 - (GetRandomControl() & 3);
	}
	int size = (GetRandomControl() & 0x1F) + 128;
	spark->dSize = size;
	spark->sSize = size >> 2;
	spark->size = size >> 2;
}
*/
/*void DrawLensFlares(ITEM_INFO* item)
{
	GAME_VECTOR pos;

	pos.x = item->pos.xPos;
	pos.y = item->pos.yPos;
	pos.z = item->pos.zPos;
	pos.roomNumber = item->roomNumber;

	SetUpLensFlare(0, 0, 0, &pos);
}*/

void TriggerFenceSparks(int x, int y, int z, int kill, int crane)
{
	SPARKS* spark = &Sparks[GetFreeSpark()];
	spark->on = 1;
	spark->sR = (GetRandomControl() & 0x3F) - 0x40;
	spark->sG = (GetRandomControl() & 0x3F) - 0x40;
	spark->sB = (GetRandomControl() & 0x3F) - 0x40;

	spark->dR = GetRandomControl() | 0xC0;
	spark->colFadeSpeed = 16;
	spark->g = 8;
	spark->dG = spark->sR >> 1;
	spark->dB = spark->sR >> 2;

	spark->life = (GetRandomControl() & 7) + 24;
	spark->sLife = (GetRandomControl() & 7) + 24;
	spark->transType = TransTypeEnum::COLADD;
	spark->dynamic = -1;

	spark->x = x;
	spark->y = y;
	spark->z = z;

	spark->xVel = ((GetRandomControl() & 0xFF) - 128) << 2;
	spark->yVel = (GetRandomControl() & 0xF) - ((kill << 5) + 8) + (crane << 4);
	spark->zVel = ((GetRandomControl() & 0xFF) - 128) << 2;

	if (crane != 0)
	{
		spark->friction = 5;
	}
	else
	{
		spark->friction = 4;
	}

	spark->flags = SP_NONE;
	spark->gravity = (GetRandomControl() & 0xF) + ((crane << 4) + 16);
	spark->maxYvel = 0;
}

void TriggerSmallSplash(int x, int y, int z, int num) 
{
	int i;
	int angle;

	for (i = 0; i < num; i++)
	{
		SPARKS* sptr = &Sparks[GetFreeSpark()];

		sptr->on = 1;

		sptr->sR = 64;
		sptr->sG = 64;
		sptr->sB = 64;

		sptr->dR = 32;
		sptr->dG = 32;
		sptr->dB = 32;

		sptr->colFadeSpeed = 4;
		sptr->fadeToBlack = 8;

		sptr->life = 24;
		sptr->sLife = 24;

		sptr->transType = TransTypeEnum::COLADD;

		angle = GetRandomControl() << 3;

		sptr->xVel = -phd_sin(angle) * 512;
		sptr->yVel = -640 - (GetRandomControl() & 0xFF);
		sptr->zVel = phd_cos(angle) * 512;

		sptr->friction = 5;
		sptr->flags = 0;

		sptr->x = x + (sptr->xVel >> 3);
		sptr->y = y - (sptr->yVel >> 5);
		sptr->z = z + (sptr->zVel >> 3);

		sptr->maxYvel = 0;
		sptr->gravity = (GetRandomControl() & 0xF) + 64; 
	}
}