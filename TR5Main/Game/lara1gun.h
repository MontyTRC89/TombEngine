#pragma once
#include "items.h"
#include "trmath.h"

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
constexpr auto GRENADE_EXPLODE_RADIUS = SECTOR(2);

void FireGrenade();
void ControlGrenade(short itemNumber);
void GrenadeExplosionEffects(int x, int y, int z, short roomNumber);
void FireHarpoon();
void ControlHarpoonBolt(short itemNumber);
void draw_shotgun(int weaponType);
void AnimateShotgun(int weaponType);
void ControlCrossbowBolt(short itemNumber);
void FireCrossbow(PHD_3DPOS* pos);
void RifleHandler(int weaponType);
void DoGrenadeDamageOnBaddie(ITEM_INFO* src, ITEM_INFO* dest);
void TriggerUnderwaterExplosion(ITEM_INFO* item, int flag);
void undraw_shotgun_meshes(int weapon);
void undraw_shotgun(int weapon);
void draw_shotgun_meshes(int weaponType);
void FireHK(int mode);
void FireShotgun();
void DoCrossbowDamage(ITEM_INFO* item1, ITEM_INFO* item2, signed int search);
void ready_shotgun(int weaponType);
void SomeSparkEffect(int x, int y, int z, int count);