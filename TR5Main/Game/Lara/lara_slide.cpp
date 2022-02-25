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

// -----------------------------
// SLIDE
// Control & Collision Functions
// -----------------------------

// State:		LS_SLIDE_FORWARD (24)
// Collision:	lara_col_slide_forward()
void lara_as_slide_forward(ITEM_INFO* item, COLL_INFO* coll)
{
	auto* info = GetLaraInfo(item);

	Camera.targetElevation = -ANGLE(45.0f);

	if (item->HitPoints <= 0)
	{
		item->TargetState = LS_DEATH;
		return;
	}

	if (TestLaraSlide(item, coll))
	{
		// TODO: Prepped for another time.
		/*if (g_GameFlow->Animations.SlideExtended)
		{
			if (TrInput & IN_LEFT)
			{
				info->Control.TurnRate -= LARA_TURN_RATE;
				if (info->Control.TurnRate < -LARA_SLIDE_TURN_MAX)
					info->Control.TurnRate = -LARA_SLIDE_TURN_MAX;

				DoLaraLean(item, coll, -LARA_LEAN_MAX, LARA_LEAN_RATE / 3 * 2);
			}
			else if (TrInput & IN_RIGHT)
			{
				info->Control.TurnRate += LARA_TURN_RATE;
				if (info->Control.TurnRate > LARA_SLIDE_TURN_MAX)
					info->Control.TurnRate = LARA_SLIDE_TURN_MAX;

				DoLaraLean(item, coll, LARA_LEAN_MAX, LARA_LEAN_RATE / 3 * 2);
			}
		}*/

		if (TrInput & IN_JUMP)
		{
			item->TargetState = LS_JUMP_FORWARD;
			StopSoundEffect(SFX_TR4_LARA_SLIPPING);
			return;
		}

		item->TargetState = LS_SLIDE_FORWARD;
		return;
	}

	if (TrInput & IN_FORWARD)
		item->TargetState = LS_RUN_FORWARD;
	else
		item->TargetState = LS_IDLE;

	StopSoundEffect(SFX_TR4_LARA_SLIPPING);
	return;
}

// State:		LS_SLIDE_FORWARD (24)
// Control:		lara_as_slide_forward()
void lara_col_slide_forward(ITEM_INFO* item, COLL_INFO* coll)
{
	auto* info = GetLaraInfo(item);

	item->Airborne = false;
	info->Control.MoveAngle = item->Position.yRot;
	coll->Setup.LowerFloorBound = NO_LOWER_BOUND;
	coll->Setup.UpperFloorBound = -STEPUP_HEIGHT;
	coll->Setup.LowerCeilingBound = 0;
	coll->Setup.ForwardAngle = info->Control.MoveAngle;
	GetCollisionInfo(coll, item);

	if (TestLaraHitCeiling(coll))
	{
		SetLaraHitCeiling(item, coll);
		return;
	}

	if (coll->Middle.Floor >= CLICK(1) && !TestEnvironment(ENV_FLAG_SWAMP, item))
	{
		SetLaraFallState(item);
		StopSoundEffect(SFX_TR4_LARA_SLIPPING);
		return;
	}

	if (TestLaraSlide(item, coll))
		SetLaraSlideState(item, coll);

	LaraDeflectEdge(item, coll);

	if (TestLaraStep(item, coll))
	{
		LaraSnapToHeight(item, coll);
		//DoLaraStep(item, coll);
		return;
	}
}

// State:		LS_SLIDE_BACK (32)
// Collision:	lara_col_slide_back()
void lara_as_slide_back(ITEM_INFO* item, COLL_INFO* coll)
{
	auto* info = GetLaraInfo(item);

	Camera.targetElevation = -ANGLE(45.0f);
	Camera.targetAngle = ANGLE(135.0f);

	if (item->HitPoints <= 0)
	{
		item->TargetState = LS_DEATH;
		return;
	}

	if (TestLaraSlide(item, coll))
	{
		// TODO: Prepped for another time.
		/*if (g_GameFlow->Animations.SlideExtended)
		{
			if (TrInput & IN_LEFT)
			{
				info->Control.TurnRate -= LARA_TURN_RATE;
				if (info->Control.TurnRate < -LARA_SLIDE_TURN_MAX)
					info->Control.TurnRate = -LARA_SLIDE_TURN_MAX;

				DoLaraLean(item, coll, LARA_LEAN_MAX, LARA_LEAN_RATE / 3 * 2);
			}
			else if (TrInput & IN_RIGHT)
			{
				info->Control.TurnRate += LARA_TURN_RATE;
				if (info->Control.TurnRate > LARA_SLIDE_TURN_MAX)
					info->Control.TurnRate = LARA_SLIDE_TURN_MAX;

				DoLaraLean(item, coll, -LARA_LEAN_MAX, LARA_LEAN_RATE / 3 * 2);
			}
		}*/

		if (TrInput & IN_JUMP)
		{
			item->TargetState = LS_JUMP_BACK;
			StopSoundEffect(SFX_TR4_LARA_SLIPPING);
			return;
		}

		item->TargetState = LS_SLIDE_BACK;
		return;
	}

	item->TargetState = LS_IDLE;
	StopSoundEffect(SFX_TR4_LARA_SLIPPING);
	return;
}

// State:		LS_SLIDE_BACK (32)
// Control:		lara_as_slide_back()
void lara_col_slide_back(ITEM_INFO* item, COLL_INFO* coll)
{
	auto* info = GetLaraInfo(item);

	item->Airborne = false;
	info->Control.MoveAngle = item->Position.yRot + ANGLE(180.0f);
	coll->Setup.LowerFloorBound = NO_LOWER_BOUND;
	coll->Setup.UpperFloorBound = -STEPUP_HEIGHT;
	coll->Setup.LowerCeilingBound = 0;
	coll->Setup.ForwardAngle = info->Control.MoveAngle;
	GetCollisionInfo(coll, item);

	if (TestLaraHitCeiling(coll))
	{
		SetLaraHitCeiling(item, coll);
		return;
	}

	if (coll->Middle.Floor >= CLICK(1) && !TestEnvironment(ENV_FLAG_SWAMP, item))
	{
		SetLaraFallBackState(item);
		StopSoundEffect(SFX_TR4_LARA_SLIPPING);
		return;
	}

	if (TestLaraSlide(item, coll))
		SetLaraSlideState(item, coll);

	LaraDeflectEdge(item, coll);

	if (TestLaraStep(item, coll))
	{
		LaraSnapToHeight(item, coll);
		//DoLaraStep(item, coll);
		return;
	}
}
