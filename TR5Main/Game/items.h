#pragma once
#include "..\Global\global.h"

//#define ItemNewRoom ((void (__cdecl*)(short, short)) 0x00440DA0)
//#define EffectNewRoom ((void (__cdecl*)(short, short)) 0x004412F0)
//#define KillEffect ((void (__cdecl*)(short)) 0x00441180)
//#define CreateNewEffect ((short (__cdecl*)(short)) 0x004410F0)
//#define InitialiseFXArray ((void (__cdecl*)(int)) 0x00441080)
//#define AddActiveItem ((void (__cdecl*)(short)) 0x00440D10)
//#define RemoveActiveItem ((void (__cdecl*)(short)) 0x00440B60)
//#define RemoveDrawnItem ((void (__cdecl*)(short)) 0x00440C40)
//#define InitialiseItem ((void (__cdecl*)(short)) 0x004408B0)
//#define CreateItem ((short (__cdecl*)()) 0x00440840)
//#define KillItem ((void (__cdecl*)(short)) 0x00440620)
#define ItemNearLara ((int (__cdecl*)(PHD_3DPOS*, int)) 0x00432580)
#define TranslateItem ((void (__cdecl*)(ITEM_INFO*, int, int, int)) 0x00415960)

void __cdecl EffectNewRoom(short fxNumber, short roomNumber);
void __cdecl ItemNewRoom(short itemNum, short roomNumber);
void __cdecl AddActiveItem(short itemNumber);
void __cdecl ClearItem(short itemNum);
short __cdecl CreateItem();
void __cdecl RemoveAllItemsInRoom(short roomNumber, short objectNumber);
void __cdecl RemoveActiveItem(short itemNum);
void __cdecl RemoveDrawnItem(short itemNum);
void __cdecl InitialiseFXArray(int allocmem);
short __cdecl CreateNewEffect(short roomNum);
void __cdecl KillEffect(short fxNumber);
void __cdecl InitialiseItem(short itemNum);
void __cdecl InitialiseItemArray(int numitems);
void __cdecl KillItem(short itemNum);

void Inject_Items();