#pragma once

#include "items.h"
#include "control.h"

int GetKeyTrigger(ITEM_INFO* item);
int GetSwitchTrigger(ITEM_INFO* item, short* itemNos, int AttatchedToSwitch);
int SwitchTrigger(short itemNum, short timer);
short* GetTriggerIndex(FLOOR_INFO* floor, int x, int y, int z);
short* GetTriggerIndex(ITEM_INFO* item);
void TestTriggers(short* data, bool heavy, int heavyFlags);
void TestTriggers(int x, int y, int z, short roomNumber, bool heavy, int heavyFlags);
void TestTriggers(ITEM_INFO* item, bool heavy, int heavyFlags);
void ProcessSectorFlags(FLOOR_INFO* floor);
void ProcessSectorFlags(int x, int y, int z, short roomNumber);
void ProcessSectorFlags(ITEM_INFO* item);