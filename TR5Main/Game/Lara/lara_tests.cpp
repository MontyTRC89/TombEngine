#include "framework.h"
#include "lara.h"
#include "lara_tests.h"
#include "input.h"
#include "level.h"
#include "animation.h"
#include "lara_climb.h"
#include "lara_monkey.h"
#include "lara_collide.h"
#include "lara_flare.h"
#include "control/control.h"
#include "control/los.h"
#include "items.h"
#include "Renderer11.h"

using namespace TEN::Renderer;
using namespace TEN::Floordata;

// -----------------------------
// TEST FUNCTIONS
// For State Control & Collision
// -----------------------------

// Test if a ledge in front of item is valid to climb.
bool TestValidLedge(ITEM_INFO* item, COLL_INFO* coll, bool ignoreHeadroom, bool heightLimit)
{
	// Determine probe base point.
	// We use 1/3 radius extents here for two purposes. First - we can't guarantee that
	// shifts weren't already applied and misfire may occur. Second - it guarantees
	// that Lara won't land on a very thin edge of diagonal geometry.

	int xf = phd_sin(coll->NearestLedgeAngle) * (coll->Setup.Radius * 1.3f);
	int zf = phd_cos(coll->NearestLedgeAngle) * (coll->Setup.Radius * 1.3f);

	// Determine probe left/right points
	int xl = xf + phd_sin(coll->NearestLedgeAngle - ANGLE(90)) * coll->Setup.Radius;
	int zl = zf + phd_cos(coll->NearestLedgeAngle - ANGLE(90)) * coll->Setup.Radius;
	int xr = xf + phd_sin(coll->NearestLedgeAngle + ANGLE(90)) * coll->Setup.Radius;
	int zr = zf + phd_cos(coll->NearestLedgeAngle + ANGLE(90)) * coll->Setup.Radius;

	// Determine probe top point
	int y = item->pos.yPos - coll->Setup.Height;

	// Get floor heights at both points
	auto left  = GetCollisionResult(item->pos.xPos + xl, y, item->pos.zPos + zl, GetRoom(item->location, item->pos.xPos, y, item->pos.zPos).roomNumber).Position.Floor;
	auto right = GetCollisionResult(item->pos.xPos + xr, y, item->pos.zPos + zr, GetRoom(item->location, item->pos.xPos, y, item->pos.zPos).roomNumber).Position.Floor;

	//g_Renderer.addDebugSphere(Vector3(item->pos.xPos + xl, left, item->pos.zPos + zl), 64, Vector4::One, RENDERER_DEBUG_PAGE::LOGIC_STATS);
	//g_Renderer.addDebugSphere(Vector3(item->pos.xPos + xr, right, item->pos.zPos + zr), 64, Vector4::One, RENDERER_DEBUG_PAGE::LOGIC_STATS);

	// Determine allowed slope difference for a given collision radius
	auto slopeDelta = ((float)STEPUP_HEIGHT / (float)WALL_SIZE) * (coll->Setup.Radius * 2);

	// If specified, limit vertical search zone only to nearest height
	if (heightLimit && (abs(left - y) > (STEP_SIZE / 2) || abs(right - y) > (STEP_SIZE / 2)))
		return false;

	// Discard if there is a slope beyond tolerance delta
	if (abs(left - right) >= slopeDelta)
		return false;

	// Discard if ledge is not within distance threshold
	if (abs(coll->NearestLedgeDistance) > coll->Setup.Radius)
		return false;

	// Discard if ledge is not within angle threshold
	if (!TestValidLedgeAngle(item, coll))
		return false; 
	
	if (!ignoreHeadroom)
	{
		auto headroom = (coll->Front.Floor + coll->Setup.Height) - coll->Middle.Ceiling;
		if (headroom < STEP_SIZE)
			return false;
	}
	
	return (coll->CollisionType == CT_FRONT);
}

bool TestValidLedgeAngle(ITEM_INFO* item, COLL_INFO* coll)
{
	return abs((short) (coll->NearestLedgeAngle - item->pos.yRot)) <= LARA_GRAB_THRESHOLD;
}

bool TestLaraVault(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	if (!(TrInput & IN_ACTION) || info->gunStatus != LG_HANDS_FREE ||
		(TestLaraSwamp(item) && info->waterSurfaceDist < -(WALL_SIZE - STEP_SIZE)))
	{
		return false;
	}

	// TODO: LUA
	info->NewAnims.CrawlExtended = true;
	info->NewAnims.MonkeyAutoJump = false;

	if (TestValidLedge(item, coll))
	{
		bool success = false;

		// Vault to crouch up one step.
		if (coll->Front.Floor < 0 &&					// Lower floor bound.
			coll->Front.Floor > -STEPUP_HEIGHT &&		// Upper floor bound.
			info->NewAnims.CrawlExtended)
		{
			if (abs((coll->Front.Ceiling - coll->Setup.Height) - coll->Front.Floor) > LARA_HEIGHT_CRAWL)		// Front clamp buffer. Presumably, nothing more is necessary, but tend to this in the future. @Sezz 2021.11.06
			{
				item->animNumber = LA_VAULT_TO_CROUCH_1CLICK;
				item->currentAnimState = LS_GRABBING;
				item->frameNumber = GetFrameNumber(item, 0);
				item->goalAnimState = LS_CROUCH_IDLE;
				item->pos.yPos += coll->Front.Floor + STEP_SIZE;
				info->gunStatus = LG_HANDS_BUSY;
				success = true;
			}
		}
		// Vault up two steps.
		else if (coll->Front.Floor <= -STEPUP_HEIGHT &&					// Lower floor bound.
			coll->Front.Floor >= -(STEP_SIZE * 2 + STEP_SIZE / 2))		// Upper floor bound.
		{
			// Vault to stand up two steps.
			if (abs((coll->Front.Ceiling - coll->Setup.Height) - coll->Front.Floor) > LARA_HEIGHT/* &&				// Front clamp buffer. BUG: Turned away from the ledge and toward a section with a low ceiling, stand-to-crawl vault will be performed instead. @Sezz 2021.11.06
				abs((coll->FrontLeft.Ceiling - coll->Setup.Height) - coll->FrontLeft.Floor) > LARA_HEIGHT &&		// Left clamp buffer. // TODO: Ceilings don't push, so these are unnecessary for now. @Sezz 2021.11.06
				abs((coll->FrontRight.Ceiling - coll->Setup.Height) - coll->FrontRight.Floor) > LARA_HEIGHT*/)		// Right clamp buffer.
			{
				item->animNumber = LA_VAULT_TO_STAND_2CLICK_START;
				item->currentAnimState = LS_GRABBING;
				item->frameNumber = GetFrameNumber(item, 0);
				item->goalAnimState = LS_IDLE;
				item->pos.yPos += coll->Front.Floor + (STEP_SIZE * 2);
				info->gunStatus = LG_HANDS_BUSY;
				success = true;
			}
			// Vault to crouch up two steps.
			else if (abs((coll->Front.Ceiling - coll->Setup.Height) - coll->Front.Floor) > LARA_HEIGHT_CRAWL &&				// Front clamp buffer.
				abs((coll->FrontLeft.Ceiling - coll->Setup.Height) - coll->FrontLeft.Floor) > LARA_HEIGHT_CRAWL &&			// Left clamp buffer.
				abs((coll->FrontRight.Ceiling - coll->Setup.Height) - coll->FrontRight.Floor) > LARA_HEIGHT_CRAWL &&		// Right clamp buffer.
				info->NewAnims.CrawlExtended)
			{
				item->animNumber = LA_VAULT_TO_CROUCH_2CLICK;
				item->frameNumber = GetFrameNumber(item, 0);
				item->currentAnimState = LS_GRABBING;
				item->goalAnimState = LS_CROUCH_IDLE;
				item->pos.yPos += coll->Front.Floor + (STEP_SIZE * 2);
				info->gunStatus = LG_HANDS_BUSY;
				success = true;
			}
		}
		// Vault up three steps.
		else if (coll->Front.Floor <= -(STEP_SIZE * 2 + STEP_SIZE / 2) &&		// Lower floor bound.
			coll->Front.Floor >= -(WALL_SIZE - STEP_SIZE / 2))					// Upper floor bound.
		{
			// Vault to stand up three steps.
			if (abs((coll->Front.Ceiling - coll->Setup.Height) - coll->Front.Floor) > LARA_HEIGHT/* &&				// Front clamp buffer. BUG: Turned away from the ledge and toward a section with a low ceiling, stand-to-crawl vault will be performed instead. @Sezz 2021.11.06
				abs((coll->FrontLeft.Ceiling - coll->Setup.Height) - coll->FrontLeft.Floor) > LARA_HEIGHT &&		// Left clamp buffer. // TODO: Ceilings don't push, so these are unnecessary for now. @Sezz 2021.11.06
				abs((coll->FrontRight.Ceiling - coll->Setup.Height) - coll->FrontRight.Floor) > LARA_HEIGHT*/)		// Right clamp buffer.
			{
				item->animNumber = LA_VAULT_TO_STAND_3CLICK;
				item->currentAnimState = LS_GRABBING;
				item->frameNumber = GetFrameNumber(item, 0);
				item->goalAnimState = LS_IDLE;
				item->pos.yPos += coll->Front.Floor + (WALL_SIZE - STEP_SIZE);
				info->gunStatus = LG_HANDS_BUSY;
				success = true;
			}
			// Vault to crouch up three steps.
			else if (abs((coll->Front.Ceiling - coll->Setup.Height) - coll->Front.Floor) > LARA_HEIGHT_CRAWL &&				// Front clamp buffer.
				abs((coll->FrontLeft.Ceiling - coll->Setup.Height) - coll->FrontLeft.Floor) > LARA_HEIGHT_CRAWL &&			// Left clamp buffer.
				abs((coll->FrontRight.Ceiling - coll->Setup.Height) - coll->FrontRight.Floor) > LARA_HEIGHT_CRAWL &&		// Right clamp buffer.
				info->NewAnims.CrawlExtended)
			{
				item->animNumber = LA_VAULT_TO_CROUCH_3CLICK;
				item->frameNumber = GetFrameNumber(item, 0);
				item->currentAnimState = LS_GRABBING;
				item->goalAnimState = LS_CROUCH_IDLE;
				item->pos.yPos += coll->Front.Floor + (WALL_SIZE - STEP_SIZE);
				info->gunStatus = LG_HANDS_BUSY;
				success = true;
			}
		}
		// Auto jump.
		else if (coll->Front.Floor >= -(WALL_SIZE * 2 - STEP_SIZE / 2) &&		// Upper floor bound.
			coll->Front.Floor <= -(WALL_SIZE - STEP_SIZE / 2) &&				// Lower floor bound.
			!TestLaraSwamp(item))
		{
			item->animNumber = LA_STAND_SOLID;
			item->frameNumber = GetFrameNumber(item, 0);
			item->goalAnimState = LS_JUMP_UP;
			item->currentAnimState = LS_IDLE;
			info->calcFallSpeed = -3 - sqrt(-9600 - 12 * coll->Front.Floor);
			AnimateLara(item);
			success = true;
		}

		if (success)
		{
			SnapItemToLedge(item, coll, 0.2f);
			return true;
		}
	}

	// Begin ladder climb.
	if (info->climbStatus)
	{
		if (coll->Front.Floor > -(WALL_SIZE * 2 - STEP_SIZE / 2) ||			// Upper front floor bound.
			coll->FrontLeft.Floor > -(WALL_SIZE * 2 - STEP_SIZE / 2) ||		// Upper left floor bound.
			coll->FrontRight.Floor > -(STEP_SIZE * 2) ||					// Upper right floor bound.
			coll->Middle.Ceiling > -(WALL_SIZE + STEP_SIZE / 2 + 6) ||		// Upper ceiling bound.
			info->waterStatus == LW_WADE)
		{
			if ((coll->Front.Floor < -WALL_SIZE || coll->Front.Ceiling >= (STEP_SIZE * 2 - 6)) &&
				coll->Middle.Ceiling <= -(STEP_SIZE * 2 + 6))
			{
				if (TestLaraClimbStance(item, coll))
				{
					item->animNumber = LA_STAND_SOLID;
					item->frameNumber = GetFrameNumber(item, 0);
					item->goalAnimState = LS_LADDER_IDLE;
					item->currentAnimState = LS_IDLE;
					info->gunStatus = LG_HANDS_BUSY;
					info->turnRate = 0;

					ShiftItem(item, coll);
					SnapItemToGrid(item, coll); // HACK: until fragile ladder code is refactored, we must exactly snap to grid.
					AnimateLara(item);

					return true;
				}
			}

			return false;
		}

		// Auto jump to ladder. TODO: Check swamps.
		item->animNumber = LA_STAND_SOLID;
		item->frameNumber = GetFrameNumber(item, 0);
		item->goalAnimState = LS_JUMP_UP;
		item->currentAnimState = LS_IDLE;
		info->calcFallSpeed = -116;
		info->turnRate = 0;
		
		ShiftItem(item, coll);
		SnapItemToGrid(item, coll); // HACK: until fragile ladder code is refactored, we must exactly snap to grid.
		AnimateLara(item);

		return true;
	}

	// Auto jump to monkey swing.
	if (info->canMonkeySwing &&
		!TestLaraSwamp(item) &&
		info->NewAnims.MonkeyAutoJump)
	{
		short roomNum = item->roomNumber;
		int ceiling = (GetCeiling(GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNum),
			item->pos.xPos, item->pos.yPos, item->pos.zPos))-(item->pos.yPos);

		if (ceiling > (WALL_SIZE * 2 - STEP_SIZE) ||
			ceiling < -(WALL_SIZE * 2 - STEP_SIZE) ||
			abs(ceiling) == (WALL_SIZE - STEP_SIZE))
		{
			return false;
		}

		item->animNumber = LA_STAND_IDLE;
		item->frameNumber = GetFrameNumber(item, 0);
		item->goalAnimState = LS_JUMP_UP;
		item->currentAnimState = LS_TEST_1;

		return true;
	}

	return false;
}

