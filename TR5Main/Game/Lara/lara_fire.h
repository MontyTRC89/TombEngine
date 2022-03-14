#pragma once
#include "Game/Lara/lara.h"

struct ITEM_INFO;
struct CollisionInfo;

constexpr auto MAX_TARGETS = 8;

	enum class FireWeaponType
{
	Miss = -1,
	NoAmmo = 0,
	PossibleHit = 1
};

struct WeaponInfo
{
	short LockAngles[4];
	short LeftAngles[4];
	short RightAngles[4];
	int AimSpeed;
	short ShotAccuracy;
	int GunHeight;
	short TargetDist;
	byte Damage;
	byte RecoilFrame;
	byte FlashTime;
	byte DrawFrame;
	short SampleNum;
	byte ExplosiveDamage;
};

enum WeaponState
{
	WEAPON_STATE_AIM = 0,
	WEAPON_STATE_DRAW = 1,
	WEAPON_STATE_RECOIL = 2,
	// 3?
	WEAPON_STATE_UNAIM = 4,
	// 5?
	WEAPON_STATE_UNDERWATER_AIM = 6,
	WEAPON_STATE_UNDERWATER_UNAIM = 7,
	WEAPON_STATE_UNDERWATER_RECOIL = 8
};

extern WeaponInfo Weapons[(int)LaraWeaponType::Total];

void SmashItem(short itemNum);
GAME_OBJECT_ID WeaponObject(LaraWeaponType weaponType);
void LaraGun(ITEM_INFO* laraItem);
Ammo& GetAmmo(ITEM_INFO* laraItem, LaraWeaponType weaponType);
void InitialiseNewWeapon(ITEM_INFO* laraItem);
GAME_OBJECT_ID WeaponObjectMesh(ITEM_INFO* laraItem, LaraWeaponType weaponType);
void AimWeapon(ITEM_INFO* laraItem, WeaponInfo* weaponInfo, ArmInfo* arm);
void HitTarget(ITEM_INFO* laraItem, ITEM_INFO* target, GAME_VECTOR* hitPos, int damage, int flag);
FireWeaponType FireWeapon(LaraWeaponType weaponType, ITEM_INFO* target, ITEM_INFO* src, short* angles);
void FindTargetPoint(ITEM_INFO* laraItem, GAME_VECTOR* target);
void LaraTargetInfo(ITEM_INFO* laraItem, WeaponInfo* weaponInfo);
bool CheckForHoldingState(LaraState state);
void LaraGetNewTarget(ITEM_INFO* laraItem, WeaponInfo* weaponInfo);
HolsterSlot HolsterSlotForWeapon(LaraWeaponType weaponType);
