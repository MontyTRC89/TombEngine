#pragma once
#include "Game/control/box.h"

extern std::vector<CREATURE_INFO*> ActiveCreatures;

void InitialiseLOTarray(int allocMem);
int EnableBaddieAI(short itemNum, int always);
void InitialiseSlot(short itemNum, short slot);
void DisableBaddieAI(short itemNumber);
void ClearLOT(LOT_INFO* LOT);
void CreateZone(ITEM_INFO* item);
