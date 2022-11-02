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
using std::pair;
using std::vector;

namespace TEN::Entities::Generic
{
	constexpr auto LADDER_STEP_HEIGHT = BLOCK(1, 8);

	enum class LadderMountType
	{
		None,
		TopFront,
		TopBack,
		Front,
		Back,
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

	const auto LadderMountedOffset = Vector3i(0, 0, -BLOCK(1, 7));
	const auto LadderInteractBounds = GameBoundingBox(
		-BLOCK(1, 4), BLOCK(1, 4),
		-LADDER_STEP_HEIGHT, LADDER_STEP_HEIGHT,
		-BLOCK(3, 8), BLOCK(3, 8)
	);

	const auto LadderMountTopFrontOffset = LadderMountedOffset; // TODO
	const auto LadderMountTopFrontOrient = EulerAngles(0, ANGLE(180.0f), 0);
	const auto LadderMountTopFrontBasis = InteractionBasis(
		LadderInteractBounds,
		pair(
			EulerAngles(ANGLE(-10.0f), ANGLE(180.0f) - LARA_GRAB_THRESHOLD, ANGLE(-10.0f)),
			EulerAngles(ANGLE(10.0f), ANGLE(180.0f) + LARA_GRAB_THRESHOLD, ANGLE(10.0f))
		)
	);

	const auto LadderMountTopBackOffset = LadderMountedOffset; // TODO
	const auto LadderMountTopBackOrient = EulerAngles::Zero;
	const auto LadderMountTopBackBasis = InteractionBasis(
		LadderInteractBounds,
		pair(
			EulerAngles(ANGLE(-10.0f), -LARA_GRAB_THRESHOLD, ANGLE(-10.0f)),
			EulerAngles(ANGLE(10.0f), LARA_GRAB_THRESHOLD, ANGLE(10.0f))
		)
	);

	const auto LadderMountFrontOffset = LadderMountedOffset;
	const auto LadderMountFrontOrient = EulerAngles::Zero;
	const auto LadderMountFrontBasis = InteractionBasis(
		LadderInteractBounds,
		pair(
			EulerAngles(ANGLE(-10.0f), -LARA_GRAB_THRESHOLD, ANGLE(-10.0f)),
			EulerAngles(ANGLE(10.0f), LARA_GRAB_THRESHOLD, ANGLE(10.0f))
		)
	);
	
	const auto LadderMountBackOffset = LadderMountedOffset;
	const auto LadderMountBackOrient = EulerAngles(0, ANGLE(180.0f), 0);
	const auto LadderMountBackBasis = InteractionBasis(
		LadderInteractBounds,
		pair(
			EulerAngles(ANGLE(-10.0f), ANGLE(180.0f) - LARA_GRAB_THRESHOLD, ANGLE(-10.0f)),
			EulerAngles(ANGLE(10.0f), ANGLE(180.0f) + LARA_GRAB_THRESHOLD, ANGLE(10.0f))
		)
	);

	const auto LadderMountLeftOffset = LadderMountedOffset + Vector3i(-BLOCK(1, 4), 0, 0);
	const auto LadderMountLeftOrient = EulerAngles(0, ANGLE(90.0f), 0);
	const auto LadderMountLeftBasis = InteractionBasis(
		LadderInteractBounds,
		pair(
			EulerAngles(ANGLE(-10.0f), ANGLE(90.0f) - LARA_GRAB_THRESHOLD, ANGLE(-10.0f)),
			EulerAngles(ANGLE(10.0f), ANGLE(90.0f) + LARA_GRAB_THRESHOLD, ANGLE(10.0f))
		)
	);

	const auto LadderMountRightOffset = LadderMountedOffset + Vector3i(BLOCK(1, 4), 0, 0);
	const auto LadderMountRightOrient = EulerAngles(0, ANGLE(-90.0f), 0);
	const auto LadderMountRightBasis = InteractionBasis(
		LadderInteractBounds,
		pair(
			EulerAngles(ANGLE(-10.0f), ANGLE(-90.0f) - LARA_GRAB_THRESHOLD, ANGLE(-10.0f)),
			EulerAngles(ANGLE(10.0f), ANGLE(-90.0f) + LARA_GRAB_THRESHOLD, ANGLE(10.0f))
		)
	);