bool TestLaraKeepCrouched(ITEM_INFO* item, COLL_INFO* coll)
{
	// TODO: Temporary. coll->Setup.Radius is currently only set to
	// LARA_RAD_CRAWL in the collision function, then reset by LaraAboveWater().
	// For tests called in control functions, then, it will store the wrong radius. @Sezz 2021.11.05
	auto radius = (item->currentAnimState == LS_CROUCH_IDLE ||
		item->currentAnimState == LS_CROUCH_TURN_LEFT ||
		item->currentAnimState == LS_CROUCH_TURN_RIGHT)
		? LARA_RAD : LARA_RAD_CRAWL;

	auto y = item->pos.yPos;
	auto probeBack = GetCollisionResult(item, coll->Setup.ForwardAngle + ANGLE(180.0f), radius, 0);

	// TODO: Cannot use as a failsafe in standing states; bugged with slanted ceilings reaching the ground.
	// In common setups, Lara may embed on such ceilings, resulting in inappropriate crouch state dispatches.
	// A buffer might help, but improved collision handling would presumably eliminate this issue entirely. @Sezz 2021.10.15
	if ((coll->Middle.Ceiling - LARA_HEIGHT_CRAWL) >= -LARA_HEIGHT ||		// Middle is not a clamp.
		(coll->Front.Ceiling - LARA_HEIGHT_CRAWL) >= -LARA_HEIGHT ||		// Front is not a clamp.
		(probeBack.Position.Ceiling - y) >= -LARA_HEIGHT)					// Back is not a clamp.
	{
		return true;
	}

	return false;
}

bool TestLaraSlide(ITEM_INFO* item, COLL_INFO* coll)
{
	if (TestLaraSwamp(item))
		return false;

	static short oldAngle = 1;

	if (abs(coll->TiltX) <= 2 && abs(coll->TiltZ) <= 2)
		return false;

	short angle = ANGLE(0.0f);
	if (coll->TiltX > 2)
		angle = -ANGLE(90.0f);
	else if (coll->TiltX < -2)
		angle = ANGLE(90.0f);

	if (coll->TiltZ > 2 && coll->TiltZ > abs(coll->TiltX))
		angle = ANGLE(180.0f);
	else if (coll->TiltZ < -2 && -coll->TiltZ > abs(coll->TiltX))
		angle = ANGLE(0.0f);

	short delta = angle - item->pos.yRot;

	ShiftItem(item, coll);

	if (delta < -ANGLE(90.0f) || delta > ANGLE(90.0f))
	{
		if (item->currentAnimState == LS_SLIDE_BACK && oldAngle == angle)
			return true;

		SetAnimation(item, LA_SLIDE_BACK_START);
		item->pos.yRot = angle + ANGLE(180.0f);
	}
	else
	{
		if (item->currentAnimState == LS_SLIDE_FORWARD && oldAngle == angle)
			return true;

		SetAnimation(item, LA_SLIDE_FORWARD);
		item->pos.yRot = angle;
	}

	Lara.moveAngle = angle;
	oldAngle = angle;

	return true;
}

bool TestLaraSwamp(ITEM_INFO* item)
{
	return (g_Level.Rooms[item->roomNumber].flags & ENV_FLAG_SWAMP);
}

bool TestLaraWater(ITEM_INFO* item)
{
	return (g_Level.Rooms[item->roomNumber].flags & ENV_FLAG_WATER);
}

SPLAT_COLL TestLaraWall(ITEM_INFO* item, int front, int right, int down)
{
	int x = item->pos.xPos;
	int y = item->pos.yPos + down;
	int z = item->pos.zPos;

	short angle = GetQuadrant(item->pos.yRot);
	short roomNum = item->roomNumber;

	FLOOR_INFO* floor;
	int h, c;

	switch (angle)
	{
	case NORTH:
		x -= right;
		break;
	case EAST:
		z -= right;
		break;
	case SOUTH:
		x += right;
		break;
	case WEST:
		z += right;
		break;
	default:
		break;
	}

	GetFloor(x, y, z, &roomNum);

	switch (angle)
	{
	case NORTH:
		z += front;
		break;
	case EAST:
		x += front;
		break;
	case SOUTH:
		z -= front;
		break;
	case WEST:
		x -= front;
		break;
	default:
		break;
	}

	floor = GetFloor(x, y, z, &roomNum);
	h = GetFloorHeight(floor, x, y, z);
	c = GetCeiling(floor, x, y, z);

	if (h == NO_HEIGHT)
		return SPLAT_COLL::WALL;

	if (y >= h || y <= c)
		return SPLAT_COLL::STEP;

	return SPLAT_COLL::NONE;
}

