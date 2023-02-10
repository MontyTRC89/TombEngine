#include "framework.h"
#include "Game/animation.h"
#include "Game/collision/collide_item.h"
#include "Game/control/control.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Objects/Generic/Switches/generic_switch.h"
#include "Specific/Input/Input.h"
#include "Specific/level.h"

using namespace TEN::Input;

namespace TEN::Entities::Switches
{
	InteractionBasis SwitchBounds = 
	{
		GameBoundingBox::Zero,
		std::pair(
			EulerAngles(ANGLE(-10.0f), ANGLE(-30.0f), ANGLE(-10.0f)),
			EulerAngles(ANGLE(10.0f), ANGLE(30.0f), ANGLE(10.0f))
		)
	};

	enum SwitchOCBs
	{
		SWT_BIG_LEVER = 0,
		SWT_SMALL_LEVER = 1,
		SWT_SMALL_BUTTON = 2,
		SWT_BIG_BUTTON = 3,
		SWT_GIANT_BUTTON = 4,
		SWT_VALVE = 5,
		SWT_WALL_HOLE = 6,
		SWT_CUSTOM = 7
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

		if (IsHeld(In::Action) &&
			laraItem->Animation.ActiveState == LS_IDLE &&
			laraItem->Animation.AnimNumber == LA_STAND_IDLE &&
			laraInfo->Control.HandStatus == HandStatus::Free &&
			switchItem->Status == ITEM_NOT_ACTIVE &&
			!(switchItem->Flags & 0x100) &&
			switchItem->TriggerFlags >= 0 ||
			laraInfo->Control.IsMoving && laraInfo->InteractedItem == itemNumber)
		{
			auto bounds = GameBoundingBox(switchItem);

			if ((switchItem->TriggerFlags == 3 || switchItem->TriggerFlags == 4) && switchItem->Animation.ActiveState == SWITCH_OFF)
				return;

			SwitchBounds.Bounds.X1 = bounds.X1 - BLOCK(0.25);
			SwitchBounds.Bounds.X2 = bounds.X2 + BLOCK(0.25);

			switch (switchItem->TriggerFlags)
			{
				SwitchBounds.Bounds.Z1 = bounds.Z1 - BLOCK(0.5f);
				SwitchBounds.Bounds.Z2 = bounds.Z2 + BLOCK(0.5f);

				case SWT_BIG_LEVER:
					SwitchPos.z = bounds.Z1 - 64;
					SwitchBounds.Bounds.Z1 = bounds.Z1 - BLOCK(0.2);
					SwitchBounds.Bounds.Z2 = bounds.Z2 + BLOCK(0.2);
					break;

				case SWT_SMALL_LEVER:
					SwitchPos.z = bounds.Z1 - 112;
					SwitchBounds.Bounds.Z1 = bounds.Z1 - BLOCK(0.25);
					SwitchBounds.Bounds.Z2 = bounds.Z2 + BLOCK(0.25);
					break;

				case SWT_SMALL_BUTTON:
					SwitchPos.z = bounds.Z1 - 156;
					SwitchBounds.Bounds.Z1 = bounds.Z1 - BLOCK(0.2);
					SwitchBounds.Bounds.Z2 = bounds.Z2 + BLOCK(0.2);
					break;

				case SWT_BIG_BUTTON:
					SwitchPos.z = bounds.Z1 - 256;
					SwitchBounds.Bounds.Z1 = bounds.Z1 - BLOCK(0.5);
					SwitchBounds.Bounds.Z2 = bounds.Z2 + BLOCK(0.5);
					break;

				case SWT_GIANT_BUTTON:
					SwitchPos.z = bounds.Z1 - 384;
					SwitchBounds.Bounds.Z1 = bounds.Z1 - BLOCK(0.5);
					SwitchBounds.Bounds.Z2 = bounds.Z2 + BLOCK(0.5);
					break;

				case SWT_VALVE:
					SwitchPos.z = bounds.Z1 - 112;
					SwitchBounds.Bounds.Z1 = bounds.Z1 - BLOCK(0.25);
					SwitchBounds.Bounds.Z2 = bounds.Z2 + BLOCK(0.25);
					break;

				case SWT_WALL_HOLE:
					SwitchPos.z = bounds.Z1 - 196;
					SwitchBounds.Bounds.Z1 = bounds.Z1 - BLOCK(0.2);
					SwitchBounds.Bounds.Z2 = bounds.Z2 + BLOCK(0.2);
					break;

				case SWT_CUSTOM:
					SwitchPos.z = bounds.Z1 - switchItem->ItemFlags[6];
					SwitchBounds.Bounds.Z1 = bounds.Z1 - BLOCK(0.5);
					SwitchBounds.Bounds.Z2 = bounds.Z2 + BLOCK(0.5);
					break;
			}

			if (TestPlayerEntityInteract(switchItem, laraItem, SwitchBounds))
			{
				if (AlignPlayerToEntity(switchItem, laraItem, SwitchPos))
				{
					int onAnim = LaraAnim::LA_WALLSWITCH_DOWN;
					int offAnim = LaraAnim::LA_WALLSWITCH_UP;

					switch (switchItem->TriggerFlags)
					{
						case SWT_BIG_LEVER:
							onAnim = LaraAnim::LA_WALLSWITCH_DOWN;
							offAnim = LaraAnim::LA_WALLSWITCH_UP;
							break;

						case SWT_SMALL_LEVER:
							onAnim = LaraAnim::LA_SWITCH_SMALL_DOWN;
							offAnim = LaraAnim::LA_SWITCH_SMALL_UP;
							break;

						case SWT_SMALL_BUTTON:
							onAnim = LaraAnim::LA_BUTTON_SMALL_PUSH;
							offAnim = LaraAnim::LA_BUTTON_SMALL_PUSH;
							break;

						case SWT_BIG_BUTTON:
							onAnim = LaraAnim::LA_BUTTON_LARGE_PUSH;
							offAnim = LaraAnim::LA_BUTTON_LARGE_PUSH;
							break;

						case SWT_GIANT_BUTTON:
							onAnim = LaraAnim::LA_BUTTON_GIANT_PUSH;
							offAnim = LaraAnim::LA_BUTTON_GIANT_PUSH;
							break;

						case SWT_VALVE:
							onAnim = LaraAnim::LA_VALVE_TURN;
							offAnim = LaraAnim::LA_VALVE_TURN;
							break;

						case SWT_WALL_HOLE:
							onAnim = LaraAnim::LA_HOLESWITCH_ACTIVATE;
							offAnim = LaraAnim::LA_HOLESWITCH_ACTIVATE;
							break;

						case SWT_CUSTOM:
							onAnim = abs(switchItem->ItemFlags[4]);
							offAnim = abs(switchItem->ItemFlags[5]);								
							break;

						default:
							onAnim = (LaraAnim)(switchItem->TriggerFlags);
							offAnim = (LaraAnim)(switchItem->TriggerFlags + 1);
							break;
					}

					if (switchItem->Animation.ActiveState == SWITCH_OFF)
					{
						SetAnimation(laraItem, offAnim);
						switchItem->Animation.TargetState = SWITCH_ON;
					}
					else
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
