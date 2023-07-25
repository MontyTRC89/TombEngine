#include "framework.h"
#include "Objects/Generic/Object/Pushables/PushableObject.h"

#include "Game/animation.h"
#include "Game/collision/collide_item.h"
#include "Game/collision/collide_room.h"
#include "Game/collision/floordata.h"
#include "Game/control/box.h"
#include "Game/control/flipeffect.h"
#include "Objects/Generic/Object/Pushables/PushableObject_Info.h"
#include "Objects/Generic/Object/Pushables/PushableObject_Sounds.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/Setup.h"
#include "Sound/sound.h"
#include "Specific/Input/Input.h"
#include "Specific/level.h"

using namespace TEN::Collision::Floordata;
using namespace TEN::Input;

namespace TEN::Entities::Generic
{
	constexpr auto PUSHABLE_FALL_VELOCITY_MAX = BLOCK(1 / 8.0f);
	constexpr auto PUSHABLE_WATER_VELOCITY_MAX = BLOCK(1 / 16.0f);
	constexpr auto PUSHABLE_FALL_RUMBLE_VELOCITY = 96.0f;
	constexpr auto PUSHABLE_HEIGHT_TOLERANCE = 32;

	constexpr auto GRAVITY_AIR = 8.0f;
	constexpr auto GRAVITY_ACCEL = 0.5f;
	constexpr auto WATER_SURFACE_DISTANCE = CLICK(0.5f);	

	auto PushableBlockPos = Vector3i::Zero;
	ObjectCollisionBounds PushableBlockBounds =
	{
		GameBoundingBox(
			0, 0,
			-CLICK(0.25f), 0,
			0, 0),
		std::pair(
			EulerAngles(ANGLE(-10.0f), -LARA_GRAB_THRESHOLD, ANGLE(-10.0f)),
			EulerAngles(ANGLE(10.0f), LARA_GRAB_THRESHOLD, ANGLE(10.0f)))
	};

	std::vector<PushableAnimationInfo> PushableAnimInfos = {};

	PushableInfo& GetPushableInfo(const ItemInfo& item)
	{
		return (PushableInfo&)item.Data;
	}

	void InitializePushableBlock(int itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];
		item.Data = PushableInfo();
		auto& pushable = GetPushableInfo(item);

		pushable.StartPos = item.Pose.Position;
		pushable.StartPos.RoomNumber = item.RoomNumber;

		if (item.ObjectNumber >= ID_PUSHABLE_OBJECT_CLIMBABLE1 && item.ObjectNumber <= ID_PUSHABLE_OBJECT_CLIMBABLE10)
		{
			pushable.UsesRoomCollision = true;
			TEN::Collision::Floordata::AddBridge(itemNumber);
		}
		else
		{
			pushable.UsesRoomCollision = false;
		}

		//pushable.Height = GetPushableHeight(item);

		// Read OCB flags.
		int ocb = item.TriggerFlags;
		pushable.CanFall = (ocb & (1 << 0)) != 0;						 // Check bit 0.
		pushable.DoAlignCenter = (ocb & (1 << 1)) != 0;					 // Check bit 1.
		pushable.IsBuoyant = (ocb & (1 << 2)) != 0;						 // Check bit 2.
		pushable.AnimationSystemIndex = ((ocb & (1 << 3)) != 0) ? 1 : 0; // Check bit 3.

