#include "framework.h"
#include "Game/control/control.h"
#include "Specific/Input/Input.h"
#include "Specific/level.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Objects/Generic/Switches/generic_switch.h"
#include "Game/animation.h"
#include "Game/collision/collide_item.h"
#include "Game/items.h"

using namespace TEN::Input;

namespace TEN::Entities::Switches
{
	ObjectCollisionBounds SwitchBounds = 
	{
		GameBoundingBox::Zero,
		std::pair(
			EulerAngles(ANGLE(-10.0f), ANGLE(-30.0f), ANGLE(-10.0f)),
			EulerAngles(ANGLE(10.0f), ANGLE(30.0f), ANGLE(10.0f))
		)
	};
	auto SwitchPos = Vector3i::Zero;

	void SwitchControl(short itemNumber)
	{
		auto* switchItem = &g_Level.Items[itemNumber];

		switchItem->Flags |= CODE_BITS;

		if (!TriggerActive(switchItem) && !(switchItem->Flags & IFLAG_INVISIBLE))
		{
			if (switchItem->ObjectNumber == ID_JUMP_SWITCH)
			{
				switchItem->Animation.TargetState = SWITCH_OFF;
				switchItem->Timer = 0;
				AnimateItem(switchItem);
			}
			else
			{
				switchItem->Animation.TargetState = SWITCH_ON;
				switchItem->Timer = 0;
			}
		}

		AnimateItem(switchItem);
	}

	void SwitchCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
	{
		auto* laraInfo = GetLaraInfo(laraItem);
		auto* switchItem = &g_Level.Items[itemNumber];

		if (TrInput & IN_ACTION &&
			laraItem->Animation.ActiveState == LS_IDLE &&
			laraItem->Animation.AnimNumber == LA_STAND_IDLE &&
			laraInfo->Control.HandStatus == HandStatus::Free &&
			switchItem->Status == ITEM_NOT_ACTIVE &&
			!(switchItem->Flags & 0x100) &&
			switchItem->TriggerFlags >= 0 ||
			laraInfo->Control.IsMoving && laraInfo->InteractedItem == itemNumber)
		{
			auto bounds = GameBoundingBox(switchItem);

			if ((switchItem->TriggerFlags == 3 || switchItem->TriggerFlags == 4) && switchItem->Animation.ActiveState == SWITCH_ON)
				return;

			SwitchBounds.BoundingBox.X1 = bounds.X1 - 256;
			SwitchBounds.BoundingBox.X2 = bounds.X2 + 256;

			if (switchItem->TriggerFlags)
			{
				SwitchBounds.BoundingBox.Z1 = bounds.Z1 - 512;
				SwitchBounds.BoundingBox.Z2 = bounds.Z2 + 512;

				if (switchItem->TriggerFlags == 3)
					SwitchPos.z = bounds.Z1 - 256;
				else
					SwitchPos.z = bounds.Z1 - 128;
			}
			else
			{
				SwitchBounds.BoundingBox.Z1 = bounds.Z1 - 200;
				SwitchBounds.BoundingBox.Z2 = bounds.Z2 + 200;
				SwitchPos.z = bounds.Z1 - 64;
			}

			if (TestLaraPosition(SwitchBounds, switchItem, laraItem))
			{
				if (MoveLaraPosition(SwitchPos, switchItem, laraItem))
				{
					auto onAnim = LaraAnim::LA_WALLSWITCH_DOWN;
					auto offAnim = LaraAnim::LA_WALLSWITCH_UP;

					switch (switchItem->TriggerFlags)
					{
					case 0:
						onAnim = LaraAnim::LA_WALLSWITCH_DOWN;
						offAnim = LaraAnim::LA_WALLSWITCH_UP;
						break;

					case 1:
						onAnim = LaraAnim::LA_SWITCH_SMALL_DOWN;
						offAnim = LaraAnim::LA_SWITCH_SMALL_UP;
						break;

					case 2:
						onAnim = LaraAnim::LA_BUTTON_SMALL_PUSH;
						offAnim = LaraAnim::LA_BUTTON_SMALL_PUSH;
						break;

					case 3:
						onAnim = LaraAnim::LA_BUTTON_LARGE_PUSH;
						offAnim = LaraAnim::LA_BUTTON_LARGE_PUSH;
						break;

					case 4:
						onAnim = LaraAnim::LA_BUTTON_GIANT_PUSH;
						offAnim = LaraAnim::LA_BUTTON_GIANT_PUSH;
						break;

					case 5:
						onAnim = LaraAnim::LA_VALVE_TURN;
						offAnim = LaraAnim::LA_VALVE_TURN;
						break;

					case 6:
						onAnim = LaraAnim::LA_HOLESWITCH_ACTIVATE;
						offAnim = LaraAnim::LA_HOLESWITCH_ACTIVATE;
						break;

					default:
						onAnim = (LaraAnim)(switchItem->TriggerFlags);
						offAnim = (LaraAnim)(switchItem->TriggerFlags + 1);
						break;
					}

					if (switchItem->Animation.ActiveState == SWITCH_OFF) /* Switch down */
					{
						SetAnimation(laraItem, offAnim);
						switchItem->Animation.TargetState = SWITCH_ON;
					}
					else /* Switch up */
					{
						SetAnimation(laraItem, onAnim);
						switchItem->Animation.TargetState = SWITCH_OFF;
					}

					ResetLaraFlex(laraItem);
					laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;
					laraInfo->Control.IsMoving = false;
					laraInfo->Control.HandStatus = HandStatus::Busy;

					AddActiveItem(itemNumber);
					switchItem->Status = ITEM_ACTIVE;
					AnimateItem(switchItem);
				}
				else
					laraInfo->InteractedItem = itemNumber;
			}
			else if (laraInfo->Control.IsMoving && laraInfo->InteractedItem == itemNumber)
			{
				laraInfo->Control.IsMoving = false;
				laraInfo->Control.HandStatus = HandStatus::Free;
			}

			return;
		}

		if (laraItem->Animation.ActiveState != LS_SWITCH_DOWN && laraItem->Animation.ActiveState != LS_SWITCH_UP)
			ObjectCollision(itemNumber, laraItem, coll);
	}
}
