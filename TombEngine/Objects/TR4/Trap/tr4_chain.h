#pragma once

struct CollisionInfo;
struct ItemInfo;

namespace TEN::Entities::Traps
{
	void ControlChain(short itemNumber);
	void CollideChain(short itemNumber, ItemInfo* playerItem, CollisionInfo* coll);
	void TriggerPendulumFlame(int itemNumber, Vector3i pos);
	Vector3i GetNodePosition(const ItemInfo& item, unsigned char node);
}
