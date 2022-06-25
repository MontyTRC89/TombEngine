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

using namespace TEN::Input;

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

	Vector3Int TurnSwitchPos = { 650, 0, 138 }; 

	OBJECT_COLLISION_BOUNDS TurnSwitchBoundsC =
	{
		512, 896,
		0, 0,
		0, 512,
		-ANGLE(10.0f), ANGLE(10.0f),
		-ANGLE(30.0f), ANGLE(30.0f),
		-ANGLE(10.0f), ANGLE(10.0f)
	};

	Vector3Int TurnSwitchPosA = { 650, 0, -138 };

	void TurnSwitchCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
	{
		auto* laraInfo = GetLaraInfo(laraItem);
		auto* switchItem = &g_Level.Items[itemNumber];

		int doSwitch = 0;

		if (TrInput & IN_ACTION &&
			laraItem->Animation.ActiveState == LS_IDLE &&
			laraItem->Animation.AnimNumber == LA_STAND_IDLE &&
			laraItem->Animation.IsAirborne == false &&
			laraInfo->Control.HandStatus == HandStatus::Free &&
			switchItem->Animation.ActiveState == TURN_SWITCH_STOP ||
			laraInfo->Control.IsMoving && laraInfo->InteractedItem == itemNumber)
		{
			if (TestLaraPosition(&TurnSwitchBoundsA, switchItem, laraItem))
			{
				if (MoveLaraPosition(&TurnSwitchPosA, switchItem, laraItem))
				{
					laraItem->Animation.AnimNumber = LA_TURNSWITCH_GRAB_COUNTER_CLOCKWISE;
					laraItem->Animation.FrameNumber = g_Level.Anims[LA_TURNSWITCH_GRAB_COUNTER_CLOCKWISE].frameBase;
					switchItem->Animation.AnimNumber = Objects[switchItem->ObjectNumber].animIndex + 4;
					switchItem->Animation.FrameNumber = g_Level.Anims[switchItem->Animation.AnimNumber].frameBase;
					switchItem->ItemFlags[0] = TURN_SWITCH_ANTICLOCKWISE;
					ForcedFixedCamera.x = switchItem->Pose.Position.x - 1024 * phd_sin(switchItem->Pose.Orientation.y);
					ForcedFixedCamera.z = switchItem->Pose.Position.z - 1024 * phd_cos(switchItem->Pose.Orientation.y);

					doSwitch = -1;
				}
				else
					laraInfo->InteractedItem = itemNumber;
			}
			else
			{
				laraItem->Pose.Orientation.y ^= (short)ANGLE(180.0f);
				if (TestLaraPosition(&TurnSwitchBoundsC, switchItem, laraItem))
				{
					if (MoveLaraPosition(&TurnSwitchPos, switchItem, laraItem))
					{
						laraItem->Animation.AnimNumber = LA_TURNSWITCH_GRAB_CLOCKWISE;
						laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;
						switchItem->ItemFlags[0] = TURN_SWITCH_CLOCKWISE;
						ForcedFixedCamera.x = switchItem->Pose.Position.x + 1024 * phd_sin(switchItem->Pose.Orientation.y);
						ForcedFixedCamera.z = switchItem->Pose.Position.z + 1024 * phd_cos(switchItem->Pose.Orientation.y);
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

				laraItem->Pose.Orientation.y ^= (short)ANGLE(180.0f);
			}
		}

		if (doSwitch)
		{
			short ItemNos[8];

			laraInfo->Control.IsMoving = false;
			ResetLaraFlex(laraItem);
			laraInfo->Control.HandStatus = HandStatus::Busy;
			laraItem->Animation.ActiveState = LA_REACH;

			UseForcedFixedCamera = true;
			ForcedFixedCamera.y = switchItem->Pose.Position.y - 2048;
			ForcedFixedCamera.roomNumber = switchItem->RoomNumber;

			AddActiveItem(itemNumber);

			switchItem->Status = ITEM_ACTIVE;
			switchItem->ItemFlags[1] = 0;

			if (GetSwitchTrigger(switchItem, ItemNos, 0))
			{
				if (!TriggerActive(&g_Level.Items[ItemNos[0]]))
				{
					g_Level.Items[ItemNos[0]].Animation.AnimNumber = Objects[g_Level.Items[ItemNos[0]].ObjectNumber].animIndex;
					g_Level.Items[ItemNos[0]].Animation.FrameNumber = g_Level.Anims[g_Level.Items[ItemNos[0]].Animation.AnimNumber].frameBase;
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
			if (switchItem->Animation.AnimNumber == Objects[switchItem->ObjectNumber].animIndex + 2)
			{
				switchItem->Pose.Orientation.y += ANGLE(90.0f);
				if (TrInput & IN_ACTION)
				{
					laraItem->Animation.AnimNumber = LA_TURNSWITCH_PUSH_CLOCKWISE_START;
					laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;

					switchItem->Animation.AnimNumber = Objects[switchItem->ObjectNumber].animIndex + 1;
					switchItem->Animation.FrameNumber = g_Level.Anims[switchItem->Animation.AnimNumber].frameBase;
				}
			}

			if (laraItem->Animation.AnimNumber == LA_TURNSWITCH_PUSH_CLOCKWISE_END &&
				laraItem->Animation.FrameNumber == g_Level.Anims[laraItem->Animation.AnimNumber].frameEnd && 
				!switchItem->ItemFlags[1])
				switchItem->ItemFlags[1] = 1;

			if (laraItem->Animation.FrameNumber >= g_Level.Anims[LA_TURNSWITCH_PUSH_CLOCKWISE_START].frameBase &&
				laraItem->Animation.FrameNumber <= g_Level.Anims[LA_TURNSWITCH_PUSH_CLOCKWISE_START].frameBase + 43 ||
				laraItem->Animation.FrameNumber >= g_Level.Anims[LA_TURNSWITCH_PUSH_CLOCKWISE_START].frameBase + 58 &&
				laraItem->Animation.FrameNumber <= g_Level.Anims[LA_TURNSWITCH_PUSH_CLOCKWISE_START].frameBase + 115)
			{
				SoundEffect(SFX_TR4_PUSHABLE_SOUND, &switchItem->Pose, SoundEnvironment::Always);
			}
		}
		else
		{
			if (switchItem->Animation.AnimNumber == Objects[ID_TURN_SWITCH].animIndex + 6)
			{
				switchItem->Pose.Orientation.y -= ANGLE(90.0f);
				if (TrInput & IN_ACTION)
				{
					laraItem->Animation.AnimNumber = LA_TURNSWITCH_PUSH_COUNTER_CLOCKWISE_START;
					laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;
					switchItem->Animation.AnimNumber = Objects[switchItem->ObjectNumber].animIndex + 5;
					switchItem->Animation.FrameNumber = g_Level.Anims[switchItem->Animation.AnimNumber].frameBase;
				}
			}

			if (laraItem->Animation.AnimNumber == LA_TURNSWITCH_PUSH_COUNTER_CLOCKWISE_END && laraItem->Animation.FrameNumber == g_Level.Anims[LA_TURNSWITCH_PUSH_COUNTER_CLOCKWISE_END].frameEnd &&
				!switchItem->ItemFlags[1])
				switchItem->ItemFlags[1] = 1;

			if (laraItem->Animation.FrameNumber >= g_Level.Anims[LA_TURNSWITCH_PUSH_COUNTER_CLOCKWISE_START].frameBase &&
				laraItem->Animation.FrameNumber <= g_Level.Anims[LA_TURNSWITCH_PUSH_COUNTER_CLOCKWISE_START].frameBase + 43 ||
				laraItem->Animation.FrameNumber >= g_Level.Anims[LA_TURNSWITCH_PUSH_COUNTER_CLOCKWISE_START].frameBase + 58 &&
				laraItem->Animation.FrameNumber <= g_Level.Anims[LA_TURNSWITCH_PUSH_COUNTER_CLOCKWISE_START].frameBase + 115)
			{
				SoundEffect(SFX_TR4_PUSHABLE_SOUND, &switchItem->Pose, SoundEnvironment::Always);
			}
		}

		AnimateItem(switchItem);

		if (switchItem->ItemFlags[1] == 1)
		{
			laraItem->Animation.AnimNumber = LA_STAND_IDLE;
			laraItem->Animation.ActiveState = LS_IDLE;
			laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;
			switchItem->Animation.AnimNumber = Objects[switchItem->ObjectNumber].animIndex;
			switchItem->Animation.FrameNumber = g_Level.Anims[switchItem->Animation.AnimNumber].frameBase;
			switchItem->Status = ITEM_NOT_ACTIVE;

			RemoveActiveItem(itemNumber);

			Lara.Control.HandStatus = HandStatus::Free;
			UseForcedFixedCamera = 0;
			switchItem->ItemFlags[1] = 2;
		}
	}
}
