#pragma once
#include "..\Global\global.h"

#define InitialiseAnimating ((void (__cdecl*)(short)) 0x00440100)
#define AnimatingControl ((void (__cdecl*)(short)) 0x00465590)

#define InitialiseSmashObject ((void (__cdecl*)(short)) 0x0043D7F0)
void SmashObject(short itemNumber);
void SmashObjectControl(short itemNumber);
void BridgeFlatFloor(ITEM_INFO* item, int x, int y, int z, int* height);
void BridgeFlatCeiling(ITEM_INFO* item, int x, int y, int z, int* height);
int GetOffset(ITEM_INFO* item, int x, int z);
void BridgeTilt1Floor(ITEM_INFO* item, int x, int y, int z, int* height);
void BridgeTilt2Floor(ITEM_INFO* item, int x, int y, int z, int* height);
void BridgeTilt1Ceiling(ITEM_INFO* item, int x, int y, int z, int* height);
void BridgeTilt2Ceiling(ITEM_INFO* item, int x, int y, int z, int* height);
void ControlAnimatingSlots(short itemNumber);
void PoleCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
void ControlTriggerTriggerer(short itemNumber);
void AnimateWaterfalls();
void ControlWaterfall(short itemNumber);
#define InitialiseTightRope ((void (__cdecl*)(short)) 0x0043ED30)
void TightRopeCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
void ParallelBarsCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
#define InitialiseXRayMachine ((void (__cdecl*)(short)) 0x0043FA20)
void ControlXRayMachine(short itemNumber);
void CutsceneRopeControl(short itemNumber);
void HybridCollision(short itemNum, ITEM_INFO* laraitem, COLL_INFO* coll);
#define InitialiseRaisingBlock  ((void (__cdecl*)(short)) 0x0043D730)
#define RaisingBlockControl ((void (__cdecl*)(short)) 0x0048C3D0)
#define InitialiseRaisingCog ((void (__cdecl*)(short)) 0x00440320)
#define RaisingCogControl ((void (__cdecl*)(short)) 0x00406040)
#define HighObject2Control ((void (__cdecl*)(short)) 0x004070D0)

void Inject_Objects();