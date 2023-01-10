#pragma once

class GameVector;
struct ItemInfo;

namespace TEN::Entities::TR4
{
	void InitialiseBaddy(short itemNumber);
	void BaddyControl(short itemNumber);
	void Baddy2Hit(ItemInfo& target, ItemInfo& instigator, std::optional<GameVector> pos, int damage, int grenade, int jointIndex);
}
