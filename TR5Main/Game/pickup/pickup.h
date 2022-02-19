#pragma once

struct ITEM_INFO;
struct COLL_INFO;
struct PHD_VECTOR;
struct BOUNDING_BOX;
enum GAME_OBJECT_ID : short;

extern int NumRPickups;
extern short RPickups[16];
extern PHD_VECTOR OldPickupPos;

void InitialisePickup(short itemNumber);
void PickedUpObject(GAME_OBJECT_ID objectNumber, int count);
void RemoveObjectFromInventory(GAME_OBJECT_ID objectNumber, int count);
int GetInventoryCount(GAME_OBJECT_ID objID);
void CollectCarriedItems(ITEM_INFO* item);
void PickupCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
void RegeneratePickups();
BOUNDING_BOX* FindPlinth(ITEM_INFO* item);

void PickupControl(short itemNum);

void InitialiseSearchObject(short itemNumber);
void SearchObjectCollision(short itemNumber, ITEM_INFO* laraitem, COLL_INFO* laracoll);
void SearchObjectControl(short itemNumber);
void DoPickup(ITEM_INFO* character);
bool UseSpecialItem(ITEM_INFO* item);
