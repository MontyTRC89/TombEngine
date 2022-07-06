#include "framework.h"
#include "Game/Lara/lara_slide.h"

#include "Game/camera.h"
#include "Game/collision/collide_room.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_collide.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/Lara/lara_tests.h"
#include "Sound/sound.h"
#include "Specific/input.h"
#include "Specific/level.h"

using namespace TEN::Input;

// -----------------------------
// SLIDE
// Control & Collision Functions
// -----------------------------

// State:		LS_SLIDE_FORWARD (24)
// Collision:	lara_col_slide_forward()
void lara_as_slide_forward(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	Camera.targetElevation = -ANGLE(45.0f);

	if (item->HitPoints <= 0)
	{
		item->Animation.TargetState = LS_DEATH;
		return;
	}

	if (TrInput & IN_LOOK)
		LookUpDown(item);

	if (TestLaraSlide(item, coll))
	{
		/*short direction = GetLaraSlideDirection(item, coll);

		if (g_GameFlow->Animations.HasSlideExtended)
		{
			ApproachLaraTargetOrientation(item, direction, 12);
			ModulateLaraSlideVelocity(item, coll);

			// TODO: Prepped for another time.
			if (TrInput & IN_LEFT)
			{
				lara->Control.TurnRate -= LARA_TURN_RATE_ACCEL;
				if (lara->Control.TurnRate < -LARA_SLIDE_TURN_RATE_MAX)
					lara->Control.TurnRate = -LARA_SLIDE_TURN_RATE_MAX;

				DoLaraLean(item, coll, -LARA_LEAN_MAX, LARA_LEAN_RATE / 3 * 2);
			}
			else if (TrInput & IN_RIGHT)
			{
				lara->Control.TurnRate += LARA_TURN_RATE_ACCEL;
				if (lara->Control.TurnRate > LARA_SLIDE_TURN_RATE_MAX)
					lara->Control.TurnRate = LARA_SLIDE_TURN_RATE_MAX;

				DoLaraLean(item, coll, LARA_LEAN_MAX, LARA_LEAN_RATE / 3 * 2);
			}
		}
		else
			ApproachLaraTargetOrientation(item, direction);*/

		if (TrInput & IN_JUMP && TestLaraSlideJump(item, coll))
		{
			item->Animation.TargetState = LS_JUMP_FORWARD;
			StopSoundEffect(SFX_TR4_LARA_SLIPPING);
			return;
		}

		item->Animation.TargetState = LS_SLIDE_FORWARD;
		return;
	}

	if (TrInput & IN_FORWARD)
		item->Animation.TargetState = LS_RUN_FORWARD;
	else
		item->Animation.TargetState = LS_IDLE;

	StopSoundEffect(SFX_TR4_LARA_SLIPPING);
	return;
}

// State:		LS_SLIDE_FORWARD (24)
// Control:		lara_as_slide_forward()
void lara_col_slide_forward(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	item->Animation.IsAirborne = false;
	lara->Control.MoveAngle = item->Pose.Orientation.y;
	coll->Setup.Height = LARA_HEIGHT_CRAWL;	// HACK: Behaves better with clamps.
	coll->Setup.LowerFloorBound = NO_LOWER_BOUND;
	coll->Setup.UpperFloorBound = -STEPUP_HEIGHT;
	coll->Setup.LowerCeilingBound = 0;
	coll->Setup.ForwardAngle = lara->Control.MoveAngle;
	GetCollisionInfo(coll, item);

	if (TestLaraHitCeiling(coll))
	{
		SetLaraHitCeiling(item, coll);
		return;
	}

	if (coll->Middle.Floor > CLICK(1) && !TestEnvironment(ENV_FLAG_SWAMP, item))
	{
		SetLaraFallAnimation(item);
		StopSoundEffect(SFX_TR4_LARA_SLIPPING);
		return;
	}

	if (TestLaraSlide(item, coll))
		SetLaraSlideAnimation(item, coll);

	LaraDeflectEdge(item, coll);

	if (TestLaraStep(item, coll))
	{
		//DoLaraStep(item, coll);
		LaraSnapToHeight(item, coll);
		return;
	}
}