bool TestLaraHangJumpUp(ITEM_INFO* item, COLL_INFO* coll)
{
	if (!(TrInput & IN_ACTION) || (Lara.gunStatus != LG_HANDS_FREE) || (coll->HitStatic))
		return false;

	if (Lara.canMonkeySwing && coll->CollisionType == CT_TOP)
	{
		SetAnimation(item, LA_JUMP_UP_TO_MONKEYSWING);
		item->gravityStatus = false;
		item->speed = 0;
		item->fallspeed = 0;

		Lara.gunStatus = LG_HANDS_BUSY;

		MonkeySwingSnap(item, coll);

		return true;
	}

	if ((coll->CollisionType != CT_FRONT) || (coll->Middle.Ceiling > -STEPUP_HEIGHT))
		return false;

	int edge;
	auto edgeCatch = TestLaraEdgeCatch(item, coll, &edge);
	if (!edgeCatch)
		return false;

	bool ladder = TestLaraHangOnClimbWall(item, coll);

	if (!(ladder && edgeCatch) &&
		!(TestValidLedge(item, coll, true, true) && edgeCatch > 0))
		return false;

	auto angle = item->pos.yRot;

	if (TestHangSwingIn(item, angle))
	{
		SetAnimation(item, LA_JUMP_UP_TO_MONKEYSWING);
	}
	else
	{
		SetAnimation(item, LA_REACH_TO_HANG, 12);

		if (TestHangFeet(item, angle))
			item->goalAnimState = LS_HANG_FEET;
	}

	auto bounds = GetBoundsAccurate(item);

	if (edgeCatch <= 0)
		item->pos.yPos = edge - bounds->Y1 + 4;
	else
		item->pos.yPos += coll->Front.Floor - bounds->Y1;

	if (ladder)
		SnapItemToGrid(item, coll); // HACK: until fragile ladder code is refactored, we must exactly snap to grid.
	else
		SnapItemToLedge(item, coll);

	item->gravityStatus = false;
	item->speed = 0;
	item->fallspeed = 0;

	Lara.gunStatus = LG_HANDS_BUSY;
	Lara.torsoYrot = 0;
	Lara.torsoXrot = 0;

	return true;
}

bool TestLaraHangJump(ITEM_INFO* item, COLL_INFO* coll)
{
	if (!(TrInput & IN_ACTION) || (Lara.gunStatus != LG_HANDS_FREE) || (coll->HitStatic))
		return false;

	if (Lara.canMonkeySwing && coll->CollisionType == CT_TOP)
	{
		Lara.headYrot  = 0;
		Lara.headXrot  = 0;
		Lara.torsoYrot = 0;
		Lara.torsoXrot = 0;
		Lara.gunStatus = LG_HANDS_BUSY;

		SetAnimation(item, LA_REACH_TO_MONKEYSWING);
		item->gravityStatus = false;
		item->speed = 0;
		item->fallspeed = 0;

		return true;
	}

	if ((coll->Middle.Ceiling > -STEPUP_HEIGHT) ||
		(coll->Middle.Floor < 200) ||
		(coll->CollisionType != CT_FRONT))
		return false;

	int edge;
	auto edgeCatch = TestLaraEdgeCatch(item, coll, &edge);
	if (!edgeCatch)
		return false;

	bool ladder = TestLaraHangOnClimbWall(item, coll);

	if (!(ladder && edgeCatch) &&
		!(TestValidLedge(item, coll, true, true) && edgeCatch > 0))
		return false;

	auto angle = item->pos.yRot;

	Lara.NewAnims.OscillateHanging = true;

	if (TestHangSwingIn(item, angle))
	{
		if (Lara.NewAnims.OscillateHanging)
		{
			Lara.headYrot = 0;
			Lara.headXrot = 0;
			Lara.torsoYrot = 0;
			Lara.torsoXrot = 0;
			SetAnimation(item, LA_REACH_TO_HANG_OSCILLATE);
		}
		else
		{
			Lara.headYrot = 0;
			Lara.headXrot = 0;
			Lara.torsoYrot = 0;
			Lara.torsoXrot = 0;
			SetAnimation(item, LA_REACH_TO_MONKEYSWING);
		}
	}
	else
	{
		SetAnimation(item, LA_REACH_TO_HANG);

		if (TestHangFeet(item, angle))
			item->goalAnimState = LS_HANG_FEET;
	}

	auto bounds = GetBoundsAccurate(item);

	if (edgeCatch <= 0)
	{
		item->pos.yPos = edge - bounds->Y1 - 20;
		item->pos.yRot = coll->NearestLedgeAngle;
	}
	else
		item->pos.yPos += coll->Front.Floor - bounds->Y1 - 20;

	if (ladder)
		SnapItemToGrid(item, coll); // HACK: until fragile ladder code is refactored, we must exactly snap to grid.
	else
		SnapItemToLedge(item, coll, 0.2f);

	item->gravityStatus = true;
	item->speed = 2;
	item->fallspeed = 1;

	Lara.gunStatus = LG_HANDS_BUSY;

	return true;
}

bool TestLaraHang(ITEM_INFO* item, COLL_INFO* coll)
{
	auto delta = 0;
	auto flag = false;
	auto angle = Lara.moveAngle;

	if (Lara.moveAngle == (short) (item->pos.yRot - ANGLE(90.0f)))
		delta = -coll->Setup.Radius;
	else if (Lara.moveAngle == (short) (item->pos.yRot + ANGLE(90.0f)))
		delta = coll->Setup.Radius;

	auto s = phd_sin(Lara.moveAngle);
	auto c = phd_cos(Lara.moveAngle);
	auto testShift = Vector2(s * delta, c * delta);

	auto hdif = LaraFloorFront(item, Lara.moveAngle, coll->Setup.Radius);
	if (hdif < 200)
		flag = true;

	auto cdif = LaraCeilingFront(item, Lara.moveAngle, coll->Setup.Radius, 0);
	auto dir = GetQuadrant(item->pos.yRot);

	// When Lara is about to move, use larger embed offset for stabilizing diagonal shimmying)
	auto embedOffset = 4;
	if (TrInput & (IN_LEFT | IN_RIGHT))
		embedOffset = 16;

	item->pos.xPos += phd_sin(item->pos.yRot) * embedOffset;
	item->pos.zPos += phd_cos(item->pos.yRot) * embedOffset;

	Lara.moveAngle = item->pos.yRot;
	coll->Setup.BadHeightDown = NO_BAD_POS;
	coll->Setup.BadHeightUp = -STEPUP_HEIGHT;
	coll->Setup.BadCeilingHeight = 0;
	coll->Setup.ForwardAngle = Lara.moveAngle;

	GetCollisionInfo(coll, item);

	bool result = false;

	if (Lara.climbStatus)
	{
		if (TrInput & IN_ACTION &&
			item->hitPoints > 0)
		{
			Lara.moveAngle = angle;

			if (!TestLaraHangOnClimbWall(item, coll))
			{
				if (item->animNumber != LA_LADDER_TO_HANG_RIGHT &&
					item->animNumber != LA_LADDER_TO_HANG_LEFT)
				{
					LaraSnapToEdgeOfBlock(item, coll, dir);
					item->pos.yPos = coll->Setup.OldPosition.y;
					SetAnimation(item, LA_REACH_TO_HANG, 21);
				}

				result = true;
			}
			else
			{
				if (item->animNumber == LA_REACH_TO_HANG &&
					item->frameNumber == GetFrameNumber(item, 21) &&
					TestLaraClimbStance(item, coll))
				{
					item->goalAnimState = LS_LADDER_IDLE;
				}
			}
		}
		else
		{
			SetAnimation(item, LA_FALL_START);
			item->pos.yPos += 256;
			item->gravityStatus = true;
			item->speed = 2;
			item->fallspeed = 1;
			Lara.gunStatus = LG_HANDS_FREE;
		}
	}
	else
	{
		if (TrInput & IN_ACTION &&
			item->hitPoints > 0 &&
			coll->Front.Floor <= 0)
		{
			if (flag && hdif > 0 && delta > 0 == coll->MiddleLeft.Floor > coll->MiddleRight.Floor)
				flag = false;

			auto front = coll->Front.Floor;
			auto dfront = coll->Front.Floor - GetBoundsAccurate(item)->Y1;
			auto flag2 = false;
			auto x = item->pos.xPos;
			auto z = item->pos.zPos;

			if (delta != 0)
			{
				x += testShift.x;
				z += testShift.y;
			}

			Lara.moveAngle = angle;

			if (256 << dir & GetClimbFlags(x, item->pos.yPos, z, item->roomNumber))
			{
				if (!TestLaraHangOnClimbWall(item, coll))
					dfront = 0;
			}
			else if (!TestValidLedge(item, coll, true))
			{
				if (delta < 0 && coll->FrontLeft.Floor != coll->Front.Floor || delta > 0 && coll->FrontRight.Floor != coll->Front.Floor)
					flag2 = true;
			}

			coll->Front.Floor = front;

			if (!flag2 && 
				coll->Middle.Ceiling < 0 && 
				coll->CollisionType == CT_FRONT && 
				!flag && 
				!coll->HitStatic && 
				cdif <= -950 && 
				abs(dfront) < SLOPE_DIFFERENCE &&
				TestValidLedgeAngle(item, coll))
			{
				if (item->speed != 0)
					SnapItemToLedge(item, coll);

				item->pos.yPos += dfront;
			}
			else
			{
				item->pos.xPos = coll->Setup.OldPosition.x;
				item->pos.yPos = coll->Setup.OldPosition.y;
				item->pos.zPos = coll->Setup.OldPosition.z;

				if (item->currentAnimState == LS_SHIMMY_LEFT ||
					item->currentAnimState == LS_SHIMMY_RIGHT)
				{
					SetAnimation(item, LA_REACH_TO_HANG, 21);
				}
				else if (item->currentAnimState == LS_SHIMMY_FEET_LEFT ||
					item->currentAnimState == LS_SHIMMY_FEET_RIGHT)
				{
					SetAnimation(item, LA_HANG_FEET_IDLE);
				}

				result = true;
			}
		}
		else
		{
			SetAnimation(item, LA_JUMP_UP, 9);
			item->pos.xPos += coll->Shift.x;
			item->pos.yPos += GetBoundsAccurate(item)->Y2 * 2.4f;
			item->pos.zPos += coll->Shift.z;
			item->gravityStatus = true;
			item->speed = 2;
			item->fallspeed = 1;
			Lara.gunStatus = LG_HANDS_FREE;
		}
	}

	return result;
}

