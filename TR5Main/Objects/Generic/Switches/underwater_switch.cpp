#include "framework.h"
#include "Specific/input.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
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
		-ANGLE(80.0f), ANGLE(80.0f),
		-ANGLE(80.0f), ANGLE(80.0f),
		-ANGLE(80.0f), ANGLE(80.0f)
	};
	PHD_VECTOR UnderwaterSwitchPos = { 0, 0, 108 };

	OBJECT_COLLISION_BOUNDS CeilingUnderwaterSwitchBounds1 =
	{
		-256, 256,
		-1280, -512,
		-512, 0,
		-ANGLE(80.0f), ANGLE(80.0f),
		-ANGLE(80.0f), ANGLE(80.0f),
		-ANGLE(80.0f), ANGLE(80.0f)
	};
	PHD_VECTOR CeilingUnderwaterSwitchPos1 = { 0, -736, -416 };

	OBJECT_COLLISION_BOUNDS CeilingUnderwaterSwitchBounds2 =
	{
		-256, 256,
		-1280, -512,
		0, 512,
		-ANGLE(80.0f), ANGLE(80.0f),
		-ANGLE(80.0f), ANGLE(80.0f),
		-ANGLE(80.0f), ANGLE(80.0f)
	};
	PHD_VECTOR CeilingUnderwaterSwitchPos2 = { 0, -736, 416 };

	void UnderwaterSwitchCollision(short itemNumber, ITEM_INFO* laraItem, COLL_INFO* coll)
	{
		auto* switchItem = &g_Level.Items[itemNumber];

		if (switchItem->TriggerFlags == 0)
			WallUnderwaterSwitchCollision(itemNumber, laraItem, coll);
		else
			CeilingUnderwaterSwitchCollision(itemNumber, laraItem, coll);
	}

	void WallUnderwaterSwitchCollision(short itemNumber, ITEM_INFO* laraItem, COLL_INFO* coll)
	{
		auto* laraInfo = GetLaraInfo(laraItem);
		auto* switchItem = &g_Level.Items[itemNumber];

		if (TrInput & IN_ACTION)
		{
			if (switchItem->Status == ITEM_NOT_ACTIVE &&
				laraInfo->Control.WaterStatus == WaterStatus::Underwater &&
				laraInfo->Control.HandStatus == HandStatus::Free &&
				laraItem->Animation.ActiveState == LS_UNDERWATER_STOP)
			{
				if (TestLaraPosition(&UnderwaterSwitchBounds, switchItem, laraItem))
				{
					if (switchItem->Animation.ActiveState == SWITCH_ON ||
						switchItem->Animation.ActiveState == SWITCH_OFF)
					{
						if (MoveLaraPosition(&UnderwaterSwitchPos, switchItem, laraItem))
						{
							laraItem->Animation.VerticalVelocity = 0;
							laraItem->Animation.TargetState = LS_SWITCH_DOWN;

							do
							{
								AnimateLara(laraItem);
							} while (laraItem->Animation.TargetState != LS_SWITCH_DOWN);

							laraItem->Animation.TargetState = LS_UNDERWATER_STOP;
							laraInfo->Control.HandStatus = HandStatus::Busy;
							switchItem->Animation.TargetState = switchItem->Animation.ActiveState != SWITCH_ON;
							switchItem->Status = ITEM_ACTIVE;
							AddActiveItem(itemNumber);
							AnimateItem(switchItem);
						}
					}
				}
			}
		}
	}

	void CeilingUnderwaterSwitchCollision(short itemNumber, ITEM_INFO* laraItem, COLL_INFO* coll)
	{
		auto* laraInfo = GetLaraInfo(laraItem);
		auto* switchItem = &g_Level.Items[itemNumber];

		int flag = 0;

		if (TrInput & IN_ACTION &&
			laraItem->Animation.ActiveState == LS_UNDERWATER_STOP &&
			laraItem->Animation.AnimNumber == LA_UNDERWATER_IDLE &&
			laraInfo->Control.WaterStatus == WaterStatus::Underwater &&
			laraInfo->Control.HandStatus == HandStatus::Free &&
			switchItem->Animation.ActiveState == SWITCH_OFF ||
			laraInfo->Control.IsMoving && laraInfo->InteractedItem == itemNumber)
		{
			if (TestLaraPosition(&CeilingUnderwaterSwitchBounds1, switchItem, laraItem))
			{
				if (MoveLaraPosition(&CeilingUnderwaterSwitchPos1, switchItem, laraItem))
					flag = 1;
				else
					laraInfo->InteractedItem = itemNumber;
			}
			else
			{
				laraItem->Position.yRot ^= 0x8000;

				if (TestLaraPosition(&CeilingUnderwaterSwitchBounds2, switchItem, laraItem))
				{
					if (MoveLaraPosition(&CeilingUnderwaterSwitchPos2, switchItem, laraItem))
						flag = 1;
					else
						laraInfo->InteractedItem = itemNumber;
				}

				laraItem->Position.yRot ^= 0x8000;
			}

			if (flag)
			{
				laraItem->Animation.ActiveState = LS_SWITCH_DOWN;
				laraItem->Animation.AnimNumber = LA_UNDERWATER_CEILING_SWITCH_PULL;
				laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;
				laraItem->Animation.VerticalVelocity = 0;
				laraInfo->Control.IsMoving = false;
				laraInfo->Control.HandStatus = HandStatus::Busy;
				switchItem->Animation.TargetState = SWITCH_ON;
				switchItem->Status = ITEM_ACTIVE;

				AddActiveItem(itemNumber);

				ForcedFixedCamera.x = switchItem->Position.xPos - 1024 * phd_sin(switchItem->Position.yRot + ANGLE(90));
				ForcedFixedCamera.y = switchItem->Position.yPos - 1024;
				ForcedFixedCamera.z = switchItem->Position.zPos - 1024 * phd_cos(switchItem->Position.yRot + ANGLE(90));
				ForcedFixedCamera.roomNumber = switchItem->RoomNumber;
			}
		}
	}
}