// State:		LS_SLIDE_BACK (32)
// Collision:	lara_col_slide_back()
void lara_as_slide_back(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	Camera.targetElevation = -ANGLE(45.0f);
	Camera.targetAngle = ANGLE(135.0f);

	if (item->HitPoints <= 0)
	{
		item->Animation.TargetState = LS_DEATH;
		return;
	}

	if (TrInput & IN_LOOK)
		LookUpDown(item);

	if (TestLaraSlide(item, coll))
	{
		/*short direction = GetLaraSlideDirection(item, coll) + ANGLE(180.0f);

		if (g_GameFlow->Animations.HasSlideExtended)
		{
			ApproachLaraTargetOrientation(item, direction, 12);
			ModulateLaraSlideVelocity(item, coll);

			// TODO: Prepped for another time.
			if (TrInput & IN_LEFT)
			{
				lara->Control.TurnRate -= LARA_TURN_RATE_ACCEL;
				if (lara->Control.TurnRate < -LARA_SLIDE_TURN_RATE_MAX)
					lara->Control.TurnRate = -LARA_SLIDE_TURN_RATE_MAX;

				DoLaraLean(item, coll, LARA_LEAN_MAX, LARA_LEAN_RATE / 3 * 2);
			}
			else if (TrInput & IN_RIGHT)
			{
				lara->Control.TurnRate += LARA_TURN_RATE_ACCEL;
				if (lara->Control.TurnRate > LARA_SLIDE_TURN_RATE_MAX)
					lara->Control.TurnRate = LARA_SLIDE_TURN_RATE_MAX;

				DoLaraLean(item, coll, -LARA_LEAN_MAX, LARA_LEAN_RATE / 3 * 2);
			}
		}
		else
			ApproachLaraTargetOrientation(item, direction);*/

		if (TrInput & IN_JUMP && TestLaraSlideJump(item, coll))
		{
			item->Animation.TargetState = LS_JUMP_BACK;
			StopSoundEffect(SFX_TR4_LARA_SLIPPING);
			return;
		}

		item->Animation.TargetState = LS_SLIDE_BACK;
		return;
	}

	item->Animation.TargetState = LS_IDLE;
	StopSoundEffect(SFX_TR4_LARA_SLIPPING);
	return;
}

// State:		LS_SLIDE_BACK (32)
// Control:		lara_as_slide_back()
void lara_col_slide_back(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	item->Animation.IsAirborne = false;
	lara->Control.MoveAngle = item->Pose.Orientation.y + ANGLE(180.0f);
	coll->Setup.Height = LARA_HEIGHT_CRAWL;	// HACK: Behaves better with clamps.
	coll->Setup.LowerFloorBound = NO_LOWER_BOUND;
	coll->Setup.UpperFloorBound = -STEPUP_HEIGHT;
	coll->Setup.LowerCeilingBound = 0;
	coll->Setup.ForwardAngle = lara->Control.MoveAngle;
	GetCollisionInfo(coll, item);

	if (TestLaraHitCeiling(coll))
	{
		SetLaraHitCeiling(item, coll);
		return;
	}

	if (coll->Middle.Floor > CLICK(1) && !TestEnvironment(ENV_FLAG_SWAMP, item))
	{
		SetLaraFallBackAnimation(item);
		StopSoundEffect(SFX_TR4_LARA_SLIPPING);
		return;
	}

	if (TestLaraSlide(item, coll))
		SetLaraSlideAnimation(item, coll);

	LaraDeflectEdge(item, coll);

	if (TestLaraStep(item, coll))
	{
		//DoLaraStep(item, coll);
		LaraSnapToHeight(item, coll);
		return;
	}
}
