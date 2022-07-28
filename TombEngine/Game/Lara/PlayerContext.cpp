#include "framework.h"
#include "Game/Lara/PlayerContext.h"

#include "Game/collision/collide_room.h"
#include "Game/control/los.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/PlayerContextStructs.h"

// -------
// PUBLIC:
// -------

namespace TEN::Entities::Player
{
	PlayerContext::PlayerContext()
	{
	}

	bool PlayerContext::CanRunForward(ItemInfo* item, CollisionInfo* coll)
	{
		MoveContextSetup contextSetup
		{
			item->Pose.Orientation.y,
			NO_LOWER_BOUND, -STEPUP_HEIGHT, // Defined by run forward state.
			false, true, false
		};

		return GetMoveContext(item, coll, contextSetup);
	}

	// --------
	// PRIVATE:
	// --------

	bool PlayerContext::GetMoveContext(ItemInfo* item, CollisionInfo* coll, MoveContextSetup contextSetup, bool useCrawlSetup)
	{
		// HACK: coll->Setup.Radius and coll->Setup.Height are set only in lara_col functions, then reset by LaraAboveWater() to defaults.
		// This means they will store the wrong values for any move context assessments conducted in crouch/crawl lara_as functions.
		// When states become objects, a dedicated state init function should eliminate the need for the useCrawlSetup parameter. @Sezz 2022.03.16
		int playerRadius = useCrawlSetup ? LARA_RADIUS_CRAWL : coll->Setup.Radius;
		int playerHeight = useCrawlSetup ? LARA_HEIGHT_CRAWL : coll->Setup.Height;

		int yPos = item->Pose.Position.y;
		auto probe = GetCollision(item, contextSetup.Angle, OFFSET_RADIUS(playerRadius), -playerHeight);

		// 1. Check for wall.
		if (probe.Position.Floor == NO_HEIGHT)
			return false;

		bool isSlopeDown = contextSetup.CheckSlopeDown ? (probe.Position.FloorSlope && probe.Position.Floor > yPos) : false;
		bool isSlopeUp = contextSetup.CheckSlopeUp ? (probe.Position.FloorSlope && probe.Position.Floor < yPos) : false;
		bool isDeathFloor = contextSetup.CheckDeathFloor ? probe.Block->Flags.Death : false;

		// 2. Check for slope or death floor (if applicable).
		if (isSlopeDown || isSlopeUp || isDeathFloor)
			return false;

		auto origin1 = GameVector(
			item->Pose.Position.x,
			yPos + contextSetup.UpperFloorBound - 1,
			item->Pose.Position.z,
			item->RoomNumber
		);
		auto target1 = GameVector(
			probe.Coordinates.x,
			yPos + contextSetup.UpperFloorBound - 1,
			probe.Coordinates.z,
			item->RoomNumber
		);

		auto origin2 = GameVector(
			item->Pose.Position.x,
			yPos - playerHeight + 1,
			item->Pose.Position.z,
			item->RoomNumber
		);
		auto target2 = GameVector(
			probe.Coordinates.x,
			probe.Coordinates.y + 1,
			probe.Coordinates.z,
			item->RoomNumber
		);

		// 3. Assess raycast collision.
		if (!LOS(&origin1, &target1) || !LOS(&origin2, &target2))
			return false;

		// 4. Assess point probe collision.
		if ((probe.Position.Floor - yPos) <= contextSetup.LowerFloorBound &&   // Floor is within lower floor bound.
			(probe.Position.Floor - yPos) >= contextSetup.UpperFloorBound &&   // Floor is within upper floor bound.
			(probe.Position.Ceiling - yPos) < -playerHeight &&				   // Ceiling is within lowest ceiling bound (i.e. player's height).
			abs(probe.Position.Ceiling - probe.Position.Floor) > playerHeight) // Space is not too narrow.
		{
			return true;
		}

		return false;
	}
}
