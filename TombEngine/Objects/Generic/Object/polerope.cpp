#include "framework.h"
#include "Specific/level.h"
#include "Game/control/control.h"
#include "Game/control/box.h"
#include "Game/items.h"
#include "Game/control/lot.h"
#include "Specific/input.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/Lara/lara_struct.h"
#include "Game/Lara/lara_tests.h"
#include "Game/Lara/lara.h"
#include "Specific/trmath.h"
#include "Game/collision/collide_item.h"
#include "Game/collision/sphere.h"

using namespace TEN::Input;

namespace TEN::Entities::Generic
{
	Vector3Int PolePos = { 0, 0, -208 };
	Vector3Int PolePosR = { 0, 0, 0 };

	OBJECT_COLLISION_BOUNDS PoleBounds = 
	{
		-256, 256, 
		0, 0, 
		-512, 512, 
		-ANGLE(10.0f), ANGLE(10.0f),
		-ANGLE(30.0f), ANGLE(30.0f),
		-ANGLE(10.0f), ANGLE(10.0f)
	};

	void PoleCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
	{
		auto* laraInfo = GetLaraInfo(laraItem);
		auto* poleItem = &g_Level.Items[itemNumber];

		bool isLara = !poleItem->IsLara();

		if (TrInput & IN_ACTION && isLara &&
			laraInfo->Control.HandStatus == HandStatus::Free &&
			laraItem->Animation.ActiveState == LS_IDLE && 
			laraItem->Animation.AnimNumber == LA_STAND_IDLE || laraInfo->Control.IsMoving &&
			laraInfo->InteractedItem == itemNumber)
		{
			short rot = poleItem->Pose.Orientation.y;
			poleItem->Pose.Orientation.y = laraItem->Pose.Orientation.y;

			if (TestLaraPosition(&PoleBounds, poleItem, laraItem))
			{
				if (MoveLaraPosition(&PolePos, poleItem, laraItem))
				{
					laraItem->Animation.AnimNumber = LA_STAND_TO_POLE;
					laraItem->Animation.ActiveState = LS_POLE_IDLE;
					laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;
					laraInfo->Control.IsMoving = false;
					laraInfo->Control.HandStatus = HandStatus::Busy;
				}
				else
					laraInfo->InteractedItem = itemNumber;

				poleItem->Pose.Orientation.y = rot;
			}
			else
			{
				if (laraInfo->Control.IsMoving && laraInfo->InteractedItem == itemNumber)
				{
					laraInfo->Control.IsMoving = false;
					laraInfo->Control.HandStatus = HandStatus::Free;
				}

				poleItem->Pose.Orientation.y = rot;
			}
		}
		else if (TrInput & IN_ACTION && isLara &&
			     laraInfo->Control.HandStatus == HandStatus::Free && 
				 laraItem->Animation.IsAirborne && 
				 laraItem->Animation.VerticalVelocity > (int)laraInfo->Control.HandStatus &&	// ?????
				 laraItem->Animation.ActiveState == LS_REACH || laraItem->Animation.ActiveState == LS_JUMP_UP)
		{
			if (TestBoundsCollide(poleItem, laraItem, 100) &&
				TestLaraPoleCollision(laraItem, coll, true, -CLICK(1)) &&
				TestLaraPoleCollision(laraItem, coll, false))
			{
				if (TestCollision(poleItem, laraItem))
				{
					short rot = poleItem->Pose.Orientation.y;
					poleItem->Pose.Orientation.y = laraItem->Pose.Orientation.y;
					if (laraItem->Animation.ActiveState == LS_REACH)
					{
						PolePosR.y = laraItem->Pose.Position.y - poleItem->Pose.Position.y + 10;
						AlignLaraPosition(&PolePosR, poleItem, laraItem);
						laraItem->Animation.AnimNumber = LA_REACH_TO_POLE;
						laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;
					}
					else
					{
						PolePosR.y = laraItem->Pose.Position.y - poleItem->Pose.Position.y + 66;
						AlignLaraPosition(&PolePosR, poleItem, laraItem);
						laraItem->Animation.AnimNumber = LA_JUMP_UP_TO_POLE;
						laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;
					}

					laraItem->Animation.ActiveState = LS_POLE_IDLE;
					laraItem->Animation.VerticalVelocity = 0;
					laraItem->Animation.IsAirborne = false;
					laraInfo->Control.HandStatus = HandStatus::Busy;
					poleItem->Pose.Orientation.y = rot;
				}
			}
		}
		else
		{
			if (!isLara || ((laraItem->Animation.ActiveState < LS_POLE_IDLE ||
				laraItem->Animation.ActiveState > LS_POLE_TURN_COUNTER_CLOCKWISE) &&
					laraItem->Animation.ActiveState != LS_JUMP_BACK))
			{
				ObjectCollision(itemNumber, laraItem, coll);
			}
		}
	}
}
