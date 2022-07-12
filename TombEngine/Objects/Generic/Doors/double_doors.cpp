#include "framework.h"
#include "Objects/Generic/Doors/generic_doors.h"
#include "Specific/level.h"
#include "Game/control/control.h"
#include "Game/control/box.h"
#include "Game/items.h"
#include "Game/control/lot.h"
#include "Game/gui.h"
#include "Specific/input.h"
#include "Game/pickup/pickup.h"
#include "Sound/sound.h"
#include "Game/animation.h"
#include "Game/collision/sphere.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/Lara/lara_struct.h"
#include "Specific/trmath.h"
#include "Game/misc.h"
#include "Objects/Generic/Doors/double_doors.h"
#include "Game/collision/collide_item.h"

using namespace TEN::Input;

namespace TEN::Entities::Doors
{
	Vector3Int DoubleDoorPos(0, 0, 220);

	OBJECT_COLLISION_BOUNDS DoubleDoorBounds =
	{
		-384, 384, 
		0, 0, 
		-1024, 512, 
		-ANGLE(10.0f), ANGLE(10.0f),
		-ANGLE(30.0f), ANGLE(30.0f),
		-ANGLE(10.0f), ANGLE(10.0f),
	};

	void DoubleDoorCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
	{
		auto* laraInfo = GetLaraInfo(laraItem);
		auto* doorItem = &g_Level.Items[itemNumber];

		if (TrInput & IN_ACTION &&
			laraItem->Animation.ActiveState == LS_IDLE &&
			laraItem->Animation.AnimNumber == LA_STAND_IDLE &&
			!laraItem->HitStatus &&
			!(doorItem->Status && doorItem->Animation.IsAirborne) &&
			laraInfo->Control.HandStatus == HandStatus::Free ||
			laraInfo->Control.IsMoving && laraInfo->InteractedItem == itemNumber)
		{
			doorItem->Pose.Orientation.y ^= ANGLE(180.0f);

			if (TestLaraPosition(&DoubleDoorBounds, doorItem, laraItem))
			{
				if (MoveLaraPosition(&DoubleDoorPos, doorItem, laraItem))
				{
					SetAnimation(laraItem, LA_DOUBLEDOOR_OPEN_PUSH);

					AddActiveItem(itemNumber);

					ResetLaraFlex(laraItem);
					laraInfo->Control.IsMoving = false;
					laraInfo->Control.HandStatus = HandStatus::Busy;
					doorItem->Status = ITEM_ACTIVE;
				}
				else
					laraInfo->InteractedItem = itemNumber;

				doorItem->Pose.Orientation.y ^= ANGLE(180.0f);
			}
			else
			{
				if (laraInfo->Control.IsMoving &&
					laraInfo->InteractedItem == itemNumber)
				{
					laraInfo->Control.IsMoving = false;
					laraInfo->Control.HandStatus = HandStatus::Free;
				}

				doorItem->Pose.Orientation.y ^= ANGLE(180.0f);
			}
		}
	}
}
