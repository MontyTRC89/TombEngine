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

using namespace TEN::Input;

namespace TEN::Entities::Switches
{
	const InteractionBasis JumpSwitchBounds =  
	{
		GameBoundingBox(
			-CLICK(0.5f), CLICK(0.5f),
			-CLICK(1), CLICK(1),
			CLICK(1.5f), SECTOR(0.5f)
		),
		std::pair(
			EulerAngles(ANGLE(-10.0f), ANGLE(-30.0f), ANGLE(-10.0f)),
			EulerAngles(ANGLE(10.0f), ANGLE(30.0f), ANGLE(10.0f))
		)
	};
	const auto JumpSwitchPos = Vector3i(0, -208, 256);

	void JumpSwitchCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
	{
		auto* laraInfo = GetLaraInfo(laraItem);
		auto* switchItem = &g_Level.Items[itemNumber];

		if (TrInput & IN_ACTION &&
			(laraItem->Animation.ActiveState == LS_REACH || laraItem->Animation.ActiveState == LS_JUMP_UP) &&
			(laraItem->Status || laraItem->Animation.IsAirborne) &&
			laraItem->Animation.Velocity.y > 0 &&
			laraInfo->Control.HandStatus == HandStatus::Free &&
			!switchItem->Animation.ActiveState)
		{
			if (TestPlayerEntityInteract(switchItem, laraItem, JumpSwitchBounds))
			{
				AlignPlayerToEntity(switchItem, laraItem, JumpSwitchPos, EulerAngles::Zero, true);

				laraItem->Animation.ActiveState = LS_SWITCH_DOWN;
				laraItem->Animation.AnimNumber = LA_JUMPSWITCH_PULL;
				laraItem->Animation.Velocity.y = 0;
				laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;
				laraItem->Animation.IsAirborne = false;
				laraInfo->Control.HandStatus = HandStatus::Busy;
				switchItem->Animation.TargetState = SWITCH_ON;
				switchItem->Status = ITEM_ACTIVE;

				AddActiveItem(itemNumber);
			}
		}
	}
}
