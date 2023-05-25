#pragma once

struct CollisionInfo;
struct ItemInfo;

namespace TEN::Traps::TR5
{
	void InitializeZipLine(short itemNumber);
	void CollideZipLine(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
	void ControlZipLine(short itemNumber);
}
