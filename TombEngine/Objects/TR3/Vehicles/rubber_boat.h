#pragma once

struct ItemInfo;
struct CollisionInfo;

namespace TEN::Entities::Vehicles
{
	void InitialiseRubberBoat(short itemNumber);
	void RubberBoatCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
	void RubberBoatControl(short itemNumber);
	void DrawRubberBoat(ItemInfo* rBoatItem);
}
