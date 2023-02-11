#pragma once
#include "Game/items.h"
#include "Math/Math.h"

void TriggerChaffEffects(int flareLife);
void TriggerChaffEffects(ItemInfo& item, int age);
void TriggerChaffEffects(ItemInfo& item, const Vector3i& pos, const Vector3i& vel, int speed, bool isUnderwater, int age);
void TriggerChaffSparkles(const Vector3i& pos, const Vector3i& vel, const ColorData& color, int age, const ItemInfo& item);
void TriggerChaffSmoke(const Vector3i& pos, const Vector3i& vel, int speed, bool isMoving, bool wind);
void TriggerChaffBubbles(const Vector3i& pos, int roomNumber);

/* void TriggerChaffEffects(ItemInfo* item, Pose pos, short angle, int speed, bool underwater);
void TriggerChaffSparkles(Pose pos, short angle, int speed);
void TriggerChaffSmoke(Pose pos, short angle, int speed, bool moving);
void TriggerChaffBubbles(Pose pos, int FlareRoomNumber); */
