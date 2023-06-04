#pragma once
#include "Game/control/box.h"

extern std::vector<CreatureInfo*> ActiveCreatures;

void InitializeLOTarray(int allocMem);
bool EnableEntityAI(short itemNum, bool always, bool makeTarget = true);
void InitializeSlot(short itemNum, bool makeTarget);
void SetEntityTarget(short itemNum, short target);
void DisableEntityAI(short itemNumber);
void ClearLOT(LOTInfo* LOT);
void CreateZone(ItemInfo* item);
