#pragma once
#include "Game/items.h"

namespace TEN::Entities::TR3
{
	void SetupShoal(int shoalNumber);
	void ControlFish(short itemNumber);
	bool FishNearLara(PHD_3DPOS* pos, int distance, ItemInfo* item);
}