		//SetStopperFlag(pushable.StartPos, true);
	}

	void PushableBlockControl(int itemNumber)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushable = GetPushableInfo(pushableItem);

		if (pushableItem.Status != ITEM_ACTIVE)
			return;

		// Check and do gravity routine if necessary.
		//if (PushableBlockManageGravity(itemNumber))
			//return;

		Lara.Context.InteractedItem = itemNumber;

		int pullAnimNumber = PushableAnimInfos[pushable.AnimationSystemIndex].PullAnimNumber;
		int pushAnimNumber = PushableAnimInfos[pushable.AnimationSystemIndex].PushAnimNumber;

		if (LaraItem->Animation.AnimNumber == pullAnimNumber || LaraItem->Animation.AnimNumber == pushAnimNumber)
		{
			pushable.GravityState = PushablePhysicState::None;
			//PushableBlockManageMoving(itemNumber);
		}
		else if (LaraItem->Animation.ActiveState == LS_IDLE)
		{
			// Do last actions and deactivate (reactivated in collision function).
			//PushableBlockManageIdle(itemNumber);
		}

		// Do sound effects.
		PushablesManageSounds(itemNumber, pushable);
	}
	
	void PushableBlockCollision(int itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushable = GetPushableInfo(pushableItem);
		auto& player = *GetLaraInfo(laraItem);

		if ((IsHeld(In::Action) &&
			!IsHeld(In::Forward) &&
			laraItem->Animation.ActiveState == LS_IDLE &&
			laraItem->Animation.AnimNumber == LA_STAND_IDLE &&
			!laraItem->Animation.IsAirborne &&
			player.Control.HandStatus == HandStatus::Free &&
			pushableItem.Status != ITEM_INVISIBLE &&
			pushableItem.TriggerFlags >= 0) ||
			(player.Control.IsMoving && player.Context.InteractedItem == itemNumber))
		{

			auto bounds = GameBoundingBox(&pushableItem);
			PushableBlockBounds.BoundingBox.X1 = (bounds.X1 / 2) - 100;
			PushableBlockBounds.BoundingBox.X2 = (bounds.X2 / 2) + 100;
			PushableBlockBounds.BoundingBox.Z1 = bounds.Z1 - 200;
			PushableBlockBounds.BoundingBox.Z2 = 0;

			short yOrient = pushableItem.Pose.Orientation.y;
			pushableItem.Pose.Orientation.y = GetQuadrant(laraItem->Pose.Orientation.y) * ANGLE(90.0f);

			if (TestLaraPosition(PushableBlockBounds, &pushableItem, laraItem))
			{
				int quadrant = GetQuadrant(pushableItem.Pose.Orientation.y);
				switch (quadrant)
				{
				case NORTH:
					PushableBlockPos.z = bounds.Z1 - CLICK(0.4f);
					PushableBlockPos.x = pushable.DoAlignCenter ? 0 : LaraItem->Pose.Position.x - pushableItem.Pose.Position.x;
					break;

				case SOUTH:
					PushableBlockPos.z = bounds.Z1 - CLICK(0.4f);
					PushableBlockPos.x = pushable.DoAlignCenter ? 0 : pushableItem.Pose.Position.x - LaraItem->Pose.Position.x;
					break;

				case EAST:
					PushableBlockPos.z = bounds.X1 - CLICK(0.4f);
					PushableBlockPos.x = pushable.DoAlignCenter ? 0 : pushableItem.Pose.Position.z - LaraItem->Pose.Position.z;
					break;

				case WEST:
					PushableBlockPos.z = bounds.X1 - CLICK(0.4f);
					PushableBlockPos.x = pushable.DoAlignCenter ? 0 : LaraItem->Pose.Position.z - pushableItem.Pose.Position.z;
					break;

				default:
					break;
				}

				if (MoveLaraPosition(PushableBlockPos, &pushableItem, laraItem))
				{
					SetAnimation(laraItem, LA_PUSHABLE_GRAB);
					laraItem->Pose.Orientation = pushableItem.Pose.Orientation;
					player.Control.IsMoving = false;
					player.Control.HandStatus = HandStatus::Busy;
					player.Context.NextCornerPos.Position.x = itemNumber; // TODO: Do this differently.
				}
				else
				{
					player.Context.InteractedItem = itemNumber;
				}
			}
			else
			{
				if (player.Control.IsMoving && player.Context.InteractedItem == itemNumber)
				{
					player.Control.IsMoving = false;
					player.Control.HandStatus = HandStatus::Free;
				}
			}

			pushableItem.Pose.Orientation.y = yOrient;
		}
		else
		{
			// If player is not grabbing pushable, simply do collision routine if needed.
			if (laraItem->Animation.ActiveState != LS_PUSHABLE_GRAB ||
				!TestLastFrame(laraItem, LA_PUSHABLE_GRAB) ||
				player.Context.NextCornerPos.Position.x != itemNumber)
			{
				if (!pushable.UsesRoomCollision)
					ObjectCollision(itemNumber, laraItem, coll);

				return;
			}

			// Otherwise, player can push/pull.
			bool hasPushAction = IsHeld(In::Forward);
			bool hasPullAction = IsHeld(In::Back);

			if (!hasPushAction && !hasPullAction)
				return;

			int quadrant = GetQuadrant(LaraItem->Pose.Orientation.y);

			bool isQuadrantAvailable = false;
			auto pos = GameVector(pushableItem.Pose.Position, pushableItem.RoomNumber);

			switch (quadrant)
			{
			case NORTH:
				if (hasPushAction)
				{
					isQuadrantAvailable = pushable.SidesMap[NORTH].Pushable;
					pos.z = pos.z + BLOCK(1);
				}
				else if (hasPullAction)
				{
					isQuadrantAvailable = pushable.SidesMap[NORTH].Pullable;
					pos.z = pos.z - BLOCK(1);
				}

				break;

			case EAST:
				if (hasPushAction)
				{
					isQuadrantAvailable = pushable.SidesMap[EAST].Pushable;
					pos.x = pos.x + BLOCK(1);
				}
				else if (hasPullAction)
				{
					isQuadrantAvailable = pushable.SidesMap[EAST].Pullable;
					pos.x = pos.x - BLOCK(1);
				}

				break;

			case SOUTH:
				if (hasPushAction)
				{
					isQuadrantAvailable = pushable.SidesMap[SOUTH].Pushable;
					pos.z = pos.z - BLOCK(1);
				}
				else if (hasPullAction)
				{
					isQuadrantAvailable = pushable.SidesMap[SOUTH].Pullable;
					pos.z = pos.z + BLOCK(1);
				}

				break;

			case WEST:
				if (hasPushAction)
				{
					isQuadrantAvailable = pushable.SidesMap[WEST].Pushable;
					pos.x = pos.x - BLOCK(1);
				}
				else if (hasPullAction)
				{
					isQuadrantAvailable = pushable.SidesMap[WEST].Pullable;
					pos.x = pos.x + BLOCK(1);
				}

				break;
			}

			if (!isQuadrantAvailable)
				return;

			//if (!IsNextSectorValid(pushableItem, pos, hasPullAction))
				//return;

			if (hasPushAction)
			{
				int pushAnim = PushableAnimInfos[pushable.AnimationSystemIndex].PushAnimNumber;
				SetAnimation(laraItem, pushAnim);
			}
			else if (hasPullAction)
			{
				int pullAnim = PushableAnimInfos[pushable.AnimationSystemIndex].PullAnimNumber;
				SetAnimation(laraItem, pullAnim);
			}

			//RemovePushableFromStack(itemNumber);
			//ManageStackBridges(itemNumber, false);

			//SetStopperFlag(pos, true);

			// If object has started to move, activate it to do its mechanics in control function.
			pushableItem.Status = ITEM_ACTIVE;
			AddActiveItem(itemNumber);
			ResetPlayerFlex(laraItem);

			pushable.StartPos = pushableItem.Pose.Position;
			pushable.StartPos.RoomNumber = pushableItem.RoomNumber;
		}
	}


}
