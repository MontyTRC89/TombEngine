#pragma once
#include "Game/items.h"
#include "Game/collision/collide_room.h"
#include "Game/control/control.h"

void InitialiseTrapDoor(short itemNumber);
void TrapDoorCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
void CeilingTrapDoorCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
void FloorTrapDoorCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
void TrapDoorControl(short itemNumber);
void CloseTrapDoor(short itemNumber);
void OpenTrapDoor(short itemNumber);
int TrapDoorFloorBorder(short itemNumber);
int TrapDoorCeilingBorder(short itemNumber);
std::optional<int> TrapDoorFloor(short itemNumber, int x, int y, int z);
std::optional<int> TrapDoorCeiling(short itemNumber, int x, int y, int z);
