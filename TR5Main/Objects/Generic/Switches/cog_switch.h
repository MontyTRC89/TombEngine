#pragma once

struct ITEM_INFO;
struct COLL_INFO;

namespace TEN::Entities::Switches
{
	constexpr auto COG_DOOR_TURN = 40;
	constexpr auto COG_DOOR_SPEED = 12;

	void CogSwitchControl(short itemNumber);
	void CogSwitchCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll);
}