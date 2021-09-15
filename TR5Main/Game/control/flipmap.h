#pragma once
#include "room.h"

constexpr auto MAX_FLIPMAP = 255;

extern byte FlipStatus;
extern int FlipStats[MAX_FLIPMAP];
extern int FlipMap[MAX_FLIPMAP];

void DoFlipMap(short group);
void AddRoomFlipItems(ROOM_INFO* r);
void RemoveRoomFlipItems(ROOM_INFO* r);