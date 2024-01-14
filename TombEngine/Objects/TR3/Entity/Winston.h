#pragma once

class GameVector;
class Vector3i;
struct ItemInfo;

namespace TEN::Entities::Creatures::TR3
{
	void InitializeWinston(short itemNumber);
	void OldControlWinston(short itemNumber);
	void ControlWinston(short itemNumber);
	void WinstonHit(ItemInfo& target, ItemInfo& source, std::optional<GameVector> pos, int damage, bool isExplosive, int jointIndex);
}
