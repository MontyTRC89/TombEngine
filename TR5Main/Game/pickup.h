#pragma once

#include "..\Global\global.h"

//#define PickupCollision ((void (__cdecl*)(short, ITEM_INFO*, COLL_INFO*)) 0x00467C00)
//#define InitialisePickup ((void (__cdecl*)(short)) 0x0043E260)
//#define PickupControl ((void (__cdecl*)(short)) 0x004679D0)

void InitialisePickup(short itemNumber);
void PickedUpObject(short objectNumber);
void RemoveObjectFromInventory(short objectNumber);
void CollectCarriedItems(ITEM_INFO* item);
int PickupTrigger(short itemNum);
int KeyTrigger(short itemNum);
void PuzzleHoleCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
void PuzzleDoneCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
void KeyHoleCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
void PickupCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
void RegeneratePickups();
short* FindPlinth(ITEM_INFO* item);
void PuzzleDone(ITEM_INFO* item, short itemNum);
void PickupControl(short itemNum);

#define _InitialiseCupboard ((void (__cdecl*)(short)) 0x0043EDB0)
#define _CupboardCollision ((void (__cdecl*)(short,ITEM_INFO*,COLL_INFO*)) 0x004699A0)
#define _CupboardControl ((void (__cdecl*)(short)) 0x00469660)
void InitialiseCupboard(short itemNumber);
void CupboardCollision(short itemNumber, ITEM_INFO* laraitem, COLL_INFO* laracoll);
void CupboardControl(short itemNumber);

void Inject_Pickup();

