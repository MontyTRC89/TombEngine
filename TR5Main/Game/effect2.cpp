#include "effect2.h"
#include "draw.h"

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
		(weapon != WEAPON_HK && weapon != WEAPON_ROCKET && weapon != WEAPON_GRENADE ? 24 : 0) + 48;
	
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

void Inject_Effect2()
{
	INJECT(0x00431240, TriggerDynamics);
	INJECT(0x004820A0, TriggerGunSmoke);
}