#include "framework.h"
#include "Objects/Generic/Traps/CrumblingPlatform.h"

#include "Game/collision/collide_item.h"
#include "Game/collision/floordata.h"
#include "Game/setup.h"
#include "Specific/clock.h"
#include "Specific/level.h"

using namespace TEN::Collision::Floordata;

// NOTES:
// ItemFlags[0]: Delay in frame time.
// ItemFlags[1]: Fall velocity.
// ItemFlags[2]: Top height of bounding box.
// ItemFlags[3]: Bottom height of bounding box.

namespace TEN::Entities::Traps
{
	constexpr auto CRUMBLING_PLATFORM_INITIAL_VELOCITY = 10.0f;
	constexpr auto CRUMBLING_PLATFORM_MAX_VELOCITY	   = 100.0f;
	constexpr auto CRUMBLING_PLATFORM_FALL_VELOCITY	   = 4.0f;

	constexpr auto CRUMBLING_PLATFORM_DELAY = 1.2f;

	enum CrumblingPlatformState
	{
		CRUMBLING_PLATFORM_STATE_IDLE = 0,
		CRUMBLING_PLATFORM_STATE_SHAKE = 1,
		CRUMBLING_PLATFORM_STATE_FALL = 2,
		CRUMBLING_PLATFORM_STATE_LAND = 3
	};

	enum CrumblingPlatformAnim
	{
		CRUMBLING_PLATFORM_ANIM_IDLE = 0,
		CRUMBLING_PLATFORM_ANIM_SHAKE = 1,
		CRUMBLING_PLATFORM_ANIM_FALL = 2,
		CRUMBLING_PLATFORM_ANIM_LAND = 3
	};

	void InitializeCrumblingPlatform(short itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];

		int delayInFrameTime = (item.TriggerFlags != 0) ? std::abs(item.TriggerFlags) : (int)round(CRUMBLING_PLATFORM_DELAY * FPS);
		item.ItemFlags[0] = delayInFrameTime;
		UpdateBridgeItem(itemNumber);

		// Store override bridge collision heights to using bounding box while shaking.
		auto bounds = GameBoundingBox(&item);
		item.ItemFlags[2] = bounds.Y1; // Floor.
		item.ItemFlags[3] = bounds.Y2; // Ceiling.
	}

	void ActivateCrumblingPlatform(short itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];

		item.Status = ITEM_ACTIVE;
		AddActiveItem(itemNumber);

		SetAnimation(item, CRUMBLING_PLATFORM_ANIM_SHAKE);
		item.Flags |= CODE_BITS;
	}

	void ControlCrumblingPlatform(short itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];

		// OCD < 0; must be activated by trigger.
		if (item.TriggerFlags < 0)
		{
			if (TriggerActive(&item))
			{
				ActivateCrumblingPlatform(itemNumber);
				item.TriggerFlags = -item.TriggerFlags;
			}

			return;
		}

		switch (item.Animation.ActiveState)
		{
			case CRUMBLING_PLATFORM_STATE_SHAKE:
			{
				if (item.ItemFlags[0] > 0)
				{
					item.ItemFlags[0]--;
				}
				else
				{
					SetAnimation(item, CRUMBLING_PLATFORM_ANIM_FALL);
					item.ItemFlags[1] = CRUMBLING_PLATFORM_INITIAL_VELOCITY;

					auto pointColl = GetCollision(item);
					pointColl.Block->RemoveBridge(itemNumber);
				}
			}

			break;

			case CRUMBLING_PLATFORM_STATE_FALL:
			{
				auto pointColl = GetCollision(item);
				int relFloorHeight = item.Pose.Position.y - pointColl.Position.Floor;

				// Airborne.
				if (relFloorHeight < 0)
				{
					item.ItemFlags[1] += CRUMBLING_PLATFORM_FALL_VELOCITY;
					if (item.ItemFlags[1] > CRUMBLING_PLATFORM_MAX_VELOCITY)
						item.ItemFlags[1] = CRUMBLING_PLATFORM_MAX_VELOCITY;

					item.Pose.Position.y += item.ItemFlags[1];
				}
				// Grounded.
				else
				{
					SetAnimation(item, CRUMBLING_PLATFORM_ANIM_LAND);
					item.Pose.Position.y = pointColl.Position.Floor;
				}

				// Update room number.
				int probedRoomNumber = pointColl.RoomNumber;
				if (item.RoomNumber != probedRoomNumber)
					ItemNewRoom(itemNumber, probedRoomNumber);
			}

			break;

			case CRUMBLING_PLATFORM_STATE_LAND:
			{
				// Align to surface.
				auto radius = Vector2(Objects[item.ObjectNumber].radius);
				AlignEntityToSurface(&item, radius);

				// Deactivate.
				if (TestLastFrame(&item))
				{
					RemoveActiveItem(itemNumber);
					item.Status = ITEM_NOT_ACTIVE;
				}
			}

			break;

			default:
				TENLog(
					"Error with crumbling platform with entity ID" + std::to_string(itemNumber) + ". animation state not recognized.",
					LogLevel::Error, LogConfig::All, false);
				break;
		}

		AnimateItem(&item);
	}

	void CollideCrumblingPlatform(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
	{
		auto& item = g_Level.Items[itemNumber];

		// OCB >= 0; activate via player collision. OCB < 0 activates via trigger.
		if (item.TriggerFlags >= 0 && item.Animation.ActiveState == CRUMBLING_PLATFORM_STATE_IDLE)
		{
			// Crumble if player is on platform.
			if (!laraItem->Animation.IsAirborne &&
				coll->LastBridgeItemNumber == item.Index)
			{
				ActivateCrumblingPlatform(itemNumber);
			}
		}
	}

	std::optional<int> CrumblingPlatformFloor(short itemNumber, int x, int y, int z)
	{
		const auto& item = g_Level.Items[itemNumber];

		if (item.Animation.ActiveState == CRUMBLING_PLATFORM_STATE_IDLE ||
			item.Animation.ActiveState == CRUMBLING_PLATFORM_STATE_SHAKE)
		{
			int height = item.ItemFlags[2];
			return (item.Pose.Position.y + height);
		}
		
		return std::nullopt;
	}

	std::optional<int> CrumblingPlatformCeiling(short itemNumber, int x, int y, int z)
	{
		const auto& item = g_Level.Items[itemNumber];

		if (item.Animation.ActiveState == CRUMBLING_PLATFORM_STATE_IDLE ||
			item.Animation.ActiveState == CRUMBLING_PLATFORM_STATE_SHAKE)
		{
			int height = item.ItemFlags[3];
			return (item.Pose.Position.y + height);
		}
		
		return std::nullopt;
	}

	int CrumblingPlatformFloorBorder(short itemNumber)
	{
		const auto& item = g_Level.Items[itemNumber];

		int height = item.ItemFlags[2];
		return height;
	}

	int CrumblingPlatformCeilingBorder(short itemNumber)
	{
		const auto& item = g_Level.Items[itemNumber];

		int height = item.ItemFlags[3];
		return height;
	}
}
