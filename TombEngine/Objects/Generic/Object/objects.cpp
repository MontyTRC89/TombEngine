#include "framework.h"
#include "Objects/Generic/Object/objects.h"

#include "Game/animation.h"
#include "Game/collision/collide_item.h"
#include "Game/control/control.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/Setup.h"
#include "Math/Math.h"
#include "Renderer/RendererEnums.h"
#include "Sound/sound.h"
#include "Specific/Input/Input.h"
#include "Specific/level.h"

using namespace TEN::Input;
using namespace TEN::Math;

const auto TightRopePos = Vector3i::Zero;
const ObjectCollisionBounds TightRopeBounds =
{
	GameBoundingBox(
		-CLICK(1), CLICK(1),
		0, 0,
		-CLICK(1), CLICK(1)),
	std::pair(
		EulerAngles(ANGLE(-10.0f), -LARA_GRAB_THRESHOLD, ANGLE(-10.0f)),
		EulerAngles(ANGLE(10.0f), LARA_GRAB_THRESHOLD, ANGLE(10.0f)))
};

void ControlAnimatingSlots(short itemNumber)
{
	// TODO: TR5 has here a series of hardcoded OCB codes, this function actually is just a placeholder
	auto* item = &g_Level.Items[itemNumber];

	if (TriggerActive(item))
		AnimateItem(item);
}

void ControlTriggerTriggerer(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];
	FloorInfo* floor = GetFloor(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, &item->RoomNumber);

	if (floor->Flags.MarkTriggerer)
	{
		if (TriggerActive(item))
			floor->Flags.MarkTriggererActive = true;
		else
			floor->Flags.MarkTriggererActive = false;
	}
}

void TightropeCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
{
	auto* laraInfo = GetLaraInfo(laraItem);
	auto* tightropeItem = &g_Level.Items[itemNumber];
	
	if ((!IsHeld(In::Action) ||
		laraItem->Animation.ActiveState != LS_IDLE ||
		laraItem->Animation.AnimNumber != LA_STAND_IDLE ||
		laraItem->Status == ITEM_INVISIBLE ||
		laraInfo->Control.HandStatus != HandStatus::Free) &&
		(!laraInfo->Control.IsMoving || laraInfo->Context.InteractedItem !=itemNumber))
	{
		if (laraItem->Animation.ActiveState == LS_TIGHTROPE_WALK &&
		   laraItem->Animation.TargetState != LS_TIGHTROPE_DISMOUNT &&
		   !laraInfo->Control.Tightrope.CanDismount)
		{
			if (tightropeItem->Pose.Orientation.y == laraItem->Pose.Orientation.y)
			{
				if (abs(tightropeItem->Pose.Position.x - laraItem->Pose.Position.x) + abs(tightropeItem->Pose.Position.z - laraItem->Pose.Position.z) < 640)
					laraInfo->Control.Tightrope.CanDismount = true;
			}
		}
	}
	else
	{
		tightropeItem->Pose.Orientation.y += -ANGLE(180.0f);

		if (TestLaraPosition(TightRopeBounds, tightropeItem, laraItem))
		{
			if (MoveLaraPosition(TightRopePos, tightropeItem, laraItem))
			{
				laraItem->Animation.ActiveState = LS_TIGHTROPE_ENTER;
				laraItem->Animation.AnimNumber = LA_TIGHTROPE_START;
				laraItem->Animation.FrameNumber = GetAnimData(laraItem).frameBase;
				laraInfo->Control.IsMoving = false;
				ResetPlayerFlex(laraItem);
				laraInfo->Control.Tightrope.Balance = 0;
				laraInfo->Control.Tightrope.CanDismount = false;
				laraInfo->Control.Tightrope.TightropeItem = itemNumber;
				laraInfo->Control.Tightrope.TimeOnTightrope = 0;
			}
			else
			{
				laraInfo->Context.InteractedItem = itemNumber;
			}

			tightropeItem->Pose.Orientation.y += -ANGLE(180.0f);
		}
		else
		{
			if (laraInfo->Control.IsMoving && laraInfo->Context.InteractedItem == itemNumber)
				laraInfo->Control.IsMoving = false;

			tightropeItem->Pose.Orientation.y += -ANGLE(180.0f);
		}
	}
}

