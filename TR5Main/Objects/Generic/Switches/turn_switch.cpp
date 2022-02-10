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
		-ANGLE(10), ANGLE(10),
		-ANGLE(30), ANGLE(30),
		-ANGLE(10), ANGLE(10)
	};

	PHD_VECTOR TurnSwitchPos = { 650, 0, 138 }; 

	OBJECT_COLLISION_BOUNDS TurnSwitchBoundsC =
	{
		512, 896,
		0, 0,
		0, 512,
		-ANGLE(10), ANGLE(10),
		-ANGLE(30), ANGLE(30),
		-ANGLE(10), ANGLE(10)
	};

	PHD_VECTOR TurnSwitchPosA = { 650, 0, -138 };

	void TurnSwitchCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll)
	{
		ITEM_INFO* item = &g_Level.Items[itemNum];
		int doSwitch = 0;

		if (item->ActiveState == TURN_SWITCH_STOP
			&& TrInput & IN_ACTION
			&& l->ActiveState == LS_IDLE
			&& l->AnimNumber == LA_STAND_IDLE
			&& l->Airborne == false
			&& Lara.Control.HandStatus == HandStatus::Free
			|| Lara.Control.IsMoving && Lara.interactedItem == itemNum)
		{
			if (TestLaraPosition(&TurnSwitchBoundsA, item, l))
			{
				if (MoveLaraPosition(&TurnSwitchPosA, item, l))
				{
					l->AnimNumber = LA_TURNSWITCH_GRAB_COUNTER_CLOCKWISE;
					l->FrameNumber = g_Level.Anims[LA_TURNSWITCH_GRAB_COUNTER_CLOCKWISE].frameBase;
					item->AnimNumber = Objects[item->ObjectNumber].animIndex + 4;
					item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
					item->ItemFlags[0] = TURN_SWITCH_ANTICLOCKWISE;
					ForcedFixedCamera.x = item->Position.xPos - 1024 * phd_sin(item->Position.yRot);
					ForcedFixedCamera.z = item->Position.zPos - 1024 * phd_cos(item->Position.yRot);

					doSwitch = -1;
				}
				else
				{
					Lara.interactedItem = itemNum;
				}
			}
			else
			{
				l->Position.yRot ^= (short)ANGLE(180);
				if (TestLaraPosition(&TurnSwitchBoundsC, item, l))
				{
					if (MoveLaraPosition(&TurnSwitchPos, item, l))
					{
						l->AnimNumber = LA_TURNSWITCH_GRAB_CLOCKWISE;
						l->FrameNumber = g_Level.Anims[l->AnimNumber].frameBase;
						item->ItemFlags[0] = TURN_SWITCH_CLOCKWISE;
						ForcedFixedCamera.x = item->Position.xPos + 1024 * phd_sin(item->Position.yRot);
						ForcedFixedCamera.z = item->Position.zPos + 1024 * phd_cos(item->Position.yRot);
						doSwitch = 1;
					}
					else
					{
						Lara.interactedItem = itemNum;
					}
				}
				else if (Lara.Control.IsMoving && Lara.interactedItem == itemNum)
				{
					Lara.Control.IsMoving = false;
					Lara.Control.HandStatus = HandStatus::Free;
				}
				l->Position.yRot ^= (short)ANGLE(180);
			}
		}

		if (doSwitch)
		{
			short ItemNos[8];

			Lara.Control.IsMoving = false;
			ResetLaraFlex(l);
			Lara.Control.HandStatus = HandStatus::Busy;
			l->ActiveState = LA_REACH;

			UseForcedFixedCamera = true;
			ForcedFixedCamera.y = item->Position.yPos - 2048;
			ForcedFixedCamera.roomNumber = item->RoomNumber;

			AddActiveItem(itemNum);

			item->Status = ITEM_ACTIVE;
			item->ItemFlags[1] = 0;

			if (GetSwitchTrigger(item, ItemNos, 0))
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
			if (coll->Setup.EnableObjectPush && TestBoundsCollide(item, l, coll->Setup.Radius))
			{
				GlobalCollisionBounds.X1 = -512;
				GlobalCollisionBounds.X2 = 512;
				GlobalCollisionBounds.Y1 = -512;
				GlobalCollisionBounds.Y2 = 0;
				GlobalCollisionBounds.Z1 = -512;
				GlobalCollisionBounds.Z2 = 512;

				ItemPushItem(item, l, coll, 0, 2);

				GlobalCollisionBounds.X1 = 256;
				GlobalCollisionBounds.X2 = 1024;
				GlobalCollisionBounds.Z1 = -128;
				GlobalCollisionBounds.Z2 = 128;

				ItemPushItem(item, l, coll, 0, 2);
			}
		}
	}

	void TurnSwitchControl(short itemNum)
	{
		ITEM_INFO* l = LaraItem;
		ITEM_INFO* item = &g_Level.Items[itemNum];

		if (g_Level.Items[itemNum].ItemFlags[0] == TURN_SWITCH_CLOCKWISE)
		{
			if (item->AnimNumber == Objects[item->ObjectNumber].animIndex + 2)
			{
				item->Position.yRot += ANGLE(90);
				if (TrInput & IN_ACTION)
				{
					l->AnimNumber = LA_TURNSWITCH_PUSH_CLOCKWISE_START;
					l->FrameNumber = g_Level.Anims[l->AnimNumber].frameBase;

					item->AnimNumber = Objects[item->ObjectNumber].animIndex + 1;
					item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
				}
			}

			if (l->AnimNumber == LA_TURNSWITCH_PUSH_CLOCKWISE_END 
				&& l->FrameNumber == g_Level.Anims[l->AnimNumber].frameEnd && 
				!item->ItemFlags[1])
				item->ItemFlags[1] = 1;

			if (l->FrameNumber >= g_Level.Anims[LA_TURNSWITCH_PUSH_CLOCKWISE_START].frameBase &&
				l->FrameNumber <= g_Level.Anims[LA_TURNSWITCH_PUSH_CLOCKWISE_START].frameBase + 43
				||
				l->FrameNumber >= g_Level.Anims[LA_TURNSWITCH_PUSH_CLOCKWISE_START].frameBase + 58 &&
				l->FrameNumber <= g_Level.Anims[LA_TURNSWITCH_PUSH_CLOCKWISE_START].frameBase + 115)
			{
				SoundEffect(SFX_TR4_PUSHABLE_SOUND, &item->Position, 2);
			}
		}
		else
		{
			if (item->AnimNumber == Objects[ID_TURN_SWITCH].animIndex + 6)
			{
				item->Position.yRot -= ANGLE(90);
				if (TrInput & IN_ACTION)
				{
					l->AnimNumber = LA_TURNSWITCH_PUSH_COUNTER_CLOCKWISE_START;
					l->FrameNumber = g_Level.Anims[l->AnimNumber].frameBase;
					item->AnimNumber = Objects[item->ObjectNumber].animIndex + 5;
					item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
				}
			}

			if (l->AnimNumber == LA_TURNSWITCH_PUSH_COUNTER_CLOCKWISE_END && l->FrameNumber == g_Level.Anims[LA_TURNSWITCH_PUSH_COUNTER_CLOCKWISE_END].frameEnd &&
				!item->ItemFlags[1])
				item->ItemFlags[1] = 1;

			if (l->FrameNumber >= g_Level.Anims[LA_TURNSWITCH_PUSH_COUNTER_CLOCKWISE_START].frameBase &&
				l->FrameNumber <= g_Level.Anims[LA_TURNSWITCH_PUSH_COUNTER_CLOCKWISE_START].frameBase + 43
				||
				l->FrameNumber >= g_Level.Anims[LA_TURNSWITCH_PUSH_COUNTER_CLOCKWISE_START].frameBase + 58 &&
				l->FrameNumber <= g_Level.Anims[LA_TURNSWITCH_PUSH_COUNTER_CLOCKWISE_START].frameBase + 115)
			{
				SoundEffect(SFX_TR4_PUSHABLE_SOUND, &item->Position, 2);
			}
		}

		AnimateItem(item);

		if (item->ItemFlags[1] == 1)
		{
			l->AnimNumber = LA_STAND_IDLE;
			l->ActiveState = LS_IDLE;
			l->FrameNumber = g_Level.Anims[l->AnimNumber].frameBase;
			item->AnimNumber = Objects[item->ObjectNumber].animIndex;
			item->FrameNumber = g_Level.Anims[item->AnimNumber].frameBase;
			item->Status = ITEM_NOT_ACTIVE;

			RemoveActiveItem(itemNum);

			Lara.Control.HandStatus = HandStatus::Free;
			UseForcedFixedCamera = 0;
			item->ItemFlags[1] = 2;
		}
	}
}