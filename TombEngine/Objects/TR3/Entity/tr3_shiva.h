#pragma once

class GameVector;
struct ItemInfo;

namespace TEN::Entities::Creatures::TR3
{
	void InitialiseShiva(short itemNumber);
	void ShivaControl(short itemNumber);

	void ShivaHit(ItemInfo& target, ItemInfo& source, std::optional<GameVector> pos, int damage, bool isExplosive, int jointIndex);
	void SpawnShivaSmoke(const Vector3& pos, int roomNumber);
}
