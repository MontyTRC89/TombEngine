#include "framework.h"
#include "collide.h"
#include "control/control.h"
#include "input.h"
#include "items.h"
#include "level.h"
#include "lara.h"
#include "lara_tests.h"
#include "lara_collide.h"
#include "setup.h"

// -----------------------------
// HELPER FUNCTIONS
// For State Control & Collision
// -----------------------------

// TODO: Some states can't make the most of this function due to missing step up/down animations.
// Try implementing leg IK as a substitute to make step animations obsolete. @Sezz 2021.10.09
void DoLaraStep(ITEM_INFO* item, COLL_INFO* coll)
{
	if (TestLaraStepUp(item, coll) &&
		!TestLaraSwamp(item))
	{
		item->goalAnimState = LS_STEP_UP;

		if (GetChange(item, &g_Level.Anims[item->animNumber]))
		{
			item->pos.yPos += coll->Middle.Floor;

			return;
		}
	}
	else if (TestLaraStepDown(item, coll) &&
		!TestLaraSwamp(item))
	{
		item->goalAnimState = LS_STEP_DOWN;

		if (GetChange(item, &g_Level.Anims[item->animNumber]))
		{
			item->pos.yPos += coll->Middle.Floor;

			return;
		}
	}

	// Height difference is below threshold for step dispatch OR step animation doesn't exist; translate Lara to new floor height.
	// TODO: This approach might cause underirable artefacts where an object pushes Lara rapidly up/down a slope or a platform rapidly ascends/descends.
	int div = 2; // 3?
	int rate = 50;
	int threshold = STEP_SIZE / 8;
	if (coll->Middle.Floor != NO_HEIGHT)
	{
		if (TestLaraSwamp(item) &&
			coll->Middle.Floor > 0)
		{
			item->pos.yPos += SWAMP_GRAVITY;
		}
		else if (abs(coll->Middle.Floor) > (STEPUP_HEIGHT / 2) &&		// Outer range.
			abs(coll->Middle.Floor) > threshold)
		{
			if (coll->Middle.Floor <= rate)				// Lower floor range.
				item->pos.yPos -= rate;
			else if (coll->Middle.Floor >= -rate)		// Upper floor range.
				item->pos.yPos += rate;
		}
		else if (abs(coll->Middle.Floor) <= (STEPUP_HEIGHT / 2) &&		// Inner range.
			abs(coll->Middle.Floor) > threshold)
		{
			item->pos.yPos += coll->Middle.Floor / div;
		}
		else
			item->pos.yPos += coll->Middle.Floor;
	}
}

void DoLaraCrawlVault(ITEM_INFO* item, COLL_INFO* coll)
{
	if (TestLaraCrawlExitDownStep(item, coll))
	{
		if (TrInput & IN_DUCK)
			item->goalAnimState = LS_STEP_DOWN;
		else [[likely]]
			item->goalAnimState = LS_CRAWL_EXIT_DOWN_STEP;

		return;
	}

	if (TestLaraCrawlExitJump(item, coll))
	{
		if (TrInput & IN_WALK)
			item->goalAnimState = LS_CRAWL_EXIT_FLIP;
		else [[likely]]
			item->goalAnimState = LS_CRAWL_EXIT_JUMP;

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

void DoLaraCrawlToHangSnap(ITEM_INFO* item, COLL_INFO* coll)
{
	coll->Setup.ForwardAngle += ANGLE(180.0f);
	GetCollisionInfo(coll, item);
	SnapItemToLedge(item, coll);
	MoveItem(item, coll->Setup.ForwardAngle, -LARA_RAD_CRAWL);
	item->pos.yRot += ANGLE(180.0f);
	LaraResetGravityStatus(item, coll);
}

// TODO: Make lean rate proportional to the turn rate, allowing for nicer aesthetics with future analog stick input.
void DoLaraLean(ITEM_INFO* item, COLL_INFO* coll, int maxAngle, short rate)
{
	if (item->speed)
	{
		int sign = copysign(1, maxAngle);
		rate = copysign(rate, maxAngle);

		if (coll->CollisionType == CT_LEFT || coll->CollisionType == CT_RIGHT)
			item->pos.zRot += std::min(abs(rate), abs((maxAngle * 3) / 5 - item->pos.zRot) / 3) * sign;
		else
			item->pos.zRot += std::min(abs(rate), abs(maxAngle - item->pos.zRot) / 3) * sign;
	}
}

void DoLaraCrawlFlex(ITEM_INFO* item, COLL_INFO* coll, short maxAngle, short rate)
{
	LaraInfo*& info = item->data;

	int sign = copysign(1, maxAngle);
	rate = copysign(rate, maxAngle);

	info->torsoZrot += std::min(abs(rate), abs(maxAngle - info->torsoZrot) / 6) * sign;

	if (!(TrInput & IN_LOOK) &&
		item->currentAnimState != LS_CRAWL_BACK)
	{
		info->headZrot = info->torsoZrot / 2;
		info->headYrot = info->headZrot;
	}
}

void SetLaraFallState(ITEM_INFO* item)
{
	SetAnimation(item, LA_FALL_START);
	item->fallspeed = 0;
	item->gravityStatus = true;
}

void SetLaraFallBackState(ITEM_INFO* item)
{
	SetAnimation(item, LA_FALL_BACK);
	item->fallspeed = 0;
	item->gravityStatus = true;
}
