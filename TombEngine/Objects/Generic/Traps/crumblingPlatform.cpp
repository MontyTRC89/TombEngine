#include "framework.h"
#include "Objects/Generic/Traps/CrumblingPlatform.h"

#include "Game/collision/collide_item.h"
#include "Game/collision/floordata.h"
#include "Game/setup.h"
#include "Specific/level.h"

using namespace TEN::Collision::Floordata;

constexpr auto CRUMBLING_PLATFORM_INITIAL_SPEED = 10;
constexpr auto CRUMBLING_PLATFORM_MAX_SPEED = 100;
constexpr auto CRUMBLING_PLATFORM_FALL_VELOCITY = 4;

constexpr auto CRUMBLING_PLATFORM_DELAY = 35;

//I clamped it to ensure the value is less than half CLICK or it may not update room properly when landing on slopes in 1 click height rooms.
constexpr auto CRUMBLING_PLATFORM_HEIGHT_TOLERANCE = std::clamp(8, 0, 128);

enum CrumblingPlatformState
{
	CRUMBLING_PLATFORM_STATE_IDLE = 0,
	CRUMBLING_PLATFORM_STATE_SHAKING = 1,
	CRUMBLING_PLATFORM_STATE_FALLING = 2,
	CRUMBLING_PLATFORM_STATE_LANDING = 3
};

enum CrumblingPlatformAnim
{
	CRUMBLING_PLATFORM_ANIM_IDLE = 0,
	CRUMBLING_PLATFORM_ANIM_SHAKING = 1,
	CRUMBLING_PLATFORM_ANIM_FALLING = 2,
	CRUMBLING_PLATFORM_ANIM_LANDING = 3
};

void InitializeCrumblingPlatform(short itemNumber)
{
	auto& item = g_Level.Items[itemNumber];

	int timerFrames = (item.TriggerFlags != 0) ? std::abs(item.TriggerFlags) : CRUMBLING_PLATFORM_DELAY;
	item.ItemFlags[0] = timerFrames;
	UpdateBridgeItem(itemNumber);

	//Store the bridge collider Ys to override them in the bridge collision (to avoid use the bounding box while is shaking).
	auto bounds = GameBoundingBox(&item);
	item.ItemFlags[2] = bounds.Y1; //floor
	item.ItemFlags[3] = bounds.Y2; //ceiling

	//ItemFlags 0 = timer
	//ItemFlags 1 = gravity velocity
	//ItemFlags 2 = Top height of bounding box
	//ItemFlags 3 = Bottom height of bounding box
}

void CrumblingPlatformCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
{
	auto& item = g_Level.Items[itemNumber];

	//If OCB is 0 or higher, activate by Lara's collision. (If OCB < 0, it will activate by trigger).
	if (item.TriggerFlags >= 0 && item.Animation.ActiveState == CRUMBLING_PLATFORM_STATE_IDLE)
	{
		//Checks if Lara and Item are in the same XZ sector. And Lara is over the platform.
		if (!((item.Pose.Position.x ^ laraItem->Pose.Position.x) & 0xFFFFFC00) &&
			!((laraItem->Pose.Position.z ^ item.Pose.Position.z) & 0xFFFFFC00) &&
			abs((item.Pose.Position.y + item.ItemFlags[2]) - laraItem->Pose.Position.y) < CRUMBLING_PLATFORM_HEIGHT_TOLERANCE)
		{
			CrumblingPlatformActivate(itemNumber);
		}
	}
}