CORNER_RESULT TestLaraHangCorner(ITEM_INFO* item, COLL_INFO* coll, float testAngle)
{
	// Lara isn't in stop state yet, bypass test
	if (item->animNumber != LA_REACH_TO_HANG && item->animNumber != LA_HANG_FEET_IDLE)
		return CORNER_RESULT::NONE;

	// Static is in the way, bypass test
	if (coll->HitStatic)
		return CORNER_RESULT::NONE;

	// INNER CORNER TESTS

	// Backup old Lara position and frontal collision
	auto oldPos = item->pos;
	int oldFrontFloor = coll->Front.Floor;

	// Quadrant is only used for ladder checks
	auto quadrant = GetQuadrant(item->pos.yRot);

	// Get bounding box height for further ledge height calculations
	auto bounds = GetBoundsAccurate(item);

	// Virtually rotate Lara 90 degrees to the right and snap to nearest ledge, if any.
	short newAngle = item->pos.yRot + ANGLE(testAngle);
	item->pos.yRot = newAngle;
	SnapItemToLedge(item, coll, item->pos.yRot);

	// Do further testing only if test angle is equal to resulting edge angle
	if (newAngle == item->pos.yRot)
	{
		// Push Lara further to the right to avoid false floor hits on the left side
		auto c = phd_cos(item->pos.yRot + ANGLE(testAngle));
		auto s = phd_sin(item->pos.yRot + ANGLE(testAngle));
		item->pos.xPos += s * coll->Setup.Radius / 2;
		item->pos.zPos += c * coll->Setup.Radius / 2;

		// Store next position
		Lara.nextCornerPos.x = item->pos.xPos;
		Lara.nextCornerPos.y = LaraCollisionAboveFront(item, item->pos.yRot, coll->Setup.Radius * 2, abs(bounds->Y1)).Position.Floor + abs(bounds->Y1);
		Lara.nextCornerPos.z = item->pos.zPos;

		auto result = TestLaraValidHangPos(item, coll);

		if (result)
		{
			if (abs(oldFrontFloor - coll->Front.Floor) <= SLOPE_DIFFERENCE)
			{
				// Restore original item positions
				item->pos = oldPos;
				Lara.moveAngle = oldPos.yRot;

				return CORNER_RESULT::INNER;
			}
		}

		if (Lara.climbStatus)
		{
			auto angleSet = testAngle > 0 ? LeftExtRightIntTab : LeftIntRightExtTab;
			if (GetClimbFlags(item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber) & (short)angleSet[quadrant])
			{
				// Restore original item positions
				item->pos = oldPos;
				Lara.moveAngle = oldPos.yRot;

				return CORNER_RESULT::INNER;
			}
		}
	}

	// Restore original item positions
	item->pos = oldPos;
	Lara.moveAngle = oldPos.yRot;

	// OUTER CORNER TESTS

	// Test if there's a material obstacles blocking outer corner pathway
	if (LaraFloorFront(item, item->pos.yRot + ANGLE(testAngle), coll->Setup.Radius + STEP_SIZE) < 0)
		return CORNER_RESULT::NONE;
	if (LaraCeilingFront(item, item->pos.yRot + ANGLE(testAngle), coll->Setup.Radius + STEP_SIZE, coll->Setup.Height) > 0)
		return CORNER_RESULT::NONE;

	// Last chance for possible diagonal vs. non-diagonal cases: ray test
	if (!LaraPositionOnLOS(item, item->pos.yRot + ANGLE(testAngle), coll->Setup.Radius + STEP_SIZE))
		return CORNER_RESULT::NONE;

	// Push Lara diagonally to other side of corner at distance of 1/2 wall size
	auto c = phd_cos(item->pos.yRot + ANGLE(testAngle / 2));
	auto s = phd_sin(item->pos.yRot + ANGLE(testAngle / 2));
	item->pos.xPos += s * WALL_SIZE / 3;
	item->pos.zPos += c * WALL_SIZE / 3;

	// Virtually rotate Lara 90 degrees to the left and snap to nearest ledge, if any.
	newAngle = item->pos.yRot - ANGLE(testAngle);
	item->pos.yRot = newAngle;
	Lara.moveAngle = item->pos.yRot;
	SnapItemToLedge(item, coll, item->pos.yRot);

	// Do further testing only if test angle is equal to resulting edge angle
	if (newAngle == item->pos.yRot)
	{
		// Store next position
		Lara.nextCornerPos.x = item->pos.xPos;
		Lara.nextCornerPos.y = LaraCollisionAboveFront(item, item->pos.yRot, coll->Setup.Radius * 2, abs(bounds->Y1)).Position.Floor + abs(bounds->Y1);
		Lara.nextCornerPos.z = item->pos.zPos;

		if (TestLaraValidHangPos(item, coll))
		{
			if (abs(oldFrontFloor - coll->Front.Floor) <= SLOPE_DIFFERENCE)
			{
				// Restore original item positions
				item->pos = oldPos;
				Lara.moveAngle = oldPos.yRot;

				return CORNER_RESULT::OUTER;
			}
		}

		if (Lara.climbStatus)
		{
			auto angleSet = testAngle > 0 ? LeftIntRightExtTab : LeftExtRightIntTab;
			if (GetClimbFlags(item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber) & (short)angleSet[quadrant])
			{
				// Restore original item positions
				item->pos = oldPos;
				Lara.moveAngle = oldPos.yRot;

				return CORNER_RESULT::OUTER;
			}
		}
	}

	// Restore original item positions
	item->pos = oldPos;
	Lara.moveAngle = oldPos.yRot;

	return CORNER_RESULT::NONE;
}

bool TestLaraValidHangPos(ITEM_INFO* item, COLL_INFO* coll)
{
	// Get incoming ledge height and own Lara's upper bound.
	// First one will be negative while first one is positive.
	// Difference between two indicates difference in height between ledges.
	auto frontFloor = LaraCollisionAboveFront(item, Lara.moveAngle, coll->Setup.Radius + STEP_SIZE / 2, LARA_HEIGHT).Position.Floor;
	auto laraUpperBound = item->pos.yPos - coll->Setup.Height;

	// If difference is above 1/2 click, return false (ledge is out of reach).
	if (abs(frontFloor - laraUpperBound) > STEP_SIZE / 2)
 		return false;

	// Embed Lara into wall to make collision test succeed
	item->pos.xPos += phd_sin(item->pos.yRot) * 8;
	item->pos.zPos += phd_cos(item->pos.yRot) * 8;

	// Setup new GCI call
	Lara.moveAngle = item->pos.yRot;
	coll->Setup.BadHeightDown = NO_BAD_POS;
	coll->Setup.BadHeightUp = -512;
	coll->Setup.BadCeilingHeight = 0;
	coll->Setup.Mode = COLL_PROBE_MODE::FREE_FLAT;
	coll->Setup.ForwardAngle = Lara.moveAngle;

	GetCollisionInfo(coll, item);

	// Filter out narrow ceiling spaces, no collision cases and statics in front.
	if (coll->Middle.Ceiling >= 0 || coll->CollisionType != CT_FRONT || coll->HitStatic)
		return false;

	// Finally, do ordinary ledge checks (slope difference etc.)
	return TestValidLedge(item, coll);
}

bool TestLaraClimbStance(ITEM_INFO* item, COLL_INFO* coll)
{
	int shift_r, shift_l;

	if (LaraTestClimbPos(item, coll->Setup.Radius, coll->Setup.Radius + 120, -700, (STEP_SIZE * 2), &shift_r) != 1)
		return false;

	if (LaraTestClimbPos(item, coll->Setup.Radius, -(coll->Setup.Radius + 120), -700, (STEP_SIZE * 2), &shift_l) != 1)
		return false;

	if (shift_r)
	{
		if (shift_l)
		{
			if (shift_r < 0 != shift_l < 0)
				return false;

			if ((shift_r < 0 && shift_l < shift_r) ||
				(shift_r > 0 && shift_l > shift_r))
			{
				item->pos.yPos += shift_l;
				return true;
			}
		}

		item->pos.yPos += shift_r;
	}
	else if (shift_l)
	{
		item->pos.yPos += shift_l;
	}

	return true;
}

