#pragma once

class Pose;
struct ItemInfo;
struct ObjectInfo;

namespace TEN::Entities::TR4
{
	void SetupSeth(ObjectInfo& object);
	void InitialiseSeth(short itemNumber);
	void SethControl(short itemNumber);

	void SethProjectileAttack(const Pose& pose, int roomNumber, int flags);
	void SethAttack(int itemNumber);
	void SethKillAttack(ItemInfo* item, ItemInfo* laraItem);
}
