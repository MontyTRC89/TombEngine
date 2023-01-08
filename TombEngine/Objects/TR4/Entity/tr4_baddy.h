#pragma once
#include "Math/Math.h"

namespace TEN::Entities::TR4
{
	void InitialiseBaddy(short itemNumber);
	void BaddyControl(short itemNumber);
	void Baddy2Hit(ItemInfo* itemhit, ItemInfo* insticator, std::optional<GameVector> hitPos, int damage, int grenade, short meshHit);
}
