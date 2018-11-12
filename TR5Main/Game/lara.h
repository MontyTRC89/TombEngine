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
	__int16 extraAnim;
};

extern LaraExtraInfo g_LaraExtra;

#define LookUpDown ((void (__cdecl*)()) 0x0044D310)
#define LookLeftRight ((void (__cdecl*)()) 0x0044D440)
#define ResetLook ((void (__cdecl*)()) 0x0044D220)
#define UpdateLaraRoom ((__int32 (__cdecl*)(ITEM_INFO*, __int32)) 0x004120F0)
#define LaraControl ((__int32 (__cdecl*)()) 0x00455830)
#define GetLaraJointPosition ((void (__cdecl*)(PHD_VECTOR*, __int32)) 0x0041E2A0)
#define CheckForHoldingState ((__int32 (__cdecl*)(__int16)) 0x00452AF0)
#define AnimateLara ((__int32 (__cdecl*)(ITEM_INFO*)) 0x004563F0)

void __cdecl LaraAboveWater(ITEM_INFO* item, COLL_INFO* coll);

void Inject_Lara();