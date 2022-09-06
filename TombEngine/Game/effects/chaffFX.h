#pragma once
#include "Specific/phd_global.h"
#include "Game/items.h"

void TriggerChaffEffects(int age);

void TriggerChaffEffects(ItemInfo* Item,int age);

void TriggerChaffEffects(ItemInfo* Item, Vector3i* pos, Vector3i* vel, int speed, bool isUnderwater,int age);

void TriggerChaffSparkles(Vector3i* pos, Vector3i* vel, CVECTOR* color,int age,ItemInfo* item);

void TriggerChaffSmoke(Vector3i* pos, Vector3i* vel, int speed, bool moving, bool wind);

void TriggerChaffBubbles(Vector3i* pos, int FlareRoomNumber);

/* void TriggerChaffEffects(ItemInfo* item, PoseData pos, short angle, int speed, bool underwater);
void TriggerChaffSparkles(PoseData pos, short angle, int speed);
void TriggerChaffSmoke(PoseData pos, short angle, int speed, bool moving);
void TriggerChaffBubbles(PoseData pos, int FlareRoomNumber); */
