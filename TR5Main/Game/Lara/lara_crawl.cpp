#include "framework.h"
#include "lara.h"
#include "input.h"
#include "level.h"
#include "lara_tests.h"
#include "lara_collide.h"
#include "animation.h"
#include "control/los.h"
#include "lara_flare.h"
#include "lara_helpers.h"
#include "collide.h"
#include "items.h"
#include "camera.h"
#include "control/control.h"

// -----------------------------
// CRAWL & CROUCH
// Control & Collision Functions
// -----------------------------

// -------
// CROUCH:
// -------

// State:		LS_CROUCH_IDLE (71)
// Collision:	lara_col_crouch_idle()
void lara_as_crouch_idle(ITEM_INFO* item, COLL_INFO* coll)
{
	// TODO: Deplete air meter if Lara's head is below the water. Original implementation had a weird buffer zone before
	// wade depth where Lara couldn't crouch at all, and if the player forced her into the crouched state by
	// crouching into the region from a run as late as possible, she wasn't able to turn or begin crawling.
	// Since Lara can now crawl at a considerable depth, a region of peril would make sense. @Sezz 2021.10.21

	LaraInfo*& info = item->data;

	info->keepCrouched = TestLaraKeepCrouched(item, coll);
	coll->Setup.EnableSpaz = false;
	coll->Setup.EnableObjectPush = true;
	Camera.targetElevation = -ANGLE(24.0f);

	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_DEATH;

		return;
	}

	if (TrInput & IN_LOOK)
		LookUpDown();

	if (TrInput & IN_LEFT)
		info->turnRate = -LARA_CRAWL_TURN;
	else if (TrInput & IN_RIGHT)
		info->turnRate = LARA_CRAWL_TURN;

	if ((TrInput & IN_DUCK || info->keepCrouched) &&
		info->waterStatus != LW_WADE)
	{
		// TODO: LUA
		info->NewAnims.CrouchRoll = true;

		if (TrInput & IN_SPRINT &&
			TestLaraCrouchRoll(item, coll) &&
			info->gunStatus == LG_HANDS_FREE &&
			info->NewAnims.CrouchRoll)
		{
			item->goalAnimState = LS_CROUCH_ROLL;

			return;
		}

		if (TrInput & (IN_FORWARD | IN_BACK) &&
			TestLaraCrouchToCrawl(item))
		{
			item->goalAnimState = LS_CRAWL_IDLE;

			return;
		}

		if (TrInput & IN_LEFT)
		{
			item->goalAnimState = LS_CROUCH_TURN_LEFT;

			return;
		}
		else if (TrInput & IN_RIGHT)
		{
			item->goalAnimState = LS_CROUCH_TURN_RIGHT;

			return;
		}

		item->goalAnimState = LS_CROUCH_IDLE;

		return;
	}

	item->goalAnimState = LS_IDLE;
}

// State:		LS_CROUCH_IDLE (71)
// Control:		lara_as_crouch_idle()
void lara_col_crouch_idle(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	info->keepCrouched = TestLaraKeepCrouched(item, coll);
	info->isDucked = true;
	info->moveAngle = item->pos.yRot;
	item->gravityStatus = false;
	item->fallspeed = 0;
	coll->Setup.Height = LARA_HEIGHT_CRAWL;
	coll->Setup.ForwardAngle = item->pos.yRot;
	coll->Setup.BadHeightDown = STEP_SIZE - 1;
	coll->Setup.BadHeightUp = -(STEP_SIZE - 1);
	coll->Setup.BadCeilingHeight = 0;
	coll->Setup.SlopesAreWalls = true;
	GetCollisionInfo(coll, item);

	if (TestLaraFall(item, coll))
	{
		info->gunStatus = LG_HANDS_FREE;
		SetLaraFallState(item);

		return;
	}

	if (TestLaraSlide(item, coll))
		return;

	ShiftItem(item, coll);
	LaraSnapToHeight(item, coll);
}

