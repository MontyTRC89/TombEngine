#pragma once

class Pose;
struct ItemInfo;

namespace TEN::Entities::TR4
{
	void InitialiseSeth(short itemNumber);
	void SethControl(short itemNumber);
	void SethProjectileAttack(const Pose& pose, int roomNumber, int flags);
	void SethAttack(int itemNumber);
	void SethKillAttack(ItemInfo* item, ItemInfo* laraItem);
}
