#pragma once

class Pose;
struct ItemInfo;

namespace TEN::Entities::TR4
{
	void InitialiseSeth(short itemNumber);
	void SethControl(short itemNumber);
	void SethAttack(int itemNumber);
	void SethKill(ItemInfo* sethItem, ItemInfo* laraItem);

	void TriggerSethSparks1(const Vector3& pos, const Vector3& velocity);
	void TriggerSethSparks2(short itemNumber, char node, int size);
	void SpawnSethProjectile(Pose* pose, int roomNumber, int flags);
}
