#pragma once
#include "items.h"
#include "collide.h"

void InitialiseTrapDoor(short itemNumber);
void TrapDoorCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* coll);
void CeilingTrapDoorCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* coll);
void FloorTrapDoorCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* coll);
void TrapDoorControl(short itemNumber);
void CloseTrapDoor(short itemNumber, ITEM_INFO* item);
void OpenTrapDoor(short itemNumber, ITEM_INFO* item);
int TrapDoorFloorBorder(short itemNumber);
int TrapDoorCeilingBorder(short itemNumber);
std::optional<int> TrapDoorFloor(short itemNumber, int x, int y, int z);
std::optional<int> TrapDoorCeiling(short itemNumber, int x, int y, int z);