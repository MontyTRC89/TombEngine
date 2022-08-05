#pragma once
#include <optional>

struct CollisionInfo;
struct ItemInfo;

void FallingBlockCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
void FallingBlockControl(short itemNumber);
std::optional<int> FallingBlockFloor(short itemNumber, int x, int y, int z);
std::optional<int> FallingBlockCeiling(short itemNumber, int x, int y, int z);
int FallingBlockFloorBorder(short itemNumber);
int FallingBlockCeilingBorder(short itemNumber);
