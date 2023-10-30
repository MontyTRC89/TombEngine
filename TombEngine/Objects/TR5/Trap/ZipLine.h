#pragma once

struct CollisionInfo;
struct ItemInfo;

namespace TEN::Traps::TR5
{
	void InitializeZipLine(short itemNumber);
	void ControlZipLine(short itemNumber);
	void CollideZipLine(short itemNumber, ItemInfo* playerItem, CollisionInfo* coll);
}
