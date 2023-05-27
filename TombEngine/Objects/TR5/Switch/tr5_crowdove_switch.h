#pragma once

struct ItemInfo;
struct CollisionInfo;

namespace TEN::Entities::Switches
{
	void InitializeCrowDoveSwitch(short itemNumber);
	void CrowDoveSwitchControl(short itemNumber);
	void CrowDoveSwitchCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
}
