#pragma once

#include "..\Global\global.h"

//#define DisableBaddieAI ((void (__cdecl*)(short)) 0x0045B150)
//#define CreateZone ((void (__cdecl*)(ITEM_INFO*)) 0x0045B5E0)
//#define ClearLOT ((void (__cdecl*)(LOT_INFO*)) 0x0045B740)
//#define SameZoneAIObject ((int (__cdecl*)(CREATURE_INFO*, short)) 0x0040C070)

void __cdecl InitialiseLOTarray(int allocMem);
int __cdecl EnableBaddieAI(short itemNum, int always);
void __cdecl InitialiseSlot(short itemNum, short slot);
void __cdecl DisableBaddieAI(short itemNumber);
void __cdecl ClearLOT(LOT_INFO* LOT);
void __cdecl CreateZone(ITEM_INFO* item);

void Inject_Lot();