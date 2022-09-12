#pragma once

enum GAME_OBJECT_ID : short;
struct BOUNDING_BOX;
struct CollisionInfo;
struct ItemInfo;
struct Vector3Int;

extern int NumRPickups;
extern short RPickups[16];
extern Vector3Int OldPickupPos;

void InitialisePickup(short itemNumber);
bool SetInventoryCount(GAME_OBJECT_ID objectID, int count);
void PickedUpObject(GAME_OBJECT_ID objectID, std::optional<int> count = std::nullopt);
void RemoveObjectFromInventory(GAME_OBJECT_ID objectID, std::optional<int> count = std::nullopt);
int GetInventoryCount(GAME_OBJECT_ID objectID);
void CollectCarriedItems(ItemInfo* item);
void PickupCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
void RegeneratePickups();
BOUNDING_BOX* FindPlinth(ItemInfo* item);

void PickupControl(short itemNumber);

void InitialiseSearchObject(short itemNumber);
void SearchObjectCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
void SearchObjectControl(short itemNumber);
void DoPickup(ItemInfo* laraItem);
bool UseSpecialItem(ItemInfo* laraItem);
