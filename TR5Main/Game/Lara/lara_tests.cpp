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
	return (abs((short)(coll->NearestLedgeAngle - item->pos.yRot)) <= LARA_GRAB_THRESHOLD);
}

bool TestLaraVault(ITEM_INFO* item, COLL_INFO* coll)
{
	if (!(TrInput & IN_ACTION) || Lara.gunStatus != LG_NO_ARMS)
		return false;

	if (TestLaraSwamp(item) && Lara.waterSurfaceDist < -768)
		return false;

	// TODO: LUA
	Lara.NewAnims.CrawlVault1click = true;
	Lara.NewAnims.CrawlVault2click = true;
	Lara.NewAnims.CrawlVault3click = true;
	Lara.NewAnims.MonkeyAutoJump = false;

	if (TestValidLedge(item, coll))
	{
		bool success = false;

		// Vault to crouch up one step.
		if (coll->Front.Floor < 0 &&					// Lower floor bound.
			coll->Front.Floor > -STEPUP_HEIGHT &&		// Upper floor bound.
			Lara.NewAnims.CrawlVault1click)
		{
			if (abs((coll->Front.Ceiling - coll->Setup.Height) - coll->Front.Floor) > LARA_HEIGHT_CRAWL)		// Front clamp buffer. Presumably, nothing more necessary, but tend to this in the future. @Sezz 2021.11.06
			{
				item->animNumber = LA_VAULT_TO_CROUCH_1CLICK;
				item->currentAnimState = LS_GRABBING;
				item->frameNumber = GF(LA_VAULT_TO_CROUCH_1CLICK, 0);
				item->goalAnimState = LS_CROUCH_IDLE;
				item->pos.yPos += coll->Front.Floor + STEP_SIZE;
				Lara.gunStatus = LG_HANDS_BUSY;
				success = true;
			}
		}
		// Vault up two steps.
		else if (coll->Front.Floor <= -STEPUP_HEIGHT &&				// Lower floor bound.
			coll->Front.Floor >= -(STOP_SIZE + STEP_SIZE / 2))		// Upper floor bound.
		{
			// Vault to stand up two steps.
			if (abs((coll->Front.Ceiling - coll->Setup.Height) - coll->Front.Floor) > LARA_HEIGHT/* &&				// Front clamp buffer. BUG: Turned away from the ledge and toward a section with a low ceiling, stand-to-crawl vault will be performed instead. @Sezz 2021.11.06
				abs((coll->FrontLeft.Ceiling - coll->Setup.Height) - coll->FrontLeft.Floor) > LARA_HEIGHT &&		// Left clamp buffer. // TODO: Ceilings don't push, so these are unnecessary for now. @Sezz 2021.11.06
				abs((coll->FrontRight.Ceiling - coll->Setup.Height) - coll->FrontRight.Floor) > LARA_HEIGHT*/)		// Right clamp buffer.
			{
				item->animNumber = LA_VAULT_TO_STAND_2CLICK_START;
				item->currentAnimState = LS_GRABBING;
				item->frameNumber = GF(LA_VAULT_TO_STAND_2CLICK_START, 0);
				item->goalAnimState = LS_STOP;
				item->pos.yPos += coll->Front.Floor + STOP_SIZE;
				Lara.gunStatus = LG_HANDS_BUSY;
				success = true;
			}
			// Vault to crouch up two steps.
			else if (abs((coll->Front.Ceiling - coll->Setup.Height) - coll->Front.Floor) > LARA_HEIGHT_CRAWL &&				// Front clamp buffer.
				abs((coll->FrontLeft.Ceiling - coll->Setup.Height) - coll->FrontLeft.Floor) > LARA_HEIGHT_CRAWL &&			// Left clamp buffer.
				abs((coll->FrontRight.Ceiling - coll->Setup.Height) - coll->FrontRight.Floor) > LARA_HEIGHT_CRAWL &&		// Right clamp buffer.
				Lara.NewAnims.CrawlVault2click)
			{
				item->animNumber = LA_VAULT_TO_CROUCH_2CLICK;
				item->frameNumber = GF(LA_VAULT_TO_CROUCH_2CLICK, 0);
				item->currentAnimState = LS_GRABBING;
				item->goalAnimState = LS_CROUCH_IDLE;
				item->pos.yPos += coll->Front.Floor + STOP_SIZE;
				Lara.gunStatus = LG_HANDS_BUSY;
				success = true;
			}
		}
		// Vault up three steps.
		else if (coll->Front.Floor <= -(STOP_SIZE + STEP_SIZE / 2) &&		// Lower floor bound.
			coll->Front.Floor >= -(WALL_SIZE - STEP_SIZE / 2))				// Upper floor bound.
		{
			// Vault to stand up three steps.
			if (abs((coll->Front.Ceiling - coll->Setup.Height) - coll->Front.Floor) > LARA_HEIGHT/* &&				// Front clamp buffer. BUG: Turned away from the ledge and toward a section with a low ceiling, stand-to-crawl vault will be performed instead. @Sezz 2021.11.06
				abs((coll->FrontLeft.Ceiling - coll->Setup.Height) - coll->FrontLeft.Floor) > LARA_HEIGHT &&		// Left clamp buffer. // TODO: Ceilings don't push, so these are unnecessary for now. @Sezz 2021.11.06
				abs((coll->FrontRight.Ceiling - coll->Setup.Height) - coll->FrontRight.Floor) > LARA_HEIGHT*/)		// Right clamp buffer.
			{
				item->animNumber = LA_VAULT_TO_STAND_3CLICK;
				item->currentAnimState = LS_GRABBING;
				item->frameNumber = GF(LA_VAULT_TO_STAND_3CLICK, 0);
				item->goalAnimState = LS_STOP;
				item->pos.yPos += coll->Front.Floor + (STOP_SIZE + STEP_SIZE);
				Lara.gunStatus = LG_HANDS_BUSY;
				success = true;
			}
			// Vault to crouch up three steps.
			else if (abs((coll->Front.Ceiling - coll->Setup.Height) - coll->Front.Floor) > LARA_HEIGHT_CRAWL &&				// Front clamp buffer.
				abs((coll->FrontLeft.Ceiling - coll->Setup.Height) - coll->FrontLeft.Floor) > LARA_HEIGHT_CRAWL &&			// Left clamp buffer.
				abs((coll->FrontRight.Ceiling - coll->Setup.Height) - coll->FrontRight.Floor) > LARA_HEIGHT_CRAWL &&		// Right clamp buffer.
				Lara.NewAnims.CrawlVault3click)
			{
				item->animNumber = LA_VAULT_TO_CROUCH_3CLICK;
				item->frameNumber = GF(LA_VAULT_TO_CROUCH_3CLICK, 0);
				item->currentAnimState = LS_GRABBING;
				item->goalAnimState = LS_CROUCH_IDLE;
				item->pos.yPos += coll->Front.Floor + (STOP_SIZE + STEP_SIZE);
				Lara.gunStatus = LG_HANDS_BUSY;
				success = true;
			}
		}
		// Auto jump.
		else if (coll->Front.Floor >= -(WALL_SIZE * 2 - STEP_SIZE / 2) &&		// Upper floor bound.
			coll->Front.Floor <= -(WALL_SIZE - STEP_SIZE / 2))					// Lower floor bound.
		{
			item->animNumber = LA_STAND_SOLID;
			item->frameNumber = GF(LA_STAND_SOLID, 0);
			item->goalAnimState = LS_JUMP_UP;
			item->currentAnimState = LS_STOP;
			Lara.calcFallSpeed = -3 - sqrt(-9600 - 12 * coll->Front.Floor);
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
	if (Lara.climbStatus)
	{
		if (coll->Front.Floor > -(WALL_SIZE * 2 - STEP_SIZE / 2) ||			// Upper front floor bound.
			coll->FrontLeft.Floor > -(WALL_SIZE * 2 - STEP_SIZE / 2) ||		// Upper left floor bound.
			coll->FrontRight.Floor > -STOP_SIZE ||							// Upper right floor bound.
			coll->Middle.Ceiling > -(WALL_SIZE + STEP_SIZE / 2 + 6) ||		// Upper ceiling bound.
			Lara.waterStatus == LW_WADE)
		{
			if ((coll->Front.Floor < -WALL_SIZE || coll->Front.Ceiling >= (STOP_SIZE - 6)) &&
				coll->Middle.Ceiling <= -(STOP_SIZE + 6))
			{
				if (TestLaraClimbStance(item, coll))
				{
					item->animNumber = LA_STAND_SOLID;
					item->frameNumber = GF(LA_STAND_SOLID, 0);
					item->goalAnimState = LS_LADDER_IDLE;
					item->currentAnimState = LS_STOP;
					Lara.gunStatus = LG_HANDS_BUSY;
					Lara.turnRate = 0;

					ShiftItem(item, coll);
					SnapItemToGrid(item, coll); // HACK: until fragile ladder code is refactored, we must exactly snap to grid.
					AnimateLara(item);

					return true;
				}
			}

			return false;
		}

		// Auto jump to ladder.
		item->animNumber = LA_STAND_SOLID;
		item->frameNumber = GF(LA_STAND_SOLID, 0);
		item->goalAnimState = LS_JUMP_UP;
		item->currentAnimState = LS_STOP;
		Lara.calcFallSpeed = -116;
		Lara.turnRate = 0;
		
		ShiftItem(item, coll);
		SnapItemToGrid(item, coll); // HACK: until fragile ladder code is refactored, we must exactly snap to grid.
		AnimateLara(item);

		return true;
	}

	// Auto jump to monkey swing.
	if (Lara.canMonkeySwing &&
		Lara.NewAnims.MonkeyAutoJump)
	{
		short roomNum = item->roomNumber;
		int ceiling = (GetCeiling(GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNum),
			item->pos.xPos, item->pos.yPos, item->pos.zPos))-(item->pos.yPos);

		if (ceiling > (WALL_SIZE * 2 - STEP_SIZE) ||
			ceiling < -(WALL_SIZE * 2 - STEP_SIZE) ||
			abs(ceiling) == (STOP_SIZE + STEP_SIZE))
		{
			return false;
		}

		item->animNumber = LA_STAND_IDLE;
		item->frameNumber = GF(LA_STAND_IDLE, 0);
		item->goalAnimState = LS_JUMP_UP;
		item->currentAnimState = LS_TEST_1;

		return true;
	}

	return false;
}

bool TestLaraKeepDucked(ITEM_INFO* item, COLL_INFO* coll)
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
	// A buffer might help, but improved collision handling would presumably eliminate this issue as a side product. @Sezz 2021.10.15
	if ((coll->Middle.Ceiling - LARA_HEIGHT_CRAWL) >= -LARA_HEIGHT ||		// Middle would clamp.
		(coll->Front.Ceiling - LARA_HEIGHT_CRAWL) >= -LARA_HEIGHT ||		// Front would clamp.
		(probeBack.Position.Ceiling - y) >= -LARA_HEIGHT)					// Back would clamp.
	{
		return true;
	}

	return false;
}

