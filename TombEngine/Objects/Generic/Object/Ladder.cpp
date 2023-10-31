#include "framework.h"
#include "Objects/Generic/Object/Ladder.h"

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
#include "Renderer/Renderer11.h"
#include "Specific/Input/Input.h"
#include "Specific/level.h"
#include "Specific/trutils.h"

#include <OISKeyboard.h>
using namespace OIS;

using namespace TEN::Input;
using namespace TEN::Math;
using namespace TEN::Renderer;
using namespace TEN::Utils;

// NOTES:
// TriggerFlags:
//	1: is double-sided.

namespace TEN::Entities::Generic
{
	constexpr auto LADDER_STEP_HEIGHT				 = CLICK(0.5f);
	constexpr auto PLAYER_MOUNT_LEAN_ANGLE_THRESHOLD = LARA_LEAN_MAX;

	enum class LadderMountType
	{
		None,
		TopFront,
		TopBack,
		Front,
		Back,
		Left,
		Right,
		JumpUp,
		JumpReach
	};

	const auto LadderMountedPlayerStates = std::vector<int>
	{
		LS_LADDER_IDLE,
		LS_LADDER_UP,
		LS_LADDER_DOWN
	};
	const auto LadderGroundedMountPlayerStates = std::vector<int>
	{
		LS_IDLE,
		LS_TURN_LEFT_SLOW,
		LS_TURN_RIGHT_SLOW,
		LS_TURN_LEFT_FAST,
		LS_TURN_RIGHT_FAST,
		LS_WALK_FORWARD,
		LS_RUN_FORWARD
	};
	const auto LadderAirborneMountPlayerStates = std::vector<int>
	{
		LS_REACH,
		LS_JUMP_UP
	};

	const auto LadderMountedOffset2D = Vector3i(0, 0, -LARA_RADIUS * 1.5f);
	const auto LadderInteractBounds2D = GameBoundingBox(
		-BLOCK(0.25f), BLOCK(0.25f),
		0, 0,
		-BLOCK(3 / 8.0f), BLOCK(3 / 8.0f));

	const auto LadderMountTopFrontBasis = InteractionBasis(
		LadderMountedOffset2D, // TODO
		EulerAngles(0, ANGLE(180.0f), 0),
		LadderInteractBounds2D,
		std::pair(
			EulerAngles(-PLAYER_MOUNT_LEAN_ANGLE_THRESHOLD, ANGLE(180.0f) - LARA_GRAB_THRESHOLD, -PLAYER_MOUNT_LEAN_ANGLE_THRESHOLD),
			EulerAngles(PLAYER_MOUNT_LEAN_ANGLE_THRESHOLD, ANGLE(180.0f) + LARA_GRAB_THRESHOLD, PLAYER_MOUNT_LEAN_ANGLE_THRESHOLD)));

	const auto LadderMountTopBackBasis = InteractionBasis(
		LadderMountedOffset2D, // TODO
		LadderInteractBounds2D,
		std::pair(
			EulerAngles(-PLAYER_MOUNT_LEAN_ANGLE_THRESHOLD, -LARA_GRAB_THRESHOLD, -PLAYER_MOUNT_LEAN_ANGLE_THRESHOLD),
			EulerAngles(PLAYER_MOUNT_LEAN_ANGLE_THRESHOLD, LARA_GRAB_THRESHOLD, PLAYER_MOUNT_LEAN_ANGLE_THRESHOLD)));

	const auto LadderMountFrontBasis = InteractionBasis(
		LadderMountedOffset2D,
		LadderInteractBounds2D,
		std::pair(
			EulerAngles(-PLAYER_MOUNT_LEAN_ANGLE_THRESHOLD, -LARA_GRAB_THRESHOLD, -PLAYER_MOUNT_LEAN_ANGLE_THRESHOLD),
			EulerAngles(PLAYER_MOUNT_LEAN_ANGLE_THRESHOLD, LARA_GRAB_THRESHOLD, PLAYER_MOUNT_LEAN_ANGLE_THRESHOLD)));

