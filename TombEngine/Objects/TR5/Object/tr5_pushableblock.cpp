#include "framework.h"
#include "Objects/TR5/Object/tr5_pushableblock.h"

//#include "Game/animation.h"
//#include "Game/items.h"
#include "Game/collision/collide_item.h"
//#include "Game/collision/collide_room.h"
//#include "Game/collision/floordata.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/control/box.h"
#include "Game/control/flipeffect.h"
#include "Sound/sound.h"
#include "Sound/sound_effects.h"
#include "Specific/Input/Input.h"
#include "Specific/level.h"
#include "Specific/setup.h"

using namespace TEN::Floordata;
using namespace TEN::Input;

namespace TEN::Entities::Generic
{
	constexpr auto PUSHABLE_FALL_VELOCITY_MAX	 = BLOCK(1 / 8.0f);
	constexpr auto PUSHABLE_FALL_RUMBLE_VELOCITY = 96.0f;

	static auto PushableBlockPos = Vector3i::Zero;
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
	
	std::unordered_map <FLOOR_MATERIAL, PushablesSounds> PushablesSoundsMap;

	PushableInfo& GetPushableInfo(const ItemInfo& item)
	{
		return (PushableInfo&)item.Data;
	}

	// Main functions

	void InitialisePushableBlock(short itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];
		item.Data = PushableInfo();
		auto& pushableInfo = GetPushableInfo(item);

		pushableInfo.StartPos = item.Pose.Position;
		pushableInfo.StartPos.RoomNumber = item.RoomNumber;

		if (IsClimbablePushable (item.ObjectNumber))
		{
			pushableInfo.hasFloorColission = true;
			TEN::Floordata::AddBridge(itemNumber);
		}
		else
		{
			pushableInfo.hasFloorColission = false;			
		}

		pushableInfo.height = GetPushableHeight(item);

		
		// Read OCB flags.
		const int ocb = item.TriggerFlags;

		pushableInfo.canFall			= (ocb & 0x01) != 0; // Check if bit 0 is set	(+1)
		pushableInfo.doAlignCenter		= (ocb & 0x02) != 0; // Check if bit 1 is set	(+2)
		pushableInfo.buoyancy			= (ocb & 0x04) != 0; // Check if bit 2 is set	(+4)
		pushableInfo.animationSystem	= ((ocb & 0x08) != 0)? PushableAnimationGroup::Statues : PushableAnimationGroup::Blocks; // Check if bit 3 is set	(+8)
		
