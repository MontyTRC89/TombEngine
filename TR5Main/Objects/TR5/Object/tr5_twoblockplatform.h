#pragma once
#include "items.h"

void InitialiseTwoBlocksPlatform(short itemNumber);
void TwoBlocksPlatformControl(short itemNumber);
BOOL IsOnTwoBlocksPlatform(ITEM_INFO* item, int x, int z);
std::optional<int> TwoBlocksPlatformFloor(short itemNumber, int x, int y, int z);
std::optional<int> TwoBlocksPlatformCeiling(short itemNumber, int x, int y, int z);
int TwoBlocksPlatformFloorBorder(short itemNumber);
int TwoBlocksPlatformCeilingBorder(short itemNumber);