	const auto LadderMountBackBasis = InteractionBasis(
		LadderMountedOffset2D,
		EulerAngles(0, ANGLE(180.0f), 0),
		LadderInteractBounds2D,
		std::pair(
			EulerAngles(-PLAYER_MOUNT_LEAN_ANGLE_THRESHOLD, ANGLE(180.0f) - LARA_GRAB_THRESHOLD, -PLAYER_MOUNT_LEAN_ANGLE_THRESHOLD),
			EulerAngles(PLAYER_MOUNT_LEAN_ANGLE_THRESHOLD, ANGLE(180.0f) + LARA_GRAB_THRESHOLD, PLAYER_MOUNT_LEAN_ANGLE_THRESHOLD)));

	const auto LadderMountLeftBasis = InteractionBasis(
		LadderMountedOffset2D + Vector3i(-BLOCK(0.25f), 0, 0),
		EulerAngles(0, ANGLE(90.0f), 0),
		LadderInteractBounds2D,
		std::pair(
			EulerAngles(-PLAYER_MOUNT_LEAN_ANGLE_THRESHOLD, ANGLE(90.0f) - LARA_GRAB_THRESHOLD, -PLAYER_MOUNT_LEAN_ANGLE_THRESHOLD),
			EulerAngles(PLAYER_MOUNT_LEAN_ANGLE_THRESHOLD, ANGLE(90.0f) + LARA_GRAB_THRESHOLD, PLAYER_MOUNT_LEAN_ANGLE_THRESHOLD)));

	const auto LadderMountRightBasis = InteractionBasis(
		LadderMountedOffset2D + Vector3i(BLOCK(0.25f), 0, 0),
		EulerAngles(0, ANGLE(-90.0f), 0),
		LadderInteractBounds2D,
		std::pair(
			EulerAngles(-PLAYER_MOUNT_LEAN_ANGLE_THRESHOLD, ANGLE(-90.0f) - LARA_GRAB_THRESHOLD, -PLAYER_MOUNT_LEAN_ANGLE_THRESHOLD),
			EulerAngles(PLAYER_MOUNT_LEAN_ANGLE_THRESHOLD, ANGLE(-90.0f) + LARA_GRAB_THRESHOLD, PLAYER_MOUNT_LEAN_ANGLE_THRESHOLD)));

	LadderObject::LadderObject(bool isDoubleSided)
	{
		_isDoubleSided = isDoubleSided;
	}

	void LadderObject::AttachItem(ItemInfo& item)
	{
		PushUnique(_attachedItems, &item);
	}
	
	void LadderObject::DetachItem(ItemInfo& item)
	{
		Remove(_attachedItems, &item);
	}

	LadderObject& GetLadderObject(ItemInfo& ladderItem)
	{
		return (LadderObject&)ladderItem.Data;
	}

	void InitializeLadder(short itemNumber)
	{
		auto& ladderItem = g_Level.Items[itemNumber];

		bool isDoubleSided = ((ladderItem.TriggerFlags & (1 << 0)) == 1);

		ladderItem.Data = LadderObject(isDoubleSided);
		auto& ladder = GetLadderObject(ladderItem);

		ladder.PrevPose = ladderItem.Pose;

		AddActiveItem(itemNumber);
	}

