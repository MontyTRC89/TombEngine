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
//#define ItemNearLara ((int (__cdecl*)(PHD_3DPOS*, int)) 0x00432580)
//#define TranslateItem ((void (__cdecl*)(ITEM_INFO*, int, int, int)) 0x00415960)

void EffectNewRoom(short fxNumber, short roomNumber);
void ItemNewRoom(short itemNum, short roomNumber);
void AddActiveItem(short itemNumber);
void ClearItem(short itemNum);
short CreateItem();
void RemoveAllItemsInRoom(short roomNumber, short objectNumber);
void RemoveActiveItem(short itemNum);
void RemoveDrawnItem(short itemNum);
void InitialiseFXArray(int allocmem);
short CreateNewEffect(short roomNum);
void KillEffect(short fxNumber);
void InitialiseItem(short itemNum);
void InitialiseItemArray(int numitems);
void KillItem(short itemNum);
ITEM_INFO* find_a_fucking_item(short objectNum);
int FindItem(short objectNum);

void Inject_Items();