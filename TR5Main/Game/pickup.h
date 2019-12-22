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

void InitialiseSearchObject(short itemNumber);
void SearchObjectCollision(short itemNumber, ITEM_INFO* laraitem, COLL_INFO* laracoll);
void SearchObjectControl(short itemNumber);

void Inject_Pickup();

