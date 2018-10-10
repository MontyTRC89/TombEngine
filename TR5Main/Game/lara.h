#pragma once

#include "..\Global\global.h"

typedef struct LaraExtraInfo {
	bool drawWeapon;
	bool overrideMeshes[15];
	__int32 overrideObject;
};

extern __int16 LaraVehicle;

#define LookUpDown ((void (__cdecl*)()) 0x0044D310)
#define LookLeftRight ((void (__cdecl*)()) 0x0044D440)
#define ResetLook ((void (__cdecl*)()) 0x0044D220)
#define UpdateLaraRoom ((__int32 (__cdecl*)(ITEM_INFO*, __int32)) 0x004120F0)
#define LaraControl ((__int32 (__cdecl*)()) 0x00455830)
#define LaraGun ((void (__cdecl*)()) 0x00452430)

void lara_as_walk(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl j_AnimateLara(ITEM_INFO* item);
void __cdecl AnimateLaraNew(ITEM_INFO* item);
void __cdecl LaraAboveWater(ITEM_INFO* item, COLL_INFO* coll);

void Inject_Lara();