#pragma once

class GameVector;
struct ItemInfo;

namespace TEN::Entities
{
	void InitialiseCorpse(short itemNumber);
	void CorpseControl(short itemNumber);
	void CorpseHit(ItemInfo& target, ItemInfo& source, std::optional<GameVector> pos, int damage, bool isExplosive, int jointIndex);
}

