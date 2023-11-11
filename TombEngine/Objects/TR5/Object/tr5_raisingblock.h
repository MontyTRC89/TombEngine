#pragma once

class Vector3i;
struct ItemInfo;

void InitializeRaisingBlock(short itemNumber);
void ControlRaisingBlock(short itemNumber);

std::optional<int> GetRaisingBlockFloorHeight(const ItemInfo& item, const Vector3i& pos);
std::optional<int> GetRaisingBlockCeilingHeight(const ItemInfo& item, const Vector3i& pos);
int GetRaisingBlockFloorBorder(const ItemInfo& item);
int GetRaisingBlockCeilingBorder(const ItemInfo& item);
