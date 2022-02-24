#pragma once

struct ITEM_INFO;
struct COLL_INFO;

namespace TEN::Entities::Doors
{
	void InitialiseSteelDoor(short itemNumber);
	void SteelDoorCollision(short itemNumber, ITEM_INFO* laraItem, COLL_INFO* coll);
}
