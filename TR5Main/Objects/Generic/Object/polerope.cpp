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

namespace TEN::Entities::Generic
{
	PHD_VECTOR PolePos = { 0, 0, -208 };
	PHD_VECTOR PolePosR = { 0, 0, 0 };

	OBJECT_COLLISION_BOUNDS PoleBounds = 
	{
		-256, 256, 
		0, 0, 
		-512, 512, 
		-ANGLE(10.0f), ANGLE(10.0f),
		-ANGLE(30.0f), ANGLE(30.0f),
		-ANGLE(10.0f), ANGLE(10.0f)
	};

	void PoleCollision(short itemNumber, ITEM_INFO* laraItem, COLL_INFO* coll)
	{
		auto* laraInfo = GetLaraInfo(laraItem);
		auto* poleItem = &g_Level.Items[itemNumber];

		bool isLara = !poleItem->Data.is<LaraInfo*>();

		if (TrInput & IN_ACTION && isLara &&
			laraInfo->Control.HandStatus == HandStatus::Free &&
			laraItem->ActiveState == LS_IDLE && 
			laraItem->AnimNumber == LA_STAND_IDLE || laraInfo->Control.IsMoving &&
			laraInfo->interactedItem == itemNumber)
		{
			short rot = poleItem->Position.yRot;
			poleItem->Position.yRot = laraItem->Position.yRot;

			if (TestLaraPosition(&PoleBounds, poleItem, laraItem))
			{
				if (MoveLaraPosition(&PolePos, poleItem, laraItem))
				{
					laraItem->AnimNumber = LA_STAND_TO_POLE;
					laraItem->ActiveState = LS_POLE_IDLE;
					laraItem->FrameNumber = g_Level.Anims[laraItem->AnimNumber].frameBase;
					laraInfo->Control.IsMoving = false;
					laraInfo->Control.HandStatus = HandStatus::Busy;
				}
				else
					laraInfo->interactedItem = itemNumber;

				poleItem->Position.yRot = rot;
			}
			else
			{
				if (laraInfo->Control.IsMoving && laraInfo->interactedItem == itemNumber)
				{
					laraInfo->Control.IsMoving = false;
					laraInfo->Control.HandStatus = HandStatus::Free;
				}

				poleItem->Position.yRot = rot;
			}
		}
		else if (TrInput & IN_ACTION && isLara &&
			     laraInfo->Control.HandStatus == HandStatus::Free && 
				 laraItem->Airborne && 
				 laraItem->VerticalVelocity > (int)laraInfo->Control.HandStatus &&	// ?????
				 laraItem->ActiveState == LS_REACH || laraItem->ActiveState == LS_JUMP_UP)
		{
			if (TestBoundsCollide(poleItem, laraItem, 100) &&
				TestLaraPoleCollision(laraItem, coll, true, -CLICK(1)) &&
				TestLaraPoleCollision(laraItem, coll, false))
			{
				if (TestCollision(poleItem, laraItem))
				{
					short rot = poleItem->Position.yRot;
					poleItem->Position.yRot = laraItem->Position.yRot;
					if (laraItem->ActiveState == LS_REACH)
					{
						PolePosR.y = laraItem->Position.yPos - poleItem->Position.yPos + 10;
						AlignLaraPosition(&PolePosR, poleItem, laraItem);
						laraItem->AnimNumber = LA_REACH_TO_POLE;
						laraItem->FrameNumber = g_Level.Anims[laraItem->AnimNumber].frameBase;
					}
					else
					{
						PolePosR.y = laraItem->Position.yPos - poleItem->Position.yPos + 66;
						AlignLaraPosition(&PolePosR, poleItem, laraItem);
						laraItem->AnimNumber = LA_JUMP_UP_TO_POLE;
						laraItem->FrameNumber = g_Level.Anims[laraItem->AnimNumber].frameBase;
					}

					laraItem->ActiveState = LS_POLE_IDLE;
					laraItem->VerticalVelocity = 0;
					laraItem->Airborne = false;
					laraInfo->Control.HandStatus = HandStatus::Busy;
					poleItem->Position.yRot = rot;
				}
			}
		}
		else
		{
			if (!isLara || ((laraItem->ActiveState < LS_POLE_IDLE ||
				laraItem->ActiveState > LS_POLE_TURN_COUNTER_CLOCKWISE) &&
					laraItem->ActiveState != LS_JUMP_BACK))
			{
				ObjectCollision(itemNumber, laraItem, coll);
			}
		}
	}
}
