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
#include "Specific/input.h"
#include "Specific/level.h"
#include "Specific/trmath.h"

using namespace TEN::Input;
using std::vector;

namespace TEN::Entities::Generic
{
	const vector<LaraState> VPoleStates =
	{
		LS_POLE_IDLE,
		LS_POLE_UP,
		LS_POLE_DOWN,
		LS_POLE_TURN_CLOCKWISE,
		LS_POLE_TURN_COUNTER_CLOCKWISE
	};
	const vector<LaraState> VPoleGroundedMountStates =
	{
		LS_IDLE,
		LS_TURN_LEFT_SLOW,
		LS_TURN_RIGHT_SLOW,
		LS_TURN_LEFT_FAST,
		LS_TURN_RIGHT_FAST,
		LS_WALK_FORWARD,
		LS_RUN_FORWARD
	};
	const vector<LaraState> VPoleAirborneMountStates =
	{
		LS_REACH,
		LS_JUMP_UP
	};

	// TODO: These might be interfering with the set position command. -- Sezz 2022.08.29
	auto VPolePos = Vector3Int(0, 0, -208);
	auto VPolePosR = Vector3Int::Zero;

	OBJECT_COLLISION_BOUNDS VPoleBounds = 
	{
		-CLICK(1), CLICK(1),
		0, 0, 
		-CLICK(2), CLICK(2),
		ANGLE(-10.0f), ANGLE(10.0f),
		ANGLE(-30.0f), ANGLE(30.0f),
		ANGLE(-10.0f), ANGLE(10.0f)
	};

	void PoleCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
	{
		auto* poleItem = &g_Level.Items[itemNumber];
		auto* lara = GetLaraInfo(laraItem);

		short deltaAngle = poleItem->Pose.Orientation.y - laraItem->Pose.Orientation.y;
		short angleBetweenPositions = poleItem->Pose.Orientation.y - GetOrientBetweenPoints(laraItem->Pose.Position, poleItem->Pose.Position).y;
		bool isFacingPole = abs(deltaAngle - angleBetweenPositions) < ANGLE(90.0f);

		// Mount while grounded.
		if (TrInput & IN_ACTION && isFacingPole &&
			CheckLaraState((LaraState)laraItem->Animation.ActiveState, VPoleGroundedMountStates) &&
			lara->Control.HandStatus == HandStatus::Free ||
			(lara->Control.IsMoving && lara->InteractedItem == itemNumber))
		{
			// Temporarily reorient pole.
			short yOrient = poleItem->Pose.Orientation.y;
			poleItem->Pose.Orientation.y = laraItem->Pose.Orientation.y;

			if (TestLaraPosition(&VPoleBounds, poleItem, laraItem))
			{
				if (MoveLaraPosition(&VPolePos, poleItem, laraItem))
				{
					SetAnimation(laraItem, LA_STAND_TO_POLE);
					lara->Control.IsMoving = false;
					lara->Control.HandStatus = HandStatus::Busy;
				}
				else
					lara->InteractedItem = itemNumber;

				poleItem->Pose.Orientation.y = yOrient;
			}
			else
			{
				if (lara->Control.IsMoving && lara->InteractedItem == itemNumber)
				{
					lara->Control.IsMoving = false;
					lara->Control.HandStatus = HandStatus::Free;
				}

				poleItem->Pose.Orientation.y = yOrient;
			}

			return;
		}

		// Mount while airborne.
		if (TrInput & IN_ACTION && isFacingPole &&
			CheckLaraState((LaraState)laraItem->Animation.ActiveState, VPoleAirborneMountStates) &&
			laraItem->Animation.IsAirborne &&
			laraItem->Animation.Velocity.y > 0.0f &&
			lara->Control.HandStatus == HandStatus::Free)
		{
			if (TestBoundsCollide(poleItem, laraItem, LARA_RADIUS + (int)round(abs(laraItem->Animation.Velocity.z))) &&
				TestLaraPoleCollision(laraItem, coll, true, -CLICK(1)) &&
				TestLaraPoleCollision(laraItem, coll, false))
			{
				if (TestCollision(poleItem, laraItem))
				{
					// Temporarily reorient pole.
					short yOrient = poleItem->Pose.Orientation.y;
					poleItem->Pose.Orientation.y = laraItem->Pose.Orientation.y;

					// Reaching.
					if (laraItem->Animation.ActiveState == LS_REACH)
					{
						VPolePosR.y = laraItem->Pose.Position.y - poleItem->Pose.Position.y + 10;
						AlignLaraPosition(&VPolePosR, poleItem, laraItem);
						SetAnimation(laraItem, LA_REACH_TO_POLE);
					}
					// Jumping up.
					else
					{
						VPolePosR.y = laraItem->Pose.Position.y - poleItem->Pose.Position.y + 66;
						AlignLaraPosition(&VPolePosR, poleItem, laraItem);
						SetAnimation(laraItem, LA_JUMP_UP_TO_POLE);
					}

					laraItem->Animation.IsAirborne = false;
					laraItem->Animation.Velocity.y = 0.0f;
					lara->Control.HandStatus = HandStatus::Busy;
					poleItem->Pose.Orientation.y = yOrient;
				}
			}

			return;
		}

		// Not interacting; do regular object collision.
		if (!CheckLaraState((LaraState)laraItem->Animation.ActiveState, VPoleStates) &&
			laraItem->Animation.ActiveState != LS_JUMP_BACK)
		{
			ObjectCollision(itemNumber, laraItem, coll);
		}
	}
}
