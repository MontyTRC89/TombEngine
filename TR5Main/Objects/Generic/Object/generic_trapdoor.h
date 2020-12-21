#pragma once
#include "items.h"
#include "collide.h"

void InitialiseTrapDoor(short itemNumber);
void TrapDoorCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* coll);
void CeilingTrapDoorCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* coll);
void FloorTrapDoorCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* coll);
void TrapDoorControl(short itemNumber);
void CloseTrapDoor(ITEM_INFO* item);
void OpenTrapDoor(ITEM_INFO* item);
