#include "framework.h"
#include "Objects/Generic/Object/objects.h"

#include "Game/animation.h"
#include "Game/collision/collide_item.h"
#include "Game/collision/sphere.h"
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

ObjectCollisionBounds HorizontalBarBounds =
{
	GameBoundingBox(
		-640, 640,
		704, 832,
		-96, 96),
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

void HorizontalBarCollision(short itemNumber, ItemInfo* playerItem, CollisionInfo* coll)
{
	static const auto HORIZONTAL_BAR_STATES = std::vector<int>
	{
		LS_HORIZONTAL_BAR_SWING,
		LS_HORIZONTAL_BAR_JUMP,
		LS_HORIZONTAL_BAR_IDLE,
		LS_HORIZONTAL_BAR_IDLE_TURN_180,
		LS_HORIZONTAL_BAR_SWING_TURN_180
	};

	auto& barItem = g_Level.Items[itemNumber];
	auto& player = GetLaraInfo(*playerItem);

	if (IsHeld(In::Action) &&
		(playerItem->Animation.ActiveState == LS_REACH || playerItem->Animation.ActiveState == LS_JUMP_UP) &&
		player.Control.HandStatus == HandStatus::Free)
	{
		// HACK: Update interaction basis.
		auto bounds = GameBoundingBox(&barItem);
		HorizontalBarBounds.BoundingBox.X1 = bounds.X1;
		HorizontalBarBounds.BoundingBox.X2 = bounds.X2;

		// Test interaction.
		bool hasFront = TestLaraPosition(HorizontalBarBounds, &barItem, playerItem);
		bool hasBack = false;
		if (!hasFront)
		{
			barItem.Pose.Orientation.y += ANGLE(-180.0f);
			hasBack = TestLaraPosition(HorizontalBarBounds, &barItem, playerItem);
			barItem.Pose.Orientation.y += ANGLE(-180.0f);
		}

		// Set player interaction.
		if (hasFront || hasBack)
		{
			SetAnimation(playerItem, (playerItem->Animation.ActiveState == LS_REACH) ? LA_REACH_TO_HORIZONTAL_BAR : LA_JUMP_UP_TO_HORIZONTAL_BAR);
			ResetPlayerFlex(playerItem);
			playerItem->Animation.Velocity.y = 0.0f;
			playerItem->Animation.IsAirborne = false;
			player.Context.InteractedItem = itemNumber;
			player.Control.HandStatus = HandStatus::Busy;

			// Calculate catch position from line.
			auto linePoint0 = Geometry::TranslatePoint(barItem.Pose.Position.ToVector3(), barItem.Pose.Orientation.y, 0.0f, 0.0f, HorizontalBarBounds.BoundingBox.X1);
			auto linePoint1 = Geometry::TranslatePoint(barItem.Pose.Position.ToVector3(), barItem.Pose.Orientation.y, 0.0f, 0.0f, HorizontalBarBounds.BoundingBox.X2);
			auto catchPos = Geometry::GetClosestPointOnLine(playerItem->Pose.Position.ToVector3(), linePoint0, linePoint1);

			// Update player pose.
			playerItem->Pose.Position = Geometry::TranslatePoint(catchPos, 0, 0.0f, coll->Setup.Height + CLICK(0.25f));
			playerItem->Pose.Orientation.y = barItem.Pose.Orientation.y + (hasFront ? ANGLE(0.0f) : ANGLE(-180.0f));
		}
		else
		{
			ObjectCollision(itemNumber, playerItem, coll);
		}
	}
	else if (!TestState(playerItem->Animation.ActiveState, HORIZONTAL_BAR_STATES))
	{
		ObjectCollision(itemNumber, playerItem, coll);
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
	auto* item = &g_Level.Items[itemNumber];

	if (!TriggerActive(item))
		return;

	item->Status = ITEM_ACTIVE;
	AnimateItem(item);

	// TODO: ID_SHOOT_SWITCH2 is probably the bell in Trajan Markets, use Lua for that.
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
		spark->blendMode = BlendMode::Additive;
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
		spark->sSize = spark->size = (GetRandomControl() & 0x0F) + 32;
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
