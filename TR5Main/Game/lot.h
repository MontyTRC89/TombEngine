#pragma once

#include "..\Global\global.h"

//#define DisableBaddieAI ((void (__cdecl*)(short)) 0x0045B150)
//#define CreateZone ((void (__cdecl*)(ITEM_INFO*)) 0x0045B5E0)
//#define ClearLOT ((void (__cdecl*)(LOT_INFO*)) 0x0045B740)
//#define SameZoneAIObject ((int (__cdecl*)(CREATURE_INFO*, short)) 0x0040C070)

void InitialiseLOTarray(int allocMem);
int EnableBaddieAI(short itemNum, int always);
void InitialiseSlot(short itemNum, short slot);
void DisableBaddieAI(short itemNumber);
void ClearLOT(LOT_INFO* LOT);
void CreateZone(ITEM_INFO* item);

void Inject_Lot();