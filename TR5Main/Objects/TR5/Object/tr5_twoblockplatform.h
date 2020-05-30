#pragma once
#include "items.h"

void InitialiseTwoBlocksPlatform(short itemNumber);
void TwoBlocksPlatformControl(short itemNumber);
void TwoBlocksPlatformFloor(ITEM_INFO* item, int x, int y, int z, int* height);
void TwoBlocksPlatformCeiling(ITEM_INFO* item, int x, int y, int z, int* height);
BOOL IsOnTwoBlocksPlatform(ITEM_INFO* item, int x, int z);