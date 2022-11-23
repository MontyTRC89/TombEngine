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

const auto DragSasGrabOffset = Vector3i(0, 0, -460);
const auto DragSasGrabBasis = InteractionBasis(
	GameBoundingBox(
		-BLOCK(1.0f / 4), BLOCK(1.0f / 4),
		-100, 100, 
		-BLOCK(1.0f / 2), -460),
	std::pair(
		EulerAngles(ANGLE(-10.0f), ANGLE(-30.0f), ANGLE(-10.0f)),
		EulerAngles(ANGLE(10.0f), ANGLE(30.0f), ANGLE(10.0f)))
);

void DragSasCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
{
	auto* item = &g_Level.Items[itemNumber];
	auto* lara = GetLaraInfo(laraItem);

	if ((IsHeld(In::Action) &&
		laraItem->Animation.ActiveState == LS_IDLE &&
		laraItem->Animation.AnimNumber == LA_STAND_IDLE &&
		!laraItem->Animation.IsAirborne &&
		lara->Control.HandStatus == HandStatus::Free &&
		!(item->Flags & IFLAG_ACTIVATION_MASK)) ||
		(lara->Control.IsMoving && lara->InteractedItem == itemNumber))
	{
		if (TestEntityInteraction(*laraItem, *item, DragSasGrabBasis))
		{
			if (AlignPlayerToEntity(item, laraItem, DragSasGrabOffset))
			{
				AddActiveItem(itemNumber);
				item->Pose.Orientation.y;
				item->Flags |= IFLAG_ACTIVATION_MASK;
				item->Status = ITEM_ACTIVE;

				SetAnimation(laraItem, LA_DRAG_BODY);
				ResetLaraFlex(laraItem);
				lara->Control.HandStatus = HandStatus::Busy;
				lara->Control.IsMoving = false;
			}
			else
				lara->InteractedItem = itemNumber;
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
