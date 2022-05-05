#pragma once
#include "Game/control/box.h"

extern std::vector<CreatureInfo*> ActiveCreatures;

void InitialiseLOTarray(int allocMem);
int EnableBaddieAI(short itemNum, int always);
void InitialiseSlot(short itemNum, short slot);
void DisableEntityAI(short itemNumber);
void ClearLOT(LOTInfo* LOT);
void CreateZone(ItemInfo* item);