bool TestLaraSlide(ITEM_INFO* item, COLL_INFO* coll)
{
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

		item->animNumber = LA_SLIDE_BACK_START;
		item->goalAnimState = LS_SLIDE_BACK;
		item->currentAnimState = LS_SLIDE_BACK;
		item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
		item->pos.yRot = angle + ANGLE(180.0f);
	}
	else
	{
		if (item->currentAnimState == LS_SLIDE_FORWARD && oldAngle == angle)
			return true;

		item->animNumber = LA_SLIDE_FORWARD;
		item->goalAnimState = LS_SLIDE_FORWARD;
		item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
		item->currentAnimState = LS_SLIDE_FORWARD;
		item->pos.yRot = angle;
	}

	Lara.moveAngle = angle;
	oldAngle = angle;

	return true;
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
	if (!(TrInput & IN_ACTION) || (Lara.gunStatus != LG_NO_ARMS) || (coll->HitStatic))
		return false;

	if (Lara.canMonkeySwing && coll->CollisionType == CT_TOP)
	{
		item->goalAnimState = LS_MONKEYSWING_IDLE;
		item->currentAnimState = LS_MONKEYSWING_IDLE;
		item->animNumber = LA_JUMP_UP_TO_MONKEYSWING;
		item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
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
		item->animNumber = LA_JUMP_UP_TO_MONKEYSWING;
		item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
		item->goalAnimState = LS_MONKEYSWING_IDLE;
		item->currentAnimState = LS_MONKEYSWING_IDLE;
	}
	else
	{
		if (TestHangFeet(item, angle))
		{
			item->animNumber = LA_REACH_TO_HANG;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase + 12;
			item->currentAnimState = LS_HANG;
			item->goalAnimState = LS_HANG_FEET;
		}
		else
		{
			item->animNumber = LA_REACH_TO_HANG;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase + 12;
			item->currentAnimState = LS_HANG;
			item->goalAnimState = LS_HANG;
		}
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
	if (!(TrInput & IN_ACTION) || (Lara.gunStatus != LG_NO_ARMS) || (coll->HitStatic))
		return false;

	if (Lara.canMonkeySwing && coll->CollisionType == CT_TOP)
	{
		Lara.headYrot  = 0;
		Lara.headXrot  = 0;
		Lara.torsoYrot = 0;
		Lara.torsoXrot = 0;
		Lara.gunStatus = LG_HANDS_BUSY;

		item->animNumber = LA_REACH_TO_MONKEYSWING;
		item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
		item->goalAnimState = LS_MONKEYSWING_IDLE;
		item->currentAnimState = LS_MONKEYSWING_IDLE;
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

	if (TestHangSwingIn(item, angle))
	{
		if (Lara.NewAnims.OscillateHanging)
		{
			Lara.headYrot = 0;
			Lara.headXrot = 0;
			Lara.torsoYrot = 0;
			Lara.torsoXrot = 0;
			item->animNumber = LA_REACH_TO_HANG_OSCILLATE;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			item->currentAnimState = LS_HANG;
			item->goalAnimState = LS_HANG;
		}
		else
		{
			Lara.headYrot = 0;
			Lara.headXrot = 0;
			Lara.torsoYrot = 0;
			Lara.torsoXrot = 0;
			item->animNumber = LA_REACH_TO_MONKEYSWING;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			item->currentAnimState = LS_MONKEYSWING_IDLE;
			item->goalAnimState = LS_MONKEYSWING_IDLE;
		}
	}
	else
	{
		if (TestHangFeet(item, angle))
		{
			item->animNumber = LA_REACH_TO_HANG;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			item->currentAnimState = LS_HANG;
			item->goalAnimState = LS_HANG_FEET;
		}
		else
		{
			item->animNumber = LA_REACH_TO_HANG;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			item->currentAnimState = LS_HANG;
			item->goalAnimState = LS_HANG;
		}
	}

	auto bounds = GetBoundsAccurate(item);

	if (edgeCatch <= 0)
	{
		item->pos.yPos = edge - bounds->Y1 - 20;
		item->pos.yRot = coll->NearestLedgeAngle;
	}
	else
	{
		item->pos.yPos += coll->Front.Floor - bounds->Y1 - 20;
	}

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
	ANIM_FRAME* frame;

	auto delta = 0;
	auto flag = 0;
	auto angle = Lara.moveAngle;

	if (angle == (short) (item->pos.yRot - ANGLE(90)))
	{
		delta = -coll->Setup.Radius;
	}
	else if (angle == (short) (item->pos.yRot + ANGLE(90)))
	{
		delta = coll->Setup.Radius;
	}

	auto s = phd_sin(Lara.moveAngle);
	auto c = phd_cos(Lara.moveAngle);
	auto testShift = Vector2(s * delta, c * delta);

	auto hdif = LaraFloorFront(item, angle, coll->Setup.Radius);
	if (hdif < 200)
		flag = 1;

	auto cdif = LaraCeilingFront(item, angle, coll->Setup.Radius, 0);
	auto dir = GetQuadrant(item->pos.yRot);

	// When Lara is about to move, use larger embed offset for stabilizing diagonal shimmying)
	auto embedOffset = 4;
	if ((TrInput & IN_LEFT) || (TrInput & IN_RIGHT))
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
		if (TrInput & IN_ACTION && item->hitPoints > 0)
		{
			Lara.moveAngle = angle;

			if (!TestLaraHangOnClimbWall(item, coll))
			{
				if (item->animNumber != LA_LADDER_TO_HANG_RIGHT && item->animNumber != LA_LADDER_TO_HANG_LEFT)
				{
					LaraSnapToEdgeOfBlock(item, coll, dir);
					item->pos.yPos = coll->Setup.OldPosition.y;
					item->currentAnimState = LS_HANG;
					item->goalAnimState = LS_HANG;
					item->animNumber = LA_REACH_TO_HANG;
					item->frameNumber = g_Level.Anims[item->animNumber].frameBase + 21;
				}
				result = true;
			}
			else
			{
				if (item->animNumber == LA_REACH_TO_HANG && item->frameNumber == g_Level.Anims[LA_REACH_TO_HANG].frameBase + 21 && TestLaraClimbStance(item, coll))
					item->goalAnimState = LS_LADDER_IDLE;
			}
		}
		else
		{
			item->animNumber = LA_FALL_START;
			item->currentAnimState = LS_JUMP_FORWARD;
			item->goalAnimState = LS_JUMP_FORWARD;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			item->pos.yPos += 256;
			item->gravityStatus = true;
			item->speed = 2;
			item->fallspeed = 1;
			Lara.gunStatus = LG_NO_ARMS;
		}
	}
	else
	{
		if (TrInput & IN_ACTION && item->hitPoints > 0 && coll->Front.Floor <= 0)
		{
			if (flag && hdif > 0 && delta > 0 == coll->MiddleLeft.Floor > coll->MiddleRight.Floor)
				flag = 0;

			frame = (ANIM_FRAME*)GetBoundsAccurate(item);
			auto front = coll->Front.Floor;
			auto dfront = coll->Front.Floor - frame->boundingBox.Y1;
			auto flag2 = 0;
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
					flag2 = 1;
			}

			coll->Front.Floor = front;

			if (!flag2 && 
				coll->Middle.Ceiling < 0 && 
				coll->CollisionType == CT_FRONT && 
				!flag && 
				!coll->HitStatic && 
				cdif <= -950 && 
				dfront >= -60 &&
				dfront <= 60 &&
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

				if (item->currentAnimState == LS_SHIMMY_LEFT || item->currentAnimState == LS_SHIMMY_RIGHT)
				{
					item->currentAnimState = LS_HANG;
					item->goalAnimState = LS_HANG;
					item->animNumber = LA_REACH_TO_HANG;
					item->frameNumber = g_Level.Anims[item->animNumber].frameBase + 21;
				}
				else if (item->currentAnimState == LS_SHIMMY_FEET_LEFT || item->currentAnimState == LS_SHIMMY_FEET_RIGHT)
				{
					item->currentAnimState = LS_HANG_FEET;
					item->goalAnimState = LS_HANG_FEET;
					item->animNumber = LA_HANG_FEET_IDLE;
					item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
				}

				result = true;
			}
		}
		else
		{
			item->currentAnimState = LS_JUMP_UP;
			item->goalAnimState = LS_JUMP_UP;
			item->animNumber = LA_JUMP_UP;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase + 9;
			frame = (ANIM_FRAME*)GetBoundsAccurate(item);
			item->pos.xPos += coll->Shift.x;
			item->pos.yPos += frame->boundingBox.Y2;
			item->pos.zPos += coll->Shift.z;
			item->gravityStatus = true;
			item->speed = 2;
			item->fallspeed = 1;
			Lara.gunStatus = LG_NO_ARMS;
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

		// FIXME? Those hacky fields are still used somewhere to align her...
		Lara.cornerX = item->pos.xPos;
		Lara.cornerZ = item->pos.zPos;

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
		// FIXME? Those hacky fields are still used somewhere to align her...
		Lara.cornerX = item->pos.xPos;
		Lara.cornerZ = item->pos.zPos;

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
	auto frontFloor = LaraFloorFront(item, Lara.moveAngle, coll->Setup.Radius + STEP_SIZE / 2) + item->pos.yPos;
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

	if (LaraTestClimbPos(item, coll->Setup.Radius, coll->Setup.Radius + 120, -700, 512, &shift_r) != 1)
		return false;

	if (LaraTestClimbPos(item, coll->Setup.Radius, -(coll->Setup.Radius + 120), -700, 512, &shift_l) != 1)
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
		short r = LaraCeilingFront(item, Lara.moveAngle, 128, 0);

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
	int h, c;

	//debug till scripting be ready
	Lara.NewAnims.OscillateHanging = 0;

	z += phd_cos(angle) * (STEP_SIZE / 2);
	x += phd_sin(angle) * (STEP_SIZE / 2);

	floor = GetFloor(x, y, z, &roomNum);
	h = GetFloorHeight(floor, x, y, z);
	c = GetCeiling(floor, x, y, z);

	if (h != NO_HEIGHT)
	{
		if (Lara.NewAnims.OscillateHanging)
		{
			if (h - y > 0 && c - y < -400)
				return true;
		}
		else
		{
			if (h - y > 0 && c - y < -400 && (y - 819 - c > -72))
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
	FLOOR_INFO* floor;
	int h, c, g, m, j;

	z += phd_cos(angle) * (STEP_SIZE / 2);
	x += phd_sin(angle) * (STEP_SIZE / 2);

	floor = GetFloor(x, y, z, &roomNum);
	h = GetFloorHeight(floor, x, y, z);
	c = GetCeiling(floor, x, y, z);
	g = h - y;
	m = c - y;
	j = y - 128 - c;

	if (h != NO_HEIGHT)
	{
		if (g > 0 && m < -128 && j > -72)
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
		item->goalAnimState = LS_JUMP_FORWARD;
		item->currentAnimState = LS_JUMP_FORWARD;
		item->animNumber = LA_FALL_START;
		item->frameNumber = g_Level.Anims[item->animNumber].frameBase;

		item->gravityStatus = true;
		item->speed = 2;
		item->pos.yPos += 256;
		item->fallspeed = 1;

		Lara.gunStatus = LG_NO_ARMS;

		item->pos.yRot += rot / 2;
	}
	else if (flip)
	{
		if (Lara.isClimbing)
		{
			item->animNumber = LA_LADDER_IDLE;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			item->goalAnimState = LS_LADDER_IDLE;
			item->currentAnimState = LS_LADDER_IDLE;
		}
		else
		{
			item->animNumber = LA_REACH_TO_HANG;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase + 21;
			item->goalAnimState = LS_HANG;
			item->currentAnimState = LS_HANG;
		}

		coll->Setup.OldPosition.x = Lara.cornerX;
		item->pos.xPos = Lara.cornerX;

		coll->Setup.OldPosition.z = Lara.cornerZ;
		item->pos.zPos = Lara.cornerZ;

		item->pos.yRot += rot;
	}
}

void SetCornerAnimFeet(ITEM_INFO* item, COLL_INFO* coll, short rot, short flip)
{
	if (item->hitPoints <= 0)
	{
		item->goalAnimState = LS_JUMP_FORWARD;
		item->currentAnimState = LS_JUMP_FORWARD;
		item->animNumber = LA_FALL_START;
		item->frameNumber = g_Level.Anims[item->animNumber].frameBase;

		item->gravityStatus = true;
		item->speed = 2;
		item->pos.yPos += 256;
		item->fallspeed = 1;

		Lara.gunStatus = LG_NO_ARMS;

		item->pos.yRot += rot / 2;
	}
	else if (flip)
	{

		item->animNumber = LA_HANG_FEET_IDLE;
		item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
		item->goalAnimState = LS_HANG_FEET;
		item->currentAnimState = LS_HANG_FEET;

		coll->Setup.OldPosition.x = Lara.cornerX;
		item->pos.xPos = Lara.cornerX;

		coll->Setup.OldPosition.z = Lara.cornerZ;
		item->pos.zPos = Lara.cornerZ;

		item->pos.yRot += rot;
	}
}

bool TestLaraStandingJump(ITEM_INFO* item, COLL_INFO* coll, short angle)
{
	auto y = item->pos.yPos;
	auto probe = GetCollisionResult(item, angle, STEP_SIZE, coll->Setup.Height);

	if (!TestLaraFacingCorner(item, angle, STEP_SIZE) &&
		probe.Position.Floor - y >= -STEPUP_HEIGHT &&
		probe.Position.Ceiling - y < -(coll->Setup.Height + LARA_HEADROOM * 0.7f))
	{
		return true;
	}

	return false;
}

bool TestLaraFacingCorner(ITEM_INFO* item, short angle, int dist)
{
	// TODO: Objects? Lara will attempt to jump against them.
	// TODO: Check for ceilings! @Sezz 2021.10.16

	auto y = item->pos.yPos;

	auto angleA = angle - ANGLE(15.0f);
	auto angleB = angle + ANGLE(15.0f);

	auto probeA = GetCollisionResult(item, angleA, dist, 0);
	auto probeB = GetCollisionResult(item, angleB, dist, 0);

	// TODO: Ceilings.
	if (probeA.Position.Floor - y < -STEPUP_HEIGHT &&
		probeB.Position.Floor - y < -STEPUP_HEIGHT)
	{
		return true;
	}

	return false;
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
	int x = item->pos.xPos + dist * phd_sin(ang);
	int y = item->pos.yPos - LARA_HEIGHT;
	int z = item->pos.zPos + dist * phd_cos(ang);

	auto collResult = GetCollisionResult(x, y, z, item->roomNumber);

	if (collResult.Position.Floor != NO_HEIGHT)
		collResult.Position.Floor -= item->pos.yPos;

	return collResult;
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
	int x = item->pos.xPos + dist * phd_sin(ang);
	int y = item->pos.yPos - h;
	int z = item->pos.zPos + dist * phd_cos(ang);

	auto collResult = GetCollisionResult(x, y, z, item->roomNumber);

	if (collResult.Position.Ceiling != NO_HEIGHT)
		collResult.Position.Ceiling += h - item->pos.yPos;

	return collResult;
}

bool LaraFallen(ITEM_INFO* item, COLL_INFO* coll)
{
	if (Lara.waterStatus == LW_WADE || coll->Middle.Floor <= STEPUP_HEIGHT)
	{
		return false;
	}

	item->animNumber = LA_FALL_START;
	item->currentAnimState = LS_JUMP_FORWARD;
	item->goalAnimState = LS_JUMP_FORWARD;
	item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
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

#ifndef NEW_TIGHTROPE
void GetTighRopeFallOff(int regularity) {
	if(LaraItem->hitPoints <= 0 || LaraItem->hitStatus)
	{
		LaraItem->goalAnimState = LS_TIGHTROPE_UNBALANCE_LEFT;
		LaraItem->currentAnimState = LS_TIGHTROPE_UNBALANCE_LEFT;
		LaraItem->animNumber = LA_TIGHTROPE_FALL_LEFT;
		LaraItem->frameNumber = g_Level.Anims[LaraItem->animNumber].frameBase;
	}

	if(!Lara.tightRopeFall && !(GetRandomControl() & regularity))
		Lara.tightRopeFall = 2 - ((GetRandomControl() & 0xF) != 0);
}
#endif // !NEW_TIGHTROPE

bool TestLaraFall(COLL_INFO* coll)
{
	if (coll->Middle.Floor <= STEPUP_HEIGHT ||
		Lara.waterStatus == LW_WADE)	// TODO: This causes a legacy floor snap BUG when lara wades off a ledge into a dry room. @Sezz 2021.09.26
	{
		return false;
	}

	return true;
}

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
	if (Lara.gunStatus == LG_NO_ARMS &&								// Hands are free.
		!(TrInput & (IN_FLARE | IN_DRAW)) &&						// Avoid unsightly concurrent actions.
		(Lara.gunType != WEAPON_FLARE || Lara.flareAge > 0) &&		// Flare is not being handled. TODO: Will she pose with weapons drawn?
		Lara.Vehicle == NO_ITEM)									// Not in a vehicle.
	{
		return true;
	}

	return false;
}

// TODO: Try using each state's BadStepUp/Down.  @Sezz 2021.10.11
bool TestLaraStep(COLL_INFO* coll)
{
	if (coll->Middle.Floor <= STEPUP_HEIGHT &&		// Lower floor bound.
		coll->Middle.Floor >= -STEPUP_HEIGHT)		// Upper floor bound.
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
		item->currentAnimState != LS_WADE_FORWARD &&		// No step up anim exists for these states.
		item->currentAnimState != LS_WALK_BACK &&
		item->currentAnimState != LS_HOP_BACK &&
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
		item->currentAnimState != LS_HOP_BACK &&
		item->currentAnimState != LS_CRAWL_IDLE &&		// Crawl step down handled differently.
		item->currentAnimState != LS_CRAWL_FORWARD)
	{
		return true;
	}

	return false;
}

// TODO: This function should become obsolete in the future with more accurate and accessible collision detection.
// For now, it supercedes old probes and is used alongside COLL_INFO. @Sezz 2021.10.24
bool TestLaraMove(ITEM_INFO* item, COLL_INFO* coll, short angle, int lowerBound, int upperBound)
{
	// TODO: coll->Setup.Radius is currently only set to
	// LARA_RAD_CRAWL in the collision function, then reset by LaraAboveWater().
	// For tests called in crawl control functions, then, it will store the wrong radius.
	// Function below (TestLaraMoveCrawl()) is a clone to account for this. @Sezz 2021.11.05
	// TODO: Current probe radius is fixed and overshoots by a wide margin at 0 degrees to a wall. No issues,
	// but for more accuracy in the future, get distance to nearest wall/ceiling/object instead. @Sezz 2021.11.04

	auto y = item->pos.yPos;
	auto probe = GetCollisionResult(item, angle, coll->Setup.Radius * sqrt(2) + 4, 0); // Offset of 4 required to account for gap between Lara and the wall. Results in slight overshoot, but avoids oscillation.

	if ((probe.Position.Floor - y) <= lowerBound &&					// Lower floor bound.
		(probe.Position.Floor - y) >= upperBound &&					// Upper floor bound.
		(probe.Position.Ceiling - y) < -coll->Setup.Height &&		// Lowest ceiling bound.
		!probe.Position.Slope &&									// No slope.
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

	if ((probe.Position.Floor - y) <= lowerBound &&					// Lower floor bound.
		(probe.Position.Floor - y) >= upperBound &&					// Upper floor bound.
		(probe.Position.Ceiling - y) < -LARA_HEIGHT_CRAWL &&		// Lowest ceiling bound.
		!probe.Position.Slope &&									// No slope.
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
	
	// BUG: This interferes with the one-step stand-to-crouch vault where the floor is lower than STEP_SIZE.
	if ((probe.Position.Ceiling - y) < 0)		// Hack to ensure Lara can run off diagonal ledges. coll->Front.Floor often holds the wrong height because of quadrant-dependent wall pushing. @Sezz 2021.11.06
		return true;

	return false;

	// TODO: TestLaraMove() call not useful yet; it can block Lara from climbing. @Sezz 2021.10.22
	// Additionally, run forward test needs to incorporate a unique ceiling check (as above). @Sezz 2021.10.28
	//return TestLaraMove(item, coll, coll->Setup.ForwardAngle, STEPUP_HEIGHT, -STEPUP_HEIGHT);		// Using BadHeightUp/Down defined in walk and run state collision functions.
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
	//return TestLaraMove(item, coll, coll->Setup.ForwardAngle, STEPUP_HEIGHT, -STEPUP_HEIGHT);		// Using BadHeightUp/Down defined in walk and run state collision functions.
}

bool TestLaraWalkBack(ITEM_INFO* item, COLL_INFO* coll)
{
	return TestLaraMove(item, coll, coll->Setup.ForwardAngle + ANGLE(180.0f), STEPUP_HEIGHT, -STEPUP_HEIGHT);		// Using BadHeightUp/Down defined in walk back state collision function.
}

bool TestLaraHopBack(ITEM_INFO* item, COLL_INFO* coll)
{
	return TestLaraMove(item, coll, coll->Setup.ForwardAngle + ANGLE(180.0f), NO_BAD_POS, -STEPUP_HEIGHT);		// Using BadHeightUp/Down defined in hop back state collision function.
}

bool TestLaraStepLeft(ITEM_INFO* item, COLL_INFO* coll)
{
	return TestLaraMove(item, coll, coll->Setup.ForwardAngle - ANGLE(90.0f), STEP_SIZE / 2, -STEP_SIZE / 2);		// Using BadHeightUp/Down defined in step left state collision functions.
}

bool TestLaraStepRight(ITEM_INFO* item, COLL_INFO* coll)
{
	return TestLaraMove(item, coll, coll->Setup.ForwardAngle + ANGLE(90.0f), STEP_SIZE / 2, -STEP_SIZE / 2);		// Using BadHeightUp/Down defined in step right state collision function.
}

bool TestLaraWalkBackSwamp(ITEM_INFO* item, COLL_INFO* coll)
{
	return TestLaraMove(item, coll, coll->Setup.ForwardAngle + ANGLE(180.0f), NO_BAD_POS, -STEPUP_HEIGHT);
}

bool TestLaraStepLeftSwamp(ITEM_INFO* item, COLL_INFO* coll)
{
	return TestLaraMove(item, coll, coll->Setup.ForwardAngle - ANGLE(90.0f), NO_BAD_POS, -STEP_SIZE / 2);
}

bool TestLaraStepRightSwamp(ITEM_INFO* item, COLL_INFO* coll)
{
	return TestLaraMove(item, coll, coll->Setup.ForwardAngle + ANGLE(90.0f), NO_BAD_POS, -STEP_SIZE / 2);
}

// Currently unused; LaraCollideStopCrawl() handles front collision. @Sezz 2021.11.08
bool TestLaraCrawlForward(ITEM_INFO* item, COLL_INFO* coll)
{
	return TestLaraMoveCrawl(item, coll, coll->Setup.ForwardAngle, STEP_SIZE - 1, -(STEP_SIZE - 1));		// Using BadHeightUp/Down defined in crawl state collision functions.
}

bool TestLaraCrawlBack(ITEM_INFO* item, COLL_INFO* coll)
{
	return TestLaraMoveCrawl(item, coll, coll->Setup.ForwardAngle + ANGLE(180.0f), STEP_SIZE - 1, -(STEP_SIZE - 1));		// Using BadHeightUp/Down defined in crawl state collision functions.
}

bool TestLaraCrouchToCrawl(ITEM_INFO* item)
{
	if (Lara.gunStatus == LG_NO_ARMS &&								// Hands are free.
		!(TrInput & (IN_FLARE | IN_DRAW)) &&						// Avoid unsightly concurrent actions.
		(Lara.gunType != WEAPON_FLARE || Lara.flareAge > 0))		// Not handling flare.
	{
		return true;
	}

	return false;
}

bool TestLaraCrouchRoll(ITEM_INFO* item, COLL_INFO* coll)
{
	if (TestLaraCrawlForward(item, coll) &&
		Lara.gunStatus == LG_NO_ARMS &&								// Hands are free.
		Lara.waterSurfaceDist >= -STEP_SIZE &&						// Water depth is optically feasible for action.
		!(TrInput & (IN_FLARE | IN_DRAW)) &&						// Avoid unsightly concurrent actions.
		(Lara.gunType != WEAPON_FLARE || Lara.flareAge > 0))		// Not handling flare.
	{
		return true;
	}

	return false;
}

bool TestLaraCrawlUpStep(ITEM_INFO* item, COLL_INFO* coll)
{
	auto y = item->pos.yPos;
	auto probe = GetCollisionResult(item, coll->Setup.ForwardAngle, STEP_SIZE, 0);

	if ((probe.Position.Floor - y) <= -STEP_SIZE &&																			// Lower floor bound. Synced with crawl states' BadHeightUp.
		(probe.Position.Floor - y) >= -STEPUP_HEIGHT &&																		// Upper floor bound.
		((coll->Front.Ceiling - LARA_HEIGHT_CRAWL) - (probe.Position.Floor - y)) <= -(STEP_SIZE / 2 + STEP_SIZE / 4) &&		// Gap is optically feasible for action.
		abs(probe.Position.Ceiling - probe.Position.Floor) > LARA_HEIGHT_CRAWL &&											// Space is not a clamp.
		probe.Position.Floor != NO_HEIGHT)
	{
		return true;
	}

	return false;
}

bool TestLaraCrawlDownStep(ITEM_INFO* item, COLL_INFO* coll)
{
	auto y = item->pos.yPos;
	auto probe = GetCollisionResult(item, coll->Setup.ForwardAngle, STEP_SIZE, 0);

	if ((probe.Position.Floor - y) <= STEPUP_HEIGHT &&									// Lower floor bound. Synced with crawl exit jump's highest floor bound.
		(probe.Position.Floor - y) >= STEP_SIZE &&										// Upper floor bound. Synced with crawl states' BadHeightDown.
		(probe.Position.Ceiling - y) <= -(STEP_SIZE / 2 + STEP_SIZE / 4) &&				// Gap is optically feasible for action.
		abs(probe.Position.Ceiling - probe.Position.Floor) > LARA_HEIGHT_CRAWL &&		// Space is not a clamp.
		probe.Position.Floor != NO_HEIGHT)
	{
		return true;
	}

	return false;
}

bool TestLaraCrawlExitDownStep(ITEM_INFO* item, COLL_INFO* coll)
{
	auto y = item->pos.yPos;
	auto probe = GetCollisionResult(item, coll->Setup.ForwardAngle, STEP_SIZE, 0);

	if ((probe.Position.Floor - y) <= STEPUP_HEIGHT &&								// Lower floor bound. Synced with crawl exit jump's highest floor bound.
		(probe.Position.Floor - y) >= STEP_SIZE &&									// Upper floor bound. Synced with crawl states' BadHeightDown.
		(probe.Position.Ceiling - y) <= -(STEP_SIZE + STEP_SIZE / 4) &&				// Gap is optically feasible for action.
		abs(probe.Position.Ceiling - probe.Position.Floor) > LARA_HEIGHT &&			// Space is not a clamp.
		probe.Position.Floor != NO_HEIGHT)
	{
		return true;
	}

	return false;
}

bool TestLaraCrawlExitJump(ITEM_INFO* item, COLL_INFO* coll)
{
	auto y = item->pos.yPos;
	auto probe = GetCollisionResult(item, coll->Setup.ForwardAngle, STEP_SIZE, 0);

	if ((probe.Position.Floor - y) > STEPUP_HEIGHT &&							// Highest floor bound. Synced with crawl down step and crawl exit down step's lower floor bounds.
		(probe.Position.Ceiling - y) <= -(STEP_SIZE + STEP_SIZE / 4) &&			// Gap is optically feasible for action.
		abs(probe.Position.Ceiling - probe.Position.Floor) > LARA_HEIGHT &&		// Space is not a clamp.
		probe.Position.Floor != NO_HEIGHT)
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
	auto probe = GetCollisionResult(item, coll->Setup.ForwardAngle + ANGLE(180.0f), LARA_RAD_CRAWL + 4, 0);

	if (!TestLaraObjectCollision(item, item->pos.yRot + ANGLE(180.0f), LARA_RAD_CRAWL + 4, 0) &&		// No obstruction.
		(probe.Position.Floor - y) >= LARA_HEIGHT_STRETCH &&											// Highest floor bound.
		(probe.Position.Ceiling - y) <= -(STEP_SIZE / 2 + STEP_SIZE / 4) &&								// Gap is optically feasible for action.
		probe.Position.Floor != NO_HEIGHT)
	{
		return true;
	}

	return false;
}

// Entirely temporary. @Sezz 2021.10.16
bool TestLaraDrawWeaponsFromCrawlIdle(ITEM_INFO* item)
{
	if (item->animNumber == LA_CRAWL_IDLE ||
		(item->animNumber == LA_CROUCH_TO_CRAWL_START && item->frameNumber >= GF(LA_CROUCH_TO_CRAWL_START, 8)))
	{
		return true;
	}

	return false;
}

bool TestLaraSwamp(ITEM_INFO* item)
{
	return g_Level.Rooms[item->roomNumber].flags & ENV_FLAG_SWAMP;
}

bool TestLaraWater(ITEM_INFO* item)
{
	return g_Level.Rooms[item->roomNumber].flags & ENV_FLAG_WATER;
}