// State:		LS_CROUCH_ROLL (72)
// Collision:	lara_as_crouch_roll()
void lara_as_crouch_roll(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	coll->Setup.EnableSpaz = false;
	coll->Setup.EnableObjectPush = true;
	Camera.targetElevation = -ANGLE(24.0f);

	if (TrInput & IN_LEFT)
	{
		info->turnRate -= LARA_TURN_RATE;
		if (info->turnRate < -LARA_CROUCH_ROLL_TURN)
			info->turnRate = -LARA_CROUCH_ROLL_TURN;

		DoLaraLean(item, coll, -LARA_LEAN_MAX, LARA_LEAN_RATE);
	}
	else if (TrInput & IN_RIGHT)
	{
		info->turnRate += LARA_TURN_RATE;
		if (info->turnRate > LARA_CROUCH_ROLL_TURN)
			info->turnRate = LARA_CROUCH_ROLL_TURN;

		DoLaraLean(item, coll, LARA_LEAN_MAX, LARA_LEAN_RATE);
	}

	item->goalAnimState = LS_CROUCH_IDLE;
}

// State:		LS_CROUCH_ROLL (72)
// Control:		lara_as_crouch_roll()
void lara_col_crouch_roll(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	info->keepCrouched = TestLaraKeepCrouched(item, coll);
	info->isDucked = true;
	info->moveAngle = item->pos.yRot;
	item->gravityStatus = 0;
	item->fallspeed = 0;
	coll->Setup.Height = LARA_HEIGHT_CRAWL;
	coll->Setup.BadHeightDown = STEP_SIZE - 1;
	coll->Setup.BadHeightUp = -(STEP_SIZE - 1);
	coll->Setup.ForwardAngle = item->pos.yRot;
	coll->Setup.BadCeilingHeight = 0;
	coll->Setup.SlopesAreWalls = true;
	GetCollisionInfo(coll, item);

	// TODO: With sufficient speed, Lara can still roll off ledges. This is particularly a problem in the uncommon scenario where
	// she becomes airborne within a crawlspace; collision handling will push her back very rapidly and potentially cause a softlock. @Sezz 2021.11.02
	if (LaraDeflectEdgeCrawl(item, coll))
	{
		item->pos.xPos = coll->Setup.OldPosition.x;
		item->pos.yPos = coll->Setup.OldPosition.y;
		item->pos.zPos = coll->Setup.OldPosition.z;
	}

	if (TestLaraFall(item, coll))
	{
		info->gunStatus = LG_HANDS_FREE;
		item->speed /= 3;				// Truncate speed to prevent flying off.
		SetLaraFallState(item);

		return;
	}

	if (TestLaraSlide(item, coll))
		return;

	ShiftItem(item, coll);

	if (TestLaraHitCeiling(coll))
	{
		SetLaraHitCeiling(item, coll);

		return;
	}

	if (TestLaraStep(coll))
	{
		DoLaraStep(item, coll);

		return;
	}
}

// State:		LS_CROUCH_TURN_LEFT (105)
// Collision:	lara_col_crouch_turn_left()
void lara_as_crouch_turn_left(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	coll->Setup.EnableSpaz = false;
	Camera.targetElevation = -ANGLE(24.0f);

	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_DEATH;

		return;
	}

	if (TrInput & IN_LOOK)
		LookUpDown();

	info->turnRate = -LARA_CRAWL_TURN;

	if ((TrInput & IN_DUCK || info->keepCrouched) &&
		info->waterStatus != LW_WADE)
	{
		if (TrInput & IN_SPRINT &&
			TestLaraCrouchRoll(item, coll) &&
			info->NewAnims.CrouchRoll)
		{
			item->goalAnimState = LS_CROUCH_ROLL;

			return;
		}

		if (TrInput & (IN_FORWARD | IN_BACK) &&
			TestLaraCrouchToCrawl(item))
		{
			item->goalAnimState = LS_CRAWL_IDLE;

			return;
		}

		if (TrInput & IN_LEFT)
		{
			item->goalAnimState = LS_CROUCH_TURN_LEFT;

			return;
		}

		item->goalAnimState = LS_CROUCH_IDLE;

		return;
	}

	item->goalAnimState = LS_IDLE;
}

