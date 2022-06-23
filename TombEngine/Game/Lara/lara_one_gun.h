#pragma once
#include "Game/control/control.h"
#include "Specific/trmath.h"

enum class LaraWeaponType;
struct ItemInfo;

#define PELLET_SCATTER ANGLE(20.0f)

constexpr auto HARPOON_DRAW_ANIM = 1;
constexpr auto ROCKET_DRAW_ANIM = 0;
constexpr auto HARPOON_VELOCITY = CLICK(1);
constexpr auto HARPOON_TIME = 10 * FPS;
constexpr auto GRENADE_TIME = 4 * FPS;
constexpr auto ROCKET_TIME = 4.5f * FPS;
constexpr auto EXPLOSION_TRIGGER_TIME = 4 * FPS - 2;
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

void AnimateShotgun(ItemInfo* laraItem, LaraWeaponType weaponType);
void ReadyShotgun(ItemInfo* laraItem, LaraWeaponType weaponType);
void FireShotgun(ItemInfo* laraItem);
void DrawShotgun(ItemInfo* laraItem, LaraWeaponType weaponType);
void UndrawShotgun(ItemInfo* laraItem, LaraWeaponType weaponType);
void DrawShotgunMeshes(ItemInfo* laraItem, LaraWeaponType weaponType);
void UndrawShotgunMeshes(ItemInfo* laraItem, LaraWeaponType weaponType);

void FireHarpoon(ItemInfo* laraItem);
void HarpoonBoltControl(short itemNumber);
void FireGrenade(ItemInfo* laraItem);
void GrenadeControl(short itemNumber);
//void GrenadeExplosionEffects(int x, int y, int z, short roomNumber);
void FireRocket(ItemInfo* laraItem);
void RocketControl(short itemNumber);
void FireCrossbow(ItemInfo* laraItem, PHD_3DPOS* pos);
void CrossbowBoltControl(short itemNumber);
void FireCrossBowFromLaserSight(ItemInfo* laraItem, GameVector* src, GameVector* target);

void FireHK(ItemInfo* laraItem, int mode);
void RifleHandler(ItemInfo* laraItem, LaraWeaponType weaponType);

void DoExplosiveDamageOnBaddy(ItemInfo* laraItem, ItemInfo* src, ItemInfo* dest, LaraWeaponType weaponType);
void SomeSparkEffect(int x, int y, int z, int count);

void HitSpecial(ItemInfo* projectile, ItemInfo* target, int flags);
