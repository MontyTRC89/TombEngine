#pragma once

#include "..\Global\global.h"

//#define _RegeneratePickups ((void (__cdecl*)()) 0x00467AF0)
//#define _PuzzleHoleCollision ((void (__cdecl*)(short, ITEM_INFO*, COLL_INFO*)) 0x00468C70)
#define _PickupCollision ((void (__cdecl*)(short, ITEM_INFO*, COLL_INFO*)) 0x00467C00)
#define _InitialisePickup ((void (__cdecl*)(short)) 0x0043E260)
#define _PickupControl ((void (__cdecl*)(short)) 0x004679D0)

void __cdecl InitialisePickup(short itemNumber);
void __cdecl PickedUpObject(short objectNumber);
void __cdecl RemoveObjectFromInventory(short objectNumber);
void __cdecl CollectCarriedItems(ITEM_INFO* item);
int __cdecl PickupTrigger(short itemNum);
void __cdecl ActivateKey();
void __cdecl ActivateCamera();
int __cdecl KeyTrigger(short itemNum);
void __cdecl PuzzleHoleCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
void __cdecl PuzzleDoneCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
void __cdecl KeyHoleCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
void __cdecl PickupCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
void __cdecl RegeneratePickups();
short* __cdecl FindPlinth(ITEM_INFO* item);
void __cdecl PuzzleDone(ITEM_INFO* item, short itemNum);
void __cdecl PickupControl(short itemNum);

void Inject_Pickup();

