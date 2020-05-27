#pragma once

#include "global.h"

#define		HARPOON_DRAW_ANIM		1
#define		ROCKET_DRAW_ANIM		0

#define 	PELLET_SCATTER  		ANGLE(20)
#define 	HARPOON_SPEED			256
#define 	HARPOON_TIME			256
#define 	ROCKET_SPEED 			512
#define 	GRENADE_SPEED 			128
#define		MAX_GRENADE_FALLSPEED	128
#define		GRENADE_YOFF			180
#define		GRENADE_ZOFF			80
#define		GRENADE_BLAST_RADIUS (WALL_SIZE * 2)
#define		CROSSBOW_DAMAGE			5
#define		CROSSBOW_AMMO1			1
#define		CROSSBOW_AMMO2			2
#define		CROSSBOW_AMMO3			2
#define		CROSSBOW_HIT_RADIUS		128
#define		CROSSBOW_EXPLODE_RADIUS	2048
#define		GRENADE_EXPLODE_RADIUS	2048

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
void TriggerUnderwaterExplosion(ITEM_INFO* item);
void undraw_shotgun_meshes(int weapon);
void undraw_shotgun(int weapon);
void draw_shotgun_meshes(int weaponType);
void FireHK(int mode);
void FireShotgun();
void CrossbowHitSwitchType78(ITEM_INFO* item1, ITEM_INFO* item2, signed int search);
void ready_shotgun(int weaponType);