	void ControlLadder(short itemNumber)
	{
		using Vector3 = SimpleMath::Vector3;

		auto& ladderItem = g_Level.Items[itemNumber];
		auto& ladder = GetLadderObject(ladderItem);

		//---------------debug

		int step = 30;
		short angle = ANGLE(3.0f);

		// Move.
		if (KeyMap[KC_I])
		{
			ladderItem.Pose.Position = Geometry::TranslatePoint(ladderItem.Pose.Position, ladderItem.Pose.Orientation.y, step);
		}
		else if (KeyMap[KC_K])
		{
			ladderItem.Pose.Position = Geometry::TranslatePoint(ladderItem.Pose.Position, ladderItem.Pose.Orientation.y, -step);
		}
		if (KeyMap[KC_J])
		{
			ladderItem.Pose.Position = Geometry::TranslatePoint(ladderItem.Pose.Position, ladderItem.Pose.Orientation.y, 0.0f, 0.0f, -step);
		}
		else if (KeyMap[KC_L])
		{
			ladderItem.Pose.Position = Geometry::TranslatePoint(ladderItem.Pose.Position, ladderItem.Pose.Orientation.y, 0.0f, 0.0f, step);
		}

		// Rotate.
		if (KeyMap[KC_T])
		{
			ladderItem.Pose.Orientation.y += angle;
		}
		else if (KeyMap[KC_Y])
		{
			ladderItem.Pose.Orientation.y -= angle;
		}

		if (KeyMap[KC_G])
		{
			ladderItem.Pose.Orientation.x -= angle;
		}
		else if (KeyMap[KC_H])
		{
			ladderItem.Pose.Orientation.x += angle;
		}
		if (KeyMap[KC_R])
		{
			ladderItem.Pose.Orientation.z -= angle;
		}
		else if (KeyMap[KC_E])
		{
			ladderItem.Pose.Orientation.z += angle;
		}

		//--------------------

		UpdateItemRoom(itemNumber);

		// Ladder has moved; 
		if (ladder.PrevPose != ladderItem.Pose &&
			!ladder._attachedItems.empty())
		{
			auto deltaPos = ladderItem.Pose.Position - ladder.PrevPose.Position;
			auto deltaOrient = ladderItem.Pose.Orientation - ladder.PrevPose.Orientation;

			// Update attached item poses.
			for (auto* itemPtr : ladder._attachedItems)
			{
				// Adapt position and orientation.
				itemPtr->Pose.Position += deltaPos;
				itemPtr->Pose.Orientation += deltaOrient;

				// Rotate with ladder.
				auto deltaItemPos = (itemPtr->Pose.Position - ladderItem.Pose.Position).ToVector3();
				auto rotMatrix = deltaOrient.ToRotationMatrix();
				itemPtr->Pose.Position = ladderItem.Pose.Position + Vector3::Transform(deltaItemPos, rotMatrix);

				UpdateItemRoom(itemPtr->Index);
			}
		}

		ladder.PrevPose = ladderItem.Pose;
	}

	static void DrawLadderDebug(ItemInfo& ladderItem)
	{
		// Draw collision bounds.
		auto bounds = GameBoundingBox(&ladderItem);
		auto box = bounds.ToBoundingOrientedBox(ladderItem.Pose);
		g_Renderer.AddDebugBox(box, Vector4(1, 0, 0, 1), RendererDebugPage::None);

		// Draw interaction bounds.
		auto interactBounds = LadderInteractBounds2D + GameBoundingBox(0, 0, bounds.Y1, bounds.Y2, 0, 0);
		auto interactBox = interactBounds.ToBoundingOrientedBox(ladderItem.Pose);
		g_Renderer.AddDebugBox(interactBox, Vector4(0, 1, 1, 1), RendererDebugPage::None);
	}

	static bool TestLadderMount(const ItemInfo& ladderItem, const ItemInfo& playerItem)
	{
		const auto& player = GetLaraInfo(playerItem);

		// 1) Check for input action.
		if (!IsHeld(In::Action))
			return false;

		// 2) Check ladder usability.
		if (ladderItem.Flags & IFLAG_INVISIBLE)
			return false;

		// 3) Check hand status.
		if (player.Control.HandStatus != HandStatus::Free)
			return false;

		// 4) Test active player state.
		if (!TestState(playerItem.Animation.ActiveState, LadderGroundedMountPlayerStates))
			return false;

		// TODO: Assess whether ladder is in front.

		return true;
	}

