#pragma once

struct CollisionInfo;
struct ItemInfo;

namespace TEN::Entities::Creatures::TR2
{
	void InitializeBartoli(short itemNumber);
	void ControlBartoli(short itemNumber);

	void SpawnDragonBlast(short sourceItemNumber, short ObjectToSpawn);
	void ControlDragonBlast(short itemNumber);

}
