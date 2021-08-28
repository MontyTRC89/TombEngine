#include "framework.h"
#include "turn_switch.h"
#include "control.h"
#include "input.h"
#include "lara.h"
#include "generic_switch.h"
#include "door.h"
#include "sound.h"
#include "switch.h"

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
		int flag = 0;

		if (item->currentAnimState
			&& TrInput & IN_ACTION
			&& l->currentAnimState == LS_STOP
			&& l->animNumber == LA_STAND_IDLE
			&& !l->gravityStatus
			&& Lara.gunStatus == LG_NO_ARMS
			|| Lara.isMoving && Lara.generalPtr == (void*)itemNum)
		{
			short ItemNos[8];
			if (TestLaraPosition(&TurnSwitchBoundsA, item, l))
			{
				if (MoveLaraPosition(&TurnSwitchPosA, item, l))
				{
					l->animNumber = LA_TURNSWITCH_GRAB_COUNTER_CLOCKWISE;
					l->frameNumber = g_Level.Anims[LA_TURNSWITCH_GRAB_COUNTER_CLOCKWISE].frameBase;
					item->animNumber = Objects[item->objectNumber].animIndex + 4;
					item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
					item->itemFlags[0] = 1;
					ForcedFixedCamera.x = item->pos.xPos - 1024 * phd_sin(item->pos.yRot);
					ForcedFixedCamera.z = item->pos.zPos - 1024 * phd_cos(item->pos.yRot);
				
					Lara.isMoving = false;
					Lara.headYrot = 0;
					Lara.headXrot = 0;
					Lara.torsoYrot = 0;
					Lara.torsoXrot = 0;
					Lara.gunStatus = LG_HANDS_BUSY;
					l->currentAnimState = LA_REACH;

					UseForcedFixedCamera = true;
					ForcedFixedCamera.y = item->pos.yPos - 2048;
					ForcedFixedCamera.roomNumber = item->roomNumber;

					AddActiveItem(itemNum);

					item->status = ITEM_ACTIVE;
					item->itemFlags[1] = 0;

					if (GetSwitchTrigger(item, ItemNos, 0))
					{
						if (!TriggerActive(&g_Level.Items[ItemNos[0]]))
						{
							g_Level.Items[ItemNos[0]].animNumber = Objects[g_Level.Items[ItemNos[0]].objectNumber].animIndex;
							g_Level.Items[ItemNos[0]].frameNumber = g_Level.Anims[g_Level.Items[ItemNos[0]].animNumber].frameBase;
						}
					}
					return;
				}
				Lara.generalPtr = (void*)itemNum;
			}
			else
			{
				l->pos.yRot ^= (short)ANGLE(180);
				if (TestLaraPosition(&TurnSwitchBoundsC, item, l))
				{
					if (MoveLaraPosition(&TurnSwitchPos, item, l))
					{
						l->animNumber = 319;
						flag = 1;
						l->frameNumber = g_Level.Anims[319].frameBase;
						item->itemFlags[0] = 2;
						ForcedFixedCamera.x = item->pos.xPos + 1024 * phd_sin(item->pos.yRot);
						ForcedFixedCamera.z = item->pos.zPos + 1024 * phd_cos(item->pos.yRot);
					}
					else
					{
						Lara.generalPtr = (void*)itemNum;
					}
				}
				else if (Lara.isMoving && Lara.generalPtr == (void*)itemNum)
				{
					Lara.isMoving = false;
					Lara.gunStatus = LG_NO_ARMS;
				}
				l->pos.yRot ^= (short)ANGLE(180);
				if (flag)
				{
					Lara.isMoving = 0;
					Lara.headYrot = 0;
					Lara.headXrot = 0;
					Lara.torsoYrot = 0;
					Lara.torsoXrot = 0;
					Lara.gunStatus = LG_HANDS_BUSY;
					l->currentAnimState = LA_REACH;
					UseForcedFixedCamera = true;
					ForcedFixedCamera.y = item->pos.yPos - 2048;
					ForcedFixedCamera.roomNumber = item->roomNumber;
					AddActiveItem(itemNum);
					item->status = ITEM_ACTIVE;
					item->itemFlags[1] = 0;
					if (GetSwitchTrigger(item, ItemNos, 0))
					{
						if (!TriggerActive(&g_Level.Items[ItemNos[0]]))
						{
							g_Level.Items[ItemNos[0]].animNumber = Objects[g_Level.Items[ItemNos[0]].objectNumber].animIndex + 4;
							g_Level.Items[ItemNos[0]].frameNumber = g_Level.Anims[g_Level.Items[ItemNos[0]].animNumber].frameBase;
						}
					}
					return;
				}
			}
		}

		if (coll->enableBaddiePush && TestBoundsCollide(item, l, coll->radius))
		{
			GlobalCollisionBounds.X1 = -512;
			GlobalCollisionBounds.X2 = 512;
			GlobalCollisionBounds.Y1 = -512;
			GlobalCollisionBounds.Y2 = 0;
			GlobalCollisionBounds.Z1 = -512;
			GlobalCollisionBounds.Z2 = 512;

			ItemPushLara(item, l, coll, 0, 2);

			GlobalCollisionBounds.X1 = 256;
			GlobalCollisionBounds.X2 = 1024;
			GlobalCollisionBounds.Z1 = -128;
			GlobalCollisionBounds.Z2 = 128;

			ItemPushLara(item, l, coll, 0, 2);
		}
	}

	void TurnSwitchControl(short itemNum)
	{
		ITEM_INFO* l = LaraItem;
		ITEM_INFO* item = &g_Level.Items[itemNum];

		if (g_Level.Items[itemNum].itemFlags[0] == TURN_SWITCH_CLOCKWISE)
		{
			if (item->animNumber == Objects[item->objectNumber].animIndex + 2)
			{
				item->pos.yRot += ANGLE(90);
				if (TrInput & IN_ACTION)
				{
					l->animNumber = LA_TURNSWITCH_PUSH_CLOCKWISE_START;
					l->frameNumber = g_Level.Anims[l->animNumber].frameBase;

					item->animNumber = Objects[item->objectNumber].animIndex + 1;
					item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
				}
			}

			if (l->animNumber == LA_TURNSWITCH_PUSH_CLOCKWISE_END 
				&& l->frameNumber == g_Level.Anims[l->animNumber].frameEnd && 
				!item->itemFlags[1])
				item->itemFlags[1] = 1;

			if (l->frameNumber >= g_Level.Anims[LA_TURNSWITCH_PUSH_CLOCKWISE_START].frameBase &&
				l->frameNumber <= g_Level.Anims[LA_TURNSWITCH_PUSH_CLOCKWISE_START].frameBase + 43
				||
				l->frameNumber >= g_Level.Anims[LA_TURNSWITCH_PUSH_CLOCKWISE_START].frameBase + 58 &&
				l->frameNumber <= g_Level.Anims[LA_TURNSWITCH_PUSH_CLOCKWISE_START].frameBase + 115)
			{
				SoundEffect(SFX_TR4_PUSHABLE_SOUND, &item->pos, 2);
			}
		}
		else
		{
			if (item->animNumber == Objects[ID_TURN_SWITCH].animIndex + 6)
			{
				item->pos.yRot -= ANGLE(90);
				if (TrInput & IN_ACTION)
				{
					l->animNumber = LA_TURNSWITCH_PUSH_COUNTER_CLOCKWISE_START;
					l->frameNumber = g_Level.Anims[l->animNumber].frameBase;
					item->animNumber = Objects[item->objectNumber].animIndex + 5;
					item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
				}
			}

			if (l->animNumber == LA_TURNSWITCH_PUSH_COUNTER_CLOCKWISE_END && l->frameNumber == g_Level.Anims[LA_TURNSWITCH_PUSH_COUNTER_CLOCKWISE_END].frameEnd &&
				!item->itemFlags[1])
				item->itemFlags[1] = 1;

			if (l->frameNumber >= g_Level.Anims[LA_TURNSWITCH_PUSH_COUNTER_CLOCKWISE_START].frameBase &&
				l->frameNumber <= g_Level.Anims[LA_TURNSWITCH_PUSH_COUNTER_CLOCKWISE_START].frameBase + 43
				||
				l->frameNumber >= g_Level.Anims[LA_TURNSWITCH_PUSH_COUNTER_CLOCKWISE_START].frameBase + 58 &&
				l->frameNumber <= g_Level.Anims[LA_TURNSWITCH_PUSH_COUNTER_CLOCKWISE_START].frameBase + 115)
			{
				SoundEffect(SFX_TR4_PUSHABLE_SOUND, &item->pos, 2);
			}
		}

		AnimateItem(item);

		if (item->itemFlags[1] == 1)
		{
			l->animNumber = LA_STAND_IDLE;
			l->currentAnimState = LS_STOP;
			l->frameNumber = g_Level.Anims[l->animNumber].frameBase;
			item->animNumber = Objects[item->objectNumber].animIndex;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			item->status = ITEM_NOT_ACTIVE;

			RemoveActiveItem(itemNum);

			Lara.gunStatus = LG_NO_ARMS;
			UseForcedFixedCamera = 0;
			item->itemFlags[1] = 2;
		}
	}
}