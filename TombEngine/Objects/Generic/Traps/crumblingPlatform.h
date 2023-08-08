#pragma once

struct CollisionInfo;
struct ItemInfo;

void InitializeCrumblingPlatform(short itemNumber);
void CrumblingPlatformCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
void CrumblingPlatformControl(short itemNumber);

void CrumblingPlatformActivate(short itemNumber);

std::optional<int> CrumblingPlatformFloor(short itemNumber, int x, int y, int z);
std::optional<int> CrumblingPlatformCeiling(short itemNumber, int x, int y, int z);
int CrumblingPlatformFloorBorder(short itemNumber);
int CrumblingPlatformCeilingBorder(short itemNumber);
