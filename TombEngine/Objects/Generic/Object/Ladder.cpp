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
#include "Specific/Input/Input.h"
#include "Specific/level.h"

using namespace TEN::Input;
using namespace TEN::Math;

namespace TEN::Entities::Generic
{
	constexpr auto LADDER_STEP_HEIGHT = BLOCK(1.0f / 8);

	enum class LadderMountType
	{
		None,
		TopFront,
		TopBack,
		Front,
		Back,
		Left,
		Right,
		JumpReach,
		JumpUp
	};

	const std::vector<int> LadderMountedStates =
	{
		LS_LADDER_IDLE,
		LS_LADDER_UP,
		LS_LADDER_DOWN
	};
	const std::vector<int> LadderGroundedMountStates =
	{
		LS_IDLE,
		LS_TURN_LEFT_SLOW,
		LS_TURN_RIGHT_SLOW,
		LS_TURN_LEFT_FAST,
		LS_TURN_RIGHT_FAST,
		LS_WALK_FORWARD,
		LS_RUN_FORWARD
	};
	const std::vector<int> LadderAirborneMountStates =
	{
		LS_REACH,
		LS_JUMP_UP
	};

	const auto LadderMountedBaseOffset = Vector3i(0, 0, -BLOCK(1.0f / 7));
	const auto LadderInteractBaseBounds = GameBoundingBox(
		-BLOCK(1.0f / 4), BLOCK(1.0f / 4),
		0, 0,
		-BLOCK(3.0f / 8), BLOCK(3.0f / 8)
	);

