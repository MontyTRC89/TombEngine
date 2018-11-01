#pragma once

#define		HARPOON_DRAW_ANIM		1
#define		ROCKET_DRAW_ANIM		0

#define 	PELLET_SCATTER  		ANGLE(20)
#define 	HARPOON_SPEED			256
#define 	HARPOON_TIME			256
#define 	ROCKET_SPEED 			512
#define 	GRENADE_SPEED 			128
#define		MAX_GRENADE_FALLSPEED	128
#define	GRENADE_YOFF	180
#define	GRENADE_ZOFF	80

#define ReadyShotgun ((void (__cdecl*)(__int32)) 0x0044DC30)  
#define DrawShotgunMeshes ((void (__cdecl*)(__int32)) 0x0044DBB0)  
#define FireCrossbow ((void (__cdecl*)(PHD_3DPOS*)) 0x0044E4B0)  
#define FireHK ((void (__cdecl*)(__int32)) 0x0044E4B0)  
#define FireShotgun ((void (__cdecl*)()) 0x0044E110)   
#define UndrawShotgun ((void (__cdecl*)(__int32)) 0x0044ECA0)  
#define RifleHandler ((void (__cdecl*)(__int32)) 0x0044DCC0)  

void __cdecl FireGrenade();
void __cdecl ControlGrenade(__int16 itemNumber);

void __cdecl FireHarpoon();
void __cdecl ControlHarpoonBolt(__int16 itemNumber);

void __cdecl DrawShotgun(__int32 weaponType);
void __cdecl AnimateShotgun(__int32 weaponType);

void __cdecl Inject_Lara1Gun();
