#pragma once

struct ItemInfo;
struct CollisionInfo;
struct OBJECT_TEXTURE;
extern OBJECT_TEXTURE* WaterfallTextures[6];
extern float WaterfallY[6];

void SmashObject(short itemNumber);
void SmashObjectControl(short itemNumber);
void ControlAnimatingSlots(short itemNumber);
void ControlTriggerTriggerer(short itemNumber);
void ControlWaterfall(short itemNumber);
void TightropeCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
void HorizontalBarCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
void CutsceneRopeControl(short itemNumber);
void HybridCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
void InitialiseSmashObject(short itemNumber);
void InitialiseTightrope(short itemNumber);
void HighObject2Control(short itemNumber);
void InitialiseAnimating(short itemNumber);
void AnimatingControl(short itemNumber);
