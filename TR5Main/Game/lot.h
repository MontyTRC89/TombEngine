#pragma once

#include "..\Global\global.h"

extern int SlotsUsed;
extern CREATURE_INFO* BaddieSlots;

void InitialiseLOTarray(int allocMem);
int EnableBaddieAI(short itemNum, int always);
void InitialiseSlot(short itemNum, short slot);
void DisableBaddieAI(short itemNumber);
void ClearLOT(LOT_INFO* LOT);
void CreateZone(ITEM_INFO* item);

void Inject_Lot();