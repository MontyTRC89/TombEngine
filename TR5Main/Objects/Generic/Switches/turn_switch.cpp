#include "framework.h"
#include "Objects/Generic/Switches/turn_switch.h"
#include "Game/control/control.h"
#include "Specific/input.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Sound/sound.h"
#include "Specific/setup.h"
#include "Game/camera.h"
#include "Specific/level.h"
#include "Game/collision/collide_item.h"
#include "Game/collision/collide_room.h"
#include "Game/animation.h"
#include "Game/items.h"

namespace TEN::Entities::Switches
{
	enum TurnSwitchDirection
	{
		TURN_SWITCH_STOP,
		TURN_SWITCH_ANTICLOCKWISE,
		TURN_SWITCH_CLOCKWISE
	};

	OBJECT_COLLISION_BOUNDS TurnSwitchBoundsA = 
	{
		512, 896,
		0, 0,
		-512, 0,
		-ANGLE(10.0f), ANGLE(10.0f),
		-ANGLE(30.0f), ANGLE(30.0f),
		-ANGLE(10.0f), ANGLE(10.0f)
	};

	PHD_VECTOR TurnSwitchPos = { 650, 0, 138 }; 

	OBJECT_COLLISION_BOUNDS TurnSwitchBoundsC =
	{
		512, 896,
		0, 0,
		0, 512,
		-ANGLE(10.0f), ANGLE(10.0f),
		-ANGLE(30.0f), ANGLE(30.0f),
		-ANGLE(10.0f), ANGLE(10.0f)
	};

	PHD_VECTOR TurnSwitchPosA = { 650, 0, -138 };

	void TurnSwitchCollision(short itemNumber, ITEM_INFO* laraItem, COLL_INFO* coll)
	{
		auto* laraInfo = GetLaraInfo(laraItem);
		auto* switchItem = &g_Level.Items[itemNumber];

		int doSwitch = 0;

		if (TrInput & IN_ACTION &&
			laraItem->ActiveState == LS_IDLE &&
			laraItem->AnimNumber == LA_STAND_IDLE &&
			laraItem->Airborne == false &&
			laraInfo->Control.HandStatus == HandStatus::Free &&
			switchItem->ActiveState == TURN_SWITCH_STOP ||
			laraInfo->Control.IsMoving && laraInfo->InteractedItem == itemNumber)
		{
			if (TestLaraPosition(&TurnSwitchBoundsA, switchItem, laraItem))
			{
				if (MoveLaraPosition(&TurnSwitchPosA, switchItem, laraItem))
				{
					laraItem->AnimNumber = LA_TURNSWITCH_GRAB_COUNTER_CLOCKWISE;
					laraItem->FrameNumber = g_Level.Anims[LA_TURNSWITCH_GRAB_COUNTER_CLOCKWISE].frameBase;
					switchItem->AnimNumber = Objects[switchItem->ObjectNumber].animIndex + 4;
					switchItem->FrameNumber = g_Level.Anims[switchItem->AnimNumber].frameBase;
					switchItem->ItemFlags[0] = TURN_SWITCH_ANTICLOCKWISE;
					ForcedFixedCamera.x = switchItem->Position.xPos - 1024 * phd_sin(switchItem->Position.yRot);
					ForcedFixedCamera.z = switchItem->Position.zPos - 1024 * phd_cos(switchItem->Position.yRot);

					doSwitch = -1;
				}
				else
					laraInfo->InteractedItem = itemNumber;
			}
			else
			{
				laraItem->Position.yRot ^= (short)ANGLE(180.0f);
				if (TestLaraPosition(&TurnSwitchBoundsC, switchItem, laraItem))
				{
					if (MoveLaraPosition(&TurnSwitchPos, switchItem, laraItem))
					{
						laraItem->AnimNumber = LA_TURNSWITCH_GRAB_CLOCKWISE;
						laraItem->FrameNumber = g_Level.Anims[laraItem->AnimNumber].frameBase;
						switchItem->ItemFlags[0] = TURN_SWITCH_CLOCKWISE;
						ForcedFixedCamera.x = switchItem->Position.xPos + 1024 * phd_sin(switchItem->Position.yRot);
						ForcedFixedCamera.z = switchItem->Position.zPos + 1024 * phd_cos(switchItem->Position.yRot);
						doSwitch = 1;
					}
					else
						laraInfo->InteractedItem = itemNumber;
				}
				else if (laraInfo->Control.IsMoving && laraInfo->InteractedItem == itemNumber)
				{
					laraInfo->Control.IsMoving = false;
					laraInfo->Control.HandStatus = HandStatus::Free;
				}

				laraItem->Position.yRot ^= (short)ANGLE(180.0f);
			}
		}

		if (doSwitch)
		{
			short ItemNos[8];

			laraInfo->Control.IsMoving = false;
			ResetLaraFlex(laraItem);
			laraInfo->Control.HandStatus = HandStatus::Busy;
			laraItem->ActiveState = LA_REACH;

			UseForcedFixedCamera = true;
			ForcedFixedCamera.y = switchItem->Position.yPos - 2048;
			ForcedFixedCamera.roomNumber = switchItem->RoomNumber;

			AddActiveItem(itemNumber);

			switchItem->Status = ITEM_ACTIVE;
			switchItem->ItemFlags[1] = 0;

			if (GetSwitchTrigger(switchItem, ItemNos, 0))
			{
				if (!TriggerActive(&g_Level.Items[ItemNos[0]]))
				{
					g_Level.Items[ItemNos[0]].AnimNumber = Objects[g_Level.Items[ItemNos[0]].ObjectNumber].animIndex;
					g_Level.Items[ItemNos[0]].FrameNumber = g_Level.Anims[g_Level.Items[ItemNos[0]].AnimNumber].frameBase;
				}
			}
		}
		else
		{
			if (coll->Setup.EnableObjectPush && TestBoundsCollide(switchItem, laraItem, coll->Setup.Radius))
			{
				GlobalCollisionBounds.X1 = -512;
				GlobalCollisionBounds.X2 = 512;
				GlobalCollisionBounds.Y1 = -512;
				GlobalCollisionBounds.Y2 = 0;
				GlobalCollisionBounds.Z1 = -512;
				GlobalCollisionBounds.Z2 = 512;

				ItemPushItem(switchItem, laraItem, coll, 0, 2);

				GlobalCollisionBounds.X1 = 256;
				GlobalCollisionBounds.X2 = 1024;
				GlobalCollisionBounds.Z1 = -128;
				GlobalCollisionBounds.Z2 = 128;

				ItemPushItem(switchItem, laraItem, coll, 0, 2);
			}
		}
	}

