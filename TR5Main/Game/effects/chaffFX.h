#pragma once
#include "Specific/phd_global.h"
#include "Game/items.h"

void TriggerChaffEffects(int age);

void TriggerChaffEffects(ItemInfo* item, int age);

void TriggerChaffEffects(ItemInfo* item, Vector3Int* pos, Vector3Int* vel, int speed, bool isUnderwater, int age);

void TriggerChaffSparkles(Vector3Int* pos, Vector3Int* vel, CVECTOR* color, int age, ItemInfo* item);

void TriggerChaffSmoke(Vector3Int* pos, Vector3Int* vel, int speed, bool moving, bool wind);

void TriggerChaffBubbles(Vector3Int* pos, int FlareRoomNumber);

/* void TriggerChaffEffects(ItemInfo* item, PoseData pos, short angle, int speed, bool underwater);
void TriggerChaffSparkles(PoseData pos, short angle, int speed);
void TriggerChaffSmoke(PoseData pos, short angle, int speed, bool moving);
void TriggerChaffBubbles(PoseData pos, int FlareRoomNumber); */