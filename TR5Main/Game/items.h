#pragma once
#include "..\Global\global.h"

//#define ItemNewRoom ((void (__cdecl*)(__int16, __int16)) 0x00440DA0)
//#define EffectNewRoom ((void (__cdecl*)(__int16, __int16)) 0x004412F0)
//#define KillEffect ((void (__cdecl*)(__int16)) 0x00441180)
//#define CreateNewEffect ((__int16 (__cdecl*)(__int16)) 0x004410F0)
//#define InitialiseFXArray ((void (__cdecl*)(__int32)) 0x00441080)
//#define AddActiveItem ((void (__cdecl*)(__int16)) 0x00440D10)
//#define RemoveActiveItem ((void (__cdecl*)(__int16)) 0x00440B60)
//#define RemoveDrawnItem ((void (__cdecl*)(__int16)) 0x00440C40)
//#define InitialiseItem ((void (__cdecl*)(__int16)) 0x004408B0)
//#define CreateItem ((__int16 (__cdecl*)()) 0x00440840)
//#define KillItem ((void (__cdecl*)(__int16)) 0x00440620)
#define ItemNearLara ((__int32 (__cdecl*)(PHD_3DPOS*, __int32)) 0x00432580)
#define TranslateItem ((void (__cdecl*)(ITEM_INFO*, __int32, __int32, __int32)) 0x00415960)

void __cdecl EffectNewRoom(__int16 fxNumber, __int16 roomNumber);
void __cdecl ItemNewRoom(__int16 itemNum, __int16 roomNumber);
void __cdecl AddActiveItem(__int16 itemNumber);
void __cdecl ClearItem(__int16 itemNum);
__int16 __cdecl CreateItem();
void __cdecl RemoveAllItemsInRoom(__int16 roomNumber, __int16 objectNumber);
void __cdecl RemoveActiveItem(__int16 itemNum);
void __cdecl RemoveDrawnItem(__int16 itemNum);
void __cdecl InitialiseFXArray(__int32 allocmem);
__int16 __cdecl CreateNewEffect(__int16 roomNum);
void __cdecl KillEffect(__int16 fxNumber);
void __cdecl InitialiseItem(__int16 itemNum);
__int16 __cdecl CreateItem();
void __cdecl InitialiseItemArray(__int32 numitems);
void __cdecl KillItem(__int16 itemNum);

void Inject_Items();