#include "framework.h"
#include "collide.h"
#include "control/control.h"
#include "input.h"
#include "items.h"
#include "level.h"
#include "lara.h"
#include "lara_tests.h"
#include "lara_collide.h"

// -----------------------------
// HELPER FUNCTIONS
// For State Control & Collision
// -----------------------------

// TODO: Some states can't make the most of this function due to missing step up/down animations.
// Try implementing leg IK as a substitute to make step animations obsolete. @Sezz 2021.10.09
void DoLaraStep(ITEM_INFO* item, COLL_INFO* coll)
{
	if (TestLaraStepUp(item, coll))
	{
		item->goalAnimState = LS_STEP_UP;

		if (GetChange(item, &g_Level.Anims[item->animNumber]))
		{
			item->pos.yPos += coll->Middle.Floor;

			return;
		}
	}
	else if (TestLaraStepDown(item, coll))
	{
		item->goalAnimState = LS_STEP_DOWN;

		if (GetChange(item, &g_Level.Anims[item->animNumber]))
		{
			item->pos.yPos += coll->Middle.Floor;

			return;
		}
	}

	// Height difference is below threshold for step dispatch or step animation doesn't exist; translate Lara to new floor height.
	// TODO: Try using bad heights native to each state.
	// TODO: Follow cube root curve instead of doing this ugly thing.
	// TODO: This might cause underirable artefacts where an object pushes Lara rapidly up a slope or a platform ascends.
	// Leg IK may correct for it, but until I get that working, for any future developments that see Lara phasing below the floor:
	// comment everything EXCEPT the last two lines. Lara will simply snap to the surface as before (although the LS_RUN and LS_WADE_FORWARD
	// states had a linear transition of 50 units per frame, so not all original behaviour can be restored from here;
	// as that is so, I have left commented legacy step code in their respective col functions). @Sezz 2021.10.09
	int div = 3;
	if (coll->Middle.Floor != NO_HEIGHT)
	{
		if (abs(coll->Middle.Floor) <= (STEPUP_HEIGHT / 2) &&			// Upper floor range.
			abs(coll->Middle.Floor) >= div)
		{
			item->pos.yPos += coll->Middle.Floor / div;
		}
		else if (abs(coll->Middle.Floor) > (STEPUP_HEIGHT / 2) &&		// Lower floor range.
			abs(coll->Middle.Floor) >= div)
		{
			if (coll->Middle.Floor >= -50)			// Upper floor range.
				item->pos.yPos += 50;
			else if (coll->Middle.Floor <= 50)		// Lower floor range.
				item->pos.yPos -= 50;
		}
		else
			item->pos.yPos += coll->Middle.Floor;
	}
}

void DoLaraCrawlVault(ITEM_INFO* item, COLL_INFO* coll)
{
	if (TestLaraCrawlExitJump(item, coll))
	{
		if (TrInput & IN_WALK)
			item->goalAnimState = LS_CRAWL_EXIT_FLIP;
		else [[likely]]
			item->goalAnimState = LS_CRAWL_EXIT_JUMP;

		return;
	}

	if (TestLaraCrawlExitDownStep(item, coll))
	{
		if (TrInput & IN_DUCK)
			item->goalAnimState = LS_STEP_DOWN;
		else [[likely]]
			item->goalAnimState = LS_CRAWL_EXIT_DOWN_STEP;

		return;
	}

	if (TestLaraCrawlUpStep(item, coll))
	{
		item->goalAnimState = LS_STEP_UP;

		return;
	}

	if (TestLaraCrawlDownStep(item, coll))
	{
		item->goalAnimState = LS_STEP_DOWN;

		return;
	}
}

// TODO: Keeping legacy quadrant snaps for now. @Sezz 2021.10.16
void DoLaraCrawlToHangSnap(ITEM_INFO* item, COLL_INFO* coll)
{
	coll->Setup.ForwardAngle += ANGLE(180);
	GetCollisionInfo(coll, item);
	SnapItemToLedge(item, coll);
	MoveItem(item, coll->Setup.ForwardAngle, -LARA_RAD_CRAWL);
	item->pos.yRot += ANGLE(180);
	LaraResetGravityStatus(item, coll);
}

// TODO: Make lean rate proportional to the turn rate, allowing for nicer aesthetics with future analog stick input.
// The following commented line (applicable to run state) makes the lean rate LINEARLY proportional, but it's visually much too subtle.
// Ideally, lean rate should be on a curve, approaching leanMax faster at Lara.turnRate values near zero
// and falling off as Lara.turnRate approaches LARA_FAST_TURN (i.e. max turn rate).
// Unfortunately I am terrible at mathematics and I don't know how to do this. 
// Would a library of easing functions be helpful here? @Sezz 2021.09.26
// item->pos.zRot += (item->pos.zRot -maxLean / LARA_FAST_TURN * Lara.turnRate) / 3;
void DoLaraLean(ITEM_INFO* item, COLL_INFO* coll, int maxAngle, int divisor)
{
	if (item->speed)
	{
		if (coll->CollisionType == CT_LEFT ||
			coll->CollisionType == CT_RIGHT)
		{
			item->pos.zRot += ((maxAngle * 3) / 5 - item->pos.zRot) / divisor;
		}
		else
			item->pos.zRot += (maxAngle - item->pos.zRot) / divisor;
	}
}

// TODO: State dispatch to a new LS_FALL state. The issue is that goal states set in collision functions are only actuated on the following
// frame, resulting in an unacceptable delay. Changing the order in which routine functions are executed is not a viable solution. @Sezz 2021.09.26
void SetLaraFallState(ITEM_INFO* item)
{
	item->animNumber = LA_FALL_START;
	item->currentAnimState = LS_JUMP_FORWARD;
	item->goalAnimState = LS_JUMP_FORWARD;
	item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
	item->fallspeed = 0;
	item->gravityStatus = true;
}

void SetLaraFallBackState(ITEM_INFO* item)
{
	item->animNumber = LA_FALL_BACK;
	item->currentAnimState = LS_FALL_BACK;
	item->goalAnimState = LS_FALL_BACK;
	item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
	item->fallspeed = 0;
	item->gravityStatus = true;
}
