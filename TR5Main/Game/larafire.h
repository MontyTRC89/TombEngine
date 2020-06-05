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
};
extern WEAPON_INFO Weapons[NUM_WEAPONS];

void SmashItem(short itemNum);
int WeaponObject(int weaponType);
void LaraGun();
short* GetAmmo(int weaponType);
void InitialiseNewWeapon();
int WeaponObjectMesh(int weaponType);
void AimWeapon(WEAPON_INFO* winfo, LARA_ARM* arm);
void HitTarget(ITEM_INFO* item, GAME_VECTOR* hitPos, int damage, int flag);
FireWeaponType FireWeapon(int weaponType, ITEM_INFO* target, ITEM_INFO* src, short* angles);
void find_target_point(ITEM_INFO* item, GAME_VECTOR* target);
void LaraTargetInfo(WEAPON_INFO* weapon);
bool CheckForHoldingState(int state);
void LaraGetNewTarget(WEAPON_INFO* weapon);
void DoProperDetection(short itemNumber, int x, int y, int z, int xv, int yv, int zv);