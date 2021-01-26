#pragma once
#include "lara.h"

typedef enum FireWeaponType
{
	FW_MISS = -1,
	FW_NOAMMO = 0,
	FW_MAYBEHIT = 1
};

typedef struct WEAPON_INFO
{
	short lockAngles[4];
	short leftAngles[4];
	short rightAngles[4];
	short aimSpeed;
	short shotAccuracy;
	short gunHeight;
	short targetDist;
	byte damage;
	byte recoilFrame;
	byte flashTime;
	byte drawFrame;
	short sampleNum;
	byte explosiveDamage;
};
enum WeaponState {
	WSTATE_AIM =0,
	WSTATE_DRAW = 1,
	WSTATE_RECOIL = 2,
	WSTATE_UNAIM = 4,
	WSTATE_UW_AIM = 6,
	WSTATE_UW_UNAIM = 7,
	WSTATE_UW_RECOIL = 8
};

extern WEAPON_INFO Weapons[static_cast<int>(LARA_WEAPON_TYPE::NUM_WEAPONS)];

void SmashItem(short itemNum);
GAME_OBJECT_ID WeaponObject(int weaponType);
void LaraGun();
Ammo& GetAmmo(int weaponType);
void InitialiseNewWeapon();
GAME_OBJECT_ID WeaponObjectMesh(int weaponType);
void AimWeapon(WEAPON_INFO* winfo, LARA_ARM* arm);
void HitTarget(ITEM_INFO* item, GAME_VECTOR* hitPos, int damage, int flag);
FireWeaponType FireWeapon(int weaponType, ITEM_INFO* target, ITEM_INFO* src, short* angles);
void find_target_point(ITEM_INFO* item, GAME_VECTOR* target);
void LaraTargetInfo(WEAPON_INFO* weapon);
bool CheckForHoldingState(int state);
void LaraGetNewTarget(WEAPON_INFO* weapon);
void DoProperDetection(short itemNumber, int x, int y, int z, int xv, int yv, int zv);
HOLSTER_SLOT HolsterSlotForWeapon(LARA_WEAPON_TYPE weapon);