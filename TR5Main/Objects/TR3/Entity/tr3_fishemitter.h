#pragma once
#include "Game/items.h"

void SetupShoal(int shoalNumber);
void ControlFish(short itemNumber);
bool FishNearLara(PoseData* pose, int distance, ITEM_INFO* item);
