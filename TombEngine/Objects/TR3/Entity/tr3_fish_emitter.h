#pragma once
#include "Game/items.h"

namespace TEN::Entities::Creatures::TR3
{
	void SetupShoal(int shoalNumber);
	void ControlFish(short itemNumber);
	bool FishNearLara(Pose* pos, int distance, ItemInfo* item);
}
