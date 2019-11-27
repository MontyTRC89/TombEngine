#pragma once
#include "..\Global\global.h"

#define InitialiseAnimating ((void (__cdecl*)(short)) 0x00440100)
#define AnimatingControl ((void (__cdecl*)(short)) 0x00465590)

#define InitialiseSmashObject ((void (__cdecl*)(short)) 0x0043D7F0)
void __cdecl SmashObject(short itemNumber);
void __cdecl SmashObjectControl(short itemNumber);
void __cdecl BridgeFlatFloor(ITEM_INFO* item, int x, int y, int z, int* height);
void __cdecl BridgeFlatCeiling(ITEM_INFO* item, int x, int y, int z, int* height);
int __cdecl GetOffset(ITEM_INFO* item, int x, int z);
void __cdecl BridgeTilt1Floor(ITEM_INFO* item, int x, int y, int z, int* height);
void __cdecl BridgeTilt2Floor(ITEM_INFO* item, int x, int y, int z, int* height);
void __cdecl BridgeTilt1Ceiling(ITEM_INFO* item, int x, int y, int z, int* height);
void __cdecl BridgeTilt2Ceiling(ITEM_INFO* item, int x, int y, int z, int* height);
void __cdecl ControlAnimatingSlots(short itemNumber);
void __cdecl PoleCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
void __cdecl ControlTriggerTriggerer(short itemNumber);
void __cdecl AnimateWaterfalls();
void __cdecl ControlWaterfall(short itemNumber);
#define InitialiseTightRope ((void (__cdecl*)(short)) 0x0043ED30)
void __cdecl TightRopeCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
void __cdecl ParallelBarsCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
#define InitialiseXRayMachine ((void (__cdecl*)(short)) 0x0043FA20)
void __cdecl ControlXRayMachine(short itemNumber);
void __cdecl CutsceneRopeControl(short itemNumber);
void __cdecl HybridCollision(short itemNum, ITEM_INFO* laraitem, COLL_INFO* coll);
#define InitialiseRaisingBlock  ((void (__cdecl*)(short)) 0x0043D730)
#define RaisingBlockControl ((void (__cdecl*)(short)) 0x0048C3D0)
#define InitialiseRaisingCog ((void (__cdecl*)(short)) 0x00440320)
#define RaisingCogControl ((void (__cdecl*)(short)) 0x00406040)
#define HighObject2Control ((void (__cdecl*)(short)) 0x004070D0)

void Inject_Objects();