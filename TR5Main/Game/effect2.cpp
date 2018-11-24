#include "effect2.h"
#include "draw.h"

#include "..\Scripting\GameFlowScript.h"

extern GameFlow* g_GameFlow;

void __cdecl TriggerDynamics(__int32 x, __int32 y, __int32 z, __int16 falloff, byte r, byte g, byte b)
{
	g_Renderer->AddDynamicLight(x, y, z, falloff, r, g, b);
}

void __cdecl TriggerGunSmoke(__int32 x, __int32 y, __int32 z, __int32 xv, __int32 yv, __int32 zv, __int32 initial, __int32 weapon, __int32 count)
{
	SMOKE_SPARKS* spark = &SmokeSparks[GetFreeSmokeSpark()];

	spark->On = true;
	spark->sShade = 0;
	spark->dShade = 4 * count;
	spark->ColFadeSpeed = 4;
	spark->FadeToBlack = 32 - 16 * initial;
	spark->Life = (GetRandomControl() & 3) + 40;
	spark->sLife = spark->Life;

	if ((weapon == WEAPON_PISTOLS || weapon == WEAPON_REVOLVER || weapon == WEAPON_UZI) && spark->dShade > 64)
		spark->dShade = 64;

	spark->TransType = 2;

	spark->x = (GetRandomControl() & 0x1F) + x - 16;
	spark->y = (GetRandomControl() & 0x1F) + y - 16;
	spark->z = (GetRandomControl() & 0x1F) + z - 16;

	if (initial)
	{
		spark->Xvel = (GetRandomControl() & 0x3FF) + xv - 512;
		spark->Yvel = (GetRandomControl() & 0x3FF) + yv - 512;
		spark->Zvel = (GetRandomControl() & 0x3FF) + zv - 512;
	}
	else
	{
		spark->Xvel = ((GetRandomControl() & 0x1FF) - 256) >> 1;
		spark->Yvel = ((GetRandomControl() & 0x1FF) - 256) >> 1;
		spark->Zvel = ((GetRandomControl() & 0x1FF) - 256) >> 1;
	}

	spark->Friction = 4;

	if (GetRandomControl() & 1)
	{
		if (Rooms[LaraItem->roomNumber].flags & 0x20)
			spark->Flags = 272;
		else
			spark->Flags = 16;

		spark->RotAng = GetRandomControl() & 255;

		if (GetRandomControl() & 1)
			spark->RotAdd = -16 - (GetRandomControl() & 15);
		else
			spark->RotAdd = (GetRandomControl() & 15) + 16;
	}
	else if (Rooms[LaraItem->roomNumber].flags & 0x20)
	{
		spark->Flags = 256;
	}
	else
	{
		spark->Flags = 0;
	}

	spark->Gravity = -2 - (GetRandomControl() & 1);
	spark->MaxYvel = -2 - (GetRandomControl() & 1);

	__int32 size = (GetRandomControl() & 15) - 
		(weapon != WEAPON_HK && weapon != WEAPON_ROCKET_LAUNCHER && weapon != WEAPON_GRENADE_LAUNCHER ? 24 : 0) + 48;
	
	if (initial)
	{
		spark->sSize = size >> 1;
		spark->Size = size >> 1;
		size = 2 * (size + 4);
	}
	else
	{
		spark->sSize = size >> 2;
		spark->Size = size >> 2;
	}

	spark->dSize = size;

	/*if (BYTE1(gfLevelFlags) & 0x20 && lara_item->room_number == gfMirrorRoom)
	{
		result = 1;
		spark->mirror = 1;
	}
	else
	{
		result = 0;
		spark->mirror = 0;
	}*/
}

void __cdecl TriggerRocketFlame(__int32 x, __int32 y, __int32 z, __int32 xv, __int32 yv, __int32 zv, __int32 itemNumber)
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

	__int32 size = (GetRandomControl() & 7) + 32;
	sptr->size = sptr->sSize = size;
}

void __cdecl TriggerRocketSmoke(__int32 x, __int32 y, __int32 z, __int32 bodyPart)
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

	__int32 size = (GetRandomControl() & 7) + 32;
	sptr->size = sptr->sSize = size >> 2;
}

void __cdecl GrenadeExplosionEffects(__int32 x, __int32 y, __int32 z, __int16 roomNumber)
{
	ROOM_INFO* room = &Rooms[roomNumber];

	bool mirror = (roomNumber == g_GameFlow->GetLevel(CurrentLevel)->Mirror.Room);

	bool water = false;
	if (room->flags & ENV_FLAG_WATER)
	{
		TriggerExplosionBubbles(x, y, z, roomNumber);
		water = true;
	}

	SMOKE_SPARKS* spark = &SmokeSparks[GetFreeSmokeSpark()];
	spark->On = true;
	spark->sShade = 0;
	spark->dShade = -128;
	spark->ColFadeSpeed = 4;
	spark->FadeToBlack = 16;
	spark->TransType = 2;
	spark->Life = spark->sLife = (GetRandomControl() & 0xF) + 64;
	spark->x = (GetRandomControl() & 0x1F) + x - 16;
	spark->y = (GetRandomControl() & 0x1F) + y - 16;
	spark->z = (GetRandomControl() & 0x1F) + z - 16;

	if (water)
	{
		spark->Xvel = spark->Yvel = GetRandomControl() & 0x3FF - 512;
		spark->Zvel = (GetRandomControl() & 0x3FF) - 512;
		spark->Friction = 68;
	}
	else
	{
		spark->Xvel = 2 * (GetRandomControl() & 0x3FF) - 1024;
		spark->Yvel = -512 - (GetRandomControl() & 0x3FF);
		spark->Zvel = 2 * (GetRandomControl() & 0x3FF) - 1024;
		spark->Friction = 85;
	}

	if (room->flags & ENV_FLAG_WIND)
		spark->Flags = 272;
	else
		spark->Flags = 16;

	spark->RotAng = GetRandomControl() & 0xFFF;
	if (GetRandomControl() & 1)
	{
		spark->RotAdd = -16 - (GetRandomControl() & 0xF);
	}
	else
	{
		spark->RotAdd = (GetRandomControl() & 0xF) + 16;
	}
	spark->MaxYvel = 0;
	spark->Gravity = 0;
	spark->sSize = spark->Size = (GetRandomControl() & 0x1F) + 64;
	spark->dSize = 2 * (spark->sSize + 4);
	spark->mirror = mirror;
}

void __cdecl GrenadeLauncherSpecialEffect1(__int32 x, __int32 y, __int32 z, __int32 flag1, __int32 flag2)
{
	__int32 dx = LaraItem->pos.xPos - x;
	__int32 dz = LaraItem->pos.zPos - z;

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

void Inject_Effect2()
{
	INJECT(0x00431240, TriggerDynamics);
	INJECT(0x004820A0, TriggerGunSmoke);
}