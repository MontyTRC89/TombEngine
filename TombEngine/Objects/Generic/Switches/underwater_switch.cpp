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

using namespace TEN::Input;

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
	Vector3Int UnderwaterSwitchPos = { 0, 0, 108 };

	OBJECT_COLLISION_BOUNDS CeilingUnderwaterSwitchBounds1 =
	{
		-256, 256,
		-1280, -512,
		-512, 0,
		-ANGLE(80.0f), ANGLE(80.0f),
		-ANGLE(80.0f), ANGLE(80.0f),
		-ANGLE(80.0f), ANGLE(80.0f)
	};
	Vector3Int CeilingUnderwaterSwitchPos1 = { 0, -736, -416 };

	OBJECT_COLLISION_BOUNDS CeilingUnderwaterSwitchBounds2 =
	{
		-256, 256,
		-1280, -512,
		0, 512,
		-ANGLE(80.0f), ANGLE(80.0f),
		-ANGLE(80.0f), ANGLE(80.0f),
		-ANGLE(80.0f), ANGLE(80.0f)
	};
	Vector3Int CeilingUnderwaterSwitchPos2 = { 0, -736, 416 };

	void UnderwaterSwitchCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
	{
		auto* switchItem = &g_Level.Items[itemNumber];

		if (switchItem->TriggerFlags == 0)
			WallUnderwaterSwitchCollision(itemNumber, laraItem, coll);
		else
			CeilingUnderwaterSwitchCollision(itemNumber, laraItem, coll);
	}

	void WallUnderwaterSwitchCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
	{
		auto* lara = GetLaraInfo(laraItem);
		auto* switchItem = &g_Level.Items[itemNumber];

		if (TrInput & IN_ACTION &&
			switchItem->Status == ITEM_NOT_ACTIVE &&
			lara->Control.WaterStatus == WaterStatus::Underwater &&
			lara->Control.HandStatus == HandStatus::Free &&
			laraItem->Animation.ActiveState == LS_UNDERWATER_IDLE)
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

						laraItem->Animation.TargetState = LS_UNDERWATER_IDLE;
						lara->Control.HandStatus = HandStatus::Busy;
						switchItem->Animation.TargetState = switchItem->Animation.ActiveState != SWITCH_ON;
						switchItem->Status = ITEM_ACTIVE;

						AddActiveItem(itemNumber);
						AnimateItem(switchItem);
					}
				}
			}
		}
	}

	void CeilingUnderwaterSwitchCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
	{
		auto* lara = GetLaraInfo(laraItem);
		auto* switchItem = &g_Level.Items[itemNumber];

		bool flag = false;

		if ((TrInput & IN_ACTION &&
			laraItem->Animation.ActiveState == LS_UNDERWATER_IDLE &&
			laraItem->Animation.AnimNumber == LA_UNDERWATER_IDLE &&
			lara->Control.WaterStatus == WaterStatus::Underwater &&
			lara->Control.HandStatus == HandStatus::Free &&
			switchItem->Animation.ActiveState == SWITCH_OFF) ||
			(lara->Control.IsMoving && lara->InteractedItem == itemNumber))
		{
			if (TestLaraPosition(&CeilingUnderwaterSwitchBounds1, switchItem, laraItem))
			{
				if (MoveLaraPosition(&CeilingUnderwaterSwitchPos1, switchItem, laraItem))
					flag = true;
				else
					lara->InteractedItem = itemNumber;
			}
			else
			{
				laraItem->Pose.Orientation.y ^= (short)ANGLE(180.0f);

				if (TestLaraPosition(&CeilingUnderwaterSwitchBounds2, switchItem, laraItem))
				{
					if (MoveLaraPosition(&CeilingUnderwaterSwitchPos2, switchItem, laraItem))
						flag = true;
					else
						lara->InteractedItem = itemNumber;
				}

				laraItem->Pose.Orientation.y ^= (short)ANGLE(180.0f);
			}

			if (flag)
			{
				SetAnimation(laraItem, LA_UNDERWATER_CEILING_SWITCH_PULL);
				laraItem->Animation.VerticalVelocity = 0;
				lara->Control.IsMoving = false;
				lara->Control.HandStatus = HandStatus::Busy;
				switchItem->Animation.TargetState = SWITCH_ON;
				switchItem->Status = ITEM_ACTIVE;

				AddActiveItem(itemNumber);

				ForcedFixedCamera.x = switchItem->Pose.Position.x - SECTOR(1) * phd_sin(switchItem->Pose.Orientation.y + ANGLE(90.0f));
				ForcedFixedCamera.y = switchItem->Pose.Position.y - SECTOR(1);
				ForcedFixedCamera.z = switchItem->Pose.Position.z - SECTOR(1) * phd_cos(switchItem->Pose.Orientation.y + ANGLE(90.0f));
				ForcedFixedCamera.roomNumber = switchItem->RoomNumber;
			}
		}
	}
}