// State:		LS_CRAWL_TURN_LEFT (105)
// Control:		lara_as_crouch_turn_left()
void lara_col_crouch_turn_left(ITEM_INFO* item, COLL_INFO* coll)
{
	lara_col_crouch_idle(item, coll);
}

// State:		LS_CROUCH_TURN_RIGHT (106)
// Collision:	lara_col_crouch_turn_right()
void lara_as_crouch_turn_right(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	coll->Setup.EnableSpaz = false;
	Camera.targetElevation = -ANGLE(24.0f);

	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_DEATH;

		return;
	}

	if (TrInput & IN_LOOK)
		LookUpDown();

	info->turnRate = LARA_CRAWL_TURN;

	if ((TrInput & IN_DUCK || info->keepCrouched) &&
		info->waterStatus != LW_WADE)
	{
		if (TrInput & IN_SPRINT &&
			TestLaraCrouchRoll(item, coll) &&
			info->NewAnims.CrouchRoll)
		{
			item->goalAnimState = LS_CROUCH_ROLL;

			return;
		}

		if (TrInput & (IN_FORWARD | IN_BACK) &&
			TestLaraCrouchToCrawl(item))
		{
			item->goalAnimState = LS_CRAWL_IDLE;

			return;
		}

		if (TrInput & IN_RIGHT)
		{
			item->goalAnimState = LS_CROUCH_TURN_RIGHT;

			return;
		}

		item->goalAnimState = LS_CROUCH_IDLE;

		return;
	}

	item->goalAnimState = LS_IDLE;
}

// State:		LS_CRAWL_TURN_RIGHT (106)
// Control:		lara_as_crouch_turn_right()
void lara_col_crouch_turn_right(ITEM_INFO* item, COLL_INFO* coll)
{
	lara_col_crouch_idle(item, coll);
}

// ------
// CRAWL:
// ------

// State:		LS_CRAWL_IDLE (80)
// Collision:	lara_col_crawl_idle()
void lara_as_crawl_idle(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	info->keepCrouched = TestLaraKeepCrouched(item, coll);
	info->isDucked = true;
	info->gunStatus = LG_HANDS_BUSY;
	coll->Setup.EnableSpaz = false;
	coll->Setup.EnableObjectPush = true;
	Camera.targetElevation = -ANGLE(24.0f);

	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_DEATH;

		return;
	}

	if (TrInput & IN_LOOK)
		LookUpDown();

	if (TrInput & IN_LEFT)
		info->turnRate = -LARA_CRAWL_TURN;
	else if (TrInput & IN_RIGHT)
		info->turnRate = LARA_CRAWL_TURN;

	// TODO: LUA
	info->NewAnims.CrawlExtended = true;

	if ((TrInput & IN_DUCK || info->keepCrouched) &&
		info->waterStatus != LW_WADE)
	{
		// TODO: Flare not working.
		if ((TrInput & IN_SPRINT && TestLaraCrouchRoll(item, coll)) ||
			(TrInput & (IN_DRAW | IN_FLARE) &&
			!IsStandingWeapon(info->gunType) &&
			item->animNumber != LA_CROUCH_TO_CRAWL_START)) // Hack.
		{
			info->gunStatus = LG_HANDS_FREE;
			item->goalAnimState = LS_CROUCH_IDLE;

			return;
		}

		if (TrInput & IN_FORWARD)
		{
			if (TrInput & (IN_ACTION | IN_JUMP) &&
				TestLaraCrawlVault(item, coll) &&
				info->NewAnims.CrawlExtended)
			{
				DoLaraCrawlVault(item, coll);

				return;
			}
			else if (TestLaraCrawlForward(item, coll)) [[likely]]
			{
				item->goalAnimState = LS_CRAWL_FORWARD;

				return;
			}
		}
		else if (TrInput & IN_BACK)
		{
			if (TrInput & (IN_ACTION | IN_JUMP) &&
				TestLaraCrawlToHang(item, coll))
			{
				DoLaraCrawlToHangSnap(item, coll);
				item->goalAnimState = LS_CRAWL_TO_HANG;

				return;
			}
			else if (TestLaraCrawlBack(item, coll)) [[likely]]
			{
				item->goalAnimState = LS_CRAWL_BACK;

				return;
			}
		}

		if (TrInput & IN_LEFT)
		{
			item->goalAnimState = LS_CRAWL_TURN_LEFT;

			return;
		}
		else if (TrInput & IN_RIGHT)
		{
			item->goalAnimState = LS_CRAWL_TURN_RIGHT;

			return;
		}

		item->goalAnimState = LS_CRAWL_IDLE;

		return;
	}

	info->gunStatus = LG_HANDS_FREE;
	item->goalAnimState = LS_CROUCH_IDLE;
}

