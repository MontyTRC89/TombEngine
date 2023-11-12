#pragma once

class Vector3i;
struct CollisionInfo;
struct ItemInfo;

void InitializeFallingBlock(short itemNumber);
void FallingBlockCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
void FallingBlockControl(short itemNumber);

std::optional<int> GetFallingBlockFloorHeight(const ItemInfo& item, const Vector3i& pos);
std::optional<int> GetFallingBlockCeilingHeight(const ItemInfo& item, const Vector3i& pos);
int GetFallingBlockFloorBorder(const ItemInfo& item);
int GetFallingBlockCeilingBorder(const ItemInfo& item);
