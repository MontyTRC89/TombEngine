#include "framework.h"
#include "Objects/Generic/Object/HorizontalBar.h"

#include "Game/animation.h"
#include "Game/collision/collide_item.h"
#include "Game/control/control.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/Setup.h"
#include "Math/Math.h"
#include "Renderer/RendererEnums.h"
#include "Sound/sound.h"
#include "Specific/Input/Input.h"
#include "Specific/level.h"

using namespace TEN::Input;
using namespace TEN::Math;

namespace TEN::Entities::Generic
{
	static const auto HORIZONTAL_BAR_STATES = std::vector<int>
	{
		LS_HORIZONTAL_BAR_SWING,
		LS_HORIZONTAL_BAR_JUMP,
		LS_HORIZONTAL_BAR_IDLE,
		LS_HORIZONTAL_BAR_IDLE_TURN_180,
		LS_HORIZONTAL_BAR_SWING_TURN_180
	};

	static auto HorizontalBarBounds = ObjectCollisionBounds
	{
		GameBoundingBox(
			-640, 640,
			704, 832,
			-96, 96),
		std::pair(
			EulerAngles(ANGLE(-10.0f), -LARA_GRAB_THRESHOLD, ANGLE(-10.0f)),
			EulerAngles(ANGLE(10.0f), LARA_GRAB_THRESHOLD, ANGLE(10.0f)))
	};

	void HorizontalBarCollision(short itemNumber, ItemInfo* playerItem, CollisionInfo* coll)
	{
		auto& barItem = g_Level.Items[itemNumber];
		auto& player = GetLaraInfo(*playerItem);

		if (IsHeld(In::Action) &&
			(playerItem->Animation.ActiveState == LS_REACH || playerItem->Animation.ActiveState == LS_JUMP_UP) &&
			player.Control.HandStatus == HandStatus::Free)
		{
			// HACK: Update interaction basis.
			auto bounds = GameBoundingBox(&barItem);
			HorizontalBarBounds.BoundingBox.X1 = bounds.X1;
			HorizontalBarBounds.BoundingBox.X2 = bounds.X2;

			// Test interaction.
			bool isFront = TestLaraPosition(HorizontalBarBounds, &barItem, playerItem);
			bool isBack = false;
			if (!isFront)
			{
				barItem.Pose.Orientation.y += ANGLE(-180.0f);
				isBack = TestLaraPosition(HorizontalBarBounds, &barItem, playerItem);
				barItem.Pose.Orientation.y += ANGLE(-180.0f);
			}

			// Set player interaction.
			if (isFront || isBack)
			{
				SetAnimation(playerItem, (playerItem->Animation.ActiveState == LS_REACH) ? LA_REACH_TO_HORIZONTAL_BAR : LA_JUMP_UP_TO_HORIZONTAL_BAR);
				ResetPlayerFlex(playerItem);
				playerItem->Animation.Velocity.y = 0.0f;
				playerItem->Animation.IsAirborne = false;
				player.Context.InteractedItem = itemNumber;
				player.Control.HandStatus = HandStatus::Busy;

				// Calculate catch position from line (fake attractor).
				auto linePoint0 = Geometry::TranslatePoint(barItem.Pose.Position.ToVector3(), barItem.Pose.Orientation.y, 0.0f, 0.0f, HorizontalBarBounds.BoundingBox.X1);
				auto linePoint1 = Geometry::TranslatePoint(barItem.Pose.Position.ToVector3(), barItem.Pose.Orientation.y, 0.0f, 0.0f, HorizontalBarBounds.BoundingBox.X2);
				auto catchPos = Geometry::GetClosestPointOnLinePerp(playerItem->Pose.Position.ToVector3(), linePoint0, linePoint1);

				// Update player position and orientation.
				playerItem->Pose.Position = Geometry::TranslatePoint(catchPos, 0, isFront ? bounds.Z1 : bounds.Z2, coll->Setup.Height + CLICK(0.25f)); // HACK: Hardcoded offset required.
				playerItem->Pose.Orientation.y = barItem.Pose.Orientation.y + (isFront ? ANGLE(0.0f) : ANGLE(-180.0f));
			}
			else
			{
				ObjectCollision(itemNumber, playerItem, coll);
			}
		}
		else if (!TestState(playerItem->Animation.ActiveState, HORIZONTAL_BAR_STATES))
		{
			ObjectCollision(itemNumber, playerItem, coll);
		}
	}
}
