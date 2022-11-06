#include "framework.h"
#include "tr4_sas_drag_bloke.h"
#include "Specific/level.h"
#include "Specific/Input/Input.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/items.h"
#include "Game/pickup/pickup.h"
#include "Specific/setup.h"
#include "Game/health.h"
#include "Math/Math.h"
#include "Game/collision/collide_item.h"

using namespace TEN::Input;
using namespace TEN::Math;

const auto DragSASPosition = Vector3i(0, 0, -460);
const auto DragSasBounds = ObjectCollisionBounds 
{
	GameBoundingBox(
		-BLOCK(1.0f / 4), BLOCK(1.0f / 4),
		-100, 100, 
		-BLOCK(1.0f / 2), -460
	),
		std::pair(
			EulerAngles(ANGLE(-10.0f), ANGLE(-30.0f), 0),
			EulerAngles(ANGLE(10.0f), ANGLE(30.0f), 0)
		)
};


void InitialiseDragSAS(short itemNumber)
{

}

void DragSASCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
{
	auto* item = &g_Level.Items[itemNumber];
	auto* laraInfo = GetLaraInfo(laraItem);

	long x, z;

	if (TrInput & IN_ACTION &&
		laraItem->Animation.ActiveState == LS_IDLE &&
		laraItem->Animation.AnimNumber == LA_STAND_IDLE &&
		laraInfo->Control.HandStatus == HandStatus::Free &&
		!(laraItem->Animation.IsAirborne) &&
		!(item->Flags & IFLAG_ACTIVATION_MASK) ||
		laraInfo->Control.IsMoving && laraInfo->InteractedItem == itemNumber)
	{
		if (TestLaraPosition(DragSasBounds, item, laraItem))
		{
			if (MoveLaraPosition(DragSASPosition, item, laraItem))
			{

				laraItem->Animation.AnimNumber = LA_DRAG_BODY;
				laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;
				laraItem->Animation.ActiveState = LS_MISC_CONTROL;
				item->Flags |= IFLAG_ACTIVATION_MASK;
				item->Status = ITEM_ACTIVE;
				laraInfo->Control.IsMoving = false;
				ResetLaraFlex(laraItem);
				laraInfo->Control.HandStatus = HandStatus::Busy;
				AddActiveItem(itemNumber);
				item->Pose.Orientation.y;
			}
			else
				laraInfo->InteractedItem == itemNumber;
		}
	}
	else
	{
		if (item->Status == ITEM_ACTIVE)
		{

			/*
			TODO: Implement Drag_sas_bloke being able to trigger stuff - Kubsy 06/11/22
			if (item->frameNumber == &g_Level.Anims[item->Animation.AnimNumber].frameEnd)
			{
				auto triggerPoint = item->Pose.Position;
				triggerPoint = Geometry::TranslatePoint(triggerPoint, item->Pose.Orientation.y + ANGLE(180.0f), -100);
				TestTriggers(x, y, z, roomNumber, heavy, heavyFlags);
				RemoveActiveItem(itemNumber);
				item->Status = ITEM_INACTIVE;
			}
			*/
			return;
		}
		ObjectCollision(itemNumber, laraItem, coll);
	}
}