	const auto LadderMountTopFrontBasis = InteractionBasis(
		LadderMountedBaseOffset, // TODO
		EulerAngles(0, ANGLE(180.0f), 0),
		LadderInteractBaseBounds,
		std::pair(
			EulerAngles(ANGLE(-10.0f), ANGLE(180.0f) - LARA_GRAB_THRESHOLD, ANGLE(-10.0f)),
			EulerAngles(ANGLE(10.0f), ANGLE(180.0f) + LARA_GRAB_THRESHOLD, ANGLE(10.0f))
		)
	);
	const auto LadderMountTopBackBasis = InteractionBasis(
		LadderMountedBaseOffset, // TODO
		EulerAngles::Zero,
		LadderInteractBaseBounds,
		std::pair(
			EulerAngles(ANGLE(-10.0f), -LARA_GRAB_THRESHOLD, ANGLE(-10.0f)),
			EulerAngles(ANGLE(10.0f), LARA_GRAB_THRESHOLD, ANGLE(10.0f))
		)
	);
	const auto LadderMountFrontBasis = InteractionBasis(
		LadderMountedBaseOffset,
		EulerAngles::Zero,
		LadderInteractBaseBounds,
		std::pair(
			EulerAngles(ANGLE(-10.0f), -LARA_GRAB_THRESHOLD, ANGLE(-10.0f)),
			EulerAngles(ANGLE(10.0f), LARA_GRAB_THRESHOLD, ANGLE(10.0f))
		)
	);
	const auto LadderMountBackBasis = InteractionBasis(
		LadderMountedBaseOffset,
		EulerAngles(0, ANGLE(180.0f), 0),
		LadderInteractBaseBounds,
		std::pair(
			EulerAngles(ANGLE(-10.0f), ANGLE(180.0f) - LARA_GRAB_THRESHOLD, ANGLE(-10.0f)),
			EulerAngles(ANGLE(10.0f), ANGLE(180.0f) + LARA_GRAB_THRESHOLD, ANGLE(10.0f))
		)
	);
	const auto LadderMountLeftBasis = InteractionBasis(
		LadderMountedBaseOffset + Vector3i(-BLOCK(1.0f / 4), 0, 0),
		EulerAngles(0, ANGLE(90.0f), 0),
		LadderInteractBaseBounds,
		std::pair(
			EulerAngles(ANGLE(-10.0f), ANGLE(90.0f) - LARA_GRAB_THRESHOLD, ANGLE(-10.0f)),
			EulerAngles(ANGLE(10.0f), ANGLE(90.0f) + LARA_GRAB_THRESHOLD, ANGLE(10.0f))
		)
	);
	const auto LadderMountRightBasis = InteractionBasis(
		LadderMountedBaseOffset + Vector3i(BLOCK(1.0f / 4), 0, 0),
		EulerAngles(0, ANGLE(-90.0f), 0),
		LadderInteractBaseBounds,
		std::pair(
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
		if ((IsHeld(In::Action) && isFacingLadder &&
			TestState(laraItem->Animation.ActiveState, LadderGroundedMountStates) &&
			player.Control.HandStatus == HandStatus::Free) ||
			(player.Control.IsMoving && player.InteractedItem == itemNumber))
		{
			auto ladderBounds = GameBoundingBox(&ladderItem);
			auto boundsExtension = GameBoundingBox(0, 0, ladderBounds.Y1, ladderBounds.Y2 + LADDER_STEP_HEIGHT, 0, 0);

			// Mount from front.
			if (TestEntityInteraction(*laraItem, ladderItem, LadderMountFrontBasis, boundsExtension))
			{
				if (!laraItem->OffsetBlend.IsActive)
				{
					auto boundsOffset = Vector3i(0, 0, GameBoundingBox(&ladderItem).Z1);
					SetEntityInteraction(*laraItem, ladderItem, LadderMountFrontBasis, boundsOffset);

					SetAnimation(laraItem, LA_LADDER_MOUNT_FRONT);
					player.Control.IsMoving = false;
					player.Control.HandStatus = HandStatus::Busy;
				}
				else
					player.InteractedItem = itemNumber;

				return;
			}

			// Mount from back.
			if (TestEntityInteraction(*laraItem, ladderItem, LadderMountBackBasis, boundsExtension))
			{
				if (!laraItem->OffsetBlend.IsActive)
				{
					auto mountOffset = LadderMountBackBasis.PosOffset + Vector3i(0, 0, GameBoundingBox(&ladderItem).Z2);

					auto targetPos = Geometry::TranslatePoint(ladderItem.Pose.Position, ladderItem.Pose.Orientation, mountOffset);
					auto posOffset = (targetPos - laraItem->Pose.Position).ToVector3();
					auto orientOffset = (ladderItem.Pose.Orientation + LadderMountBackBasis.OrientOffset) - laraItem->Pose.Orientation;
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
			if (TestEntityInteraction(*laraItem, ladderItem, LadderMountRightBasis, boundsExtension))
			{
				auto mountOffset = LadderMountRightBasis.PosOffset + Vector3i(0, 0, GameBoundingBox(&ladderItem).Z1);

				//if (AlignPlayerToEntity(&ladderItem, laraItem, mountOffset, LadderMountRightOrient))
				if (!laraItem->OffsetBlend.IsActive)
				{
					auto targetPos = Geometry::TranslatePoint(ladderItem.Pose.Position, ladderItem.Pose.Orientation, mountOffset);
					auto posOffset = (targetPos - laraItem->Pose.Position).ToVector3();
					auto orientOffset = (ladderItem.Pose.Orientation + LadderMountRightBasis.OrientOffset) - laraItem->Pose.Orientation;
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
					auto mountOffset = Vector3i(0, vOffset, ladderBounds.Z1) + LadderMountedBaseOffset;

					if (AlignPlayerToEntity(&ladderItem, laraItem, mountOffset, LadderMountFrontBasis.OrientOffset, true))
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

	LadderMountType GetLadderMountType(const ItemInfo& ladderItem, ItemInfo& laraItem)
	{
		const auto& player = *GetLaraInfo(&laraItem);

		// Check ladder usability.
		if (ladderItem.Flags & IFLAG_INVISIBLE)
			return LadderMountType::None;

		// Check hand status.
		if (player.Control.HandStatus != HandStatus::Free)
			return LadderMountType::None;

		bool isFacingLadder = Geometry::IsPointInFront(laraItem.Pose, ladderItem.Pose.Position.ToVector3());
		if (!isFacingLadder || !TestState(laraItem.Animation.ActiveState, LadderGroundedMountStates))
			return LadderMountType::None;

		if (IsHeld(In::Action))
		{
			if (TestEntityInteraction(ladderItem, laraItem, LadderMountTopFrontBasis))
				return LadderMountType::TopFront;

			if (TestEntityInteraction(ladderItem, laraItem, LadderMountTopBackBasis))
				return LadderMountType::TopBack;

			if (TestEntityInteraction(ladderItem, laraItem, LadderMountFrontBasis))
				return LadderMountType::Front;

			if (TestEntityInteraction(ladderItem, laraItem, LadderMountBackBasis))
				return LadderMountType::Back;

			if (TestEntityInteraction(ladderItem, laraItem, LadderMountLeftBasis))
				return LadderMountType::Left;

			if (TestEntityInteraction(ladderItem, laraItem, LadderMountRightBasis))
				return LadderMountType::Right;
		}

		return LadderMountType::None;
	}
}
