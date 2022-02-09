#include "framework.h"
#include "Specific/input.h"
#include "Game/Lara/lara.h"
#include "Objects/Generic/Switches/underwater_switch.h"
#include "Objects/Generic/Switches/generic_switch.h"
#include "Game/camera.h"
#include "Game/collision/collide_item.h"
#include "Specific/level.h"
#include "Game/animation.h"
#include "Game/items.h"


namespace TEN::Entities::Switches
{ 
	OBJECT_COLLISION_BOUNDS UnderwaterSwitchBounds =
	{
		-1024, 1024,
		-1024, 1024,
		-1024, 512,
		-ANGLE(80), ANGLE(80),
		-ANGLE(80), ANGLE(80),
		-ANGLE(80), ANGLE(80)
	};

	PHD_VECTOR UnderwaterSwitchPos = { 0, 0, 108 };

	OBJECT_COLLISION_BOUNDS CeilingUnderwaterSwitchBounds1 =
	{
		-256, 256,
		-1280, -512,
		-512, 0,
		-ANGLE(80), ANGLE(80),
		-ANGLE(80), ANGLE(80),
		-ANGLE(80), ANGLE(80)
	};

	PHD_VECTOR CeilingUnderwaterSwitchPos1 = { 0, -736, -416 };

	OBJECT_COLLISION_BOUNDS CeilingUnderwaterSwitchBounds2 =
	{
		-256, 256,
		-1280, -512,
		0, 512,
		-ANGLE(80), ANGLE(80),
		-ANGLE(80), ANGLE(80),
		-ANGLE(80), ANGLE(80)
	};

	PHD_VECTOR CeilingUnderwaterSwitchPos2 = { 0, -736, 416 };

	void UnderwaterSwitchCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll)
	{
		ITEM_INFO* item = &g_Level.Items[itemNum];

		if (item->TriggerFlags == 0)
		{
			WallUnderwaterSwitchCollision(itemNum, l, coll);
		}
		else
		{
			CeilingUnderwaterSwitchCollision(itemNum, l, coll);
		}
	}

	void WallUnderwaterSwitchCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll)
	{
		ITEM_INFO* item = &g_Level.Items[itemNum];

		if (TrInput & IN_ACTION)
		{
			if (item->Status == ITEM_NOT_ACTIVE
				&& Lara.waterStatus == LW_UNDERWATER
				&& !Lara.gunStatus
				&& l->ActiveState == LS_UNDERWATER_STOP)
			{
				if (TestLaraPosition(&UnderwaterSwitchBounds, item, l))
				{
					if (item->ActiveState == SWITCH_ON
						|| item->ActiveState == SWITCH_OFF)
					{
						if (MoveLaraPosition(&UnderwaterSwitchPos, item, l))
						{
							l->VerticalVelocity = 0;
							l->TargetState = LS_SWITCH_DOWN;

							do
							{
								AnimateLara(l);
							} while (l->TargetState != LS_SWITCH_DOWN);

							l->TargetState = LS_UNDERWATER_STOP;
							Lara.gunStatus = LG_HANDS_BUSY;
							item->TargetState = item->ActiveState != SWITCH_ON;
							item->Status = ITEM_ACTIVE;
							AddActiveItem(itemNum);
							AnimateItem(item);
						}
					}
				}
			}
		}
	}

	void CeilingUnderwaterSwitchCollision(short itemNum, ITEM_INFO* l, COLL_INFO* coll)
	{
		ITEM_INFO* item = &g_Level.Items[itemNum];
		int flag = 0;

		if (TrInput & IN_ACTION
			&& Lara.waterStatus == LW_UNDERWATER
			&& l->ActiveState == LS_UNDERWATER_STOP
			&& l->AnimNumber == LA_UNDERWATER_IDLE
			&& !Lara.gunStatus
			&& (item->ActiveState == SWITCH_OFF)
			|| Lara.Control.IsMoving && Lara.interactedItem == itemNum)
		{
			if (TestLaraPosition(&CeilingUnderwaterSwitchBounds1, item, l))
			{
				if (MoveLaraPosition(&CeilingUnderwaterSwitchPos1, item, l))
					flag = 1;
				else
					Lara.interactedItem = itemNum;
			}
			else
			{
				l->Position.yRot ^= 0x8000;

				if (TestLaraPosition(&CeilingUnderwaterSwitchBounds2, item, l))
				{
					if (MoveLaraPosition(&CeilingUnderwaterSwitchPos2, item, l))
						flag = 1;
					else
						Lara.interactedItem = itemNum;
				}

				l->Position.yRot ^= 0x8000;
			}

			if (flag)
			{
				l->ActiveState = LS_SWITCH_DOWN;
				l->AnimNumber = LA_UNDERWATER_CEILING_SWITCH_PULL;
				l->FrameNumber = g_Level.Anims[l->AnimNumber].frameBase;
				l->VerticalVelocity = 0;
				Lara.Control.IsMoving = false;
				Lara.gunStatus = LG_HANDS_BUSY;
				item->TargetState = SWITCH_ON;
				item->Status = ITEM_ACTIVE;

				AddActiveItem(itemNum);

				ForcedFixedCamera.x = item->Position.xPos - 1024 * phd_sin(item->Position.yRot + ANGLE(90));
				ForcedFixedCamera.y = item->Position.yPos - 1024;
				ForcedFixedCamera.z = item->Position.zPos - 1024 * phd_cos(item->Position.yRot + ANGLE(90));
				ForcedFixedCamera.roomNumber = item->RoomNumber;
			}
		}
	}
}