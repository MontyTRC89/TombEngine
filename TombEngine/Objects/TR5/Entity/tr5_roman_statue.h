#pragma once
#include "Math/Math.h"

namespace TEN::Entities::Creatures::TR5
{
	void InitialiseRomanStatue(short itemNumber);
	void RomanStatueControl(short itemNumber);
	void RomanStatueHit(ItemInfo* itemhit, ItemInfo* insticator, std::optional<GameVector> hitPos, int damage, int grenade, short meshHit);
	void TriggerRomanStatueMissileSparks(Vector3i* pos, char fxObject);
}