// State:		LS_CRAWL_IDLE (80)
// Control:		lara_as_crawl_idle()
void lara_col_crawl_idle(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	info->keepCrouched = TestLaraKeepCrouched(item, coll);
	info->isDucked = true;
	info->moveAngle = item->pos.yRot;
	info->torsoXrot = 0;
	info->torsoYrot = 0;
	item->fallspeed = 0;
	item->gravityStatus = false;
	coll->Setup.ForwardAngle = info->moveAngle;
	coll->Setup.Radius = LARA_RAD_CRAWL;
	coll->Setup.Height = LARA_HEIGHT_CRAWL;
	coll->Setup.BadHeightDown = STEP_SIZE - 1;
	coll->Setup.BadHeightUp = -(STEP_SIZE - 1);
	coll->Setup.BadCeilingHeight = LARA_HEIGHT_CRAWL;
	coll->Setup.SlopesAreWalls = true;
	coll->Setup.SlopesArePits = true;
	GetCollisionInfo(coll, item);

	if (TestLaraFall(item, coll))
	{
		info->gunStatus = LG_HANDS_FREE;
		SetLaraFallState(item);

		return;
	}

	if (TestLaraSlide(item, coll))
		return;

	ShiftItem(item, coll);
	LaraSnapToHeight(item, coll);
}

// State:		LS_CRAWL_FORWARD (81)
// Collision:	lara_col_crawl_forward()
void lara_as_crawl_forward(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	info->keepCrouched = TestLaraKeepCrouched(item, coll);
	info->isDucked = true;
	info->gunStatus = LG_HANDS_BUSY;
	coll->Setup.EnableSpaz = false;
	coll->Setup.EnableObjectPush = true;
	Camera.targetElevation = -ANGLE(24.0f);

	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_DEATH;

		return;
	}

	if (TrInput & IN_LEFT)
	{
		info->turnRate -= LARA_CRAWL_MOVE_TURN_RATE;
		if (info->turnRate < -LARA_CRAWL_MOVE_TURN)
			info->turnRate = -LARA_CRAWL_MOVE_TURN;

		DoLaraCrawlFlex(item, coll, -LARA_CRAWL_FLEX, LARA_CRAWL_FLEX_RATE);
	}
	else if (TrInput & IN_RIGHT)
	{
		info->turnRate += LARA_CRAWL_MOVE_TURN_RATE;
		if (info->turnRate > LARA_CRAWL_MOVE_TURN)
			info->turnRate = LARA_CRAWL_MOVE_TURN;

		DoLaraCrawlFlex(item, coll, LARA_CRAWL_FLEX, LARA_CRAWL_FLEX_RATE);
	}

	if ((TrInput & IN_DUCK || info->keepCrouched) &&
		info->waterStatus != LW_WADE)
	{
		if (TrInput & IN_SPRINT &&
			TestLaraCrouchRoll(item, coll))
		{
			item->goalAnimState = LS_CRAWL_IDLE;

			return;
		}

		if (TrInput & IN_FORWARD)
		{
			if (TrInput & (IN_ACTION | IN_JUMP) &&
				TestLaraCrawlVault(item, coll) &&
				info->NewAnims.CrawlExtended)
			{
				DoLaraCrawlVault(item, coll);

				return;
			}
			else [[likely]]
			{
				item->goalAnimState = LS_CRAWL_FORWARD;

				return;
			}
		}

		item->goalAnimState = LS_CRAWL_IDLE;

		return;
	}

	item->goalAnimState = LS_CRAWL_IDLE;
}

