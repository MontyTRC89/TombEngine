#pragma once

#include "..\Global\global.h"

#define SmashObject ((__int32 (__cdecl*)(__int16)) 0x00465200)
#define LaraGun ((void (__cdecl*)()) 0x00452430)
#define FindTargetPoint ((void (__cdecl*)(ITEM_INFO*, GAME_VECTOR*)) 0x004533A0)
#define HitTarget ((__int16  (__cdecl*)(ITEM_INFO*, GAME_VECTOR*, __int32, __int32)) 0x00453930)

extern WEAPON_INFO Weapons[NUM_WEAPONS];

void __cdecl SmashItem(__int16 itemNum);

void Inject_LaraFire();