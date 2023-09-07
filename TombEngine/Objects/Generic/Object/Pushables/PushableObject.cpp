#include "framework.h"
#include "Objects/Generic/Object/Pushables/PushableObject.h"

#include "Game/animation.h"
#include "Game/collision/collide_item.h"
#include "Game/collision/collide_room.h"
#include "Game/collision/floordata.h"
#include "Game/control/box.h"
#include "Game/control/flipeffect.h"
#include "Objects/Generic/Object/Pushables/PushableObject_BridgeCol.h"
#include "Objects/Generic/Object/Pushables/PushableObject_Info.h"
#include "Objects/Generic/Object/Pushables/PushableObject_Physics.h"
#include "Objects/Generic/Object/Pushables/PushableObject_Scans.h"
#include "Objects/Generic/Object/Pushables/PushableObject_Sounds.h"
#include "Objects/Generic/Object/Pushables/PushableObject_Stack.h"
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

	PushableInfo& GetPushableInfo(const ItemInfo& item)
	{
		return (PushableInfo&)item.Data;
	}

	std::vector<PushableAnimationInfo> PushableAnimInfos =
	{
		{ LA_PUSHABLE_PULL, LA_PUSHABLE_PUSH, LA_PUSHABLE_EDGE, true }, // TR4-TR5 animations.
		{ LA_PUSHABLE_CLASSIC_PULL, LA_PUSHABLE_CLASSIC_PUSH, LA_PUSHABLE_CLASSIC_EDGE, false } // TR1-TR3 animations.
	};

	void InitializePushableBlock(int itemNumber)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		if (pushableItem.Data == NULL) // Is first pushableItem in initialize.
		{
			InitializePushableStateMap();
			InitializePushableStacks();
		}
		
		//pushableItem.Data = PushableInfo(); // Moved to InitializePushablesStacks.
		auto& pushable = GetPushableInfo(pushableItem);

		pushable.StartPos = pushableItem.Pose.Position;
		pushable.StartPos.RoomNumber = pushableItem.RoomNumber;

		pushable.Height = GetPushableHeight(pushableItem);

		if (pushableItem.ObjectNumber >= ID_PUSHABLE_OBJECT_CLIMBABLE1 && pushableItem.ObjectNumber <= ID_PUSHABLE_OBJECT_CLIMBABLE10)
		{
			pushable.UsesRoomCollision = true;
			ActivateClimbablePushableCollider(itemNumber);
		}
		else
		{
			pushable.UsesRoomCollision = false;
		}

		SetPushableStopperFlag(true, pushableItem.Pose.Position, pushableItem.RoomNumber);

		// Read OCB flags.
		int ocb = pushableItem.TriggerFlags;
		pushable.CanFall = (ocb & (1 << 0)) != 0;						 // Check bit 0.
		pushable.DoAlignCenter = (ocb & (1 << 1)) != 1;					 // Check bit 1.
		pushable.IsBuoyant = (ocb & (1 << 2)) != 0;						 // Check bit 2.
		pushable.AnimationSystemIndex = ((ocb & (1 << 3)) != 0) ? 1 : 0; // Check bit 3.

		pushableItem.Status = ITEM_ACTIVE;
		AddActiveItem(itemNumber);
	}

	void PushableBlockControl(int itemNumber)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushable = GetPushableInfo(pushableItem);

		if (Lara.Context.InteractedItem == itemNumber && Lara.Control.IsMoving)
			return;

		// Call state handler function based current state. Functions defined in PushableObject_Physics class.
		auto stateHandlerIterator = PUSHABLE_STATE_MAP.find(pushable.BehaviourState);
		if (stateHandlerIterator != PUSHABLE_STATE_MAP.end())
		{
			stateHandlerIterator->second(itemNumber);
		}
		else
		{
			TENLog("Unknown pushable state.", LogLevel::Error, LogConfig::All, true);
		}

		// Play sound effects.
		PushablesManageSounds(itemNumber, pushable);

		// Update room number.
		AddBridgePushableStack(itemNumber, false);
		int probedRoomNumber = GetCollision(&pushableItem).RoomNumber;
		AddBridgePushableStack(itemNumber, true);

		if (pushableItem.RoomNumber != probedRoomNumber)
		{
			ItemNewRoom(itemNumber, probedRoomNumber);
			pushable.StartPos.RoomNumber = pushableItem.RoomNumber;
		}
	}
	
	void PushableBlockCollision(int itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
	{
		//This function does two actions:
		//- if Lara is pressing Action, then it start to align her and eventually activating her grabbing animation.
		//- Otherwise, this code just activates the normal object collision.

		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushable = GetPushableInfo(pushableItem);
		auto& player = *GetLaraInfo(laraItem);

		int quadrant = GetQuadrant(LaraItem->Pose.Orientation.y);
		auto& pushableSidesAttributes = pushable.SidesMap[quadrant]; // NOTE: 0 = north, 1 = east, 2 = south, 3 = west.

		// Align player to pushable.
		if ((IsHeld(In::Action) &&
			!IsHeld(In::Forward) &&
			laraItem->Animation.ActiveState == LS_IDLE &&
			laraItem->Animation.AnimNumber == LA_STAND_IDLE &&
			!laraItem->Animation.IsAirborne &&
			player.Control.HandStatus == HandStatus::Free &&
			IsPushableValid(itemNumber)) &&
			pushable.BehaviourState == PushableState::Idle && 
			(pushableSidesAttributes.Pushable || pushableSidesAttributes.Pullable) || // Player can interact with this side.
			(player.Control.IsMoving && player.Context.InteractedItem == itemNumber)) // Player already interacting.
		{
			// Set pushable bounds.
			auto bounds = GameBoundingBox(&pushableItem);
			PushableBlockBounds.BoundingBox.X1 = (bounds.X1 / 2) - 100;
			PushableBlockBounds.BoundingBox.X2 = (bounds.X2 / 2) + 100;
			PushableBlockBounds.BoundingBox.Z1 = bounds.Z1 - 200;
			PushableBlockBounds.BoundingBox.Z2 = 0;

			short yOrient = pushableItem.Pose.Orientation.y;
			pushableItem.Pose.Orientation.y = GetQuadrant(laraItem->Pose.Orientation.y) * ANGLE(90.0f);

			// If player is within interaction range, calculate target position for alignment.
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

				// Align player.
				if (MoveLaraPosition(PushableBlockPos, &pushableItem, laraItem))
				{
					SetAnimation(laraItem, LA_PUSHABLE_GRAB);
					laraItem->Pose.Orientation = pushableItem.Pose.Orientation;
					player.Control.IsMoving = false;
					player.Control.HandStatus = HandStatus::Busy;
					player.Context.NextCornerPos.Position.x = itemNumber; // TODO: Do this differently.
				}

				player.Context.InteractedItem = itemNumber;
			}
			else
			{
				// If player is outside iteraction range, set flags IsMoving false to indicate that.
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
			// If player is not holding Action, do normal collision routine.
			if (laraItem->Animation.ActiveState != LS_PUSHABLE_GRAB ||
				!TestLastFrame(laraItem, LA_PUSHABLE_GRAB) ||
				player.Context.NextCornerPos.Position.x != itemNumber)
			{
				// NOTE: If using room collision, leave it up to bridge collision system.
				if (!pushable.UsesRoomCollision)
					ObjectCollision(itemNumber, laraItem, coll);

				return;
			}
		}
	}

	int GetPushableHeight(ItemInfo& item)
	{
		int heightBoundingBox = -GameBoundingBox(&item).Y1;
		int heightWorldAligned = (heightBoundingBox / CLICK(0.5)) * CLICK(0.5);
		return heightWorldAligned;
	}

	void SetPushableStopperFlag(bool value, Vector3i& pos, int roomNumber)
	{
		auto pointColl = GetCollision(pos, roomNumber);
		pointColl.Block->Stopper = value; 

		// TODO: There is a problem, it also has to set/reset the flag in the flipped room.
		// Because when flipmaps happens, it forgets about the old flag.
	}
}