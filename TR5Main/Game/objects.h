#pragma once

#include "collide.h"

extern OBJECT_TEXTURE* WaterfallTextures[6];
extern float WaterfallY[6];

void SmashObject(short itemNumber);
void SmashObjectControl(short itemNumber);
void InitialiseBridge(short itemNumber);
int BridgeFlatFloor(short itemNumber, int x, int y, int z);
int BridgeFlatCeiling(short itemNumber, int x, int y, int z);
int GetOffset(ITEM_INFO* item, int x, int z);
int BridgeTilt1Floor(short itemNumber, int x, int y, int z);
int BridgeTilt2Floor(short itemNumber, int x, int y, int z);
int BridgeTilt1Ceiling(short itemNumber, int x, int y, int z);
int BridgeTilt2Ceiling(short itemNumber, int x, int y, int z);
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