#pragma once

#include "..\Global\global.h"

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

//#define ReadyShotgun ((void (__cdecl*)(__int32)) 0x0044DC30)  
//#define DrawShotgunMeshes ((void (__cdecl*)(__int32)) 0x0044DBB0)  
//#define UndrawShotgunMeshes ((void (__cdecl*)(__int32)) 0x0044DBF0)  
//#define FireHK ((void (__cdecl*)(__int32)) 0x0044E4B0)  
//#define FireShotgun ((void (__cdecl*)()) 0x0044E110)   
//#define UndrawShotgun ((void (__cdecl*)(__int32)) 0x0044ECA0)  
//#define CrossbowHitSwitchType78 ((void (__cdecl*)(ITEM_INFO*, ITEM_INFO*, __int32)) 0x0044E5E0)  
//#define DoGrenadeDamageOnBaddie ((void (__cdecl*)(ITEM_INFO*, ITEM_INFO*)) 0x0044F690)  

void __cdecl FireGrenade();
void __cdecl ControlGrenade(__int16 itemNumber);
void __cdecl GrenadeExplosionEffects(__int32 x, __int32 y, __int32 z, __int16 roomNumber);
void __cdecl FireHarpoon();
void __cdecl ControlHarpoonBolt(__int16 itemNumber);
void __cdecl DrawShotgun(__int32 weaponType);
void __cdecl AnimateShotgun(__int32 weaponType);
void __cdecl ControlCrossbowBolt(__int16 itemNumber);
void __cdecl FireCrossbow(PHD_3DPOS* pos);
void __cdecl RifleHandler(__int32 weaponType);
void __cdecl DoGrenadeDamageOnBaddie(ITEM_INFO* src, ITEM_INFO* dest);
void __cdecl TriggerUnderwaterExplosion(ITEM_INFO* item);
void __cdecl undraw_shotgun_meshes(__int32 weapon);
void __cdecl undraw_shotgun(__int32 weapon);
void __cdecl draw_shotgun_meshes(__int32 weaponType);
void __cdecl FireHK(__int32 mode);
void __cdecl FireShotgun();
void __cdecl CrossbowHitSwitchType78(ITEM_INFO* item1, ITEM_INFO* item2, signed int search);
void __cdecl ready_shotgun(__int32 weaponType);

void __cdecl Inject_Lara1Gun();
