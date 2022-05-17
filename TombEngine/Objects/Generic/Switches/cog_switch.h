#pragma once

struct ItemInfo;
struct CollisionInfo;

namespace TEN::Entities::Switches
{
	constexpr auto COG_DOOR_TURN = 40;
	constexpr auto COG_DOOR_SPEED = 12;

	void CogSwitchControl(short itemNumber);
	void CogSwitchCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
}