		SetStopperFlag(pushableInfo.StartPos, true);
	}

	void PushableBlockControl(short itemNumber)
	{
		auto& pushableItem = g_Level.Items[itemNumber];

		if (pushableItem.Status != ITEM_ACTIVE)
			return;

		Lara.InteractedItem = itemNumber;
			
		if (PushableBlockManageFalling(itemNumber))
			return;

		switch (LaraItem->Animation.AnimNumber)
		{
		case LA_PUSHABLE_PULL:
		case LA_PUSHABLE_PUSH:
			PushableBlockManageMoving(itemNumber);
			break;

		case LA_PUSHABLE_GRAB:
		case LA_PUSHABLE_RELEASE:
		case LA_PUSHABLE_PUSH_TO_STAND:
		case LA_PUSHABLE_PULL_TO_STAND:
			break;

		default:
			PushableBlockManageIdle(itemNumber);
			break;
		}

		// Do sound effects.
		PushablesManageSounds(itemNumber);
	}

	void PushableBlockCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushableInfo = GetPushableInfo(pushableItem);
		auto& laraInfo = *GetLaraInfo(laraItem);

		if ((IsHeld(In::Action) &&
			 !IsHeld(In::Forward) &&
			 laraItem->Animation.ActiveState == LS_IDLE &&
			 laraItem->Animation.AnimNumber == LA_STAND_IDLE &&
			 !laraItem->Animation.IsAirborne &&
			 laraInfo.Control.HandStatus == HandStatus::Free &&
			 pushableItem.Status != ITEM_INVISIBLE &&
			 pushableItem.TriggerFlags >= 0) ||
			 (laraInfo.Control.IsMoving && laraInfo.InteractedItem == itemNumber))
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
					PushableBlockPos.x = pushableInfo.doAlignCenter ? 0 : LaraItem->Pose.Position.x - pushableItem.Pose.Position.x;
					break;

				case SOUTH:
					PushableBlockPos.z = bounds.Z1 - CLICK(0.4f);
					PushableBlockPos.x = pushableInfo.doAlignCenter ? 0 : pushableItem.Pose.Position.x - LaraItem->Pose.Position.x;
					break;

				case EAST:
					PushableBlockPos.z = bounds.X1 - CLICK(0.4f);
					PushableBlockPos.x = pushableInfo.doAlignCenter ? 0 : pushableItem.Pose.Position.z - LaraItem->Pose.Position.z;
					break;

				case WEST:
					PushableBlockPos.z = bounds.X1 - CLICK(0.4f);
					PushableBlockPos.x = pushableInfo.doAlignCenter ? 0 : LaraItem->Pose.Position.z - pushableItem.Pose.Position.z;
					break;

				default:
					break;
				}

				if (MoveLaraPosition(PushableBlockPos, &pushableItem, laraItem))
				{
					SetAnimation(laraItem, LA_PUSHABLE_GRAB);
					laraItem->Pose.Orientation = pushableItem.Pose.Orientation;
					laraInfo.Control.IsMoving = false;
					laraInfo.Control.HandStatus = HandStatus::Busy;
					laraInfo.NextCornerPos.Position.x = itemNumber;
				}
				else
				{
					laraInfo.InteractedItem = itemNumber;
				}
			}
			else
			{
				if (laraInfo.Control.IsMoving && laraInfo.InteractedItem == itemNumber)
				{
					laraInfo.Control.IsMoving = false;
					laraInfo.Control.HandStatus = HandStatus::Free;
				}
			}

			pushableItem.Pose.Orientation.y = yOrient;
		}
		else
		{
			//If player is not grabbing the pushable, then just do collision rutine if needed.
			if (laraItem->Animation.ActiveState != LS_PUSHABLE_GRAB ||
				!TestLastFrame(laraItem, LA_PUSHABLE_GRAB) ||
				laraInfo.NextCornerPos.Position.x != itemNumber)
			{
				if (!pushableInfo.hasFloorColission)
					ObjectCollision(itemNumber, laraItem, coll);

				return;
			}

			//Otherwise, player can input push/pull actions
			bool isPushAction = IsHeld(In::Forward);
			bool isPullAction = IsHeld(In::Back);

			if (!isPushAction && !isPullAction)
			{
				return;
			}

			int quadrant = GetQuadrant(LaraItem->Pose.Orientation.y);

			bool isQuadrantAvailable = false;
			GameVector DetectionPoint = pushableItem.Pose.Position;
			DetectionPoint.RoomNumber = pushableItem.RoomNumber;
		
			switch (quadrant)
			{
			case NORTH:
				if (isPushAction)
				{
					isQuadrantAvailable = pushableInfo.SidesMap[NORTH].pushable;
					DetectionPoint.z = DetectionPoint.z + BLOCK(1);
				}
				else if (isPullAction)
				{
					isQuadrantAvailable = pushableInfo.SidesMap[NORTH].pullable;
					DetectionPoint.z = DetectionPoint.z - BLOCK(1);
				}
				break;
			case EAST:
				if (isPushAction)
				{
					isQuadrantAvailable = pushableInfo.SidesMap[EAST].pushable;
					DetectionPoint.x = DetectionPoint.x + BLOCK(1);
				}
				else if (isPullAction)
				{
					isQuadrantAvailable = pushableInfo.SidesMap[EAST].pullable;
					DetectionPoint.x = DetectionPoint.x - BLOCK(1);
				}
				break;
			case SOUTH:
				if (isPushAction)
				{
					isQuadrantAvailable = pushableInfo.SidesMap[SOUTH].pushable;
					DetectionPoint.z = DetectionPoint.z - BLOCK(1);
				}
				else if (isPullAction)
				{
					isQuadrantAvailable = pushableInfo.SidesMap[SOUTH].pullable;
					DetectionPoint.z = DetectionPoint.z + BLOCK(1);
				}
				break;
			case WEST:
				if (isPushAction)
				{
					isQuadrantAvailable = pushableInfo.SidesMap[WEST].pushable;
					DetectionPoint.x = DetectionPoint.x - BLOCK(1);
				}
				else if (isPullAction)
				{
					isQuadrantAvailable = pushableInfo.SidesMap[WEST].pullable;
					DetectionPoint.x = DetectionPoint.x + BLOCK(1);
				}
				break;
			}

			if (!isQuadrantAvailable)
				return;
			
			if (!IsNextSectorValid(pushableItem, DetectionPoint, isPullAction))
				return;

			if (isPushAction)
			{
				laraItem->Animation.TargetState = LS_PUSHABLE_PUSH;
			}
			else if (isPullAction)
			{
				laraItem->Animation.TargetState = LS_PUSHABLE_PULL;
			}

			RemovePushableFromStack(itemNumber);
			ManageStackBridges(itemNumber, false);

			SetStopperFlag(DetectionPoint, true);

			//If the object has started to move, we activate it to do its mechanics in the Control function 
			pushableItem.Status = ITEM_ACTIVE;
			AddActiveItem(itemNumber);
			ResetLaraFlex(laraItem);

			pushableInfo.StartPos = pushableItem.Pose.Position;
			pushableInfo.StartPos.RoomNumber = pushableItem.RoomNumber;
		}
	}

	// Behaviour functions

	bool PushableBlockManageFalling(const short itemNumber)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushableInfo = GetPushableInfo(pushableItem);

		// Check if the pushable block is falling.
		if (!pushableItem.Animation.IsAirborne)
		{
			return false;
		}

		const int floorHeight = GetCollision(
												pushableItem.Pose.Position.x, 
												pushableItem.Pose.Position.y + pushableInfo.gravity, 
												pushableItem.Pose.Position.z, 
												pushableItem.RoomNumber
											).Position.Floor;

		const float currentY = pushableItem.Pose.Position.y;
		const float velocityY = pushableItem.Animation.Velocity.y;

		if (currentY < (floorHeight - velocityY))
		{
			// Apply gravity to the pushable block.
			const float newVelocityY = velocityY + pushableInfo.gravity;
			pushableItem.Animation.Velocity.y = std::min(newVelocityY, PUSHABLE_FALL_VELOCITY_MAX);
			
			// Update the pushable block's position and move the block's stack.
			pushableItem.Pose.Position.y = currentY + pushableItem.Animation.Velocity.y;
			MoveStackY(itemNumber, pushableItem.Animation.Velocity.y);
			UpdateRoomNumbers(itemNumber);
		}
		else
		{
			// The pushable block has hit the ground.
			
			const int differenceY = floorHeight - currentY;
			MoveStackY(itemNumber, differenceY);

			pushableItem.Pose.Position.y = floorHeight;

			// Shake the floor if the pushable block fell at a high enough velocity.
			if (velocityY >= PUSHABLE_FALL_RUMBLE_VELOCITY)
			{
				FloorShake(&pushableItem);
			}

			pushableItem.Animation.IsAirborne = false;
			pushableItem.Animation.Velocity.y = 0.0f;

			GameVector detectionPoint = pushableItem.Pose.Position;
			detectionPoint.RoomNumber = pushableItem.RoomNumber;

			SoundEffect(GetPushableSound(FALL, detectionPoint), &pushableItem.Pose, SoundEnvironment::Always);

			DeactivationRoutine(itemNumber);
		}

		return true;
	}

	void PushableBlockManageIdle(const short itemNumber)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushableInfo = GetPushableInfo(pushableItem);

		//If it's not moving, it places at center, do some last checks and then it deactivate itself.
		pushableItem.Pose.Position = PlaceInSectorCenter(pushableItem.Pose.Position);

		MoveStackXZ(itemNumber);
		
		DeactivationRoutine(itemNumber);
	}
	
	void PushableBlockManageMoving(const short itemNumber)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushableInfo = GetPushableInfo(pushableItem);

		// Moves pushable based on player bounds.Z2.

		const bool isLaraPulling = LaraItem->Animation.AnimNumber == LA_PUSHABLE_PULL; //else, she is pushing.

		int quadrantDir = GetQuadrant(LaraItem->Pose.Orientation.y);
		int newPosX = pushableInfo.StartPos.x;
		int newPosZ = pushableInfo.StartPos.z;
		int displaceDepth = 0;
		int displaceBox = GameBoundingBox(LaraItem).Z2;

		if (pushableInfo.soundState == PushableSoundState::Moving)
			pushableInfo.soundState = PushableSoundState::Stopping;

		displaceDepth = GetLastFrame(GAME_OBJECT_ID::ID_LARA, LaraItem->Animation.AnimNumber)->boundingBox.Z2;

		displaceBox -= isLaraPulling ? BLOCK(1) + displaceDepth : displaceDepth - BLOCK(1);
		
		if (LaraItem->Animation.FrameNumber != g_Level.Anims[LaraItem->Animation.AnimNumber].frameEnd - 1)
		{
			// Update the position

			switch (quadrantDir)
			{
			case NORTH:
				newPosZ += displaceBox;
				break;

			case EAST:
				newPosX += displaceBox;
				break;

			case SOUTH:
				newPosZ -= displaceBox;
				break;

			case WEST:
				newPosX -= displaceBox;
				break;

			default:
				break;
			}

			if (abs(pushableItem.Pose.Position.z - newPosZ) < BLOCK(0.5f))
			{
				if (isLaraPulling)
				{
					if ((quadrantDir == NORTH && pushableItem.Pose.Position.z > newPosZ) ||
						(quadrantDir == SOUTH && pushableItem.Pose.Position.z < newPosZ))
					{
						pushableItem.Pose.Position.z = newPosZ;
						pushableInfo.soundState = PushableSoundState::Moving;
					}
				}
				else
				{
					if ((quadrantDir == NORTH && pushableItem.Pose.Position.z < newPosZ) ||
						(quadrantDir == SOUTH && pushableItem.Pose.Position.z > newPosZ))
					{
						pushableItem.Pose.Position.z = newPosZ;
						pushableInfo.soundState = PushableSoundState::Moving;
					}
				}
			}

			if (abs(pushableItem.Pose.Position.x - newPosX) < BLOCK(0.5f))
			{
				if (isLaraPulling)
				{
					if ((quadrantDir == EAST && pushableItem.Pose.Position.x > newPosX) ||
						(quadrantDir == WEST && pushableItem.Pose.Position.x < newPosX))
					{
						pushableItem.Pose.Position.x = newPosX;
						pushableInfo.soundState = PushableSoundState::Moving;
					}
				}
				else
				{
					if ((quadrantDir == EAST && pushableItem.Pose.Position.x < newPosX) ||
						(quadrantDir == WEST && pushableItem.Pose.Position.x > newPosX))
					{
						pushableItem.Pose.Position.x = newPosX;
						pushableInfo.soundState = PushableSoundState::Moving;
					}
				}
			}

			MoveStackXZ(itemNumber);

		}
		else
		{
			//Manage the end of the animation

			pushableItem.Pose.Position = PlaceInSectorCenter(pushableItem.Pose.Position);

			MoveStackXZ(itemNumber);
			UpdateRoomNumbers(itemNumber);

			TestTriggers(&pushableItem, true, pushableItem.Flags & IFLAG_ACTIVATION_MASK);

			SetStopperFlag(pushableInfo.StartPos, false);

			// Check if pushing pushable through an edge into falling.Then she can't keep pushing/pulling
			if (pushableInfo.canFall && !isLaraPulling)
			{
				int floorHeight = GetCollision(pushableItem.Pose.Position.x, pushableItem.Pose.Position.y + 10, pushableItem.Pose.Position.z, pushableItem.RoomNumber).Position.Floor;// repeated?
				if (floorHeight > pushableItem.Pose.Position.y)
				{
					LaraItem->Animation.TargetState = LS_IDLE;
					pushableItem.Animation.IsAirborne = true;
					pushableInfo.soundState = PushableSoundState::None;

					return;
				}
			}

			//Otherwise, just check if action key is still pressed.
			GameVector NextPos = pushableItem.Pose.Position;
			NextPos.RoomNumber = pushableItem.RoomNumber;

			if (isLaraPulling)
				quadrantDir = (quadrantDir + 2) % 4; //Rotates the orientation 180º.

			switch (quadrantDir)
			{
			case NORTH:
				NextPos.z = NextPos.z + BLOCK(1);
				break;

			case EAST:
				NextPos.x = NextPos.x + BLOCK(1);
				break;

			case SOUTH:
				NextPos.z = NextPos.z - BLOCK(1);
				break;

			case WEST:
				NextPos.x = NextPos.x - BLOCK(1);
				break;
			}

			if (IsHeld(In::Action) &&
				IsNextSectorValid(pushableItem, NextPos, isLaraPulling))
			{
				pushableInfo.StartPos = pushableItem.Pose.Position;
				pushableInfo.StartPos.RoomNumber = pushableItem.RoomNumber;
				SetStopperFlag(NextPos, true);
			}
			else 
			{
				LaraItem->Animation.TargetState = LS_IDLE;
			}
		}
		return;
	}

	// Sound functions

	void InitializePushablesSoundsMap()
	{
		PushablesSoundsMap =
		{
			{FLOOR_MATERIAL::Mud,		PushablesSounds(SFX_TR4_PUSHABLE_SOUND, SFX_TR4_PUSH_BLOCK_END, SFX_TR4_BOULDER_FALL)},
			{FLOOR_MATERIAL::Snow,		PushablesSounds(SFX_TR4_PUSHABLE_SOUND, SFX_TR4_PUSH_BLOCK_END, SFX_TR4_BOULDER_FALL)},
			{FLOOR_MATERIAL::Sand,		PushablesSounds(SFX_TR4_PUSHABLE_SOUND, SFX_TR4_PUSH_BLOCK_END, SFX_TR4_BOULDER_FALL)},
			{FLOOR_MATERIAL::Gravel,	PushablesSounds(SFX_TR4_PUSHABLE_SOUND, SFX_TR4_PUSH_BLOCK_END, SFX_TR4_BOULDER_FALL)},
			{FLOOR_MATERIAL::Ice,		PushablesSounds(SFX_TR4_PUSHABLE_SOUND, SFX_TR4_PUSH_BLOCK_END, SFX_TR4_BOULDER_FALL)},
			{FLOOR_MATERIAL::Water,		PushablesSounds(SFX_TR4_PUSHABLE_SOUND, SFX_TR4_PUSH_BLOCK_END, SFX_TR4_BOULDER_FALL)},
			{FLOOR_MATERIAL::Stone,		PushablesSounds(SFX_TR4_PUSHABLE_SOUND, SFX_TR4_PUSH_BLOCK_END, SFX_TR4_BOULDER_FALL)},
			{FLOOR_MATERIAL::Wood,		PushablesSounds(SFX_TR4_PUSHABLE_SOUND, SFX_TR4_PUSH_BLOCK_END, SFX_TR4_BOULDER_FALL)},
			{FLOOR_MATERIAL::Metal,		PushablesSounds(SFX_TR4_PUSHABLE_SOUND, SFX_TR4_PUSH_BLOCK_END, SFX_TR4_BOULDER_FALL)},
			{FLOOR_MATERIAL::Marble,	PushablesSounds(SFX_TR4_PUSHABLE_SOUND, SFX_TR4_PUSH_BLOCK_END, SFX_TR4_BOULDER_FALL)},
			{FLOOR_MATERIAL::Grass,		PushablesSounds(SFX_TR4_PUSHABLE_SOUND, SFX_TR4_PUSH_BLOCK_END, SFX_TR4_BOULDER_FALL)},
			{FLOOR_MATERIAL::Concrete,	PushablesSounds(SFX_TR4_PUSHABLE_SOUND, SFX_TR4_PUSH_BLOCK_END, SFX_TR4_BOULDER_FALL)},
			{FLOOR_MATERIAL::OldWood,	PushablesSounds(SFX_TR4_PUSHABLE_SOUND, SFX_TR4_PUSH_BLOCK_END, SFX_TR4_BOULDER_FALL)},
			{FLOOR_MATERIAL::OldMetal,	PushablesSounds(SFX_TR4_PUSHABLE_SOUND, SFX_TR4_PUSH_BLOCK_END, SFX_TR4_BOULDER_FALL)},
			{FLOOR_MATERIAL::Custom1,	PushablesSounds(SFX_TR4_PUSHABLE_SOUND, SFX_TR4_PUSH_BLOCK_END, SFX_TR4_BOULDER_FALL)},
			{FLOOR_MATERIAL::Custom2,	PushablesSounds(SFX_TR4_PUSHABLE_SOUND, SFX_TR4_PUSH_BLOCK_END, SFX_TR4_BOULDER_FALL)},
			{FLOOR_MATERIAL::Custom3,	PushablesSounds(SFX_TR4_PUSHABLE_SOUND, SFX_TR4_PUSH_BLOCK_END, SFX_TR4_BOULDER_FALL)},
			{FLOOR_MATERIAL::Custom4,	PushablesSounds(SFX_TR4_PUSHABLE_SOUND, SFX_TR4_PUSH_BLOCK_END, SFX_TR4_BOULDER_FALL)},
			{FLOOR_MATERIAL::Custom5,	PushablesSounds(SFX_TR4_PUSHABLE_SOUND, SFX_TR4_PUSH_BLOCK_END, SFX_TR4_BOULDER_FALL)},
			{FLOOR_MATERIAL::Custom6,	PushablesSounds(SFX_TR4_PUSHABLE_SOUND, SFX_TR4_PUSH_BLOCK_END, SFX_TR4_BOULDER_FALL)},
			{FLOOR_MATERIAL::Custom7,	PushablesSounds(SFX_TR4_PUSHABLE_SOUND, SFX_TR4_PUSH_BLOCK_END, SFX_TR4_BOULDER_FALL)},
			{FLOOR_MATERIAL::Custom8,	PushablesSounds(SFX_TR4_PUSHABLE_SOUND, SFX_TR4_PUSH_BLOCK_END, SFX_TR4_BOULDER_FALL)}
		};
	}

	int GetPushableSound(const PushableSoundsType& type, const GameVector& detectionPoint)
	{
		const auto col = GetCollision(detectionPoint);
		const auto materialID = col.BottomBlock->Material;

		int resultSound = 0;
		switch (type)
		{ 
		case (LOOP):
			
			resultSound = PushablesSoundsMap[materialID].loopSound;
			break;

		case (STOP):
			resultSound = PushablesSoundsMap[materialID].stopSound;
			break;

		case (FALL):
			resultSound = PushablesSoundsMap[materialID].fallSound;
			break;

		default:
			assert(false);
			break;
		}

		return resultSound;
	}

	void PushablesManageSounds(const short itemNumber)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushableInfo = GetPushableInfo(pushableItem);

		GameVector detectionPoint = pushableItem.Pose.Position;
		detectionPoint.RoomNumber = pushableItem.RoomNumber;

		if (pushableInfo.soundState == PushableSoundState::Moving)
		{
			SoundEffect(GetPushableSound(LOOP, detectionPoint), &pushableItem.Pose, SoundEnvironment::Always);
		}
		else if (pushableInfo.soundState == PushableSoundState::Stopping)
		{
			pushableInfo.soundState = PushableSoundState::None;
			SoundEffect(GetPushableSound(STOP, detectionPoint), &pushableItem.Pose, SoundEnvironment::Always);
		}
	}

	//General functions

	void InitialisePushablesGeneral()
	{
		//To execute on level start and on level loading.
		InitializePushablesSoundsMap();
		UpdateAllPushablesStackLinks();
	}

	void DeactivationRoutine(const short itemNumber)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushableInfo = GetPushableInfo(pushableItem);

		ManageStackBridges(itemNumber, true);

		UpdateAllPushablesStackLinks();

		// If fallen on top of existing pushable, don't test triggers.
		if (pushableInfo.stackLowerItem == NO_ITEM)
		{
			TestTriggers(&pushableItem, true, pushableItem.Flags & IFLAG_ACTIVATION_MASK);
		}

		RemoveActiveItem(itemNumber);
		pushableItem.Status = ITEM_NOT_ACTIVE;
	}

	std::vector<int> FindAllPushables(const std::vector<ItemInfo>& objectsList)
	{
		std::vector<int> pushables;

		for (int i = 0; i < objectsList.size(); i++)
		{
			auto& item = objectsList[i];

			if (IsObjectPushable(item.ObjectNumber) || IsClimbablePushable(item.ObjectNumber))
				pushables.push_back(i);
		}

		return pushables;
	}

	bool IsClimbablePushable(const int ObjectNumber)
	{
		return (ObjectNumber >= ID_PUSHABLE_OBJECT_CLIMBABLE1 && ObjectNumber <= ID_PUSHABLE_OBJECT_CLIMBABLE10);
	}

	bool IsObjectPushable(const int ObjectNumber)
	{
		return (ObjectNumber >= ID_PUSHABLE_OBJECT1 && ObjectNumber <= ID_PUSHABLE_OBJECT10);
	}

	void UpdateRoomNumbers(const short itemNumber)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushableInfo = GetPushableInfo(pushableItem);

		//Check and update the room number of the pushables and others linked in the stack.
		auto col = GetCollision(pushableItem.Pose.Position);
		if (col.RoomNumber != pushableItem.RoomNumber)
		{
			ItemNewRoom(itemNumber, col.RoomNumber);
			pushableInfo.StartPos.RoomNumber = col.RoomNumber;
		}

		if (pushableInfo.stackUpperItem == NO_ITEM)
			return;

		auto pushableLinkedItem = g_Level.Items[itemNumber];
		auto& pushableLinkedInfo = GetPushableInfo(pushableLinkedItem);

		while (pushableLinkedInfo.stackUpperItem != NO_ITEM)
		{
			auto col = GetCollision(pushableLinkedItem.Pose.Position);
			if (col.RoomNumber != pushableLinkedItem.RoomNumber)
			{
				ItemNewRoom(itemNumber, col.RoomNumber);
				pushableLinkedInfo.StartPos.RoomNumber = col.RoomNumber;
			}

			pushableLinkedItem = g_Level.Items[pushableLinkedInfo.stackUpperItem];
			pushableLinkedInfo = GetPushableInfo(pushableLinkedItem);
		}
	}

	int GetPushableHeight(ItemInfo& item)
	{
		int heightBoundingBox = -GameBoundingBox(&item).Y1;
		int heightWorldAligned = (heightBoundingBox / CLICK(0.5)) * CLICK(0.5);
		return heightWorldAligned;
	}

	void ClearMovableBlockSplitters(const Vector3i& pos, short roomNumber) // TODO: Update with the new collision functions
	{
		FloorInfo* floor = GetFloor(pos.x, pos.y, pos.z, &roomNumber);
		if (floor->Box == NO_BOX) //Not walkable (blue floor flag)
			return;

		g_Level.Boxes[floor->Box].flags &= ~BLOCKED;  //Unblock the isolated box (grey floor flag)

		int height = g_Level.Boxes[floor->Box].height;
		int baseRoomNumber = roomNumber;

		floor = GetFloor(pos.x + BLOCK(1), pos.y, pos.z, &roomNumber);
		if (floor->Box != NO_BOX)
		{
			if (g_Level.Boxes[floor->Box].height == height && (g_Level.Boxes[floor->Box].flags & BLOCKABLE) && (g_Level.Boxes[floor->Box].flags & BLOCKED))
				ClearMovableBlockSplitters(Vector3i(pos.x + BLOCK(1), pos.y, pos.z), roomNumber);
		}

		roomNumber = baseRoomNumber;
		floor = GetFloor(pos.x - BLOCK(1), pos.y, pos.z, &roomNumber);
		if (floor->Box != NO_BOX)
		{
			if (g_Level.Boxes[floor->Box].height == height && (g_Level.Boxes[floor->Box].flags & BLOCKABLE) && (g_Level.Boxes[floor->Box].flags & BLOCKED))
				ClearMovableBlockSplitters(Vector3i(pos.x - BLOCK(1), pos.y, pos.z), roomNumber);
		}

		roomNumber = baseRoomNumber;
		floor = GetFloor(pos.x, pos.y, pos.z + BLOCK(1), &roomNumber);
		if (floor->Box != NO_BOX)
		{
			if (g_Level.Boxes[floor->Box].height == height && (g_Level.Boxes[floor->Box].flags & BLOCKABLE) && (g_Level.Boxes[floor->Box].flags & BLOCKED))
				ClearMovableBlockSplitters(Vector3i(pos.x, pos.y, pos.z + BLOCK(1)), roomNumber);
		}

		roomNumber = baseRoomNumber;
		floor = GetFloor(pos.x, pos.y, pos.z - BLOCK(1), &roomNumber);
		if (floor->Box != NO_BOX)
		{
			if (g_Level.Boxes[floor->Box].height == height && (g_Level.Boxes[floor->Box].flags & BLOCKABLE) && (g_Level.Boxes[floor->Box].flags & BLOCKED))
				ClearMovableBlockSplitters(Vector3i(pos.x, pos.y, pos.z - BLOCK(1)), roomNumber);
		}
	}

	// Test functions

	bool IsPushableOnValidSurface(ItemInfo& pushableItem)
	{
		auto pushableInfo = GetPushableInfo(pushableItem);

		CollisionResult col;

		if (pushableInfo.hasFloorColission)
		{
			RemoveBridge(pushableItem.Index);
			col = GetCollision(&pushableItem);
			AddBridge(pushableItem.Index);
		}
		else
		{
			col = GetCollision(&pushableItem);
		}

		//It's in a wall
		if (col.Block->IsWall(pushableItem.Pose.Position.x, pushableItem.Pose.Position.z))
			return false;

		//Or it isn't on the floor
		if (col.Position.Floor != pushableItem.Pose.Position.y)
			return false;

		return true;
	}

	bool IsNextSectorValid(ItemInfo& pushableItem, const GameVector& targetPoint, const bool checkIfLaraFits)
	{
		if (!IsPushableOnValidSurface(pushableItem))
			return false;

		if (!CheckStackLimit(pushableItem))
			return false;

		const auto& pushableInfo = GetPushableInfo(pushableItem);
		auto col = GetCollision(targetPoint);

		//It's in a wall
		if (col.Block->IsWall(targetPoint.x, targetPoint.z))
			return false;

		//Is it a sliding slope?
		if (col.Position.FloorSlope)
			return false;

		//Is diagonal floor?
		if (col.Position.DiagonalStep)
			return false;

		if ((col.Block->FloorSlope(0) != Vector2::Zero) || (col.Block->FloorSlope(1) != Vector2::Zero))
			return false;

		//Is a stopper flag tile?
		if (col.Block->Stopper)
		{
			if (col.Position.Floor <= pushableItem.Pose.Position.y)
			{
				return false;
			}
			//else
			//{
				// Maybe the stopper is because there is another pushable down there.
				// TODO: Need better search tools

			//}
		}
			

		//Is a gap or a step?, (Can it fall down?) (Only available for pushing).
		int floorDiference = abs(col.Position.Floor - pushableItem.Pose.Position.y);
		if (pushableInfo.canFall)
		{
			if ((col.Position.Floor < pushableItem.Pose.Position.y) && (floorDiference >= 32))
				return false;
		}
		else
		{
			if (floorDiference >= 32)
				return false;
		}

		//Is ceiling (square or diagonal) high enough?
		int distanceToCeiling = abs(col.Position.Ceiling - col.Position.Floor);
		const int blockHeight = GetStackHeight(pushableItem);
		if (distanceToCeiling < blockHeight)
			return false;

		//Is there any enemy or object?
		auto prevPos = pushableItem.Pose.Position;
		pushableItem.Pose.Position = targetPoint.ToVector3i();
		GetCollidedObjects(&pushableItem, BLOCK(0.25f), true, &CollidedItems[0], &CollidedMeshes[0], true);
		pushableItem.Pose.Position = prevPos;

		if (CollidedMeshes[0])
			return false;

		for (int i = 0; i < MAX_COLLIDED_OBJECTS; i++)
		{
			if (!CollidedItems[i])
				break;

			if (Objects[CollidedItems[i]->ObjectNumber].isPickup)
				continue;

			if (Objects[CollidedItems[i]->ObjectNumber].floor == nullptr) //¿¿??
				return false;

			const auto& object = Objects[CollidedItems[i]->ObjectNumber];
			int collidedIndex = CollidedItems[i] - g_Level.Items.data(); // Index of CollidedItems[i].

			auto colPos = CollidedItems[i]->Pose.Position;

			// Check if floor function returns nullopt.
			if (object.floor(collidedIndex, colPos.x, colPos.y, colPos.z) == std::nullopt)
				return false;
		}

		if (checkIfLaraFits)
		{
			if (!IsValidForLara(pushableItem, pushableInfo, targetPoint))
				return false;
		}

		return true;
	}

	bool IsValidForLara(const ItemInfo& pushableItem, const PushableInfo& pushableInfo, const GameVector& targetPoint)
	{
		auto playerOffset = Vector3i::Zero;
		auto dirVector = targetPoint.ToVector3i() - pushableItem.Pose.Position;
		
		if (dirVector.z > 0)
		{
			playerOffset.z = GetBestFrame(LaraItem)->offsetZ + BLOCK(1);
		}
		else if (dirVector.x > 0)
		{
			playerOffset.x = GetBestFrame(LaraItem)->offsetZ + BLOCK(1);
		}
		else if (dirVector.z < 0)
		{
			playerOffset.z = -GetBestFrame(LaraItem)->offsetZ - BLOCK(1);
		}
		else
		{
			playerOffset.x = -GetBestFrame(LaraItem)->offsetZ - BLOCK(1);
		}
		
		GameVector laraDetectionPoint = LaraItem->Pose.Position + playerOffset;
		laraDetectionPoint.RoomNumber = LaraItem->RoomNumber;

		CollisionResult col; 
		if (pushableInfo.hasFloorColission)
		{
			RemoveBridge(pushableItem.Index);
			col = GetCollision(laraDetectionPoint);
			AddBridge(pushableItem.Index);
		}
		else
		{
			col = GetCollision(laraDetectionPoint);
		}

		//Is a stopper flag tile? (Lara may not need this, otherwise, it's needed to remove the stopper flag in the pushable to check this condition).
		//if (col.Block->Stopper)
			//return false;

		//If floor is not flat
		if (col.Position.Floor != LaraItem->Pose.Position.y)
			return false;

		//Is ceiling (square or diagonal) high enough?
		int distanceToCeiling = abs(col.Position.Ceiling - col.Position.Floor);
		if (distanceToCeiling < LARA_HEIGHT)
			return false;

		//Is there any enemy or object?
		auto prevPos = LaraItem->Pose.Position;
		LaraItem->Pose.Position = laraDetectionPoint.ToVector3i();
		GetCollidedObjects(LaraItem, LARA_RADIUS, true, &CollidedItems[0], &CollidedMeshes[0], true);
		LaraItem->Pose.Position = prevPos;

		if (CollidedMeshes[0])
			return false;

		for (int i = 0; i < MAX_COLLIDED_OBJECTS; i++)
		{
			if (!CollidedItems[i])
				break;

			if (CollidedItems[i] == &pushableItem) // If collided item is not pushblock in which lara embedded
				continue;

			if (Objects[CollidedItems[i]->ObjectNumber].isPickup) // If it isn't a picukp
				continue;

			if (Objects[CollidedItems[i]->ObjectNumber].floor == nullptr)
				return false;
			else
			{
				const auto& object = Objects[CollidedItems[i]->ObjectNumber];
				int collidedIndex = CollidedItems[i] - g_Level.Items.data();
				int xCol = CollidedItems[i]->Pose.Position.x;
				int yCol = CollidedItems[i]->Pose.Position.y;
				int zCol = CollidedItems[i]->Pose.Position.z;

				if (object.floor(collidedIndex, xCol, yCol, zCol) == std::nullopt)
					return false;
			}
		}

		return true;
	}

	// Stack utilities functions

	void MoveStack(const short itemNumber, const Vector3i& GoalPos)
	{
		auto pushableItem = g_Level.Items[itemNumber];
		auto pushableInfo = GetPushableInfo(pushableItem);

		// Move stack together with bottom pushable
		while (pushableInfo.stackUpperItem != NO_ITEM)
		{
			pushableItem = g_Level.Items[pushableInfo.stackUpperItem];
			pushableInfo = GetPushableInfo(pushableItem);

			pushableItem.Pose.Position.x = GoalPos.x;
			pushableItem.Pose.Position.z = GoalPos.z;
			pushableItem.Pose.Position.y += GoalPos.y; //The vertical movement receives a velocity, not a fixed value.
			
			pushableInfo.StartPos = pushableItem.Pose.Position;
			pushableInfo.StartPos.RoomNumber = pushableItem.RoomNumber;

		}
	}
	void MoveStackXZ (const short itemNumber)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		MoveStack(itemNumber, Vector3i (pushableItem.Pose.Position.x, 0, pushableItem.Pose.Position.z));
	}
	void MoveStackY (const short itemNumber, const int y)
	{
		MoveStack(itemNumber, Vector3i (0, y, 0));
	}

	void ManageStackBridges(short itemNumber, bool addBridge)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushableInfo = GetPushableInfo(pushableItem);

		if (pushableInfo.hasFloorColission)
		{
			if (addBridge)
				TEN::Floordata::AddBridge(itemNumber);
			else
				TEN::Floordata::RemoveBridge(itemNumber);
		}

		if (pushableInfo.stackUpperItem == NO_ITEM)
		{
			return;
		}
		
		auto pushableLinkedItem = g_Level.Items[pushableInfo.stackUpperItem];
		auto pushableLinkedInfo = GetPushableInfo(pushableLinkedItem);

		while (pushableLinkedInfo.stackUpperItem != NO_ITEM)
		{
			if (pushableLinkedInfo.hasFloorColission)
			{
				if (addBridge)
					TEN::Floordata::AddBridge(pushableLinkedItem.Index);
				else
					TEN::Floordata::RemoveBridge(pushableLinkedItem.Index);
			}

			pushableLinkedItem = g_Level.Items[pushableLinkedInfo.stackUpperItem];
			pushableLinkedInfo = GetPushableInfo(pushableLinkedItem);
		}
	}

	void RemovePushableFromStack(short itemNumber) 
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		auto& pushableInfo = GetPushableInfo(pushableItem);

		if (pushableInfo.stackLowerItem != NO_ITEM)
		{
			auto& lowerPushable = g_Level.Items[pushableInfo.stackLowerItem];
			auto& lowerPushableInfo = GetPushableInfo(lowerPushable);
			
			lowerPushableInfo.stackUpperItem = NO_ITEM;
			pushableInfo.stackLowerItem = NO_ITEM;
		}
	}	

	void UpdateAllPushablesStackLinks()
	{
		auto& pushablesNumbersList = FindAllPushables(g_Level.Items);
	
		if (pushablesNumbersList.empty())
			return;

		std::sort(pushablesNumbersList.begin(), pushablesNumbersList.end(), CompareItemByXZ);

		for (std::size_t i = 0; i < pushablesNumbersList.size() - 1; ++i) 
		{
			auto& objectA = g_Level.Items[pushablesNumbersList[i]];
			auto& objectB = g_Level.Items[pushablesNumbersList[i + 1]];

			// Are they in the same sector?
			if ((objectA.Pose.Position.x == objectB.Pose.Position.x) && (objectA.Pose.Position.z == objectB.Pose.Position.z))
			{
				// Determine which object is up and which is down
				auto& upperPushableItem = (objectA.Pose.Position.y < objectB.Pose.Position.y) ? objectA : objectB;
				auto& lowerPushableItem = (objectA.Pose.Position.y < objectB.Pose.Position.y) ? objectB : objectA;
				
				// Set the stackUpperItem and stackLowerItem variables accordingly
				auto& upperPushableInfo = GetPushableInfo(upperPushableItem);
				auto& lowerPushableInfo = GetPushableInfo(lowerPushableItem);
				upperPushableInfo.stackLowerItem = lowerPushableItem.Index;
				lowerPushableInfo.stackUpperItem = upperPushableItem.Index;
			}
		}
	}

	int GetStackHeight(ItemInfo& item)
	{
		auto pushableItem = item;
		auto pushableInfo = GetPushableInfo(pushableItem);
		
		int height = pushableInfo.height;
		
		while (pushableInfo.stackUpperItem != NO_ITEM)
		{
			pushableItem = g_Level.Items[pushableInfo.stackUpperItem];
			pushableInfo = GetPushableInfo(pushableItem);

			height += pushableInfo.height;
		}

		return height;
	}
	bool CheckStackLimit(ItemInfo& item)
	{
		auto pushableItem = item;
		auto pushableInfo = GetPushableInfo(pushableItem);

		int limit = pushableInfo.stackLimit;
		int count = 1;
		
		while (pushableInfo.stackUpperItem != NO_ITEM)
		{
			pushableItem = g_Level.Items[pushableInfo.stackUpperItem];
			pushableInfo = GetPushableInfo(pushableItem);

			count++;

			if (count > limit)
				return false;
		}

		return true;
	}

	// Floor data collision functions

	std::optional<int> PushableBlockFloor(short itemNumber, int x, int y, int z)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		const auto& pushableInfo = GetPushableInfo(pushableItem);

		auto boxHeight = GetBridgeItemIntersect(itemNumber, x, y, z, false);
	
		if (pushableItem.Status != ITEM_INVISIBLE && pushableInfo.hasFloorColission && boxHeight.has_value())
		{
			const auto height = pushableItem.Pose.Position.y - GetPushableHeight(pushableItem);

			return std::optional{height};
		}

		return std::nullopt;
	}

	std::optional<int> PushableBlockCeiling(short itemNumber, int x, int y, int z)
	{
		auto& pushableItem = g_Level.Items[itemNumber];
		const auto& pushableInfo = GetPushableInfo(pushableItem);

		auto boxHeight = GetBridgeItemIntersect(itemNumber, x, y, z, true);

		if (pushableItem.Status != ITEM_INVISIBLE && pushableInfo.hasFloorColission && boxHeight.has_value())
			return std::optional{ pushableItem.Pose.Position.y};

		return std::nullopt;
	}

	int PushableBlockFloorBorder(short itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];

		const auto height = item.Pose.Position.y - GetPushableHeight(item);

		return height;
	}

	int PushableBlockCeilingBorder(short itemNumber)
	{
		const auto* item = &g_Level.Items[itemNumber];

		return item->Pose.Position.y;
	}
}
