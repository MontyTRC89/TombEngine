#include "framework.h"
#include "Objects/Generic/Switches/jump_switch.h"
#include "Game/control/control.h"
#include "Specific/input.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Objects/Generic/Switches/generic_switch.h"
#include "Specific/level.h"
#include "Game/collision/collide_item.h"
#include "Game/items.h"

namespace TEN::Entities::Switches
{
	OBJECT_COLLISION_BOUNDS JumpSwitchBounds =  
	{
		-128, 128,
		-256, 256,
		384, 512,
		-ANGLE(10.0f), ANGLE(10.0f),
		-ANGLE(30.0f), ANGLE(30.0f),
		-ANGLE(10.0f), ANGLE(10.0f)
	};

	PHD_VECTOR JumpSwitchPos = { 0, -208, 256 };  

	void JumpSwitchCollision(short itemNumber, ITEM_INFO* laraItem, COLL_INFO* coll)
	{
		auto* laraInfo = GetLaraInfo(laraItem);
		auto* switchItem = &g_Level.Items[itemNumber];

		if (TrInput & IN_ACTION &&
			(laraItem->ActiveState == LS_REACH || laraItem->ActiveState == LS_JUMP_UP) &&
			(laraItem->Status || laraItem->Airborne) &&
			laraItem->VerticalVelocity > 0 &&
			laraInfo->Control.HandStatus == HandStatus::Free &&
			!switchItem->ActiveState)
		{
			if (TestLaraPosition(&JumpSwitchBounds, switchItem, laraItem))
			{
				AlignLaraPosition(&JumpSwitchPos, switchItem, laraItem);

				laraItem->ActiveState = LS_SWITCH_DOWN;
				laraItem->AnimNumber = LA_JUMPSWITCH_PULL;
				laraItem->VerticalVelocity = 0;
				laraItem->FrameNumber = g_Level.Anims[laraItem->AnimNumber].frameBase;
				laraItem->Airborne = false;
				laraInfo->Control.HandStatus = HandStatus::Busy;
				switchItem->TargetState = SWITCH_ON;
				switchItem->Status = ITEM_ACTIVE;

				AddActiveItem(itemNumber);
			}
		}
	}
}