bool TestLaraHangOnClimbWall(ITEM_INFO* item, COLL_INFO* coll)
{
	int shift, result;

	if (Lara.climbStatus == 0)
		return false;

	if (item->fallspeed < 0)
		return false;
	   
	// HACK: Climb wall tests are highly fragile and depend on quadrant shifts.
	// Until climb wall tests are fully refactored, we need to recalculate COLL_INFO.

	auto coll2 = *coll;
	coll2.Setup.Mode = COLL_PROBE_MODE::QUADRANTS;
	GetCollisionInfo(&coll2, item);

	switch (GetQuadrant(item->pos.yRot))
	{
	case NORTH:
	case SOUTH:
		item->pos.zPos += coll2.Shift.z;
		break;

	case EAST:
	case WEST:
		item->pos.xPos += coll2.Shift.x;
		break;

	default:
		break;
	}

	auto bounds = GetBoundsAccurate(item);

	if (Lara.moveAngle != item->pos.yRot)
	{
		short l = LaraCeilingFront(item, item->pos.yRot, 0, 0);
		short r = LaraCeilingFront(item, Lara.moveAngle, STEP_SIZE / 2, 0);

		if (abs(l - r) > SLOPE_DIFFERENCE)
			return false;
	}

	if (LaraTestClimbPos(item, LARA_RAD,  LARA_RAD, bounds->Y1, bounds->Y2 - bounds->Y1, &shift) &&
		LaraTestClimbPos(item, LARA_RAD, -LARA_RAD, bounds->Y1, bounds->Y2 - bounds->Y1, &shift))
	{
		result = LaraTestClimbPos(item, LARA_RAD, 0, bounds->Y1, bounds->Y2 - bounds->Y1, &shift);
		if (result)
		{
			if (result != 1)
				item->pos.yPos += shift;

			return true;
		}
	}

	return false;
}

int TestLaraEdgeCatch(ITEM_INFO* item, COLL_INFO* coll, int* edge)
{
	BOUNDING_BOX* bounds = GetBoundsAccurate(item);
	int hdif = coll->Front.Floor - bounds->Y1;

	if (hdif < 0 == hdif + item->fallspeed < 0)
	{
		hdif = item->pos.yPos + bounds->Y1;

		if ((hdif + item->fallspeed & 0xFFFFFF00) != (hdif & 0xFFFFFF00))
		{
			if (item->fallspeed > 0)
				*edge = (hdif + item->fallspeed) & 0xFFFFFF00;
			else
				*edge = hdif & 0xFFFFFF00;

			return -1;
		}

		return 0;
	}

	if (!TestValidLedge(item, coll, true))
		return 0;

	return 1;
}

bool TestHangSwingIn(ITEM_INFO* item, short angle)
{
	int x = item->pos.xPos;
	int y = item->pos.yPos;
	int z = item->pos.zPos;
	short roomNum = item->roomNumber;
	FLOOR_INFO* floor;
	int floorHeight, ceilingHeight;

	Lara.NewAnims.OscillateHanging = true;

	z += phd_cos(angle) * (STEP_SIZE / 2);
	x += phd_sin(angle) * (STEP_SIZE / 2);

	floor = GetFloor(x, y, z, &roomNum);
	floorHeight = GetFloorHeight(floor, x, y, z);
	ceilingHeight = GetCeiling(floor, x, y, z);

	if (floorHeight != NO_HEIGHT)
	{
		if (Lara.NewAnims.OscillateHanging)
		{
			if (floorHeight - y > 0 && ceilingHeight - y < -400)
				return true;
		}
		else
		{
			if (floorHeight - y > 0 && ceilingHeight - y < -400 && (y - 819 - ceilingHeight > -72))
				return true;
		}
	}

	return false;
}

bool TestHangFeet(ITEM_INFO* item, short angle)
{
	//##LUA debug etc.
	Lara.NewAnims.FeetHang = 0;

	if (Lara.climbStatus || !Lara.NewAnims.FeetHang)
		return false;

	int x = item->pos.xPos;
	int y = item->pos.yPos;
	int z = item->pos.zPos;
	short roomNum = item->roomNumber;

	z += phd_cos(angle) * (STEP_SIZE / 2);
	x += phd_sin(angle) * (STEP_SIZE / 2);

	auto floor = GetFloor(x, y, z, &roomNum);
	int floorHeight = GetFloorHeight(floor, x, y, z);
	int ceilingHeight = GetCeiling(floor, x, y, z);
	int m = ceilingHeight - y;
	int j = y - (STEP_SIZE / 2) - ceilingHeight;

	if (floorHeight != NO_HEIGHT)
	{
		if (floorHeight < y && m < -(STEP_SIZE / 2) && j > -(STEP_SIZE / 4 + STEP_SIZE / 32))
			return true;
	}

	return false;
}

bool TestLaraHangSideways(ITEM_INFO* item, COLL_INFO* coll, short angle)
{
	int oldx = item->pos.xPos;
	int oldz = item->pos.zPos;
	int x = item->pos.xPos;
	int z = item->pos.zPos;

	Lara.moveAngle = item->pos.yRot + angle;
	
	z += phd_cos(Lara.moveAngle) * 16;
	x += phd_sin(Lara.moveAngle) * 16;

	item->pos.xPos = x;
	item->pos.zPos = z;

	coll->Setup.OldPosition.y = item->pos.yPos;

	auto res = TestLaraHang(item, coll);

	item->pos.xPos = oldx;
	item->pos.zPos = oldz;

	Lara.moveAngle = item->pos.yRot + angle;

	return !res;
}

void SetCornerAnim(ITEM_INFO* item, COLL_INFO* coll, short rot, short flip)
{
	if (item->hitPoints <= 0)
	{
		SetAnimation(item, LA_FALL_START);

		item->gravityStatus = true;
		item->speed = 2;
		item->pos.yPos += STEP_SIZE;
		item->fallspeed = 1;

		Lara.gunStatus = LG_HANDS_FREE;

		item->pos.yRot += rot / 2;
	}
	else if (flip)
	{
		if (Lara.isClimbing)
		{
			SetAnimation(item, LA_LADDER_IDLE);
		}
		else
		{
			SetAnimation(item, LA_REACH_TO_HANG, 21);
		}

		coll->Setup.OldPosition.x = item->pos.xPos = Lara.nextCornerPos.x;
		coll->Setup.OldPosition.y = item->pos.yPos = Lara.nextCornerPos.y;
		coll->Setup.OldPosition.z = item->pos.zPos = Lara.nextCornerPos.z;
		item->pos.yRot += rot;
	}
}

void SetCornerAnimFeet(ITEM_INFO* item, COLL_INFO* coll, short rot, short flip)
{
	if (item->hitPoints <= 0)
	{
		SetAnimation(item, LA_FALL_START);

		item->gravityStatus = true;
		item->speed = 2;
		item->pos.yPos += STEP_SIZE;
		item->fallspeed = 1;

		Lara.gunStatus = LG_HANDS_FREE;

		item->pos.yRot += rot / 2;
	}
	else if (flip)
	{
		SetAnimation(item, LA_HANG_FEET_IDLE);

		coll->Setup.OldPosition.x = item->pos.xPos = Lara.nextCornerPos.x;
		coll->Setup.OldPosition.y = item->pos.yPos = Lara.nextCornerPos.y;
		coll->Setup.OldPosition.z = item->pos.zPos = Lara.nextCornerPos.z;
		item->pos.yRot += rot;
	}
}

bool TestLaraStandingJump(ITEM_INFO* item, COLL_INFO* coll, short angle)
{
	auto y = item->pos.yPos;
	auto probe = GetCollisionResult(item, angle, STEP_SIZE, coll->Setup.Height);

	if (!TestLaraFacingCorner(item, angle, STEP_SIZE) &&
		probe.Position.Floor - y >= -STEPUP_HEIGHT &&									// Highest floor bound.
		probe.Position.Ceiling - y < -(coll->Setup.Height + LARA_HEADROOM * 0.7f))		// Lowest ceiling bound.
	{
		return true;
	}

	return false;
}

bool TestLaraFacingCorner(ITEM_INFO* item, short angle, int dist)
{
	auto angle1 = angle + ANGLE(15.0f);
	auto angle2 = angle - ANGLE(15.0f);

	auto vec1 = GAME_VECTOR(item->pos.xPos + dist * phd_sin(angle1),
		item->pos.yPos - STEPUP_HEIGHT,
		item->pos.zPos + dist * phd_cos(angle1),
		item->roomNumber);

	auto vec2 = GAME_VECTOR(item->pos.xPos + dist * phd_sin(angle2),
		item->pos.yPos - STEPUP_HEIGHT,
		item->pos.zPos + dist * phd_cos(angle2),
		item->roomNumber);

	auto pos = GAME_VECTOR(item->pos.xPos,
		item->pos.yPos - STEPUP_HEIGHT,
		item->pos.zPos,
		item->roomNumber);

	auto result1 = LOS(&pos, &vec1);
	auto result2 = LOS(&pos, &vec2);

	return (result1 == 0 && result2 == 0);
}

bool LaraPositionOnLOS(ITEM_INFO* item, short ang, int dist)
{
	auto pos1 = GAME_VECTOR(item->pos.xPos,
						    item->pos.yPos - LARA_HEADROOM,
						    item->pos.zPos,
						    item->roomNumber);

	auto pos2 = GAME_VECTOR(item->pos.xPos,
						    item->pos.yPos - LARA_HEIGHT + LARA_HEADROOM,
						    item->pos.zPos,
						    item->roomNumber);
	
	auto vec1 = GAME_VECTOR(item->pos.xPos + dist * phd_sin(ang),
						    item->pos.yPos - LARA_HEADROOM,
						    item->pos.zPos + dist * phd_cos(ang),
						    item->roomNumber);

	auto vec2 = GAME_VECTOR(item->pos.xPos + dist * phd_sin(ang),
						    item->pos.yPos - LARA_HEIGHT + LARA_HEADROOM,
						    item->pos.zPos + dist * phd_cos(ang),
						    item->roomNumber);

	auto result1 = LOS(&pos1, &vec1);
	auto result2 = LOS(&pos2, &vec2);

	return (result1 != 0 && result2 != 0);
}

