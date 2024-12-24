#pragma once

struct CollisionInfo;
struct ItemInfo;

namespace TEN::Entities::Traps
{
	void InitializeZipLine(short itemNumber);
	void ControlZipLine(short itemNumber);
	void CollideZipLine(short itemNumber, ItemInfo* collided, CollisionInfo* coll);
}
