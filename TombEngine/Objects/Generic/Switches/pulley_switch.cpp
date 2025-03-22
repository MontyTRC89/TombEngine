#include "framework.h"
#include "Objects/Generic/Switches/pulley_switch.h"
#include "Game/control/control.h"
#include "Specific/Input/Input.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Objects/Generic/Switches/generic_switch.h"
#include "Sound/sound.h"
#include "Game/pickup/pickup.h"
#include "Specific/level.h"
#include "Game/collision/collide_item.h"
#include "Game/items.h"

using namespace TEN::Input;

namespace TEN::Entities::Switches
{
	const ObjectCollisionBounds PulleyBounds =
	{
		GameBoundingBox(
			-CLICK(1), CLICK(1),
			0, 0,
			-BLOCK(0.5f), BLOCK(0.5f)
		),
		std::pair(
			EulerAngles(ANGLE(-10.0f), ANGLE(-30.0f), ANGLE(-10.0f)),
			EulerAngles(ANGLE(10.0f), ANGLE(30.0f), ANGLE(10.0f))
		)
	};
	const auto PulleyPos = Vector3i(0, 0, -148);

	const ObjectCollisionBounds UnderwaterPulleyBounds =
	{
		GameBoundingBox(
			-BLOCK(0.5f), BLOCK(0.5f),
			-BLOCK(2.0f), 0,
			-BLOCK(0.5f), BLOCK(0.5f)
		),
		std::pair(
			EulerAngles(ANGLE(-80.0f), ANGLE(-80.0f), ANGLE(-80.0f)),
			EulerAngles(ANGLE(80.0f), ANGLE(80.0f), ANGLE(80.0f))
		)
	};

	//-306.05f is the hardcoded distance in the animation.
	const auto UnderwaterPulleyPos = Vector3i(0, -BLOCK(1.0f), -BLOCK(1.0f / 4));

	void InitializePulleySwitch(short itemNumber)
	{
		auto* switchItem = &g_Level.Items[itemNumber];

		switchItem->ItemFlags[3] = switchItem->TriggerFlags;
		switchItem->TriggerFlags = abs(switchItem->TriggerFlags);

		if (switchItem->Status == ITEM_INVISIBLE)
		{
			switchItem->ItemFlags[1] = 1;
			switchItem->Status = ITEM_NOT_ACTIVE;
		}
	}

