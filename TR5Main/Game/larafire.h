#pragma once

#include "..\Global\global.h"

//#define SmashObject ((int (__cdecl*)(short)) 0x00465200)
//#define FindTargetPoint ((void (__cdecl*)(ITEM_INFO*, GAME_VECTOR*)) 0x004533A0)
//#define HitTarget ((short  (__cdecl*)(ITEM_INFO*, GAME_VECTOR*, int, int)) 0x00453930)
#define DoProperDetection ((void  (__cdecl*)(short, int, int, int, int, int)) 0x00453BE0)
#define DoFlameTorch ((void  (__cdecl*)()) 0x00433EA0)
//#define GetAmmo ((int  (__cdecl*)(int)) 0x004546C0)
#define LaraGetNewTarget ((void (__cdecl*)(WEAPON_INFO*)) 0x00452ED0)
//#define LaraTargetInfo ((void (__cdecl*)(WEAPON_INFO*)) 0x00452CC0)
#define TorchControl ((void (__cdecl*)(short)) 0x00434390)
//#define FireWeapon ((int (__cdecl*)(int,ITEM_INFO*,ITEM_INFO*,short*)) 0x00453580)

extern WEAPON_INFO Weapons[NUM_WEAPONS];

void SmashItem(short itemNum);
int WeaponObject(int weaponType);
void LaraGun();
short* GetAmmo(int weaponType);
void InitialiseNewWeapon();
int WeaponObjectMesh(int weaponType);
void AimWeapon(WEAPON_INFO* winfo, LARA_ARM* arm);
void HitTarget(ITEM_INFO* item, GAME_VECTOR* hitPos, int damage, int flag);
int FireWeapon(int weaponType, ITEM_INFO* target, ITEM_INFO* src, short* angles);
void find_target_point(ITEM_INFO* item, GAME_VECTOR* target);
void LaraTargetInfo(WEAPON_INFO* weapon);

void Inject_LaraFire();