#pragma once

#include "..\Global\global.h"

#define DisableBaddieAI ((void (__cdecl*)(__int16)) 0x0045B150)
#define CreateZone ((void (__cdecl*)(ITEM_INFO*)) 0x0045B5E0)
#define ClearLOT ((void (__cdecl*)(LOT_INFO*)) 0x0045B740)

void __cdecl InitialiseLOTarray(__int32 allocMem);
__int32 __cdecl EnableBaddieAI(__int16 itemNum, __int32 always);
void __cdecl InitialiseSlot(__int16 itemNum, __int16 slot);

void Inject_Lot();