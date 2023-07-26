#include "framework.h"
#include "Objects/TR4/Trap/SpikyCeiling.h"

#include "Game/animation.h"
#include "Game/collision/collide_item.h"
#include "Game/collision/collide_room.h"
#include "Game/collision/sphere.h"
#include "Game/control/control.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Sound/sound.h"
#include "Specific/level.h"

namespace TEN::Entities::Traps
{
	// NOTES:
	// ItemFlags[0] = Vertical velocity. Positive value moves down, negative value moves up.

	constexpr auto SPIKY_CEILING_HARM_DAMAGE = 15;

	void InitializeSpikyCeiling(short itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];

		item.ItemFlags[0] = item.TriggerFlags;
	}

	void ControlSpikyCeiling(short itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];

		if (!TriggerActive(&item) || item.Status == ITEM_DEACTIVATED)
			return;

		// Determine height bounds.
		auto bounds = GameBoundingBox(&item);
		int upperFloorBound = bounds.Y2;
		int lowerCeilBound = bounds.Y1;

		// Get point collision.
		auto pointColl = GetCollision(&item);
		int relFloorHeight = pointColl.Position.Floor - item.Pose.Position.y;
		int relCeilHeight = pointColl.Position.Ceiling - item.Pose.Position.y;

		int verticalVel = item.ItemFlags[0];

		// Stop moving.
		if ((verticalVel > 0 && relFloorHeight <= upperFloorBound) ||
			(verticalVel < 0 && relCeilHeight >= lowerCeilBound))
		{
			item.Status = ITEM_DEACTIVATED;
			StopSoundEffect(SFX_TR4_ROLLING_BALL);
		}
		// Move.
		else
		{
			item.Pose.Position.y += verticalVel;

			if (pointColl.RoomNumber != item.RoomNumber)
				ItemNewRoom(itemNumber, pointColl.RoomNumber);

			SoundEffect(SFX_TR4_ROLLING_BALL, &item.Pose);
		}
	}

	void CollideSpikyCeiling(short itemNumber, ItemInfo* playerItem, CollisionInfo* coll)
	{
		auto& item = g_Level.Items[itemNumber];

		if (item.ObjectNumber != ID_SPIKY_CEILING)
			return;

		// Collide with objects.
		if (item.Status == ITEM_ACTIVE)
		{
			if (!TestBoundsCollide(&item, playerItem, coll->Setup.Radius))
				return;

			TestCollision(&item, playerItem);
		}
		else if (item.Status != ITEM_INVISIBLE)
		{
			ObjectCollision(itemNumber, playerItem, coll);
		}
		
		// Damage entity.
		if (TestBoundsCollide(&item, playerItem, coll->Setup.Radius))
		{
			DoDamage(playerItem, SPIKY_CEILING_HARM_DAMAGE);
			DoLotsOfBlood(playerItem->Pose.Position.x, playerItem->Pose.Position.y + CLICK(3), playerItem->Pose.Position.z, 4, playerItem->Pose.Orientation.y, playerItem->RoomNumber, 3);
			playerItem->TouchBits.ClearAll();

			SoundEffect(SFX_TR4_LARA_GRABFEET, &playerItem->Pose);
		}
	}
}
