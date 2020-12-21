#pragma once
#include "box.h"

constexpr auto NUM_SLOTS = 32;
extern int SlotsUsed;
extern std::vector<CREATURE_INFO> BaddieSlots;

void InitialiseLOTarray(int allocMem);
int EnableBaddieAI(short itemNum, int always);
void InitialiseSlot(short itemNum, short slot);
void DisableBaddieAI(short itemNumber);
void ClearLOT(LOT_INFO* LOT);
void CreateZone(ITEM_INFO* item);
