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
void PickedUpObject(GAME_OBJECT_ID objectID, int count);
void RemoveObjectFromInventory(GAME_OBJECT_ID objectID, int count);
int GetInventoryCount(GAME_OBJECT_ID objectID);
void CollectCarriedItems(ITEM_INFO* item);
void PickupCollision(short itemNumber, ITEM_INFO* laraItem, COLL_INFO* coll);
void RegeneratePickups();
BOUNDING_BOX* FindPlinth(ITEM_INFO* item);

void PickupControl(short itemNumber);

void InitialiseSearchObject(short itemNumber);
void SearchObjectCollision(short itemNumber, ITEM_INFO* laraItem, COLL_INFO* coll);
void SearchObjectControl(short itemNumber);
void DoPickup(ITEM_INFO* laraItem);
bool UseSpecialItem(ITEM_INFO* item);