	void TurnSwitchControl(short itemNumber)
	{
		auto* laraItem = LaraItem;
		auto* switchItem = &g_Level.Items[itemNumber];

		if (g_Level.Items[itemNumber].ItemFlags[0] == TURN_SWITCH_CLOCKWISE)
		{
			if (switchItem->AnimNumber == Objects[switchItem->ObjectNumber].animIndex + 2)
			{
				switchItem->Position.yRot += ANGLE(90.0f);
				if (TrInput & IN_ACTION)
				{
					laraItem->AnimNumber = LA_TURNSWITCH_PUSH_CLOCKWISE_START;
					laraItem->FrameNumber = g_Level.Anims[laraItem->AnimNumber].frameBase;

					switchItem->AnimNumber = Objects[switchItem->ObjectNumber].animIndex + 1;
					switchItem->FrameNumber = g_Level.Anims[switchItem->AnimNumber].frameBase;
				}
			}

			if (laraItem->AnimNumber == LA_TURNSWITCH_PUSH_CLOCKWISE_END &&
				laraItem->FrameNumber == g_Level.Anims[laraItem->AnimNumber].frameEnd && 
				!switchItem->ItemFlags[1])
				switchItem->ItemFlags[1] = 1;

			if (laraItem->FrameNumber >= g_Level.Anims[LA_TURNSWITCH_PUSH_CLOCKWISE_START].frameBase &&
				laraItem->FrameNumber <= g_Level.Anims[LA_TURNSWITCH_PUSH_CLOCKWISE_START].frameBase + 43 ||
				laraItem->FrameNumber >= g_Level.Anims[LA_TURNSWITCH_PUSH_CLOCKWISE_START].frameBase + 58 &&
				laraItem->FrameNumber <= g_Level.Anims[LA_TURNSWITCH_PUSH_CLOCKWISE_START].frameBase + 115)
			{
				SoundEffect(SFX_TR4_PUSHABLE_SOUND, &switchItem->Position, 2);
			}
		}
		else
		{
			if (switchItem->AnimNumber == Objects[ID_TURN_SWITCH].animIndex + 6)
			{
				switchItem->Position.yRot -= ANGLE(90.0f);
				if (TrInput & IN_ACTION)
				{
					laraItem->AnimNumber = LA_TURNSWITCH_PUSH_COUNTER_CLOCKWISE_START;
					laraItem->FrameNumber = g_Level.Anims[laraItem->AnimNumber].frameBase;
					switchItem->AnimNumber = Objects[switchItem->ObjectNumber].animIndex + 5;
					switchItem->FrameNumber = g_Level.Anims[switchItem->AnimNumber].frameBase;
				}
			}

			if (laraItem->AnimNumber == LA_TURNSWITCH_PUSH_COUNTER_CLOCKWISE_END && laraItem->FrameNumber == g_Level.Anims[LA_TURNSWITCH_PUSH_COUNTER_CLOCKWISE_END].frameEnd &&
				!switchItem->ItemFlags[1])
				switchItem->ItemFlags[1] = 1;

			if (laraItem->FrameNumber >= g_Level.Anims[LA_TURNSWITCH_PUSH_COUNTER_CLOCKWISE_START].frameBase &&
				laraItem->FrameNumber <= g_Level.Anims[LA_TURNSWITCH_PUSH_COUNTER_CLOCKWISE_START].frameBase + 43 ||
				laraItem->FrameNumber >= g_Level.Anims[LA_TURNSWITCH_PUSH_COUNTER_CLOCKWISE_START].frameBase + 58 &&
				laraItem->FrameNumber <= g_Level.Anims[LA_TURNSWITCH_PUSH_COUNTER_CLOCKWISE_START].frameBase + 115)
			{
				SoundEffect(SFX_TR4_PUSHABLE_SOUND, &switchItem->Position, 2);
			}
		}

		AnimateItem(switchItem);

		if (switchItem->ItemFlags[1] == 1)
		{
			laraItem->AnimNumber = LA_STAND_IDLE;
			laraItem->ActiveState = LS_IDLE;
			laraItem->FrameNumber = g_Level.Anims[laraItem->AnimNumber].frameBase;
			switchItem->AnimNumber = Objects[switchItem->ObjectNumber].animIndex;
			switchItem->FrameNumber = g_Level.Anims[switchItem->AnimNumber].frameBase;
			switchItem->Status = ITEM_NOT_ACTIVE;

			RemoveActiveItem(itemNumber);

			Lara.Control.HandStatus = HandStatus::Free;
			UseForcedFixedCamera = 0;
			switchItem->ItemFlags[1] = 2;
		}
	}
}