	void LadderCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
	{
		auto& ladderItem = g_Level.Items[itemNumber];
		auto& player = *GetLaraInfo(laraItem);

		bool isFacingLadder = Geometry::IsPointInFront(laraItem->Pose, ladderItem.Pose.Position.ToVector3());

		// Mount while grounded.
		if ((TrInput & IN_ACTION && isFacingLadder &&
			TestState(laraItem->Animation.ActiveState, LadderGroundedMountStates) &&
			player.Control.HandStatus == HandStatus::Free) ||
			(player.Control.IsMoving && player.InteractedItem == itemNumber))
		{
			// Mount from front.
			if (LadderMountFrontBasis.TestInteraction(ladderItem, *laraItem))
			{
				if (!laraItem->OffsetBlend.IsActive)
				{
					auto mountOffset = LadderMountFrontOffset + Vector3i(0, 0, GameBoundingBox(&ladderItem).Z1);

					auto targetPos = Geometry::TranslatePoint(ladderItem.Pose.Position, ladderItem.Pose.Orientation, mountOffset);
					auto posOffset = (targetPos - laraItem->Pose.Position).ToVector3();
					auto orientOffset = (ladderItem.Pose.Orientation + LadderMountFrontOrient) - laraItem->Pose.Orientation;
					laraItem->SetOffsetBlend(posOffset, orientOffset);

					SetAnimation(laraItem, LA_LADDER_MOUNT_FRONT);
					player.Control.IsMoving = false;
					player.Control.HandStatus = HandStatus::Busy;
				}
				else
					player.InteractedItem = itemNumber;

				return;
			}

			// Mount from back.
			if (LadderMountBackBasis.TestInteraction(ladderItem, *laraItem))
			{
				if (!laraItem->OffsetBlend.IsActive)
				{
					auto mountOffset = LadderMountBackOffset + Vector3i(0, 0, GameBoundingBox(&ladderItem).Z2);

					auto targetPos = Geometry::TranslatePoint(ladderItem.Pose.Position, ladderItem.Pose.Orientation, mountOffset);
					auto posOffset = (targetPos - laraItem->Pose.Position).ToVector3();
					auto orientOffset = (ladderItem.Pose.Orientation + LadderMountBackOrient) - laraItem->Pose.Orientation;
					laraItem->SetOffsetBlend(posOffset, orientOffset);

					SetAnimation(laraItem, LA_LADDER_MOUNT_FRONT);
					player.Control.IsMoving = false;
					player.Control.HandStatus = HandStatus::Busy;
				}
				else
					player.InteractedItem = itemNumber;

				return;
			}

			// Mount from right.
			if (LadderMountRightBasis.TestInteraction(ladderItem, *laraItem))
			{
				auto mountOffset = LadderMountRightOffset + Vector3i(0, 0, GameBoundingBox(&ladderItem).Z1);

				//if (AlignPlayerToEntity(&ladderItem, laraItem, mountOffset, LadderMountRightOrient))
				if (!laraItem->OffsetBlend.IsActive)
				{
					auto targetPos = Geometry::TranslatePoint(ladderItem.Pose.Position, ladderItem.Pose.Orientation, mountOffset);
					auto posOffset = (targetPos - laraItem->Pose.Position).ToVector3();
					auto orientOffset = (ladderItem.Pose.Orientation + LadderMountRightOrient) - laraItem->Pose.Orientation;
					laraItem->SetOffsetBlend(posOffset, orientOffset);

					SetAnimation(laraItem, LA_LADDER_MOUNT_RIGHT);
					player.Control.IsMoving = false;
					player.Control.HandStatus = HandStatus::Busy;
				}
				else
					player.InteractedItem = itemNumber;

				return;
			}

			if (player.Control.IsMoving && player.InteractedItem == itemNumber)
			{
				player.Control.HandStatus = HandStatus::Free;
				player.Control.IsMoving = false;
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
			if (TestBoundsCollide(&ladderItem, laraItem, coll->Setup.Radius))//LARA_RADIUS + (int)round(abs(laraItem->Animation.Velocity.z))))
			{
				if (TestCollision(&ladderItem, laraItem))
				{
					auto ladderBounds = GameBoundingBox(&ladderItem);
					int ladderVPos = ladderItem.Pose.Position.y + ladderBounds.Y1;
					int playerVPos = laraItem->Pose.Position.y - LARA_HEIGHT;

					int vOffset = -abs(playerVPos - ladderVPos) / LADDER_STEP_HEIGHT;
					auto mountOffset = Vector3i(0, vOffset, ladderBounds.Z1) + LadderMountedOffset;

					if (AlignPlayerToEntity(&ladderItem, laraItem, mountOffset, LadderMountFrontOrient, true))
					{
						// Reaching.
						if (laraItem->Animation.ActiveState == LS_REACH)
							SetAnimation(laraItem, LA_LADDER_IDLE);// LA_LADDER_MOUNT_JUMP_REACH);
						// Jumping up.
						else
							SetAnimation(laraItem, LA_LADDER_IDLE);// LA_LADDER_MOUNT_JUMP_UP);

						laraItem->Animation.IsAirborne = false;
						laraItem->Animation.Velocity.y = 0.0f;
						player.Control.HandStatus = HandStatus::Busy;
					}
				}
			}

			return;
		}

		// Player is not interacting with ladder; do regular object collision.
		if (!TestState(laraItem->Animation.ActiveState, LadderMountedStates) &&
			laraItem->Animation.ActiveState != LS_JUMP_BACK) // TODO: Player can jump through ladders.
		{
			ObjectCollision(itemNumber, laraItem, coll);
		}
	}
}
