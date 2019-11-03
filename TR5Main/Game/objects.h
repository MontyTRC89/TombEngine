#pragma once
#include "..\Global\global.h"

#define AnimatingControl ((void (__cdecl*)(__int16)) 0x00465590)
//#define SmashObject ((__int32(__cdecl*)(__int16)) 0x00465200)

void __cdecl SmashObject(__int16 itemNumber);
void __cdecl SmashObjectControl(__int16 itemNumber);
void __cdecl BridgeFlatFloor(ITEM_INFO* item, __int32 x, __int32 y, __int32 z, __int32* height);
void __cdecl BridgeFlatCeiling(ITEM_INFO* item, __int32 x, __int32 y, __int32 z, __int32* height);
__int32 __cdecl GetOffset(ITEM_INFO* item, __int32 x, __int32 z);
void __cdecl BridgeTilt1Floor(ITEM_INFO* item, __int32 x, __int32 y, __int32 z, __int32* height);
void __cdecl BridgeTilt2Floor(ITEM_INFO* item, __int32 x, __int32 y, __int32 z, __int32* height);
void __cdecl BridgeTilt1Ceiling(ITEM_INFO* item, __int32 x, __int32 y, __int32 z, __int32* height);
void __cdecl BridgeTilt2Ceiling(ITEM_INFO* item, __int32 x, __int32 y, __int32 z, __int32* height);
void __cdecl ControlAnimatingSlots(__int16 itemNumber);
void __cdecl PoleCollision(__int16 itemNum, ITEM_INFO* l, COLL_INFO* coll);
void __cdecl ControlTriggerTriggerer(__int16 itemNumber);
void __cdecl AnimateWaterfalls();
void __cdecl ControlWaterfall(__int16 itemNumber);
void __cdecl TightRopeCollision(__int16 itemNum, ITEM_INFO* l, COLL_INFO* coll);
void __cdecl ParallelBarsCollision(__int16 itemNum, ITEM_INFO* l, COLL_INFO* coll);
void __cdecl ControlXRayMachine(__int16 itemNumber);
void __cdecl CutsceneRopeControl(__int16 itemNumber);
void __cdecl HybridCollision(__int16 itemNum, ITEM_INFO* laraitem, COLL_INFO* coll);

void Inject_Objects();