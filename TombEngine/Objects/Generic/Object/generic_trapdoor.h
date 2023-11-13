#pragma once

class Vector3i;
struct CollisionInfo;
struct ItemInfo;

void InitializeTrapDoor(short itemNumber);
void TrapDoorCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
void CeilingTrapDoorCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
void FloorTrapDoorCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
void TrapDoorControl(short itemNumber);
void CloseTrapDoor(short itemNumber);
void OpenTrapDoor(short itemNumber);

std::optional<int> GetTrapDoorFloorHeight(const ItemInfo& item, const Vector3i& pos);
std::optional<int> GetTrapDoorCeilingHeight(const ItemInfo& item, const Vector3i& pos);
int GetTrapDoorFloorBorder(const ItemInfo& item);
int GetTrapDoorCeilingBorder(const ItemInfo& item);
