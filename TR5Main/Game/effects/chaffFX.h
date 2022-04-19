#pragma once
#include "Specific/phd_global.h"
#include "Game/items.h"

void TriggerChaffEffects(int age);

void TriggerChaffEffects(ITEM_INFO* Item,int age);

void TriggerChaffEffects(ITEM_INFO* Item, Vector3Int* pos, Vector3Int* vel, int speed, bool isUnderwater,int age);

void TriggerChaffSparkles(Vector3Int* pos, Vector3Int* vel, CVECTOR* color,int age,ITEM_INFO* item);

void TriggerChaffSmoke(Vector3Int* pos, Vector3Int* vel, int speed, bool moving, bool wind);

void TriggerChaffBubbles(Vector3Int* pos, int FlareRoomNumber);

/* void TriggerChaffEffects(ITEM_INFO* item, PHD_3DPOS pos, short angle, int speed, bool underwater);
void TriggerChaffSparkles(PHD_3DPOS pos, short angle, int speed);
void TriggerChaffSmoke(PHD_3DPOS pos, short angle, int speed, bool moving);
void TriggerChaffBubbles(PHD_3DPOS pos, int FlareRoomNumber); */