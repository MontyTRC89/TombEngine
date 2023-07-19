#include "framework.h"
#include "Objects/TR4/Trap/SpikyWall.h"

#include "Game/collision/collide_item.h"
#include "Game/collision/collide_room.h"
#include "Game/collision/sphere.h"
#include "Game/control/control.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Math/Math.h"
#include "Sound/sound.h"
#include "Specific/level.h"

using namespace TEN::Math;

namespace TEN::Entities::Traps
{
	constexpr auto SPIKY_WALL_HARM_DAMAGE = 15;

	void InitializeSpikyWall(short itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];

		item.ItemFlags[0] = item.TriggerFlags;
	}

	void ControlSpikyWall(short itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];

		if (!TriggerActive(&item) || item.Status == ITEM_DEACTIVATED)
			return;

		// Determine wall bounds.
		auto bounds = GameBoundingBox(&item);
		int frontWallBound = bounds.Z2;
		int backWallBound = bounds.Z1;

		// Get point collision.
		int forwardVel = item.ItemFlags[0];
		auto pointColl = GetCollision(&item, item.Pose.Orientation.y, (forwardVel >= 0) ? frontWallBound : backWallBound);

		// Stop moving.
		if (pointColl.Position.Floor != item.Pose.Position.y)
		{
			item.Status = ITEM_DEACTIVATED;
			StopSoundEffect(SFX_TR4_ROLLING_BALL);
		}
		// Move.
		else
		{
			item.Pose.Position = Geometry::TranslatePoint(item.Pose.Position, item.Pose.Orientation.y, forwardVel);

			if (pointColl.RoomNumber != item.RoomNumber)
				ItemNewRoom(itemNumber, pointColl.RoomNumber);

			SoundEffect(SFX_TR4_ROLLING_BALL, &item.Pose);
		}
	}

	void CollideSpikyWall(short itemNumber, ItemInfo* playerItem, CollisionInfo* coll)
	{
		auto& item = g_Level.Items[itemNumber];

		if (item.ObjectNumber != ID_SPIKY_WALL)
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
			DoDamage(playerItem, SPIKY_WALL_HARM_DAMAGE);
			DoLotsOfBlood(playerItem->Pose.Position.x, playerItem->Pose.Position.y + CLICK(2), playerItem->Pose.Position.z, 4, playerItem->Pose.Orientation.y, playerItem->RoomNumber, 3);
			playerItem->TouchBits.ClearAll();

			SoundEffect(SFX_TR4_LARA_GRABFEET, &playerItem->Pose);
		}
	}
}
