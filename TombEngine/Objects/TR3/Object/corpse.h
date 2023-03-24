#pragma once

class GameVector;
struct ItemInfo;

namespace TEN::Entities::TR3
{

	enum class CorpseFlags
	{
		Lying = (1 << 0),
		Hanging = (1 << 1),
		Falling = (1 << 2)
	};

	void InitialiseCorpse(short itemNumber);
	void CorpseControl(short itemNumber);
	void CorpseHit(ItemInfo& target, ItemInfo& source, std::optional<GameVector> pos, int damage, bool isExplosive, int jointIndex);
}