// State:		LS_CRAWL_FORWARD (81)
// Control:		lara_as_crawl_forward()
void lara_col_crawl_forward(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	info->keepCrouched = TestLaraKeepCrouched(item, coll);
	info->isDucked = true;
	info->moveAngle = item->pos.yRot;
	info->torsoXrot = 0;
	info->torsoYrot = 0;
	item->gravityStatus = false;
	item->fallspeed = 0;
	coll->Setup.Radius = LARA_RAD_CRAWL;
	coll->Setup.Height = LARA_HEIGHT_CRAWL;
	coll->Setup.BadHeightDown = STEP_SIZE - 1;		// Offset of 1 is required or Lara will crawl up/down steps.
	coll->Setup.BadHeightUp = -(STEP_SIZE - 1);		// TODO: Stepping approach is different from walk/run because crawl step anims do not submerge Lara. Resolve this someday. @Sezz 2021.10.31
	coll->Setup.BadCeilingHeight = LARA_HEIGHT_CRAWL;
	coll->Setup.SlopesArePits = true;
	coll->Setup.SlopesAreWalls = true;
	coll->Setup.DeathFlagIsPit = true;
	coll->Setup.ForwardAngle = info->moveAngle;
	GetCollisionInfo(coll, item, true);

	if (LaraDeflectEdgeCrawl(item, coll))
		LaraCollideStopCrawl(item, coll);
	
	if (TestLaraFall(item, coll))
	{
		info->gunStatus = LG_HANDS_FREE;
		SetLaraFallState(item);

		return;
	}
	
	if (TestLaraSlide(item, coll))
		return;

	ShiftItem(item, coll);

	if (TestLaraStep(coll))
	{
		DoLaraStep(item, coll);

		return;
	}
}

// State:		LS_CRAWL_BACK (86)
// Collision:	lara_col_crawl_back()
void lara_as_crawl_back(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	info->keepCrouched = TestLaraKeepCrouched(item, coll);
	info->isDucked = true;
	info->look = false;
	info->gunStatus = LG_HANDS_BUSY;
	coll->Setup.EnableSpaz = false;
	coll->Setup.EnableObjectPush = true;
	Camera.targetElevation = -ANGLE(24.0f);

	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_DEATH;

		return;
	}

	if (TrInput & IN_LEFT)
	{
		info->turnRate -= LARA_CRAWL_MOVE_TURN_RATE;
		if (info->turnRate < -LARA_CRAWL_MOVE_TURN)
			info->turnRate = -LARA_CRAWL_MOVE_TURN;

		DoLaraCrawlFlex(item, coll, LARA_CRAWL_FLEX, LARA_CRAWL_FLEX_RATE);
	}
	else if (TrInput & IN_RIGHT)
	{
		info->turnRate += LARA_CRAWL_MOVE_TURN_RATE;
		if (info->turnRate > LARA_CRAWL_MOVE_TURN)
			info->turnRate = LARA_CRAWL_MOVE_TURN;

		DoLaraCrawlFlex(item, coll, -LARA_CRAWL_FLEX, LARA_CRAWL_FLEX_RATE);
	}

	if ((TrInput & IN_DUCK || info->keepCrouched) &&
		info->waterStatus != LW_WADE)
	{
		if (TrInput & IN_BACK)
		{
			// TODO: Crawl-to-hang dispatch not working directly from here. Must be idle. Hardly noticeable, but a seamless dispatch would be preferred. @Sezz 2021.11.23
			/*if (TrInput & (IN_ACTION | IN_JUMP) &&
				TestLaraCrawlToHang(item, coll))
			{
				DoLaraCrawlToHangSnap(item, coll);
				item->goalAnimState = LS_CRAWL_TO_HANG;

				return;
			}
			else [[likely]]*/
			{
				item->goalAnimState = LS_CRAWL_BACK;

				return;
			}
		}

		item->goalAnimState = LS_CRAWL_IDLE;

		return;
	}

	item->goalAnimState = LS_CRAWL_IDLE;
}