	static LadderMountType GetLadderMountType(const ItemInfo& ladderItem, const ItemInfo& playerItem)
	{
		const auto& player = GetLaraInfo(playerItem);

		// Test ladder mountability.
		if (!TestLadderMount(ladderItem, playerItem))
			return LadderMountType::None;

		// Define extension for height of interaction bounds.
		// TODO: Get height of full ladder stack. Must probe above and below for ladder objects. Steal from vertical pole?
		auto bounds = GameBoundingBox(&ladderItem);
		auto boundsExtension = GameBoundingBox(0, 0, bounds.Y1, bounds.Y2 + LADDER_STEP_HEIGHT, 0, 0);

		/*if (LadderMountTopFrontBasis.TestInteraction(laraItem, ladderItem, boundsExtension))
		return LadderMountType::TopFront;

		if (LadderMountTopBackBasis.TestInteraction(laraItem, ladderItem, boundsExtension))
		return LadderMountType::TopBack;*/

		// Front mount.
		if (LadderMountFrontBasis.TestInteraction(playerItem, ladderItem, boundsExtension))
		{
			// Jump case.
			if (playerItem.Animation.IsAirborne && playerItem.Animation.Velocity.y > 0.0f)
			{
				if (playerItem.Animation.ActiveState == LS_JUMP_UP)
				{
					return LadderMountType::JumpUp;
				}
				else if (playerItem.Animation.ActiveState == LS_REACH)
				{
					return LadderMountType::JumpReach;
				}
			}
			// Regular case.
			else
			{
				return LadderMountType::Front;
			}
		}

		if (LadderMountBackBasis.TestInteraction(playerItem, ladderItem, boundsExtension))
			return LadderMountType::Back;

		/*if (LadderMountLeftBasis.TestInteraction(laraItem, ladderItem, boundsExtension))
		return LadderMountType::Left;*/

		if (LadderMountRightBasis.TestInteraction(playerItem, ladderItem, boundsExtension))
			return LadderMountType::Right;

		return LadderMountType::None;
	}

	static void HandleLadderMount(ItemInfo& ladderItem, ItemInfo& playerItem, LadderMountType mountType)
	{
		auto& ladder = GetLadderObject(ladderItem);
		auto& player = *GetLaraInfo(&playerItem);

		// Avoid interference if already interacting.
		if (playerItem.OffsetBlend.IsActive)
			player.Context.InteractedItem = ladderItem.Index;

		//ModulateLaraTurnRateY(&playerItem, 0, 0, 0);

		auto bounds = GameBoundingBox(&ladderItem);

		switch (mountType)
		{
		default:
		case LadderMountType::None:
			return;

		case LadderMountType::TopFront:
			break;

		case LadderMountType::TopBack:
			break;

		case LadderMountType::Front:
		{
			auto boundsOffset = Vector3i(0, 0, bounds.Z1);
			SetEntityInteraction(playerItem, ladderItem, LadderMountFrontBasis, boundsOffset);
			SetAnimation(&playerItem, LA_LADDER_MOUNT_FRONT);
			break;
		}

		case LadderMountType::Back:
		{
			auto boundsOffset = Vector3i(0, 0, bounds.Z2);
			SetEntityInteraction(playerItem, ladderItem, LadderMountBackBasis, boundsOffset);
			SetAnimation(&playerItem, LA_LADDER_MOUNT_FRONT);
			break;
		}

		case LadderMountType::Left:
			break;

		case LadderMountType::Right:
		{
			auto boundsOffset = Vector3i(0, 0, bounds.Z1)/* +
				Vector3i(
				0,
				round(laraItem.Pose.Position.y / ladderItem.Pose.Position.y) * LADDER_STEP_HEIGHT, // Target closest step on ladder.
				0)*/;

			SetEntityInteraction(playerItem, ladderItem, LadderMountRightBasis, boundsOffset);
			SetAnimation(&playerItem, LA_LADDER_MOUNT_RIGHT);
			break;
		}

		case LadderMountType::JumpUp:
			break;

		case LadderMountType::JumpReach:
			break;
		}

		ladder.AttachItem(playerItem);
		player.Control.IsMoving = false;
		player.Control.HandStatus = HandStatus::Busy;
	}

