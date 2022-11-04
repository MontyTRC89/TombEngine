#include "framework.h"
#include "Objects/TR5/Trap/tr5_deathslide.h"

#include "Game/animation.h"
#include "Game/collision/collide_item.h"
#include "Game/control/box.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Math/Math.h"
#include "Sound/sound.h"
#include "Specific/Input/Input.h"
#include "Specific/setup.h"

using namespace TEN::Input;

static constexpr auto ZIP_LINE_VELOCITY_ACCEL = 5.0f;
static constexpr auto ZIP_LINE_VELOCITY_MAX	  = 100.0f;

static const auto ZIP_LINE_STEEPNESS_ANGLE = -ANGLE(11.25f);

const auto ZipLineMountedOffset = Vector3i(0, 0, 371);
const auto ZipLineMountBasis = ObjectCollisionBounds
{
	GameBoundingBox(
		-CLICK(1), CLICK(1),
		-CLICK(1), CLICK(1),
		CLICK(1), CLICK(2)
	),
	std::pair(
		EulerAngles(0, ANGLE(-25.0f), 0),
		EulerAngles(0, ANGLE(25.0f), 0)
	)
};

void InitialiseZipLine(short itemNumber)
{
	auto& zipLineItem = g_Level.Items[itemNumber];
	zipLineItem.Data = GameVector();
	auto& pos = *(GameVector*)zipLineItem.Data;

	pos = GameVector(zipLineItem.Pose.Position, zipLineItem.RoomNumber);
}

void ZipLineCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
{
	auto& zipLineItem = g_Level.Items[itemNumber];
	auto& lara = *GetLaraInfo(laraItem);

	if (zipLineItem.Status != ITEM_NOT_ACTIVE)
		return;

	if (!(TrInput & IN_ACTION) ||
		laraItem->Animation.ActiveState != LS_IDLE ||
		laraItem->Animation.IsAirborne ||
		lara.Control.HandStatus != HandStatus::Free)
	{
		return;
	}

	if (TestLaraPosition(ZipLineMountBasis, &zipLineItem, laraItem))
	{
		AlignLaraPosition(ZipLineMountedOffset, &zipLineItem, laraItem);
		lara.Control.HandStatus = HandStatus::Busy;

		laraItem->Animation.TargetState = LS_ZIP_LINE;
		do
			AnimateItem(laraItem);
		while (laraItem->Animation.ActiveState != LS_GRABBING);

		if (!zipLineItem.Active)
			AddActiveItem(itemNumber);

		zipLineItem.Status = ITEM_ACTIVE;
		zipLineItem.Flags |= IFLAG_INVISIBLE;

		lara.Control.IsMoving = false;
		lara.Control.HandStatus = HandStatus::Busy;
	}
}

void ControlZipLine(short itemNumber)
{
	auto* zipLineItem = &g_Level.Items[itemNumber];
	auto* laraItem = LaraItem;

	if (zipLineItem->Status == ITEM_ACTIVE)
	{
		if (!(zipLineItem->Flags & IFLAG_INVISIBLE))
		{
			auto* prevPos = (GameVector*)zipLineItem->Data;

			zipLineItem->Pose.Position = prevPos->ToVector3i();

			if (prevPos->RoomNumber != zipLineItem->RoomNumber)
				ItemNewRoom(itemNumber, prevPos->RoomNumber);

			zipLineItem->Status = ITEM_NOT_ACTIVE;
			SetAnimation(zipLineItem, 0);

			RemoveActiveItem(itemNumber);
			return;
		}

		if (zipLineItem->Animation.ActiveState == 1)
		{
			AnimateItem(zipLineItem);
			return;
		}

		AnimateItem(zipLineItem);

		// Accelerate.
		if (zipLineItem->Animation.Velocity.y < ZIP_LINE_VELOCITY_MAX)
			zipLineItem->Animation.Velocity.y += ZIP_LINE_VELOCITY_ACCEL;

		// Translate.
		auto headingOrient = EulerAngles(ZIP_LINE_STEEPNESS_ANGLE, zipLineItem->Pose.Orientation.y, 0);
		TranslateItem(zipLineItem, headingOrient, zipLineItem->Animation.Velocity.y);

		int vPos = zipLineItem->Pose.Position.y + CLICK(0.25f);
		auto pointColl = GetCollision(zipLineItem, zipLineItem->Pose.Orientation.y, zipLineItem->Animation.Velocity.y);
		
		// Update zip line room number.
		if (pointColl.RoomNumber != zipLineItem->RoomNumber)
			ItemNewRoom(itemNumber, pointColl.RoomNumber);

		// "Parent" player to zip line.
		if (laraItem->Animation.ActiveState == LS_ZIP_LINE)
			laraItem->Pose.Position = zipLineItem->Pose.Position;

		if (pointColl.Position.Floor <= (vPos + CLICK(1)) || pointColl.Position.Ceiling >= (vPos - CLICK(1)))
		{
			// Whizz sound.
			SoundEffect(SFX_TR4_TRAIN_DOOR_CLOSE, &zipLineItem->Pose);
		}
		// Dismount.
		else
		{
			if (laraItem->Animation.ActiveState == LS_ZIP_LINE)
			{
				laraItem->Animation.TargetState = LS_JUMP_FORWARD;
				AnimateLara(laraItem);
				laraItem->Animation.IsAirborne = true;
				laraItem->Animation.Velocity.y = zipLineItem->Animation.Velocity.y / 4;
				laraItem->Animation.Velocity.z = zipLineItem->Animation.Velocity.y;
			}

			SoundEffect(SFX_TR4_VONCROY_KNIFE_SWISH, &zipLineItem->Pose);
			RemoveActiveItem(itemNumber);
			zipLineItem->Status = ITEM_NOT_ACTIVE;
			zipLineItem->Flags -= IFLAG_INVISIBLE;
		}
	}
}