int LaraFloorFront(ITEM_INFO* item, short ang, int dist)
{
	return LaraCollisionFront(item, ang, dist).Position.Floor;
}

COLL_RESULT LaraCollisionFront(ITEM_INFO* item, short ang, int dist)
{
	auto probe = GetCollisionResult(item, ang, dist, -LARA_HEIGHT);

	if (probe.Position.Floor != NO_HEIGHT)
		probe.Position.Floor -= item->pos.yPos;

	return probe;
}

COLL_RESULT LaraCollisionAboveFront(ITEM_INFO* item, short ang, int dist, int h)
{
	int x = item->pos.xPos + dist * phd_sin(ang);
	int y = item->pos.yPos - h;
	int z = item->pos.zPos + dist * phd_cos(ang);

	return GetCollisionResult(x, y, z, GetCollisionResult(item->pos.xPos, y, item->pos.zPos, item->roomNumber).RoomNumber);
}

int LaraCeilingFront(ITEM_INFO* item, short ang, int dist, int h)
{
	return LaraCeilingCollisionFront(item, ang, dist, h).Position.Ceiling;
}

COLL_RESULT LaraCeilingCollisionFront(ITEM_INFO* item, short ang, int dist, int h)
{
	auto probe = GetCollisionResult(item, ang, dist, -h);

	if (probe.Position.Ceiling != NO_HEIGHT)
		probe.Position.Ceiling += h - item->pos.yPos;

	return probe;
}

bool TestLaraFall(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	if (coll->Middle.Floor <= STEPUP_HEIGHT ||
		info->waterStatus == LW_WADE)	// TODO: This causes a legacy floor snap bug when lara wades off a ledge into a dry room. @Sezz 2021.09.26
	{
		return false;
	}

	return true;
}

// TODO: Gradually replace calls.
bool LaraFallen(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	if (info->waterStatus == LW_WADE || coll->Middle.Floor <= STEPUP_HEIGHT)
		return false;

	SetAnimation(item, LA_FALL_START);
	item->fallspeed = 0;
	item->gravityStatus = true;

	return true;
}

bool LaraLandedBad(ITEM_INFO* item, COLL_INFO* coll)
{
	int landspeed = item->fallspeed - 140;

	if (landspeed > 0)
	{
		if (landspeed <= 14)
		{
			item->hitPoints -= 1000 * SQUARE(landspeed) / 196;

			return item->hitPoints <= 0;
		}
		else
		{
			item->hitPoints = -1;

			return true;
		}
	}

	return false;
}

bool TestLaraWaterStepOut(ITEM_INFO* item, COLL_INFO* coll)
{
	if (coll->CollisionType == CT_FRONT ||
		coll->Middle.Slope ||
		coll->Middle.Floor >= 0)
	{
		return false;
	}

	if (coll->Middle.Floor >= -(STEP_SIZE / 2))
	{
		SetAnimation(item, LA_STAND_IDLE);
	}
	else
	{
		SetAnimation(item, LA_ONWATER_TO_WADE_1CLICK);
		item->goalAnimState = LS_IDLE;
	}

	item->pos.yPos += coll->Middle.Floor + (STEP_SIZE * 2 + STEP_SIZE / 2 + STEP_SIZE / 4 - 9);

	UpdateItemRoom(item, -(STEPUP_HEIGHT - 3));

	item->pos.zRot = 0;
	item->pos.xRot = 0;
	item->gravityStatus = false;
	item->speed = 0;
	item->fallspeed = 0;
	Lara.waterStatus = LW_WADE;

	return true;
}

bool TestLaraWaterClimbOut(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	if (coll->CollisionType != CT_FRONT || !(TrInput & IN_ACTION))
		return false;

	// TODO: LUA
	info->NewAnims.CrawlExtended = true;

	if (info->gunStatus &&
		(info->gunStatus != LG_READY || info->gunType != WEAPON_FLARE))
	{
		return false;
	}

	if (coll->Middle.Ceiling > -STEPUP_HEIGHT)
		return false;

	int frontFloor = coll->Front.Floor + LARA_HEIGHT_SURFSWIM;
	if (frontFloor <= -(STEP_SIZE * 2) ||
		frontFloor > (STEP_SIZE + STEP_SIZE / 4 - 4))
	{
		return false;
	}

	if (!TestValidLedge(item, coll))
		return false;

	auto surface = LaraCollisionAboveFront(item, coll->Setup.ForwardAngle, (STEP_SIZE * 2), STEP_SIZE);
	auto headroom = surface.Position.Floor - surface.Position.Ceiling;

	if (frontFloor <= -STEP_SIZE)
	{
		if (headroom < LARA_HEIGHT)
		{
			if (info->NewAnims.CrawlExtended)
				SetAnimation(item, LA_ONWATER_TO_CROUCH_1CLICK);
			else
				return false;
		}
		else
			SetAnimation(item, LA_ONWATER_TO_STAND_1CLICK);
	}
	else if (frontFloor > (STEP_SIZE / 2))
	{
		if (headroom < LARA_HEIGHT)
		{
			if (info->NewAnims.CrawlExtended)
				SetAnimation(item, LA_ONWATER_TO_CROUCH_M1CLICK);
			else
				return false;
		}
		else
			SetAnimation(item, LA_ONWATER_TO_STAND_M1CLICK);
	}

	else
	{
		if (headroom < LARA_HEIGHT)
		{
			if (info->NewAnims.CrawlExtended)
				SetAnimation(item, LA_ONWATER_TO_CROUCH_0CLICK);
			else
				return false;
		}
		else
			SetAnimation(item, LA_ONWATER_TO_STAND_0CLICK);
	}

	UpdateItemRoom(item, -LARA_HEIGHT / 2);
	SnapItemToLedge(item, coll, 1.7f);

	item->pos.yPos += frontFloor - 5;
	item->currentAnimState = LS_ONWATER_EXIT;
	item->gravityStatus = false;
	item->speed = 0;
	item->fallspeed = 0;
	info->gunStatus = LG_HANDS_BUSY;
	info->waterStatus = LW_ABOVE_WATER;

	return true;
}

bool TestLaraLadderClimbOut(ITEM_INFO* item, COLL_INFO* coll) // NEW function for water to ladder move
{
	if (!(TrInput & IN_ACTION) ||
		!Lara.climbStatus ||
		coll->CollisionType != CT_FRONT)
	{
		return false;
	}

	if (Lara.gunStatus &&
		(Lara.gunStatus != LG_READY || Lara.gunType != WEAPON_FLARE))
	{
		return false;
	}

	if (!TestLaraClimbStance(item, coll))
		return false;

	short rot = item->pos.yRot;

	if (rot >= -ANGLE(35.0f) && rot <= ANGLE(35.0f))
		rot = 0;
	else if (rot >= ANGLE(55.0f) && rot <= ANGLE(125.0f))
		rot = ANGLE(90.0f);
	else if (rot >= ANGLE(145.0f) || rot <= -ANGLE(145.0f))
		rot = ANGLE(180.0f);
	else if (rot >= -ANGLE(125.0f) && rot <= -ANGLE(55.0f))
		rot = -ANGLE(90.0f);

	if (rot & 0x3FFF)
		return false;

	switch ((unsigned short)rot / ANGLE(90.0f))
	{
	case NORTH:
		item->pos.zPos = (item->pos.zPos | (WALL_SIZE - 1)) - LARA_RAD - 1;
		break;

	case EAST:
		item->pos.xPos = (item->pos.xPos | (WALL_SIZE - 1)) - LARA_RAD - 1;
		break;

	case SOUTH:
		item->pos.zPos = (item->pos.zPos & -WALL_SIZE) + LARA_RAD + 1;
		break;

	case WEST:
		item->pos.xPos = (item->pos.xPos & -WALL_SIZE) + LARA_RAD + 1;
		break;
	}

	SetAnimation(item, LA_ONWATER_IDLE);
	item->goalAnimState = LS_LADDER_IDLE;
	AnimateLara(item);

	item->pos.yRot = rot;
	item->pos.yPos -= 10;//otherwise she falls back into the water
	item->pos.zRot = 0;
	item->pos.xRot = 0;
	item->gravityStatus = false;
	item->speed = 0;
	item->fallspeed = 0;
	Lara.gunStatus = LG_HANDS_BUSY;
	Lara.waterStatus = LW_ABOVE_WATER;

	return true;
}

void TestLaraWaterDepth(ITEM_INFO* item, COLL_INFO* coll)
{
	short roomNumber = item->roomNumber;
	FLOOR_INFO* floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
	int waterDepth = GetWaterDepth(item->pos.xPos, item->pos.yPos, item->pos.zPos, roomNumber);

	if (waterDepth == NO_HEIGHT)
	{
		item->fallspeed = 0;
		item->pos.xPos = coll->Setup.OldPosition.x;
		item->pos.yPos = coll->Setup.OldPosition.y;
		item->pos.zPos = coll->Setup.OldPosition.z;
	}

	// Height check was at STEP_SIZE * 2 before but changed to this 
	// because now Lara surfaces on a head level, not mid-body level.

	if (waterDepth <= LARA_HEIGHT - LARA_HEADROOM / 2)
	{
		SetAnimation(item, LA_UNDERWATER_TO_STAND);
		item->goalAnimState = LS_IDLE;
		item->pos.zRot = 0;
		item->pos.xRot = 0;
		item->speed = 0;
		item->fallspeed = 0;
		item->gravityStatus = false;
		Lara.waterStatus = LW_WADE;
		item->pos.yPos = GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);
	}
}

