#pragma once

#include "..\Global\global.h"

#define SmashObject ((__int32 (__cdecl*)(__int16)) 0x00465200)
//#define LaraGun ((void (__cdecl*)()) 0x00452430)
#define FindTargetPoint ((void (__cdecl*)(ITEM_INFO*, GAME_VECTOR*)) 0x004533A0)
#define HitTarget ((__int16  (__cdecl*)(ITEM_INFO*, GAME_VECTOR*, __int32, __int32)) 0x00453930)
#define DoProperDetection ((void  (__cdecl*)(__int16, __int32, __int32, __int32, __int32, __int32, __int32)) 0x00453BE0)
#define DoFlameTorch ((void  (__cdecl*)()) 0x00433EA0)
//#define InitialiseNewWeapon ((void  (__cdecl*)()) 0x00452B30)
//#define GetAmmo ((__int32  (__cdecl*)(__int32)) 0x004546C0)

extern WEAPON_INFO Weapons[NUM_WEAPONS];

void __cdecl SmashItem(__int16 itemNum);
__int32 __cdecl WeaponObject(__int32 weaponType);
void __cdecl LaraGun();
__int32 __cdecl GetAmmo(__int32 weaponType);
void __cdecl InitialiseNewWeapon();
__int32 __cdecl WeaponObjectMesh(__int32 weaponType);

void Inject_LaraFire();