void CrumblingPlatformControl(short itemNumber)
{
	auto& item = g_Level.Items[itemNumber];

	//It ocb < 0, it must be activated by trigger.
	if (item.TriggerFlags < 0)
	{
		if (TriggerActive(&item))
		{
			CrumblingPlatformActivate(itemNumber);
			item.TriggerFlags = -item.TriggerFlags;
		}
		return;
	}

	switch (item.Animation.ActiveState)
	{
		case CRUMBLING_PLATFORM_STATE_SHAKING:
		{
			if (item.ItemFlags[0] > 0)
			{
				item.ItemFlags[0]--;
			}
			else
			{
				SetAnimation(&item, CRUMBLING_PLATFORM_ANIM_FALLING);

				item.ItemFlags[1] = CRUMBLING_PLATFORM_INITIAL_SPEED;

				auto& collisionResult = GetCollision(item);
				collisionResult.Block->RemoveBridge(itemNumber);
			}
		}
		break;

		case CRUMBLING_PLATFORM_STATE_FALLING:
		{
			auto collisionResult = GetCollision(item);
			auto height = item.Pose.Position.y - collisionResult.Position.Floor;

			if (height < 0)
			{//Is falling

				item.ItemFlags[1] += CRUMBLING_PLATFORM_FALL_VELOCITY;

				if (item.ItemFlags[1] > CRUMBLING_PLATFORM_MAX_SPEED)
					item.ItemFlags[1] = CRUMBLING_PLATFORM_MAX_SPEED;

				item.Pose.Position.y += item.ItemFlags[1];
			}
			else
			{//Has reached the ground

				item.Pose.Position.y = collisionResult.Position.Floor;

				SetAnimation(&item, CRUMBLING_PLATFORM_ANIM_LANDING);
			}

			//Update Room Number
			int probedRoomNumber = collisionResult.RoomNumber;
			if (item.RoomNumber != probedRoomNumber)
				ItemNewRoom(itemNumber, probedRoomNumber);
		}
		break;

		case CRUMBLING_PLATFORM_STATE_LANDING:
		{
			//Is hitting the ground.
			
			//Align to surface
			auto radius = Vector2(Objects[item.ObjectNumber].radius);
			AlignEntityToSurface(&item, radius);

			//Check if it's the last frame to deactivate the item.
			int frameEnd = GetAnimData(Objects[item.ObjectNumber], CRUMBLING_PLATFORM_ANIM_LANDING).frameEnd;
			if (item.Animation.FrameNumber >= frameEnd)
			{
				RemoveActiveItem(itemNumber);
				item.Status = ITEM_NOT_ACTIVE;
			}
		}
		break;

		default:
			TENLog("Error with Crumbling Platform object: " + std::to_string(itemNumber) + ", animation state not recognized.", LogLevel::Error, LogConfig::All, false);
		break;

	}

	AnimateItem(&item);
}

void CrumblingPlatformActivate(short itemNumber)
{
	auto& item = g_Level.Items[itemNumber];
	
	item.Status = ITEM_ACTIVE;
	AddActiveItem(itemNumber);

	SetAnimation(&item, CRUMBLING_PLATFORM_ANIM_SHAKING);
		
	item.Flags |= CODE_BITS;
}

std::optional<int> CrumblingPlatformFloor(short itemNumber, int x, int y, int z)
{
	ItemInfo& item = g_Level.Items[itemNumber];

	if (item.Animation.ActiveState <= CRUMBLING_PLATFORM_STATE_SHAKING)
		return item.Pose.Position.y + item.ItemFlags[2];
	else
		return std::nullopt;
}

std::optional<int> CrumblingPlatformCeiling(short itemNumber, int x, int y, int z)
{
	ItemInfo& item = g_Level.Items[itemNumber];

	if (item.Animation.ActiveState <= CRUMBLING_PLATFORM_STATE_SHAKING)
		return item.Pose.Position.y + item.ItemFlags[3];
	else
		return std::nullopt;
}

int CrumblingPlatformFloorBorder(short itemNumber)
{
	auto& item = g_Level.Items[itemNumber];
	return item.ItemFlags[2];
}

int CrumblingPlatformCeilingBorder(short itemNumber)
{
	auto& item = g_Level.Items[itemNumber];
	return item.ItemFlags[3];
}
