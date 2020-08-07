#pragma once

#include "collide.h"

void InitialisePickup(short itemNumber);
void PickedUpObject(short objectNumber);
void RemoveObjectFromInventory(short objectNumber, int count);
void CollectCarriedItems(ITEM_INFO* item);
int PickupTrigger(short itemNum);
int KeyTrigger(short itemNum);
void PuzzleHoleCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
void PuzzleDoneCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
void KeyHoleCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
void PickupCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
void RegeneratePickups();
BOUNDING_BOX* FindPlinth(ITEM_INFO* item);
void PuzzleDone(ITEM_INFO* item, short itemNum);
void PickupControl(short itemNum);

void InitialiseSearchObject(short itemNumber);
void SearchObjectCollision(short itemNumber, ITEM_INFO* laraitem, COLL_INFO* laracoll);
void SearchObjectControl(short itemNumber);
void do_pickup();
void do_puzzle();
