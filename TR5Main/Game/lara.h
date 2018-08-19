#pragma once

#include "..\Global\global.h"

typedef struct LaraExtraInfo {
	bool drawWeapon;
	bool overrideMeshes[15];
	__int32 overrideObject;
};

extern LaraExtraInfo g_LaraExtra;

#define LaraControl ((__int32 (__cdecl*)()) 0x00455830)
#define UpdateLaraRoom ((__int32 (__cdecl*)(ITEM_INFO*, __int32)) 0x004120F0)

void lara_as_walk(ITEM_INFO* item, COLL_INFO* coll);
void __cdecl j_AnimateLara(ITEM_INFO* item);
void __cdecl AnimateLaraNew(ITEM_INFO* item);

void Inject_Lara();