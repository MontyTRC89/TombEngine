#pragma once

#include "..\Global\global.h"

typedef struct CarriedWeaponInfo {
	bool Present;
	__int16 Ammo[3];
	byte SelectedAmmo;
	bool HasLasersight;
	bool HasSilencer;
};

typedef struct DiaryInfo {
	bool Present;
};

typedef struct WaterskinInfo {
	bool Present;
	__int32 Quantity;
};

typedef struct LaraExtraInfo {
	__int16 Vehicle;
	__int16 ExtraAnim;
	CarriedWeaponInfo Weapons[NUM_WEAPONS];
	DiaryInfo Diary;
	WaterskinInfo Waterskin1;
	WaterskinInfo Waterskin2;
};

extern LaraExtraInfo g_LaraExtra;

//#define LookUpDown ((void (__cdecl*)()) 0x0044D310)
//#define LookLeftRight ((void (__cdecl*)()) 0x0044D440)
//#define ResetLook ((void (__cdecl*)()) 0x0044D220)
#define UpdateLaraRoom ((__int32 (__cdecl*)(ITEM_INFO*, __int32)) 0x004120F0)
#define LaraControl ((__int32 (__cdecl*)()) 0x00455830)
#define GetLaraJointPosition ((void (__cdecl*)(PHD_VECTOR*, __int32)) 0x0041E2A0)
#define CheckForHoldingState ((__int32 (__cdecl*)(__int16)) 0x00452AF0)
#define AnimateLara ((__int32 (__cdecl*)(ITEM_INFO*)) 0x004563F0)

#define LaraFloorFront ((__int32 (__cdecl*)(ITEM_INFO*, __int16, __int32)) 0x004438F0)
#define LaraCeilingFront ((__int32 (__cdecl*)(ITEM_INFO*, __int16, __int32, __int32)) 0x00442DB0)
#define GetLaraCollisionInfo ((__int32 (__cdecl*)(ITEM_INFO*, COLL_INFO*)) 0x00444F80)
#define TestLaraVault ((__int32 (__cdecl*)(ITEM_INFO*, COLL_INFO*)) 0x00445100)
#define ShiftItem ((__int32 (__cdecl*)(ITEM_INFO*, COLL_INFO*)) 0x004120A0)
//#define lara_as_wade ((void (__cdecl*)(ITEM_INFO*, COLL_INFO*)) 0x0044B770)
//#define lara_as_back ((void (__cdecl*)(ITEM_INFO*, COLL_INFO*)) 0x0044AE20)
//#define lara_as_run ((void (__cdecl*)(ITEM_INFO*, COLL_INFO*)) 0x00449330)
//#define lara_as_walk ((void (__cdecl*)(ITEM_INFO*, COLL_INFO*)) 0x00449260)

extern void(*lara_control_routines[NUM_LARA_STATES + 1])(ITEM_INFO* item, COLL_INFO* coll);
extern void(*lara_collision_routines[NUM_LARA_STATES + 1])(ITEM_INFO* item, COLL_INFO* coll);

void __cdecl LaraAboveWater(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl lara_as_stop(ITEM_INFO* item, COLL_INFO* coll);
__int32 __cdecl UseSpecialItem(ITEM_INFO* item);
void __cdecl LookUpDown();
void __cdecl LookLeftRight();
void __cdecl ResetLook();

void Inject_Lara();