	void CollisionPulleySwitch(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
	{
		auto* laraInfo = GetLaraInfo(laraItem);
		auto* switchItem = &g_Level.Items[itemNumber];

		bool isUnderwater = (laraInfo->Control.WaterStatus == WaterStatus::Underwater);

		const auto& bounds = isUnderwater ? UnderwaterPulleyBounds : PulleyBounds;
		const auto& position = isUnderwater ? UnderwaterPulleyPos : PulleyPos;

		bool isActionActive = laraInfo->Control.IsMoving && laraInfo->Context.InteractedItem == itemNumber;
		bool isActionReady = IsHeld(In::Action);
		bool isPlayerAvailable = laraInfo->Control.HandStatus == HandStatus::Free;

		//polish animations and locations
		bool isPlayerIdle = (!isUnderwater && laraItem->Animation.ActiveState == LS_IDLE && laraItem->Animation.AnimNumber == LA_STAND_IDLE && laraItem->Animation.IsAirborne == false) ||
			(isUnderwater && laraItem->Animation.ActiveState == LS_UNDERWATER_IDLE && laraItem->Animation.AnimNumber == LA_UNDERWATER_IDLE);
		if (switchItem->Status == ITEM_NOT_ACTIVE)
		{
			if (isActionActive || (isActionReady && isPlayerAvailable && isPlayerIdle))
			{
				short oldYrot = switchItem->Pose.Orientation.y;

				if (!isUnderwater)
					switchItem->Pose.Orientation.y = laraItem->Pose.Orientation.y;

				if (TestLaraPosition(bounds, switchItem, laraItem))
				{
					if (switchItem->ItemFlags[1])
					{
						if (OldPickupPos.x != laraItem->Pose.Position.x || OldPickupPos.y != laraItem->Pose.Position.y || OldPickupPos.z != laraItem->Pose.Position.z)
						{
							OldPickupPos.x = laraItem->Pose.Position.x;
							OldPickupPos.y = laraItem->Pose.Position.y;
							OldPickupPos.z = laraItem->Pose.Position.z;
							SayNo();
						}
					}
					else if (MoveLaraPosition(position, switchItem, laraItem))
					{
						ResetPlayerFlex(laraItem);
						laraItem->Animation.AnimNumber = isUnderwater ? LA_UNDERWATER_PULLEY_GRAB : LA_PULLEY_GRAB;
						laraItem->Animation.ActiveState = LS_PULLEY;
						laraItem->Animation.TargetState = LS_PULLEY;
						laraItem->Animation.FrameNumber = GetAnimData(laraItem).frameBase;
						laraInfo->Control.IsMoving = false;
						laraInfo->Control.HandStatus = HandStatus::Busy;
						laraInfo->Context.InteractedItem = itemNumber;

						AddActiveItem(itemNumber);
						switchItem->Status = ITEM_ACTIVE;
						switchItem->Animation.TargetState = SWITCH_ON;

						if (!isUnderwater)
							switchItem->Pose.Orientation.y = oldYrot;
					}
					else
						laraInfo->Context.InteractedItem = itemNumber;

					if (!isUnderwater)
						switchItem->Pose.Orientation.y = oldYrot;
				}
				else
				{
					if (laraInfo->Control.IsMoving && laraInfo->Context.InteractedItem == itemNumber)
					{
						laraInfo->Control.IsMoving = false;
						laraInfo->Control.HandStatus = HandStatus::Free;
					}

					if (!isUnderwater)
						switchItem->Pose.Orientation.y = oldYrot;
				}
			}
		}
		else if (laraItem->Animation.ActiveState != LS_PULLEY)
			ObjectCollision(itemNumber, laraItem, coll);
	}

	void ControlPulleySwitch(short itemNumber)
	{
		auto* switchItem = &g_Level.Items[itemNumber];
		AnimateItem(switchItem);

		bool isUnderwater = (Lara.Control.WaterStatus == WaterStatus::Underwater);

		if (switchItem->Animation.ActiveState == SwitchStatus::SWITCH_ON)
		{
			if (switchItem->Animation.TargetState == SwitchStatus::SWITCH_ON && !IsHeld(In::Action) && (LaraItem->Animation.AnimNumber == LA_PULLEY_PULL || LaraItem->Animation.AnimNumber == LA_UNDERWATER_PULLEY_PULL) &&
				LaraItem->Animation.FrameNumber == GetAnimData(*LaraItem).frameEnd - 1) 
			{
				TENLog("SWITCH OFF1", LogLevel::Warning);
				LaraItem->Animation.TargetState = LS_PULLEY_UNGRAB;
				switchItem->Animation.TargetState = SwitchStatus::SWITCH_OFF;
			}

			if ((LaraItem->Animation.AnimNumber == LA_PULLEY_PULL || LaraItem->Animation.AnimNumber == LA_UNDERWATER_PULLEY_PULL) &&
				LaraItem->Animation.FrameNumber == GetAnimData(*LaraItem).frameBase)
			{
				TENLog("TEST TRUE", LogLevel::Warning);
				if (switchItem->TriggerFlags)  //CHECKS IF OCB IS PRESENT
				{
					TENLog("TriggerFlags: " + std::to_string(switchItem->TriggerFlags), LogLevel::Warning);
					if (!switchItem->ItemFlags[1]) //CHECK FOR THE SWITCH BEING HIDDEN
					{
						switchItem->TriggerFlags--;
						if (switchItem->TriggerFlags) 
						{
							if (switchItem->ItemFlags[2])
							{
								switchItem->ItemFlags[2] = 0;
								switchItem->Status = ITEM_NOT_ACTIVE;
							}
						}
						else
						{
							switchItem->ItemFlags[2] = 1;
							switchItem->Status = ITEM_NOT_ACTIVE;

							if (switchItem->ItemFlags[3] >= 0)
								switchItem->TriggerFlags = abs(switchItem->ItemFlags[3]);
							else
								switchItem->ItemFlags[0] = 1;
						}
					}
				}
			}
		}
		else
		{
			if ((switchItem->Animation.FrameNumber == GetAnimData(switchItem).frameEnd)
				&& (LaraItem->Animation.AnimNumber == LA_PULLEY_RELEASE))
			{
				switchItem->Animation.ActiveState = SWITCH_OFF;
				switchItem->Status = ITEM_NOT_ACTIVE;

				RemoveActiveItem(itemNumber);

				Lara.Control.HandStatus = HandStatus::Free;
			}
			else
			{
				//If Lara is repeating the PULL animation (because player dropped after the frame check).
				//do the wheel animation again.
				switchItem->Animation.TargetState = SWITCH_ON;
			}
		}

	}
}