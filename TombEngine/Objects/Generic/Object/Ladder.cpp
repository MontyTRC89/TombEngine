#include "framework.h"
#include "Objects/Generic/Object/polerope.h"

#include "Game/collision/collide_item.h"
#include "Game/collision/sphere.h"
#include "Game/control/box.h"
#include "Game/control/control.h"
#include "Game/control/lot.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/Lara/lara_struct.h"
#include "Game/Lara/lara_tests.h"
#include "Math/Math.h"
#include "Specific/input.h"
#include "Specific/level.h"

using namespace TEN::Input;
using namespace TEN::Math;
using std::vector;

namespace TEN::Entities::Generic
{
	enum class LadderMountType
	{
		TopFront,
		TopBack,
		Bottom,
		Left,
		Right
	};

	const vector<int> LadderMountedStates =
	{
		LS_LADDER_IDLE,
		LS_LADDER_UP,
		LS_LADDER_DOWN
	};
	const vector<int> LadderGroundedMountStates =
	{
		LS_IDLE,
		LS_TURN_LEFT_SLOW,
		LS_TURN_RIGHT_SLOW,
		LS_TURN_LEFT_FAST,
		LS_TURN_RIGHT_FAST,
		LS_WALK_FORWARD,
		LS_RUN_FORWARD
	};
	const vector<int> LadderAirborneMountStates =
	{
		LS_REACH,
		LS_JUMP_UP
	};

	const auto LadderMountOffset = Vector3i(0, 0, -CLICK(0.55f));
	const ObjectCollisionBounds LadderFrontBounds =
	{
		GameBoundingBox(
			-CLICK(1), CLICK(1),
			-CLICK(1), CLICK(1),
			-CLICK(1.5f), CLICK(1.5f)
		),
		std::pair(
			EulerAngles(ANGLE(-10.0f), -LARA_GRAB_THRESHOLD, ANGLE(-10.0f)),
			EulerAngles(ANGLE(10.0f), LARA_GRAB_THRESHOLD, ANGLE(10.0f))
		)
	};
	const ObjectCollisionBounds LadderBackBounds =
	{
		GameBoundingBox(
			-CLICK(1), CLICK(1),
			-CLICK(1), CLICK(1),
			-CLICK(1.5f), CLICK(1.5f)
		),
		std::pair(
			EulerAngles(ANGLE(-10.0f), ANGLE(180.0f) - LARA_GRAB_THRESHOLD, ANGLE(-10.0f)),
			EulerAngles(ANGLE(10.0f), ANGLE(180.0f) + LARA_GRAB_THRESHOLD, ANGLE(10.0f))
		)
	};
	const ObjectCollisionBounds LadderLeftBounds =
	{
		GameBoundingBox(
			-CLICK(1), CLICK(1),
			-CLICK(1), CLICK(1),
			-CLICK(1.5f), CLICK(1.5f)
		),
		std::pair(
			EulerAngles(ANGLE(-10.0f), ANGLE(90.0f) - LARA_GRAB_THRESHOLD, ANGLE(-10.0f)),
			EulerAngles(ANGLE(10.0f), ANGLE(90.0f) + LARA_GRAB_THRESHOLD, ANGLE(10.0f))
		)
	};
	const ObjectCollisionBounds LadderRightBounds =
	{
		GameBoundingBox(
			-CLICK(1), CLICK(1),
			-CLICK(1), CLICK(1),
			-CLICK(1.5f), CLICK(1.5f)
		),
		std::pair(
			EulerAngles(ANGLE(-10.0f), ANGLE(-90.0f) - LARA_GRAB_THRESHOLD, ANGLE(-10.0f)),
			EulerAngles(ANGLE(10.0f), ANGLE(-90.0f) + LARA_GRAB_THRESHOLD, ANGLE(10.0f))
		)
	};

	void LadderCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
	{
		auto& ladderItem = g_Level.Items[itemNumber];
		auto& player = *GetLaraInfo(laraItem);

		bool isFacingLadder = Geometry::IsPointInFront(laraItem->Pose, ladderItem.Pose.Position.ToVector3());

		// Mount while grounded.
		if (TrInput & IN_ACTION && isFacingLadder &&
			TestState(laraItem->Animation.ActiveState, LadderGroundedMountStates) &&
			player.Control.HandStatus == HandStatus::Free ||
			(player.Control.IsMoving && player.InteractedItem == itemNumber))
		{
			// Mount at bottom.
			if (TestLaraPosition(LadderFrontBounds, &ladderItem, laraItem))
			{
				auto mountPos = Vector3i(0, 0, GameBoundingBox(&ladderItem).Z1) + LadderMountOffset;
				if (MoveLaraPosition(mountPos, &ladderItem, laraItem))
				{
					SetAnimation(laraItem, LA_LADDER_MOUNT_BOTTOM);
					player.Control.IsMoving = false;
					player.Control.HandStatus = HandStatus::Busy;
				}
				else
					player.InteractedItem = itemNumber;
			}
			// Mount from right.
			else if (TestLaraPosition(LadderRightBounds, &ladderItem, laraItem))
			{
				auto mountPos = Vector3i(GameBoundingBox(&ladderItem).Z1 + CLICK(0.55f), 0, 0);
				if (MoveLaraPosition(mountPos, &ladderItem, laraItem))
				{
					SetAnimation(laraItem, LA_LADDER_MOUNT_RIGHT);
					player.Control.IsMoving = false;
					player.Control.HandStatus = HandStatus::Busy;
				}
				else
					player.InteractedItem = itemNumber;
			}
			else
			{
				if (player.Control.IsMoving && player.InteractedItem == itemNumber)
				{
					player.Control.IsMoving = false;
					player.Control.HandStatus = HandStatus::Free;
				}
			}

			return;
		}

		// Mount while airborne.
		if (TrInput & IN_ACTION && isFacingLadder &&
			TestState(laraItem->Animation.ActiveState, LadderAirborneMountStates) &&
			laraItem->Animation.IsAirborne &&
			laraItem->Animation.Velocity.y > 0.0f &&
			player.Control.HandStatus == HandStatus::Free)
		{
			// Test bounds collision.
			if (TestBoundsCollide(&ladderItem, laraItem, LARA_RADIUS + (int)round(abs(laraItem->Animation.Velocity.z))))
			{
				if (TestCollision(&ladderItem, laraItem))
				{
					// Reaching.
					if (laraItem->Animation.ActiveState == LS_REACH)
					{
						auto bounds = GameBoundingBox(&ladderItem);
						auto offset = Vector3i(
							0,
							bounds.Y1 - fmod(abs(bounds.Y1 - laraItem->Pose.Position.y), CLICK(1)),
							0
						);
						AlignLaraPosition(offset, &ladderItem, laraItem);
						SetAnimation(laraItem, LA_LADDER_MOUNT_JUMP_REACH);
					}
					// Jumping up.
					/*else
					{

					}*/

					laraItem->Animation.IsAirborne = false;
					laraItem->Animation.Velocity.y = 0.0f;
					player.Control.HandStatus = HandStatus::Busy;
				}
			}

			return;
		}

		// Player is not interacting with ladder; do regular object collision.
		if (!TestState(laraItem->Animation.ActiveState, LadderMountedStates) &&
			laraItem->Animation.ActiveState != LS_JUMP_BACK)
		{
			ObjectCollision(itemNumber, laraItem, coll);
		}
	}
}