#ifndef NEW_TIGHTROPE
void GetTighRopeFallOff(int regularity) {
	if (LaraItem->hitPoints <= 0 || LaraItem->hitStatus)
		SetAnimation(LaraItem, LA_TIGHTROPE_FALL_LEFT);

	if (!Lara.tightRopeFall && !(GetRandomControl() & regularity))
		Lara.tightRopeFall = 2 - ((GetRandomControl() & 0xF) != 0);
}
#endif

bool IsStandingWeapon(LARA_WEAPON_TYPE gunType)
{
	if (gunType == LARA_WEAPON_TYPE::WEAPON_SHOTGUN ||
		gunType == LARA_WEAPON_TYPE::WEAPON_HK ||
		gunType == LARA_WEAPON_TYPE::WEAPON_CROSSBOW ||
		gunType == LARA_WEAPON_TYPE::WEAPON_TORCH ||
		gunType == LARA_WEAPON_TYPE::WEAPON_GRENADE_LAUNCHER ||
		gunType == LARA_WEAPON_TYPE::WEAPON_HARPOON_GUN ||
		gunType == LARA_WEAPON_TYPE::WEAPON_ROCKET_LAUNCHER ||
		gunType == LARA_WEAPON_TYPE::WEAPON_SNOWMOBILE)
	{
		return true;
	}

	return false;
}

bool TestLaraPose(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	if (!TestLaraSwamp(item) &&
		info->gunStatus == LG_HANDS_FREE &&								// Hands are free.
		!(TrInput & (IN_FLARE | IN_DRAW)) &&							// Avoid unsightly concurrent actions.
		(info->gunType != WEAPON_FLARE || info->flareAge > 0) &&		// Flare is not being handled. TODO: Will she pose with weapons drawn?
		info->Vehicle == NO_ITEM)										// Not in a vehicle.
	{
		return true;
	}

	return false;
}

bool TestLaraStep(COLL_INFO* coll)
{
	if (abs(coll->Middle.Floor) > 0 &&
		//coll->Middle.Floor <= STEPUP_HEIGHT &&		// Lower floor bound. BUG: Wading in water over a pit, Lara will not descend.
		coll->Middle.Floor >= -STEPUP_HEIGHT &&			// Upper floor bound.
		coll->Middle.Floor != NO_HEIGHT)
	{
		return true;
	}

	return false;
}

bool TestLaraStepUp(ITEM_INFO* item, COLL_INFO* coll)
{
	if (coll->Middle.Floor < -(STEP_SIZE / 2) &&			// Lower floor bound.
		coll->Middle.Floor >= -STEPUP_HEIGHT &&				// Upper floor bound.
		coll->Middle.Floor != NO_HEIGHT &&
		item->currentAnimState != LS_WALK_BACK &&
		item->currentAnimState != LS_RUN_BACK &&
		item->currentAnimState != LS_CRAWL_IDLE &&			// Crawl step up handled differently.
		item->currentAnimState != LS_CRAWL_FORWARD)
	{
		return true;
	}

	return false;
}

bool TestLaraStepDown(ITEM_INFO* item, COLL_INFO* coll)
{
	if (coll->Middle.Floor > (STEP_SIZE / 2) &&			// Upper floor bound.
		coll->Middle.Floor <= STEPUP_HEIGHT &&			// Lower floor bound.
		coll->Middle.Floor != NO_HEIGHT &&
		item->currentAnimState != LS_RUN_FORWARD &&		// No step down anim exists for these states.
		item->currentAnimState != LS_SPRINT &&
		item->currentAnimState != LS_WADE_FORWARD &&
		item->currentAnimState != LS_RUN_BACK &&
		item->currentAnimState != LS_CRAWL_IDLE &&		// Crawl step down handled differently.
		item->currentAnimState != LS_CRAWL_FORWARD)
	{
		return true;
	}

	return false;
}

// TODO: This function should become obsolete with more accurate and accessible collision detection in the future.
// For now, it supercedes old probes and is used alongside COLL_INFO. @Sezz 2021.10.24
bool TestLaraMove(ITEM_INFO* item, COLL_INFO* coll, short angle, int lowerBound, int upperBound)
{
	// TODO: coll->Setup.Radius is currently only set to
	// LARA_RAD_CRAWL in the collision function, then reset by LaraAboveWater().
	// For tests called in crawl control functions, then, it will store the wrong radius.
	// Function below (TestLaraMoveCrawl()) is a clone to account for this. @Sezz 2021.11.05

	auto y = item->pos.yPos;
	auto probe = GetCollisionResult(item, angle, coll->Setup.Radius * sqrt(2) + 4, 0);	// Offset required to account for gap between Lara and the wall. Results in slight overshoot, but avoids oscillation.

	if ((probe.Position.Floor - y) <= lowerBound &&										// Lower floor bound.
		(probe.Position.Floor - y) >= upperBound &&										// Upper floor bound.
		(probe.Position.Ceiling - y) < -coll->Setup.Height &&							// Lowest ceiling bound.
		abs(probe.Position.Ceiling - probe.Position.Floor) > LARA_HEIGHT &&				// Space is not a clamp.
		(!probe.Position.Slope || !(TrInput & IN_WALK) || TestLaraSwamp(item))	 &&		// No slope. WALK input and swamp room checks are hacks required due to the limits of this approach.
		probe.Position.Floor != NO_HEIGHT)
	{
		return true;
	}

	return false;
}

bool TestLaraMoveCrawl(ITEM_INFO* item, COLL_INFO* coll, short angle, int lowerBound, int upperBound)
{
	auto y = item->pos.yPos;
	auto probe = GetCollisionResult(item, angle, LARA_RAD_CRAWL * sqrt(2) + 4, 0);

	if ((probe.Position.Floor - y) <= lowerBound &&										// Lower floor bound.
		(probe.Position.Floor - y) >= upperBound &&										// Upper floor bound.
		(probe.Position.Ceiling - y) < -LARA_HEIGHT_CRAWL &&							// Lowest ceiling bound.
		abs(probe.Position.Ceiling - probe.Position.Floor) > LARA_HEIGHT_CRAWL &&		// Space is not a clamp.
		!probe.Position.Slope &&														// No slope.
		probe.Position.Floor != NO_HEIGHT)
	{
		return true;
	}

	return false;
}

bool TestLaraRunForward(ITEM_INFO* item, COLL_INFO* coll)
{
	auto y = item->pos.yPos - coll->Setup.Height;
	auto probe = GetCollisionResult(item, coll->Setup.ForwardAngle, coll->Setup.Radius * sqrt(2) + 4, 0);
	
	// BUG: This interferes with the one-step stand-to-crouch vault where the ceiling is lower than Lara's height.
	if ((probe.Position.Ceiling - y) < 0)		// Hack to ensure Lara can run off diagonal ledges. coll->Front.Floor often holds the wrong height because of quadrant-dependent wall pushing. @Sezz 2021.11.06
		return true;

	return false;

	// TODO: TestLaraMove() call not useful yet; it can block Lara from climbing. @Sezz 2021.10.22
	// Additionally, run forward test needs to incorporate a unique ceiling check (as above). @Sezz 2021.10.28
	//return TestLaraMove(item, coll, item->pos.yRot, STEPUP_HEIGHT, -STEPUP_HEIGHT);		// Using BadHeightUp/Down defined in walk and run state collision functions.
}

bool TestLaraWalkForward(ITEM_INFO* item, COLL_INFO* coll)
{
	if (coll->CollisionType != CT_FRONT &&
		coll->CollisionType != CT_TOP_FRONT)
	{
		return true;
	}

	return false;

	// TODO: Same issues as in TestLaraRunForward().
	//return TestLaraMove(item, coll, item->pos.yRot, STEPUP_HEIGHT, -STEPUP_HEIGHT);		// Using BadHeightUp/Down defined in walk and run state collision functions.
}

bool TestLaraWalkBack(ITEM_INFO* item, COLL_INFO* coll)
{
	return TestLaraMove(item, coll, item->pos.yRot + ANGLE(180.0f), STEPUP_HEIGHT, -STEPUP_HEIGHT);		// Using BadHeightUp/Down defined in walk back state collision function.
}

bool TestLaraHopBack(ITEM_INFO* item, COLL_INFO* coll)
{
	return TestLaraMove(item, coll, item->pos.yRot + ANGLE(180.0f), NO_BAD_POS, -STEPUP_HEIGHT);		// Using BadHeightUp/Down defined in hop back state collision function.
}

bool TestLaraStepLeft(ITEM_INFO* item, COLL_INFO* coll)
{
	return TestLaraMove(item, coll, item->pos.yRot - ANGLE(90.0f), STEP_SIZE / 2, -STEP_SIZE / 2);		// Using BadHeightUp/Down defined in step left state collision function.
}

bool TestLaraStepRight(ITEM_INFO* item, COLL_INFO* coll)
{
	return TestLaraMove(item, coll, item->pos.yRot + ANGLE(90.0f), STEP_SIZE / 2, -STEP_SIZE / 2);		// Using BadHeightUp/Down defined in step right state collision function.
}

bool TestLaraWalkBackSwamp(ITEM_INFO* item, COLL_INFO* coll)
{
	return TestLaraMove(item, coll, item->pos.yRot + ANGLE(180.0f), NO_BAD_POS, -STEPUP_HEIGHT);		// Using BadHeightUp defined in walk back state collision function.
}

