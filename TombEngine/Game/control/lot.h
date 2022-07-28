#pragma once
#include "Game/control/box.h"

extern std::vector<CreatureInfo*> ActiveCreatures;

void InitialiseLOTarray(int allocMem);
int EnableEntityAI(short itemNum, int always, bool makeTarget = true);
void InitialiseSlot(short itemNum, short slot, bool makeTarget);
void SetBaddyTarget(short itemNum, short target);
void DisableEntityAI(short itemNumber);
void ClearLOT(LOTInfo* LOT);
void CreateZone(ItemInfo* item);
