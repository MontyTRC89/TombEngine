#pragma once

struct ITEM_INFO;
struct CollisionInfo;
struct Vector3Int;
struct BOUNDING_BOX;
enum GAME_OBJECT_ID : short;

extern int NumRPickups;
extern short RPickups[16];
extern Vector3Int OldPickupPos;

void InitialisePickup(short itemNumber);
void PickedUpObject(GAME_OBJECT_ID objectID, int count);
void RemoveObjectFromInventory(GAME_OBJECT_ID objectID, int count);
int GetInventoryCount(GAME_OBJECT_ID objectID);
void CollectCarriedItems(ITEM_INFO* item);
void PickupCollision(short itemNumber, ITEM_INFO* laraItem, CollisionInfo* coll);
void RegeneratePickups();
BOUNDING_BOX* FindPlinth(ITEM_INFO* item);

void PickupControl(short itemNumber);

void InitialiseSearchObject(short itemNumber);
void SearchObjectCollision(short itemNumber, ITEM_INFO* laraItem, CollisionInfo* coll);
void SearchObjectControl(short itemNumber);
void DoPickup(ITEM_INFO* laraItem);
bool UseSpecialItem(ITEM_INFO* item);
