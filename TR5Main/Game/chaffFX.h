#pragma once
#include "../Global/global.h"

void TriggerChaffEffects();

void TriggerChaffEffects(ITEM_INFO* Item);

void TriggerChaffEffects(ITEM_INFO* Item, PHD_VECTOR* pos, PHD_VECTOR* vel, int speed, bool isUnderwater);

void TriggerChaffSparkles(PHD_VECTOR* pos, PHD_VECTOR* vel, CVECTOR* color);

void TriggerChaffSmoke(PHD_VECTOR* pos, PHD_VECTOR* vel, int speed, bool moving, bool wind);

void TriggerChaffBubbles(PHD_VECTOR* pos, int FlareRoomNumber);

/* void TriggerChaffEffects(ITEM_INFO* item, PHD_3DPOS pos, short angle, int speed, bool underwater);
void TriggerChaffSparkles(PHD_3DPOS pos, short angle, int speed);
void TriggerChaffSmoke(PHD_3DPOS pos, short angle, int speed, bool moving);
void TriggerChaffBubbles(PHD_3DPOS pos, int FlareRoomNumber); */