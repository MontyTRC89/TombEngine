#include "framework.h"
#include "Game/animation.h"
#include "Game/collision/collide_item.h"
#include "Game/collision/sphere.h"
#include "Game/control/control.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Objects/Generic/Object/objects.h"
#include "Renderer/Renderer11Enums.h"
#include "Sound/sound.h"
#include "Specific/Input/Input.h"
#include "Specific/level.h"
#include "Specific/setup.h"

using namespace TEN::Input;

const auto TightRopePos = Vector3i::Zero;
const ObjectCollisionBounds TightRopeBounds =
{
	GameBoundingBox(
		-CLICK(1), CLICK(1),
		0, 0,
		-CLICK(1), CLICK(1)
	),
	std::pair(
		EulerAngles(ANGLE(-10.0f), ANGLE(-30.0f), ANGLE(-10.0f)),
		EulerAngles(ANGLE(10.0f), ANGLE(30.0f), ANGLE(10.0f))
	)
};
const ObjectCollisionBounds ParallelBarsBounds =
{
	GameBoundingBox(
		-640, 640,
		704, 832,
		-96, 96
	),
	std::pair(
		EulerAngles(ANGLE(-10.0f), ANGLE(-30.0f), ANGLE(-10.0f)),
		EulerAngles(ANGLE(10.0f), ANGLE(30.0f), ANGLE(10.0f))
	)
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
	
	if ((!(TrInput & IN_ACTION) ||
		laraItem->Animation.ActiveState != LS_IDLE ||
		laraItem->Animation.AnimNumber != LA_STAND_IDLE ||
		laraItem->Status == ITEM_INVISIBLE ||
		laraInfo->Control.HandStatus != HandStatus::Free) &&
		(!laraInfo->Control.IsMoving || laraInfo->InteractedItem !=itemNumber))
	{
#ifdef NEW_TIGHTROPE
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

#else // !NEW_TIGHTROPE
		if (laraItem->Animation.ActiveState == LS_TIGHTROPE_WALK &&
		   laraItem->Animation.TargetState != LS_TIGHTROPE_DISMOUNT &&
		   !laraInfo->Control.Tightrope.Off)
		{
			if (item->Pose.Orientation.y == laraItem->Pose.Orientation.y)
			{
				if (abs(item->Pose.Position.x - laraItem->Pose.Position.x) + abs(item->Pose.Position.z - laraItem->Pose.Position.z) < 640)
					laraInfo->tightRopeOff = true;
			}
		}
#endif
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
				laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;
				laraInfo->Control.IsMoving = false;
				ResetLaraFlex(laraItem);
#ifdef NEW_TIGHTROPE
				laraInfo->Control.Tightrope.Balance = 0;
				laraInfo->Control.Tightrope.CanDismount = false;
				laraInfo->Control.Tightrope.TightropeItem = itemNumber;
				laraInfo->Control.Tightrope.TimeOnTightrope = 0;
#else // !NEW_TIGHTROPE
				laraInfo->Control.Tightrope.OnCount = 60;
				laraInfo->Control.Tightrope.Off = 0;
				laraInfo->Control.Tightrope.Fall = 0;
#endif
			}
			else
				laraInfo->InteractedItem = itemNumber;

			tightropeItem->Pose.Orientation.y += -ANGLE(180.0f);
		}
		else
		{
			if (laraInfo->Control.IsMoving && laraInfo->InteractedItem == itemNumber)
				laraInfo->Control.IsMoving = false;

			tightropeItem->Pose.Orientation.y += -ANGLE(180.0f);
		}
	}
}

void HorizontalBarCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
{
	auto* laraInfo = GetLaraInfo(laraItem);
	auto* barItem = &g_Level.Items[itemNumber];

	if (TrInput & IN_ACTION &&
		laraItem->Animation.ActiveState == LS_REACH &&
		laraItem->Animation.AnimNumber == LA_REACH &&
		laraInfo->Control.HandStatus == HandStatus::Free)
	{
		int test1 = TestLaraPosition(ParallelBarsBounds, barItem, laraItem);
		int test2 = 0;
		if (!test1)
		{
			barItem->Pose.Orientation.y += -ANGLE(180.0f);
			test2 = TestLaraPosition(ParallelBarsBounds, barItem, laraItem);
			barItem->Pose.Orientation.y += -ANGLE(180);
		}

		if (test1 || test2)
		{
			laraItem->Animation.ActiveState = LS_MISC_CONTROL;
			laraItem->Animation.AnimNumber = LA_SWINGBAR_GRAB;
			laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;
			laraItem->Animation.Velocity.y = false;
			laraItem->Animation.IsAirborne = false;

			ResetLaraFlex(barItem);

			if (test1)
				laraItem->Pose.Orientation.y = barItem->Pose.Orientation.y;
			else
				laraItem->Pose.Orientation.y = barItem->Pose.Orientation.y - ANGLE(180.0f);

			auto pos1 = GetJointPosition(laraItem, LM_LHAND, Vector3i(0, -128, 512));
			auto pos2 = GetJointPosition(laraItem, LM_RHAND, Vector3i(0, -128, 512));
		
			if (laraItem->Pose.Orientation.y & 0x4000)
				laraItem->Pose.Position.x += barItem->Pose.Position.x - ((pos1.x + pos2.x) >> 1);
			else
				laraItem->Pose.Position.z += barItem->Pose.Position.z - ((pos1.z + pos2.z) / 2);
			laraItem->Pose.Position.y += barItem->Pose.Position.y - ((pos1.y + pos2.y) / 2);

			laraInfo->InteractedItem = itemNumber;
		}
		else
			ObjectCollision(itemNumber, laraItem, coll);
	}
	else if (laraItem->Animation.ActiveState != LS_HORIZONTAL_BAR_SWING)
		ObjectCollision(itemNumber, laraItem, coll);
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

void InitialiseTightrope(short itemNumber)
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

void InitialiseAnimating(short itemNumber)
{
	/*auto* item = &g_Level.Items[itemNumber];
	item->ActiveState = 0;
	item->animNumber = Objects[item->objectNumber].animIndex;
	item->frameNumber = g_Level.Anims[item->animNumber].frameBase;*/
}

void AnimatingControl(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	if (!TriggerActive(item))
		return;

	item->Status = ITEM_ACTIVE;
	AnimateItem(item);

	// TODO: ID_SHOOT_SWITCH2 probably the bell in Trajan Markets, use LUA for that
	/*if (item->frameNumber >= g_Level.Anims[item->animNumber].frameEnd)
	{
		item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
		RemoveActiveItem(itemNumber);
		item->aiBits = 0;
		item->status = ITEM_NOT_ACTIVE;
	}*/
}

void HighObject2Control(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	if (!TriggerActive(item))
		return;

	if (!item->ItemFlags[2])
	{
		int div = item->TriggerFlags % 10 << 10;
		int mod = item->TriggerFlags / 10 << 10;
		item->ItemFlags[0] = GetRandomControl() % div;
		item->ItemFlags[1] = GetRandomControl() % mod;
		item->ItemFlags[2] = (GetRandomControl() & 0xF) + 15;
	}

	if (--item->ItemFlags[2] < 15)
	{
		auto* spark = GetFreeParticle();
		spark->on = 1;
		spark->sR = -1;
		spark->sB = 16;
		spark->sG = (GetRandomControl() & 0x1F) + 48;
		spark->dR = (GetRandomControl() & 0x3F) - 64;
		spark->dB = 0;
		spark->dG = (GetRandomControl() & 0x3F) + -128;
		spark->fadeToBlack = 4;
		spark->colFadeSpeed = (GetRandomControl() & 3) + 4;
		spark->blendMode = BLEND_MODES::BLENDMODE_ADDITIVE;
		spark->life = spark->sLife = (GetRandomControl() & 3) + 24;
		spark->x = item->ItemFlags[1] + (GetRandomControl() & 0x3F) + item->Pose.Position.x - 544;
		spark->y = item->Pose.Position.y;
		spark->z = item->ItemFlags[0] + (GetRandomControl() & 0x3F) + item->Pose.Position.z - 544;
		spark->xVel = (GetRandomControl() & 0x1FF) - 256;
		spark->friction = 6;
		spark->zVel = (GetRandomControl() & 0x1FF) - 256;
		spark->rotAng = GetRandomControl() & 0xFFF;
		spark->rotAdd = (GetRandomControl() & 0x3F) - 32;
		spark->maxYvel = 0;
		spark->yVel = -512 - (GetRandomControl() & 0x3FF);
		spark->sSize = spark->size = (GetRandomControl() & 0xF) + 32;
		spark->dSize = spark->size / 4;

		if (GetRandomControl() & 3)
		{
			spark->flags = SP_ROTATE | SP_DEF | SP_SCALE | SP_EXPDEF;
			spark->scalar = 3;
			spark->gravity = (GetRandomControl() & 0x3F) + 32;
		}
		else
		{
			spark->flags = SP_ROTATE | SP_DEF | SP_SCALE;
			spark->spriteIndex = Objects[ID_DEFAULT_SPRITES].meshIndex + SPR_UNDERWATERDUST;
			spark->scalar = 1;
			spark->gravity = (GetRandomControl() & 0xF) + 64;
		}
	}
}
