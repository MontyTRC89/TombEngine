#pragma once

class GameVector;
class Vector3i;
struct ItemInfo;

namespace TEN::Entities::Creatures::TR5
{
	void InitialiseRomanStatue(short itemNumber);
	void RomanStatueControl(short itemNumber);
	void RomanStatueHit(ItemInfo& target, ItemInfo& source, std::optional<GameVector> pos, int damage, int grenade, int jointIndex);
	void TriggerRomanStatueMissileSparks(Vector3i* pos, char fxObject);;
}
