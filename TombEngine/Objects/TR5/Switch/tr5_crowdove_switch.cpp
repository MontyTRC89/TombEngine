#include "framework.h"
#include "tr5_crowdove_switch.h"
#include "Game/control/control.h"
#include "Specific/input.h"
#include "Specific/level.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Objects/Generic/Switches/generic_switch.h"
#include "Sound/sound.h"
#include "Game/animation.h"
#include "Game/items.h"
#include "Game/collision/collide_item.h"
#include "Game/effects/debris.h"

using namespace TEN::Input;
using namespace TEN::Entities::Switches;

namespace TEN::Entities::TR5
{
	OBJECT_COLLISION_BOUNDS CrowDoveBounds =
	{
		-256, 256,
		0, 0,
		-512, 512,
		-ANGLE(10.0f), ANGLE(10.0f),
		-ANGLE(30.0f), ANGLE(30.0f),
		-ANGLE(10.0f), ANGLE(10.0f)
	};

	auto CrowDovePos = Vector3Int(0, 0, -400);

	void InitialiseCrowDoveSwitch(short itemNumber)
	{
		g_Level.Items[itemNumber].MeshBits = 3;
	}

	void CrowDoveSwitchCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
	{
		auto* laraInfo = GetLaraInfo(laraItem);
		auto* switchItem = &g_Level.Items[itemNumber];

		if (switchItem->Flags & IFLAG_INVISIBLE ||
			!(switchItem->MeshBits & 4) ||
			(!(TrInput & IN_ACTION) ||
				laraItem->Animation.ActiveState != LS_IDLE ||
				laraItem->Animation.AnimNumber != LA_STAND_IDLE ||
				laraItem->Animation.IsAirborne ||
				laraInfo->Control.HandStatus != HandStatus::Free) &&
			(!laraInfo->Control.IsMoving || laraInfo->InteractedItem != itemNumber))
		{
			if (laraItem->Animation.ActiveState != LS_DOVE_SWITCH)
				ObjectCollision(itemNumber, laraItem, coll);
		}
		else
		{
			int oldYrot = switchItem->Pose.Orientation.y;
			switchItem->Pose.Orientation.y = laraItem->Pose.Orientation.y;
			if (TestLaraPosition(&CrowDoveBounds, switchItem, laraItem))
			{
				if (MoveLaraPosition(&CrowDovePos, switchItem, laraItem))
				{
					laraItem->Animation.AnimNumber = LA_DOVESWITCH_TURN;
					laraItem->Animation.ActiveState = LS_DOVE_SWITCH;
					laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;

					AddActiveItem(itemNumber);

					// NOTE: In original TR5 the switch was used together with heavy switches.
					// This little fix make it usable normaly and less hardcoded.
					switchItem->ItemFlags[0] = 0;

					switchItem->Status = ITEM_ACTIVE;
					switchItem->Pose.Orientation.y = oldYrot;
					ResetLaraFlex(laraItem);
					laraInfo->Control.IsMoving = false;
					laraInfo->Control.HandStatus = HandStatus::Busy;
					laraInfo->InteractedItem = itemNumber;
				}
				else
					laraInfo->InteractedItem = itemNumber;
				
				switchItem->Pose.Orientation.y = oldYrot;
			}
			else
			{
				if (laraInfo->Control.IsMoving && laraInfo->InteractedItem == itemNumber)
				{
					laraInfo->Control.IsMoving = false;
					laraInfo->Control.HandStatus = HandStatus::Free;
				}

				switchItem->Pose.Orientation.y = oldYrot;
			}
		}
	}

	void CrowDoveSwitchControl(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		if (item->MeshBits & 2) 
		{
			ExplodeItemNode(item, 1, 0, 256); 
			SoundEffect(SFX_TR5_RAVEN_SWITCH_EXPLODE, &item->Pose);
			item->MeshBits = 5;	
			RemoveActiveItem(itemNumber);

			// NOTE: In original TR5 the switch was used together with heavy switches.
			// This little fix make it usable normaly and less hardcoded.
			item->ItemFlags[0] = 1; 
		}
		else if (item->ItemFlags[0] == 0)
		{
			if (item->Animation.ActiveState == SWITCH_OFF)
				item->Animation.TargetState = SWITCH_ON;

			AnimateItem(item);

			if (item->Animation.ActiveState == SWITCH_OFF)
				item->Pose.Orientation.y += ANGLE(90.0f);
		}
	}
}
