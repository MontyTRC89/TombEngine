#pragma once
#include "Game/Lara/lara.h"

struct ITEM_INFO;
struct COLL_INFO;

constexpr auto MAX_TARGETS = 8;

enum FireWeaponType
{
	FW_MISS = -1,
	FW_NOAMMO = 0,
	FW_MAYBEHIT = 1
};

struct WEAPON_INFO
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
void LaraGun(ITEM_INFO* laraItem);
Ammo& GetAmmo(ITEM_INFO* lara, int weaponType);
void InitialiseNewWeapon(ITEM_INFO* lara);
GAME_OBJECT_ID WeaponObjectMesh(ITEM_INFO* lara, int weaponType);
void AimWeapon(ITEM_INFO* lara, WEAPON_INFO* winfo, ArmInfo* arm);
void HitTarget(ITEM_INFO* lara, ITEM_INFO* target, GAME_VECTOR* hitPos, int damage, int flag);
FireWeaponType FireWeapon(LARA_WEAPON_TYPE weaponType, ITEM_INFO* target, ITEM_INFO* src, short* angles);
void find_target_point(ITEM_INFO* item, GAME_VECTOR* target);
void LaraTargetInfo(ITEM_INFO* lara, WEAPON_INFO* weapon);
bool CheckForHoldingState(int state);
void LaraGetNewTarget(ITEM_INFO* lara, WEAPON_INFO* weapon);
HOLSTER_SLOT HolsterSlotForWeapon(LARA_WEAPON_TYPE weapon);
