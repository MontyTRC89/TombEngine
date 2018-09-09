#pragma once
#include "..\Global\global.h"

//#define ItemNewRoom ((void (__cdecl*)(__int16, __int16)) 0x00440DA0)
#define EffectNewRoom ((void (__cdecl*)(__int16, __int16)) 0x004412F0)
#define KillEffect ((void (__cdecl*)(__int16)) 0x00441180)
#define CreateEffectTR5 ((__int16 (__cdecl*)(__int16)) 0x00440620)
#define InitialiseFXArray ((void (__cdecl*)(__int32)) 0x00441080)
#define AddActiveItem ((void (__cdecl*)(__int16)) 0x00440D10)
#define RemoveActiveItem ((void (__cdecl*)(__int16)) 0x00440D10)
#define InitialiseItem ((void (__cdecl*)(__int16)) 0x004408B0)
#define CreateItem ((__int16 (__cdecl*)()) 0x00440840)
#define KillItem ((void (__cdecl*)(__int16)) 0x00440620)
#define ItemNearLara ((__int32 (__cdecl*)(PHD_3DPOS*, __int32)) 0x00432580)
#define TranslateItem ((void (__cdecl*)(ITEM_INFO*, __int32, __int32, __int32)) 0x00415960)

void __cdecl ClearItem(__int16 itemNum);
void __cdecl ItemNewRoom(__int16 itemNum, __int16 room);

void Inject_Items();