#pragma once

struct ITEM_INFO;
struct COLL_INFO;
struct OBJECT_TEXTURE;
extern OBJECT_TEXTURE* WaterfallTextures[6];
extern float WaterfallY[6];

void SmashObject(short itemNumber);
void SmashObjectControl(short itemNumber);
void ControlAnimatingSlots(short itemNumber);
void ControlTriggerTriggerer(short itemNumber);
void AnimateWaterfalls();
void ControlWaterfall(short itemNumber);
void TightropeCollision(short itemNumber, ITEM_INFO* laraItem, COLL_INFO* coll);
void HorizontalBarCollision(short itemNumber, ITEM_INFO* laraItem, COLL_INFO* coll);
void CutsceneRopeControl(short itemNumber);
void HybridCollision(short itemNumber, ITEM_INFO* laraItem, COLL_INFO* coll);
void InitialiseSmashObject(short itemNumber);
void InitialiseTightrope(short itemNumber);
void HighObject2Control(short itemNumber);
void InitialiseAnimating(short itemNumber);
void AnimatingControl(short itemNumber);
