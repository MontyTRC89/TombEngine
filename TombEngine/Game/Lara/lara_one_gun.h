#pragma once
#include "Math/Math.h"

using namespace TEN::Math;

enum class LaraWeaponType;
struct ItemInfo;

enum class GrenadeType
{
	Normal,
	Super,
	Flash,
	Ultra,
	Flags
};

enum class ProjectileType
{
	Normal,
	Poison,
	Harpoon,
	Explosive,
	Grenade,
	FragGrenade,
	FlashGrenade
};

void AnimateShotgun(ItemInfo& laraItem, LaraWeaponType weaponType);
void ReadyShotgun(ItemInfo& laraItem, LaraWeaponType weaponType);
void FireShotgun(ItemInfo& laraItem);
void DrawShotgun(ItemInfo& laraItem, LaraWeaponType weaponType);
void UndrawShotgun(ItemInfo& laraItem, LaraWeaponType weaponType);
void DrawShotgunMeshes(ItemInfo& laraItem, LaraWeaponType weaponType);
void UndrawShotgunMeshes(ItemInfo& laraItem, LaraWeaponType weaponType);

bool FireHarpoon(ItemInfo& laraItem, const std::optional<Pose>& pose = std::nullopt);
void HarpoonBoltControl(short itemNumber);
void FireGrenade(ItemInfo& laraItem);
void GrenadeControl(short itemNumber);
void FireRocket(ItemInfo& laraItem);
void FireRocket(ItemInfo& laraItem);
void RocketControl(short itemNumber);
void FireCrossbow(ItemInfo& laraItem, const std::optional<Pose>& pose = std::nullopt);
void FireCrossBowFromLaserSight(ItemInfo& laraItem, GameVector* origin, GameVector* target);
void CrossbowBoltControl(short itemNumber);

void FireHK(ItemInfo& laraItem, int mode);
void RifleHandler(ItemInfo& laraItem, LaraWeaponType weaponType);
void LasersightWeaponHandler(ItemInfo& item, LaraWeaponType weaponType);

void DoExplosiveDamage(ItemInfo& emitter, ItemInfo& target, ItemInfo& projectile, int damage);
void HandleProjectile(ItemInfo& projectile, ItemInfo& emitter, const Vector3i& prevPos, ProjectileType type, int damage);
void SomeSparkEffect(int x, int y, int z, int count);
