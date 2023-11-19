#pragma once

class Vector3i;
struct ItemInfo;

void InitializeTwoBlockPlatform(short itemNumber);
void TwoBlockPlatformControl(short itemNumber);

std::optional<int> GetTwoBlockPlatformFloorHeight(const ItemInfo& item, const Vector3i& pos);
std::optional<int> GetTwoBlockPlatformCeilingHeight(const ItemInfo& item, const Vector3i& pos);
int GetTwoBlockPlatformFloorBorder(const ItemInfo& item);
int GetTwoBlockPlatformCeilingBorder(const ItemInfo& item);
