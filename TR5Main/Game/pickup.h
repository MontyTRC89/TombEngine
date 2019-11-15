#pragma once

#include "..\Global\global.h"

//#define _RegeneratePickups ((void (__cdecl*)()) 0x00467AF0)
//#define _PuzzleHoleCollision ((void (__cdecl*)(__int16, ITEM_INFO*, COLL_INFO*)) 0x00468C70)
//#define _PickupCollision ((void (__cdecl*)(__int16, ITEM_INFO*, COLL_INFO*)) 0x00467C00)
//#define _InitialisePickup ((void (__cdecl*)(__int16)) 0x0043E260)
//#define _PickupControl ((void (__cdecl*)(__int16)) 0x004679D0)

void __cdecl InitialisePickup(__int16 itemNumber);
void __cdecl PickedUpObject(__int16 objectNumber);
void __cdecl RemoveObjectFromInventory(__int16 objectNumber);
void __cdecl CollectCarriedItems(ITEM_INFO* item);
__int32 __cdecl PickupTrigger(__int16 itemNum);
void __cdecl ActivateKey();
void __cdecl ActivateCamera();
__int32 __cdecl KeyTrigger(__int16 itemNum);
void __cdecl PuzzleHoleCollision(__int16 itemNum, ITEM_INFO* l, COLL_INFO* coll);
void __cdecl PuzzleDoneCollision(__int16 itemNum, ITEM_INFO* l, COLL_INFO* coll);
void __cdecl KeyHoleCollision(__int16 itemNum, ITEM_INFO* l, COLL_INFO* coll);
void __cdecl PickupCollision(__int16 itemNum, ITEM_INFO* l, COLL_INFO* coll);
void __cdecl RegeneratePickups();
__int16* __cdecl FindPlinth(ITEM_INFO* item);
void __cdecl PuzzleDone(ITEM_INFO* item, __int16 itemNum);
void __cdecl PickupControl(__int16 itemNum);

void Inject_Pickup();

