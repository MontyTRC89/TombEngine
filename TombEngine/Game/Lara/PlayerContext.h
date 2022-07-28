#pragma once
#include "Game/Lara/PlayerContextStructs.h"

struct CollisionInfo;
struct ItemInfo;

namespace TEN::Entities::Player
{
	class PlayerContext
	{
	public:
		PlayerContext();

		static bool CanRunForward(ItemInfo* item, CollisionInfo* coll);

	private:
		static bool GetMoveContext(ItemInfo* item, CollisionInfo* coll, MoveContextSetup contextSetup, bool useCrawlSetup = false);
	};
}