void CutsceneRopeControl(short itemNumber) 
{
	auto* ropeItem = &g_Level.Items[itemNumber];

	auto pos1 = GetJointPosition(&g_Level.Items[ropeItem->ItemFlags[2]], 0, Vector3i(-128, -72, -16));
	auto pos2 = GetJointPosition(&g_Level.Items[ropeItem->ItemFlags[3]], 0, Vector3i(830, -12, 0));

	ropeItem->Pose.Position = pos2;

	int dx = (pos2.x - pos1.x) * (pos2.x - pos1.x);
	int dy = (pos2.y - pos1.y) * (pos2.y - pos1.y);
	int dz = (pos2.z - pos1.z) * (pos2.z - pos1.z);

	ropeItem->ItemFlags[1] = ((sqrt(dx + dy + dz) * 2) + sqrt(dx + dy + dz)) * 2;
	ropeItem->Pose.Orientation.x = ANGLE(-26.75f);
}

void HybridCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll) 
{
	auto* item = &g_Level.Items[itemNumber];

	/*if (gfCurrentLevel == LVL5_SINKING_SUBMARINE)
	{
		if (item->frameNumber < g_Level.Anims[item->animNumber].frame_end)
		{
			ObjectCollision(itemNumber, laraitem, coll);
		}
	}*/
}

void InitializeTightrope(short itemNumber)
{
	auto* tightropeItem = &g_Level.Items[itemNumber];

	if (tightropeItem->Pose.Orientation.y > 0)
	{
		if (tightropeItem->Pose.Orientation.y == ANGLE(90.0f))
			tightropeItem->Pose.Position.x -= CLICK(1);
	}
	else if (tightropeItem->Pose.Orientation.y)
	{
		if (tightropeItem->Pose.Orientation.y == ANGLE(-180.0f))
			tightropeItem->Pose.Position.z += CLICK(1);
		else if (tightropeItem->Pose.Orientation.y == ANGLE(-90.0f))
			tightropeItem->Pose.Position.x += CLICK(1);
	}
	else
		tightropeItem->Pose.Position.z -= CLICK(1);
}

void InitializeAnimating(short itemNumber)
{
	/*auto* item = &g_Level.Items[itemNumber];
	item->ActiveState = 0;
	item->animNumber = Objects[item->objectNumber].animIndex;
	item->frameNumber = g_Level.Anims[item->animNumber].frameBase;*/
}

void AnimatingControl(short itemNumber)
{
	auto& item = g_Level.Items[itemNumber];

	if (TriggerActive(&item))
	{
		item.Status = ITEM_ACTIVE;
		AnimateItem(&item);

		if (item.TriggerFlags == 666) //OCB used for the helicopter animating in the Train level.
		{
			auto pos = GetJointPosition(item, 0);
			SoundEffect(SFX_TR4_HELICOPTER_LOOP, (Pose*)&pos);

			if (item.Animation.FrameNumber == GetAnimData(item).frameEnd)
			{
				item.Flags &= 0xC1;
				RemoveActiveItem(itemNumber);
				item.Status = ITEM_NOT_ACTIVE;
			}
		}
	}
	else if (item.TriggerFlags == 2) //Make the animating dissapear when anti-triggered.
	{
		RemoveActiveItem(itemNumber);
		item.Status = ITEM_INVISIBLE;
	}

	// TODO: ID_SHOOT_SWITCH2 is probably the bell in Trajan Markets, use Lua for that.
	/*if (item->frameNumber >= g_Level.Anims[item->animNumber].frameEnd)
	{
		item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
		RemoveActiveItem(itemNumber);
		item->aiBits = 0;
		item->status = ITEM_NOT_ACTIVE;
	}*/
}


