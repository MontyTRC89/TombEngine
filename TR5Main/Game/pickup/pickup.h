#pragma once

#include "collide.h"

enum GAME_OBJECT_ID : short;

extern int NumRPickups;
extern short RPickups[16];
extern PHD_VECTOR OldPickupPos;

void InitialisePickup(short itemNumber);
void PickedUpObject(GAME_OBJECT_ID objectNumber, int count);
void RemoveObjectFromInventory(GAME_OBJECT_ID objectNumber, int count);
int GetInventoryCount(GAME_OBJECT_ID objID);
void CollectCarriedItems(ITEM_INFO* item);
int PickupTrigger(short itemNum);
void PickupCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
void RegeneratePickups();
BOUNDING_BOX* FindPlinth(ITEM_INFO* item);

void PickupControl(short itemNum);

void InitialiseSearchObject(short itemNumber);
void SearchObjectCollision(short itemNumber, ITEM_INFO* laraitem, COLL_INFO* laracoll);
void SearchObjectControl(short itemNumber);
void do_pickup();
int UseSpecialItem(ITEM_INFO* item);