#pragma once
#include "global.h"

extern OBJECT_TEXTURE* WaterfallTextures[6];
extern float WaterfallY[6];

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
void TightRopeCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
void ParallelBarsCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
void ControlXRayMachine(short itemNumber);
void CutsceneRopeControl(short itemNumber);
void HybridCollision(short itemNum, ITEM_INFO* laraitem, COLL_INFO* coll);
void InitialiseSmashObject(short itemNumber);
void InitialiseTightRope(short itemNumber);
void HighObject2Control(short itemNumber);
void InitialiseAnimating(short itemNumber);
void AnimatingControl(short itemNumber);