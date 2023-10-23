#pragma once

class Vector3i;
struct ItemInfo;

void InitializeExpandingPlatform(short itemNumber);
void ControlExpandingPlatform(short itemNumber);

std::optional<int> GetExpandingPlatformFloorHeight(const ItemInfo& item, const Vector3i& pos);
std::optional<int> GetExpandingPlatformCeilingHeight(const ItemInfo& item, const Vector3i& pos);
int ExpandingPlatformFloorBorder(const ItemInfo& item);
int ExpandingPlatformCeilingBorder(const ItemInfo& item);
