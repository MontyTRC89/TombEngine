#include "framework.h"
#include "Objects/TR4/Object/tr4_sas_drag_bloke.h"

#include "Game/collision/collide_item.h"
#include "Game/health.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/pickup/pickup.h"
#include "Math/Math.h"
#include "Specific/Input/Input.h"
#include "Specific/level.h"
#include "Specific/setup.h"

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

void DragSasCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
{
	auto* item = &g_Level.Items[itemNumber];
	auto* lara = GetLaraInfo(laraItem);

	if (TrInput & IN_ACTION &&
		laraItem->Animation.ActiveState == LS_IDLE &&
		laraItem->Animation.AnimNumber == LA_STAND_IDLE &&
		lara->Control.HandStatus == HandStatus::Free &&
		!(laraItem->Animation.IsAirborne) &&
		!(item->Flags & IFLAG_ACTIVATION_MASK) ||
		lara->Control.IsMoving && lara->InteractedItem == itemNumber)
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

				lara->Control.IsMoving = false;
				ResetLaraFlex(laraItem);
				lara->Control.HandStatus = HandStatus::Busy;

				AddActiveItem(itemNumber);
				item->Pose.Orientation.y;
			}
			else
				lara->InteractedItem == itemNumber;
		}
	}
	else
	{
		if (item->Status != ITEM_ACTIVE)
		{
			ObjectCollision(itemNumber, laraItem, coll);
			return;
		}

		if (!TestLastFrame(item))
			return;

		auto pos = GetJointPosition(item, 0);
		TestTriggers(pos.x, pos.y, pos.z, item->RoomNumber, true);
		RemoveActiveItem(itemNumber);
		item->Status = ITEM_DEACTIVATED;
	}
}