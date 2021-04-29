#pragma once
#include "items.h"
#include "trmath.h"

enum GRENADE_TYPE
{
	GRENADE_NORMAL,
	GRENADE_SUPER,
	GRENADE_FLASH,
	GRENADE_ULTRA,
	GRENADE_FLAGS
};

#define PELLET_SCATTER ANGLE(20.0f)
constexpr auto HARPOON_DRAW_ANIM = 1;
constexpr auto ROCKET_DRAW_ANIM = 0;
constexpr auto HARPOON_SPEED = 256;
constexpr auto HARPOON_TIME = 256;
constexpr auto ROCKET_SPEED = 512;
constexpr auto GRENADE_SPEED = 128;
constexpr auto MAX_GRENADE_FALLSPEED = 128;
constexpr auto GRENADE_YOFF = 180;
constexpr auto GRENADE_ZOFF = 80;
constexpr auto CROSSBOW_DAMAGE = 5;
constexpr auto CROSSBOW_AMMO1 = 1;
constexpr auto CROSSBOW_AMMO2 = 2;
constexpr auto CROSSBOW_AMMO3 = 2;
constexpr auto CROSSBOW_HIT_RADIUS = 128;
constexpr auto CROSSBOW_EXPLODE_RADIUS = SECTOR(2);
constexpr auto GRENADE_HIT_RADIUS = 128;
constexpr auto GRENADE_EXPLODE_RADIUS = SECTOR(2);
constexpr auto ROCKET_HIT_RADIUS = 128;
constexpr auto ROCKET_EXPLODE_RADIUS = SECTOR(2);
constexpr auto HARPOON_HIT_RADIUS = 128;

void FireGrenade();
void ControlGrenade(short itemNumber);
//void GrenadeExplosionEffects(int x, int y, int z, short roomNumber);
void FireHarpoon();
void ControlHarpoonBolt(short itemNumber);
void draw_shotgun(int weaponType);
void AnimateShotgun(int weaponType);
void ControlCrossbowBolt(short itemNumber);
void FireCrossbow(PHD_3DPOS* pos);
void RifleHandler(int weaponType);
void DoExplosiveDamageOnBaddie(ITEM_INFO* src, ITEM_INFO* dest, int weapon);
void TriggerUnderwaterExplosion(ITEM_INFO* item, int flag);
void undraw_shotgun_meshes(int weapon);
void undraw_shotgun(int weapon);
void draw_shotgun_meshes(int weaponType);
void FireHK(int mode);
void FireShotgun();
void HitSpecial(ITEM_INFO* projectile, ITEM_INFO* target, int flags);
void ready_shotgun(int weaponType);
void SomeSparkEffect(int x, int y, int z, int count);
void FireRocket();
void ControlRocket(short itemNumber);