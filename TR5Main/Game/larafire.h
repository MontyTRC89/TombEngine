#pragma once

#include "..\Global\global.h"

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
int CheckForHoldingState(int state);
void LaraGetNewTarget(WEAPON_INFO* winfo);
void DoProperDetection(short itemNumber, int x, int y, int z, int xv, int yv, int zv);