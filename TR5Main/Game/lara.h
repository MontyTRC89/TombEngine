#pragma once

#include "..\Global\global.h"

typedef struct LaraExtraInfo {
	__int16 vehicle;
	bool hasHarpoon;
	__int16 harpoonAmmo;
	__int16 numHarpoonAmmos;
	bool hasGrenadeLauncher;
	__int16 grenadeAmmo;
	__int16 numGrenadeAmmos;
	bool hasRocketLauncher;
	__int16 rocketAmmo;
	__int16 numRocketAmmos;
};

extern LaraExtraInfo g_LaraExtra;

#define LookUpDown ((void (__cdecl*)()) 0x0044D310)
#define LookLeftRight ((void (__cdecl*)()) 0x0044D440)
#define ResetLook ((void (__cdecl*)()) 0x0044D220)
#define UpdateLaraRoom ((__int32 (__cdecl*)(ITEM_INFO*, __int32)) 0x004120F0)
#define LaraControl ((__int32 (__cdecl*)()) 0x00455830)
//#define LaraGun ((void (__cdecl*)()) 0x00452430)
#define GetLaraJointPosition ((void (__cdecl*)(PHD_VECTOR*, __int32)) 0x0041E2A0)
#define CheckForHoldingState ((__int32 (__cdecl*)(__int16)) 0x00452AF0)

void lara_as_walk(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl j_AnimateLara(ITEM_INFO* item);
void __cdecl AnimateLaraNew(ITEM_INFO* item);
void __cdecl LaraAboveWater(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl LaraGun();

void Inject_Lara();