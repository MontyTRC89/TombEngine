#pragma once
#include "Specific/trmath.h"

enum class LaraWeaponType;
struct ITEM_INFO;

#define PELLET_SCATTER Angle::DegToRad(20.0f)

constexpr auto HARPOON_DRAW_ANIM = 1;
constexpr auto ROCKET_DRAW_ANIM = 0;
constexpr auto HARPOON_VELOCITY = CLICK(1);
constexpr auto HARPOON_TIME = 300;
constexpr auto ROCKET_VELOCITY = CLICK(2);
constexpr auto GRENADE_VELOCITY = CLICK(0.5f);
constexpr auto MAX_GRENADE_VERTICAL_VELOCITY = CLICK(0.5f);
constexpr auto GRENADE_Y_OFFSET = 180;
constexpr auto GRENADE_Z_OFFSET = 80;
constexpr auto CROSSBOW_DAMAGE = 5;
constexpr auto CROSSBOW_AMMO1 = 1;
constexpr auto CROSSBOW_AMMO2 = 2;
constexpr auto CROSSBOW_AMMO3 = 2;
constexpr auto CROSSBOW_HIT_RADIUS = CLICK(0.5f);
constexpr auto CROSSBOW_EXPLODE_RADIUS = SECTOR(2);
constexpr auto GRENADE_HIT_RADIUS = CLICK(0.5f);
constexpr auto GRENADE_EXPLODE_RADIUS = SECTOR(2);
constexpr auto ROCKET_HIT_RADIUS = CLICK(0.5f);
constexpr auto ROCKET_EXPLODE_RADIUS = SECTOR(2);
constexpr auto HARPOON_HIT_RADIUS = CLICK(0.5f);

enum class GrenadeType
{
	Normal,
	Super,
	Flash,
	Ultra,
	Flags
};

void AnimateShotgun(ITEM_INFO* laraItem, LaraWeaponType weaponType);
void ReadyShotgun(ITEM_INFO* laraItem, LaraWeaponType weaponType);
void FireShotgun(ITEM_INFO* laraItem);
void DrawShotgun(ITEM_INFO* laraItem, LaraWeaponType weaponType);
void UndrawShotgun(ITEM_INFO* laraItem, LaraWeaponType weaponType);
void DrawShotgunMeshes(ITEM_INFO* laraItem, LaraWeaponType weaponType);
void UndrawShotgunMeshes(ITEM_INFO* laraItem, LaraWeaponType weaponType);

void FireHarpoon(ITEM_INFO* laraItem);
void HarpoonBoltControl(short itemNumber);
void FireGrenade(ITEM_INFO* laraItem);
void GrenadeControl(short itemNumber);
//void GrenadeExplosionEffects(int x, int y, int z, short roomNumber);
void FireRocket(ITEM_INFO* laraItem);
void RocketControl(short itemNumber);
void FireCrossbow(ITEM_INFO* laraItem, PoseData* pos);
void CrossbowBoltControl(short itemNumber);
void FireCrossBowFromLaserSight(ITEM_INFO* laraItem, GameVector* src, GameVector* target);

void FireHK(ITEM_INFO* laraItem, int mode);
void RifleHandler(ITEM_INFO* laraItem, LaraWeaponType weaponType);

void DoExplosiveDamageOnBaddie(ITEM_INFO* laraItem, ITEM_INFO* src, ITEM_INFO* dest, LaraWeaponType weaponType);
void TriggerUnderwaterExplosion(ITEM_INFO* item, int flag);
void SomeSparkEffect(int x, int y, int z, int count);

void HitSpecial(ITEM_INFO* projectile, ITEM_INFO* target, int flags);
