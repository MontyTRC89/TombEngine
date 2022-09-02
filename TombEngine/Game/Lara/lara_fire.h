#pragma once
#include "Game/Lara/lara.h"

struct CollisionInfo;
struct ItemInfo;
struct Vector3Shrt;

constexpr auto MAX_TARGETS = 8;

enum class FireWeaponType
{
	Miss = -1,
	NoAmmo = 0,
	PossibleHit = 1
};

struct WeaponInfo
{
	Vector3Shrt LockAngles[2];
	Vector3Shrt LeftAngles[2];
	Vector3Shrt RightAngles[2];

	int AimSpeed;
	short ShotAccuracy;
	int GunHeight;
	short TargetDist;
	int Damage;
	int RecoilFrame;
	int FlashTime;
	int DrawFrame;
	short SampleNum;
	int ExplosiveDamage;
};

enum WeaponState
{
	WEAPON_STATE_AIM = 0,
	WEAPON_STATE_DRAW = 1,
	WEAPON_STATE_RECOIL = 2,
	WEAPON_STATE_UNDRAW = 3,
	WEAPON_STATE_UNAIM = 4,
	WEAPON_STATE_RELOAD = 5,
	WEAPON_STATE_UNDERWATER_AIM = 6,
	WEAPON_STATE_UNDERWATER_UNAIM = 7,
	WEAPON_STATE_UNDERWATER_RECOIL = 8
};

extern WeaponInfo Weapons[(int)LaraWeaponType::NumWeapons];
extern int FlashGrenadeAftershockTimer;

void SmashItem(short itemNum);
GAME_OBJECT_ID WeaponObject(LaraWeaponType weaponType);
void LaraGun(ItemInfo* laraItem);
Ammo& GetAmmo(ItemInfo* laraItem, LaraWeaponType weaponType);
void InitialiseNewWeapon(ItemInfo* laraItem);
GAME_OBJECT_ID WeaponObjectMesh(ItemInfo* laraItem, LaraWeaponType weaponType);
void AimWeapon(ItemInfo* laraItem, WeaponInfo* weaponInfo, ArmInfo* arm);
void HitTarget(ItemInfo* laraItem, ItemInfo* target, GameVector* hitPos, int damage, int flag);
FireWeaponType FireWeapon(LaraWeaponType weaponType, ItemInfo* target, ItemInfo* src, Vector3Shrt armOrient);
void FindTargetPoint(ItemInfo* laraItem, GameVector* target);
void LaraTargetInfo(ItemInfo* laraItem, WeaponInfo* weaponInfo);
void LaraGetNewTarget(ItemInfo* laraItem, WeaponInfo* weaponInfo);
HolsterSlot HolsterSlotForWeapon(LaraWeaponType weaponType);
