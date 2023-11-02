#include "framework.h"
#include "Objects/Generic/Switches/underwater_switch.h"

#include "Game/animation.h"
#include "Game/camera.h"
#include "Game/collision/collide_item.h"
#include "Game/collision/Interaction.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Objects/Generic/Switches/generic_switch.h"
#include "Specific/Input/Input.h"
#include "Specific/level.h"

using namespace TEN::Collision;
using namespace TEN::Input;

namespace TEN::Entities::Switches
{
	const auto UNDERWATER_WALL_SWITCH_BASIS = InteractionBasis(
		Vector3i(0, 0, 108),
		EulerAngles::Zero,
		GameBoundingBox(
			-BLOCK(3 / 8.0f), BLOCK(3 / 8.0f),
			-BLOCK(3 / 8.0f), BLOCK(3 / 8.0f),
			0, BLOCK(3 / 4.0f)),
		std::pair(
			EulerAngles(-LARA_GRAB_THRESHOLD, -LARA_GRAB_THRESHOLD, -LARA_GRAB_THRESHOLD) * 2,
			EulerAngles(LARA_GRAB_THRESHOLD, LARA_GRAB_THRESHOLD, LARA_GRAB_THRESHOLD) * 2));

	const auto UNDERWATER_CEILING_SWITCH_FRONT_BASIS = InteractionBasis(
		Vector3i(0, -736, -416),
		EulerAngles::Zero,
		GameBoundingBox(
			-BLOCK(3 / 8.0f), BLOCK(3 / 8.0f),
			-BLOCK(17 / 16.0f), -BLOCK(1 / 2.0f),
			-BLOCK(1 / 2.0f), 0),
		std::pair(
			EulerAngles(-ANGLE(90.0f), -(LARA_GRAB_THRESHOLD * 2), -(LARA_GRAB_THRESHOLD * 2)),
			EulerAngles(LARA_GRAB_THRESHOLD, LARA_GRAB_THRESHOLD * 2, LARA_GRAB_THRESHOLD * 2)));
	
	// TODO: Orient constraint not working.
	const auto UNDERWATER_CEILING_SWITCH_BACK_BASIS = InteractionBasis(
		Vector3i(0, -736, -416),
		EulerAngles(0, ANGLE(180.0f), 0),
		GameBoundingBox(
			-BLOCK(3 / 8.0f), BLOCK(3 / 8.0f),
			-BLOCK(17 / 16.0f), -BLOCK(1 / 2.0f),
			0, BLOCK(1 / 2.0f)),
		std::pair(
			EulerAngles(-ANGLE(90.0f), -(LARA_GRAB_THRESHOLD * 2) + ANGLE(180.0f), -(LARA_GRAB_THRESHOLD * 2)),
			EulerAngles(LARA_GRAB_THRESHOLD, (LARA_GRAB_THRESHOLD * 2) + ANGLE(180.0f), LARA_GRAB_THRESHOLD * 2)));

	static bool CanPlayerUseUnderwaterWallSwitch(const ItemInfo& playerItem, const ItemInfo& switchItem)
	{
		auto& player = GetLaraInfo(playerItem);

		// Check switch status.
		if (switchItem.Status != ITEM_NOT_ACTIVE)
			return false;

		// Check for player input action.
		if (!IsHeld(In::Action))
			return false;

		// Check player status.
		if (playerItem.Animation.ActiveState == LS_UNDERWATER_IDLE &&
			player.Control.WaterStatus == WaterStatus::Underwater &&
			player.Control.HandStatus == HandStatus::Free)
		{
			return true;
		}

		return false;
	}

	static void InteractUnderwaterWallSwitch(ItemInfo& playerItem, ItemInfo& switchItem)
	{
		auto& player = GetLaraInfo(playerItem);

		if (switchItem.Animation.ActiveState != SWITCH_ON &&
			switchItem.Animation.ActiveState != SWITCH_OFF)
		{
			return;
		}

		switchItem.Status = ITEM_ACTIVE;
		switchItem.Animation.TargetState = (switchItem.Animation.ActiveState != SWITCH_ON);
		AddActiveItem(switchItem.Index);

		SetAnimation(playerItem, LA_UNDERWATER_SWITCH_PULL);
		playerItem.Animation.TargetState = LS_UNDERWATER_IDLE;
	}

	void CollideUnderwaterWallSwitch(short itemNumber, ItemInfo* playerItem, CollisionInfo* coll)
	{
		auto& switchItem = g_Level.Items[itemNumber];

		if (CanPlayerUseUnderwaterWallSwitch(*playerItem, switchItem))
			HandlePlayerInteraction(*playerItem, switchItem, UNDERWATER_WALL_SWITCH_BASIS, InteractUnderwaterWallSwitch);
	}

	static bool CanPlayerUseUnderwaterCeilingSwitch(const ItemInfo& playerItem, const ItemInfo& switchItem)
	{
		auto& player = GetLaraInfo(playerItem);

		// Check switch status.
		if (switchItem.Status != ITEM_NOT_ACTIVE ||
			switchItem.Animation.ActiveState != SWITCH_OFF)
		{
			return false;
		}

		// Check for player input action.
		if (!IsHeld(In::Action))
			return false;

		// Check player status.
		if (playerItem.Animation.ActiveState == LS_UNDERWATER_IDLE &&
			playerItem.Animation.AnimNumber == LA_UNDERWATER_IDLE &&
			player.Control.WaterStatus == WaterStatus::Underwater &&
			player.Control.HandStatus == HandStatus::Free)
		{
			return true;
		}

		return false;
	}

	static void InteractUnderwaterCeilingSwitch(ItemInfo& playerItem, ItemInfo& switchItem)
	{
		switchItem.Status = ITEM_ACTIVE;
		switchItem.Animation.TargetState = SWITCH_ON;
		AddActiveItem(switchItem.Index);

		SetAnimation(playerItem, LA_UNDERWATER_CEILING_SWITCH_PULL);
		playerItem.Animation.TargetState = LS_UNDERWATER_IDLE;
	}

	void CollideUnderwaterCeilingSwitch(short itemNumber, ItemInfo* playerItem, CollisionInfo* coll)
	{
		auto& switchItem = g_Level.Items[itemNumber];

		if (CanPlayerUseUnderwaterCeilingSwitch(*playerItem, switchItem))
		{
			if (HandlePlayerInteraction(*playerItem, switchItem, UNDERWATER_CEILING_SWITCH_FRONT_BASIS, InteractUnderwaterCeilingSwitch))
				return;

			if (HandlePlayerInteraction(*playerItem, switchItem, UNDERWATER_CEILING_SWITCH_BACK_BASIS, InteractUnderwaterCeilingSwitch))
				return;
		}
	}
}
