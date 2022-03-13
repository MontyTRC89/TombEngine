#pragma once

struct ITEM_INFO;
struct CollisionInfo;

namespace TEN::Entities::Switches
{
	constexpr auto COG_DOOR_TURN = 40;
	constexpr auto COG_DOOR_SPEED = 12;

	void CogSwitchControl(short itemNumber);
	void CogSwitchCollision(short itemNumber, ITEM_INFO* laraItem, CollisionInfo* coll);
}
