#pragma once
#include "Specific/phd_global.h"
#include "Game/items.h"

void TriggerChaffEffects(int age);

void TriggerChaffEffects(ITEM_INFO* Item,int age);

void TriggerChaffEffects(ITEM_INFO* Item, PHD_VECTOR* pos, PHD_VECTOR* vel, int speed, bool isUnderwater,int age);

void TriggerChaffSparkles(PHD_VECTOR* pos, PHD_VECTOR* vel, CVECTOR* color,int age,ITEM_INFO* item);

void TriggerChaffSmoke(PHD_VECTOR* pos, PHD_VECTOR* vel, int speed, bool moving, bool wind);

void TriggerChaffBubbles(PHD_VECTOR* pos, int FlareRoomNumber);

/* void TriggerChaffEffects(ITEM_INFO* item, PHD_3DPOS pos, short angle, int speed, bool underwater);
void TriggerChaffSparkles(PHD_3DPOS pos, short angle, int speed);
void TriggerChaffSmoke(PHD_3DPOS pos, short angle, int speed, bool moving);
void TriggerChaffBubbles(PHD_3DPOS pos, int FlareRoomNumber); */