// State:		LS_CRAWL_BACK (86)
// Control:		lara_as_crawl_back()
void lara_col_crawl_back(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	info->keepCrouched = TestLaraKeepCrouched(item, coll);
	info->isDucked = true;
	info->moveAngle = item->pos.yRot + ANGLE(180.0f);
	item->gravityStatus = false;
	item->fallspeed = 0;
	coll->Setup.Radius = LARA_RAD_CRAWL;
	coll->Setup.Height = LARA_HEIGHT_CRAWL;
	coll->Setup.BadHeightDown = STEP_SIZE - 1;		// Offset of 1 is required or Lara will crawl up/down steps.
	coll->Setup.BadHeightUp = -(STEP_SIZE - 1);
	coll->Setup.BadCeilingHeight = LARA_HEIGHT_CRAWL;
	coll->Setup.SlopesArePits = true;
	coll->Setup.SlopesAreWalls = true;
	coll->Setup.DeathFlagIsPit = true;
	coll->Setup.ForwardAngle = info->moveAngle;
	GetCollisionInfo(coll, item, true);

	if (LaraDeflectEdgeCrawl(item, coll))
		LaraCollideStopCrawl(item, coll);

	if (TestLaraFall(item, coll))
	{
		info->gunStatus = LG_HANDS_FREE;
		SetLaraFallState(item);

		return;
	}

	if (TestLaraSlide(item, coll))
		return;

	ShiftItem(item, coll);

	if (TestLaraStep(coll))
	{
		DoLaraStep(item, coll);

		return;
	}
}

// State:		LS_CRAWL_TURN_LEFT (84)
// Collision:	lara_col_crawl_turn_left()
void lara_as_crawl_turn_left(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	info->keepCrouched = TestLaraKeepCrouched(item, coll);
	info->isDucked = true;
	info->gunStatus = LG_HANDS_BUSY;
	coll->Setup.EnableSpaz = false;
	coll->Setup.EnableObjectPush = true;
	Camera.targetElevation = -ANGLE(24.0f);

	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_DEATH;

		return;
	}

	info->turnRate = -LARA_CRAWL_TURN;

	if ((TrInput & IN_DUCK || info->keepCrouched) &&
		info->waterStatus != LW_WADE)
	{
		if (TrInput & IN_SPRINT && TestLaraCrouchRoll(item, coll))
		{
			item->goalAnimState = LS_CRAWL_IDLE;

			return;
		}

		if (TrInput & IN_FORWARD &&
			TestLaraCrawlForward(item, coll))
		{
			item->goalAnimState = LS_CRAWL_FORWARD;

			return;
		}

		if (TrInput & IN_BACK &&
			TestLaraCrawlBack(item, coll))
		{
			item->goalAnimState = LS_CRAWL_BACK;

			return;
		}

		if (TrInput & IN_LEFT)
		{
			item->goalAnimState = LS_CRAWL_TURN_LEFT;

			return;
		}

		item->goalAnimState = LS_CRAWL_IDLE;

		return;
	}

	item->goalAnimState = LS_CRAWL_IDLE;
}