bool TestLaraStepLeftSwamp(ITEM_INFO* item, COLL_INFO* coll)
{
	return TestLaraMove(item, coll, item->pos.yRot - ANGLE(90.0f), NO_BAD_POS, -STEP_SIZE / 2);			// Using BadHeightUp defined in step left state collision function.
}

bool TestLaraStepRightSwamp(ITEM_INFO* item, COLL_INFO* coll)
{
	return TestLaraMove(item, coll, item->pos.yRot + ANGLE(90.0f), NO_BAD_POS, -STEP_SIZE / 2);			// Using BadHeightUp defined in step right state collision function.
}

bool TestLaraCrawlForward(ITEM_INFO* item, COLL_INFO* coll)
{
	return TestLaraMoveCrawl(item, coll, item->pos.yRot, STEP_SIZE - 1, -(STEP_SIZE - 1));				// Using BadHeightUp/Down defined in crawl state collision functions.
}

bool TestLaraCrawlBack(ITEM_INFO* item, COLL_INFO* coll)
{
	return TestLaraMoveCrawl(item, coll, item->pos.yRot + ANGLE(180.0f), STEP_SIZE - 1, -(STEP_SIZE - 1));		// Using BadHeightUp/Down defined in crawl state collision functions.
}

bool TestLaraCrouchToCrawl(ITEM_INFO* item)
{
	LaraInfo*& info = item->data;

	if (info->gunStatus == LG_HANDS_FREE &&							// Hands are free.
		!(TrInput & (IN_FLARE | IN_DRAW)) &&						// Avoid unsightly concurrent actions.
		(info->gunType != WEAPON_FLARE || info->flareAge > 0))		// Not handling flare.
	{
		return true;
	}

	return false;
}

bool TestLaraCrouchRoll(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	auto y = item->pos.yPos;
	auto probe = GetCollisionResult(item, item->pos.yRot, WALL_SIZE, 0);

	if ((probe.Position.Floor - y) <= (STEP_SIZE - 1) &&			// Lower floor bound.
		(probe.Position.Floor - y) >= -(STEP_SIZE - 1) &&			// Upper floor bound.
		(probe.Position.Ceiling - y) < -LARA_HEIGHT_CRAWL &&		// Lowest ceiling bound.
		!probe.Position.Slope &&									// Not a slope.
		info->waterSurfaceDist >= -STEP_SIZE &&						// Water depth is optically feasible for action.
		!(TrInput & (IN_FLARE | IN_DRAW)) &&						// Avoid unsightly concurrent actions.
		(info->gunType != WEAPON_FLARE || info->flareAge > 0))		// Not handling flare.
	{
		return true;
	}

	return false;
}

bool TestLaraCrawlUpStep(ITEM_INFO* item, COLL_INFO* coll)
{
	auto y = item->pos.yPos;
	auto probeA = GetCollisionResult(item, coll->Setup.ForwardAngle, STEP_SIZE * 2, 0);
	auto probeB = GetCollisionResult(item, coll->Setup.ForwardAngle, STEP_SIZE, 0);

	if ((probeA.Position.Floor - y) <= -STEP_SIZE &&																			// Lower floor bound. Synced with crawl states' BadHeightUp.
		(probeA.Position.Floor - y) >= -STEPUP_HEIGHT &&																		// Upper floor bound.
		(probeB.Position.Floor - y) <= -STEP_SIZE &&																			// Ledge isn't too far away.
		(probeB.Position.Floor - y) >= -STEPUP_HEIGHT &&
		((coll->Middle.Ceiling - LARA_HEIGHT_CRAWL) - (probeA.Position.Floor - y)) <= -(STEP_SIZE / 2 + STEP_SIZE / 4) &&		// Gap is optically feasible for action.
		abs(probeA.Position.Ceiling - probeA.Position.Floor) > LARA_HEIGHT_CRAWL &&												// Space is not a clamp.
		!probeA.Position.Slope &&																								// Not a slope.
		probeA.Position.Floor != NO_HEIGHT)
	{
		return true;
	}

	return false;
}

bool TestLaraCrawlDownStep(ITEM_INFO* item, COLL_INFO* coll)
{
	auto y = item->pos.yPos;
	auto probeA = GetCollisionResult(item, coll->Setup.ForwardAngle, STEP_SIZE * 2, 0);
	auto probeB = GetCollisionResult(item, coll->Setup.ForwardAngle, STEP_SIZE, 0);

	if ((probeA.Position.Floor - y) <= STEPUP_HEIGHT &&									// Lower floor bound. Synced with crawl exit jump's highest floor bound.
		(probeA.Position.Floor - y) >= STEP_SIZE &&										// Upper floor bound. Synced with crawl states' BadHeightDown.
		(probeB.Position.Floor - y) <= STEPUP_HEIGHT &&									// Ledge isn't too far away.
		(probeB.Position.Floor - y) >= STEP_SIZE &&
		(probeA.Position.Ceiling - y) <= -(STEP_SIZE / 2 + STEP_SIZE / 4) &&			// Gap is optically feasible for action.
		abs(probeA.Position.Ceiling - probeA.Position.Floor) > LARA_HEIGHT_CRAWL &&		// Space is not a clamp.
		!probeA.Position.Slope &&														// Not a slope.
		probeA.Position.Floor != NO_HEIGHT)
	{
		return true;
	}

	return false;
}

bool TestLaraCrawlExitDownStep(ITEM_INFO* item, COLL_INFO* coll)
{
	auto y = item->pos.yPos;
	auto probeA = GetCollisionResult(item, coll->Setup.ForwardAngle, STEP_SIZE + STEP_SIZE / 2, 0);
	auto probeB = GetCollisionResult(item, coll->Setup.ForwardAngle, STEP_SIZE, 0);


	if ((probeA.Position.Floor - y) <= STEPUP_HEIGHT &&								// Lower floor bound. Synced with crawl exit jump's highest floor bound.
		(probeA.Position.Floor - y) >= STEP_SIZE &&									// Upper floor bound. Synced with crawl states' BadHeightDown.
		(probeB.Position.Floor - y) <= STEPUP_HEIGHT &&								// Ledge isn't too far away.
		(probeB.Position.Floor - y) >= STEP_SIZE &&
		(probeA.Position.Ceiling - y) <= -(STEP_SIZE + STEP_SIZE / 4) &&			// Gap is optically feasible for action.
		abs(probeA.Position.Ceiling - probeA.Position.Floor) > LARA_HEIGHT &&		// Space is not a clamp.
		!probeA.Position.Slope &&													// Not a slope.
		probeA.Position.Floor != NO_HEIGHT)
	{
		return true;
	}

	return false;
}

bool TestLaraCrawlExitJump(ITEM_INFO* item, COLL_INFO* coll)
{
	auto y = item->pos.yPos;
	auto probeA = GetCollisionResult(item, coll->Setup.ForwardAngle, STEP_SIZE + STEP_SIZE / 2, 0);
	auto probeB = GetCollisionResult(item, coll->Setup.ForwardAngle, STEP_SIZE, 0);

	if ((probeA.Position.Floor - y) > STEPUP_HEIGHT &&								// Highest floor bound. Synced with crawl down step and crawl exit down step's lower floor bounds.
		(probeB.Position.Floor - y) > STEPUP_HEIGHT &&								// Ledge isn't too far away.
		(probeA.Position.Ceiling - y) <= -(STEP_SIZE + STEP_SIZE / 4) &&			// Gap is optically feasible for action.
		abs(probeA.Position.Ceiling - probeA.Position.Floor) > LARA_HEIGHT &&		// Space is not a clamp.
		!probeA.Position.Slope &&													// Not a slope.
		probeA.Position.Floor != NO_HEIGHT)
	{
		return true;
	}

	return false;
}

bool TestLaraCrawlVault(ITEM_INFO* item, COLL_INFO* coll)
{
	if (TestLaraCrawlExitJump(item, coll) ||
		TestLaraCrawlExitDownStep(item, coll) ||
		TestLaraCrawlUpStep(item, coll) ||
		TestLaraCrawlDownStep(item, coll))
	{
		return true;
	}

	return false;
}

bool TestLaraCrawlToHang(ITEM_INFO* item, COLL_INFO* coll)
{
	auto y = item->pos.yPos;
	auto probe = GetCollisionResult(item, coll->Setup.ForwardAngle + ANGLE(180.0f), LARA_RAD_CRAWL * sqrt(2) + 4, 0);
	auto objectCollided = TestLaraObjectCollision(item, item->pos.yRot + ANGLE(180.0f), LARA_RAD_CRAWL * sqrt(2) + 4, 0);

	if (!objectCollided &&														// No obstruction.
		(probe.Position.Floor - y) >= LARA_HEIGHT_STRETCH &&					// Highest floor bound.
		(probe.Position.Ceiling - y) <= -(STEP_SIZE / 2 + STEP_SIZE / 4) &&		// Gap is optically feasible for action.
		probe.Position.Floor != NO_HEIGHT)
	{
		return true;
	}

	return false;
}

bool TestLaraPoleUp(ITEM_INFO* item, COLL_INFO* coll)
{
	// TODO: Accuracy.
	return (coll->Middle.Ceiling < -STEP_SIZE);
}

bool TestLaraPoleDown(ITEM_INFO* item, COLL_INFO* coll)
{
	return (coll->Middle.Floor > 0);
}