	void CollideLadder(short itemNumber, ItemInfo* playerItem, CollisionInfo* coll)
	{
		auto& ladderItem = g_Level.Items[itemNumber];
		auto& player = *GetLaraInfo(playerItem);

		DrawLadderDebug(ladderItem);

		// TODO!! This will be MUCH cleaner.
		auto mountType = GetLadderMountType(ladderItem, *playerItem);
		if (mountType == LadderMountType::None)
		{
			if (!TestState(playerItem->Animation.ActiveState, LadderMountedPlayerStates) &&
				playerItem->Animation.ActiveState != LS_JUMP_BACK) // TODO: Player can jump through ladders.
			{
				ObjectCollision(itemNumber, playerItem, coll);
			}
		}

		HandleLadderMount(ladderItem, *playerItem, mountType);
		return;
		
		// Grounded mount.
		if ((IsHeld(In::Action) &&
			TestState(playerItem->Animation.ActiveState, LadderGroundedMountPlayerStates) &&
			player.Control.HandStatus == HandStatus::Free) ||
			(player.Control.IsMoving && player.Context.InteractedItem == itemNumber))
		{
			if (player.Control.IsMoving && player.Context.InteractedItem == itemNumber)
			{
				player.Control.HandStatus = HandStatus::Free;
				player.Control.IsMoving = false;
			}

			return;
		}

		// Airborne mount.
		if (IsHeld(In::Action) &&
			TestState(playerItem->Animation.ActiveState, LadderAirborneMountPlayerStates) &&
			playerItem->Animation.IsAirborne &&
			playerItem->Animation.Velocity.y > 0.0f &&
			player.Control.HandStatus == HandStatus::Free)
		{
			// Test bounds collision.
			if (TestBoundsCollide(&ladderItem, playerItem, coll->Setup.Radius))//LARA_RADIUS + (int)round(abs(laraItem->Animation.Velocity.z))))
			{
				if (TestCollision(&ladderItem, playerItem))
				{
					auto bounds = GameBoundingBox(&ladderItem);
					int ladderVPos = ladderItem.Pose.Position.y + bounds.Y1;
					int playerVPos = playerItem->Pose.Position.y - LARA_HEIGHT;

					int vOffset = -abs(playerVPos - ladderVPos) / LADDER_STEP_HEIGHT;
					auto mountOffset = Vector3i(0, vOffset, bounds.Z1) + LadderMountedOffset2D;

					if (AlignPlayerToEntity(&ladderItem, playerItem, mountOffset, LadderMountFrontBasis.OrientOffset, true))
					{
						// Reaching.
						if (playerItem->Animation.ActiveState == LS_REACH)
						{
							SetAnimation(playerItem, LA_LADDER_IDLE);// LA_LADDER_MOUNT_JUMP_REACH);
						}
						// Jumping up.
						else
						{
							SetAnimation(playerItem, LA_LADDER_IDLE);// LA_LADDER_MOUNT_JUMP_UP);
						}

						playerItem->Animation.IsAirborne = false;
						playerItem->Animation.Velocity.y = 0.0f;
						player.Control.HandStatus = HandStatus::Busy;
					}
				}
			}

			return;
		}

		// Player is not interacting with ladder; do regular object collision.
		if (!TestState(playerItem->Animation.ActiveState, LadderMountedPlayerStates) &&
			playerItem->Animation.ActiveState != LS_JUMP_BACK) // TODO: Player can jump through ladders.
		{
			ObjectCollision(itemNumber, playerItem, coll);
		}
	}
}