// State:		LS_CRAWL_TURN_LEFT (84)
// Control:		lara_as_crawl_turn_left()
void lara_col_crawl_turn_left(ITEM_INFO* item, COLL_INFO* coll)
{
	lara_col_crawl_idle(item, coll);
}

// State:		LS_CRAWL_TURN_RIGHT (85)
// Collision:	lara_col_crawl_turn_right()
void lara_as_crawl_turn_right(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	info->keepCrouched = TestLaraKeepCrouched(item, coll);
	info->isDucked = true;
	info->gunStatus = LG_HANDS_BUSY;
	coll->Setup.EnableSpaz = false;
	coll->Setup.EnableObjectPush = true;
	Camera.targetElevation = -ANGLE(24.0f);

	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_DEATH;

		return;
	}

	info->turnRate = LARA_CRAWL_TURN;

	if ((TrInput & IN_DUCK || info->keepCrouched) &&
		info->waterStatus != LW_WADE)
	{
		if (TrInput & IN_SPRINT &&
			TestLaraCrouchRoll(item, coll))
		{
			item->goalAnimState = LS_CRAWL_IDLE;

			return;
		}

		if (TrInput & IN_FORWARD &&
			TestLaraCrawlForward(item, coll))
		{
			item->goalAnimState = LS_CRAWL_FORWARD;

			return;
		}

		if (TrInput & IN_BACK &&
			TestLaraCrawlBack(item, coll))
		{
			item->goalAnimState = LS_CRAWL_BACK;

			return;
		}

		if (TrInput & IN_RIGHT)
		{
			item->goalAnimState = LS_CRAWL_TURN_RIGHT;

			return;
		}

		item->goalAnimState = LS_CRAWL_IDLE;

		return;
	}

	item->goalAnimState = LS_CRAWL_IDLE;
}

// State:		LS_CRAWL_TURN_RIGHT (85)
// Control:		lara_as_crawl_turn_right()
void lara_col_crawl_turn_right(ITEM_INFO* item, COLL_INFO* coll)
{
	lara_col_crawl_idle(item, coll);
}

void lara_col_crawl2hang(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	coll->Setup.EnableSpaz = false;
	coll->Setup.EnableObjectPush = false;
	Camera.targetAngle = 0;
	Camera.targetElevation = -ANGLE(45.0f);

	if (item->animNumber == LA_CRAWL_TO_HANG_END)
	{
		info->moveAngle = item->pos.yRot;
		coll->Setup.Height = LARA_HEIGHT_STRETCH;
		coll->Setup.BadHeightDown = NO_BAD_POS;
		coll->Setup.BadHeightUp = -STEPUP_HEIGHT;
		coll->Setup.BadCeilingHeight = BAD_JUMP_CEILING;
		coll->Setup.ForwardAngle = info->moveAngle;

		MoveItem(item, item->pos.yRot, -STEP_SIZE);
		GetCollisionInfo(coll, item);
		SnapItemToLedge(item, coll);

		if (TestHangSwingIn(item, item->pos.yRot))
		{
			SetAnimation(item, LA_JUMP_UP_TO_MONKEYSWING);
			info->headYrot = 0;
			info->headXrot = 0;
			info->torsoYrot = 0;
			info->torsoXrot = 0;
		}
		else
		{
			SetAnimation(item, LA_REACH_TO_HANG, 12);
			if (TestHangFeet(item, item->pos.yRot))
				item->goalAnimState = LS_HANG_FEET;
		}

		GetCollisionInfo(coll, item);
		info->gunStatus = LG_HANDS_BUSY;
		item->pos.yPos += coll->Front.Floor - GetBoundsAccurate(item)->Y1 - 20;
		item->gravityStatus = true;
		item->speed = 2;
		item->fallspeed = 1;
	}
}
