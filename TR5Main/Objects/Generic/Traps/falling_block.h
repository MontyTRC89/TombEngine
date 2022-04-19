#pragma once
#include <optional>

struct COLL_INFO;
struct ITEM_INFO;

void FallingBlockCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
void FallingBlockControl(short itemNumber);
std::optional<int> FallingBlockFloor(short itemNumber, int x, int y, int z);
std::optional<int> FallingBlockCeiling(short itemNumber, int x, int y, int z);
int FallingBlockFloorBorder(short itemNumber);
int FallingBlockCeilingBorder(short itemNumber);