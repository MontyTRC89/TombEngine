#pragma once
#include "Game/items.h"

namespace TEN::Entities::TR3
{
	void SetupShoal(int shoalNumber);
	void ControlFish(short itemNumber);
	bool FishNearLara(PoseData* pos, int distance, ItemInfo* item);
}
