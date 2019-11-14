#pragma once

#include "..\Global\global.h"

//#define Targetable ((__int32 (__cdecl*)(ITEM_INFO*, AI_INFO*)) 0x004672F0) 
//#define TargetVisible ((__int32 (__cdecl*)(ITEM_INFO*, AI_INFO*)) 0x004671E0)
//#define ShotLara ((__int32 (__cdecl*)(ITEM_INFO*, AI_INFO*, BITE_INFO*, __int16, __int32)) 0x00467610)

__int32 __cdecl ShotLara(ITEM_INFO* item, AI_INFO* info, BITE_INFO* gun, __int16 extra_rotation, __int32 damage);
__int16 __cdecl GunMiss(__int32 x, __int32 y, __int32 z, __int16 speed, __int16 yrot, __int16 roomNumber);
__int16 __cdecl GunHit(__int32 x, __int32 y, __int32 z, __int16 speed, __int16 yrot, __int16 roomNumber);
__int16 __cdecl GunShot(__int32 x, __int32 y, __int32 z, __int16 speed, __int16 yrot, __int16 roomNumber);
__int32 __cdecl Targetable(ITEM_INFO* item, AI_INFO* info);
__int32 __cdecl TargetVisible(ITEM_INFO* item, AI_INFO* info);

void Inject_People();