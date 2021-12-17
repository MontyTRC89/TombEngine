#include "framework.h"
#include "lara.h"
#include "lara_tests.h"
#include "input.h"
#include "level.h"
#include "animation.h"
#include "lara_climb.h"
#include "lara_monkey.h"
#include "lara_collide.h"
#include "control/control.h"
#include "control/los.h"
#include "items.h"
#include "Renderer11.h"
#include "Scripting/GameFlowScript.h"

using namespace TEN::Renderer;
using namespace TEN::Floordata;

/*this file has all the generic test functions called in lara's state code*/

// Test if a ledge in front of item is valid to climb.
bool TestValidLedge(ITEM_INFO* item, COLL_INFO* coll, bool ignoreHeadroom, bool heightLimit)
{
	// Determine probe base left/right points
	int xl = phd_sin(coll->NearestLedgeAngle - ANGLE(90)) * coll->Setup.Radius;
	int zl = phd_cos(coll->NearestLedgeAngle - ANGLE(90)) * coll->Setup.Radius;
	int xr = phd_sin(coll->NearestLedgeAngle + ANGLE(90)) * coll->Setup.Radius;
	int zr = phd_cos(coll->NearestLedgeAngle + ANGLE(90)) * coll->Setup.Radius;

	// Determine probe top point
	int y = item->pos.yPos - coll->Setup.Height;

	// Get frontal collision data
	auto frontLeft  = GetCollisionResult(item->pos.xPos + xl, y, item->pos.zPos + zl, GetRoom(item->location, item->pos.xPos, y, item->pos.zPos).roomNumber);
	auto frontRight = GetCollisionResult(item->pos.xPos + xr, y, item->pos.zPos + zr, GetRoom(item->location, item->pos.xPos, y, item->pos.zPos).roomNumber);

	// If any of the frontal collision results intersects item bounds, return false, because there is material intersection.
	// This check helps to filter out cases when Lara is formally facing corner but ledge check returns true because probe distance is fixed.
	if (frontLeft.Position.Floor < (item->pos.yPos - CLICK(0.5f)) || frontRight.Position.Floor < (item->pos.yPos - CLICK(0.5f)))
		return false;
	if (frontLeft.Position.Ceiling > (item->pos.yPos - coll->Setup.Height) || frontRight.Position.Ceiling > (item->pos.yPos - coll->Setup.Height))
		return false;

	//g_Renderer.addDebugSphere(Vector3(item->pos.xPos + xl, left, item->pos.zPos + zl), 64, Vector4::One, RENDERER_DEBUG_PAGE::LOGIC_STATS);
	//g_Renderer.addDebugSphere(Vector3(item->pos.xPos + xr, right, item->pos.zPos + zr), 64, Vector4::One, RENDERER_DEBUG_PAGE::LOGIC_STATS);
	
	// Determine ledge probe embed offset.
	// We use 0.2f radius extents here for two purposes. First - we can't guarantee that shifts weren't already applied
	// and misfire may occur. Second - it guarantees that Lara won't land on a very thin edge of diagonal geometry.
	int xf = phd_sin(coll->NearestLedgeAngle) * (coll->Setup.Radius * 1.2f);
	int zf = phd_cos(coll->NearestLedgeAngle) * (coll->Setup.Radius * 1.2f);

	// Get floor heights at both points
	auto left = GetCollisionResult(item->pos.xPos + xf + xl, y, item->pos.zPos + zf + zl, GetRoom(item->location, item->pos.xPos, y, item->pos.zPos).roomNumber).Position.Floor;
	auto right = GetCollisionResult(item->pos.xPos + xf + xr, y, item->pos.zPos + zf + zr, GetRoom(item->location, item->pos.xPos, y, item->pos.zPos).roomNumber).Position.Floor;

	// If specified, limit vertical search zone only to nearest height
	if (heightLimit && (abs(left - y) > (STEP_SIZE / 2) || abs(right - y) > (STEP_SIZE / 2)))
		return false;

	// Determine allowed slope difference for a given collision radius
	auto slopeDelta = ((float)STEPUP_HEIGHT / (float)WALL_SIZE) * (coll->Setup.Radius * 2);

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
	return (abs((short) (coll->NearestLedgeAngle - item->pos.yRot)) <= LARA_GRAB_THRESHOLD);
}

bool TestLaraVault(ITEM_INFO* item, COLL_INFO* coll)
{
	if (!(TrInput & IN_ACTION) || Lara.gunStatus != LG_NO_ARMS)
		return false;

	if (TestLaraSwamp(item) && Lara.waterSurfaceDist < -(STOP_SIZE + STEP_SIZE))
		return false;

	if (TestValidLedge(item, coll))
	{
		bool success = false;

		if (coll->Front.Floor < 0 && coll->Front.Floor >= -CLICK(1))
		{
			if (g_GameFlow->Animations.CrawlExtra && (abs(coll->Front.Ceiling - coll->Front.Floor) < CLICK(1)))
			{
				item->animNumber = LA_VAULT_TO_CROUCH_1CLICK;
				item->currentAnimState = LS_GRABBING;
				item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
				item->goalAnimState = LS_CROUCH_IDLE;
				item->pos.yPos += coll->Front.Floor + CLICK(1);
				Lara.gunStatus = LG_HANDS_BUSY;
				success = true;
			}
		}
		else if (coll->Front.Floor >= -CLICK(2.5f) && coll->Front.Floor <= -CLICK(1.5f))
		{
			if (coll->Front.Floor - coll->Front.Ceiling >= 0 &&
				coll->FrontLeft.Floor - coll->FrontLeft.Ceiling >= 0 &&
				coll->FrontRight.Floor - coll->FrontRight.Ceiling >= 0)
			{
				item->animNumber = LA_VAULT_TO_STAND_2CLICK_START;
				item->currentAnimState = LS_GRABBING;
				item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
				item->goalAnimState = LS_STOP;
				item->pos.yPos += coll->Front.Floor + CLICK(2);
				Lara.gunStatus = LG_HANDS_BUSY;
				success = true;
			}
			else if (g_GameFlow->Animations.CrawlExtra && (abs(coll->Front.Ceiling - coll->Front.Floor) < CLICK(1)))
			{
				item->animNumber = LA_VAULT_TO_CROUCH_2CLICK;
				item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
				item->currentAnimState = LS_GRABBING;
				item->goalAnimState = LS_CROUCH_IDLE;
				item->pos.yPos += coll->Front.Floor + CLICK(2);
				Lara.gunStatus = LG_HANDS_BUSY;
				success = true;
			}
		}
		else if (coll->Front.Floor >= -CLICK(3.5f) && coll->Front.Floor <= -CLICK(2.5f))
		{
			if (coll->Front.Floor - coll->Front.Ceiling >= 0 &&
				coll->FrontLeft.Floor - coll->FrontLeft.Ceiling >= 0 &&
				coll->FrontRight.Floor - coll->FrontRight.Ceiling >= 0)
			{
				item->animNumber = LA_VAULT_TO_STAND_3CLICK;
				item->currentAnimState = LS_GRABBING;
				item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
				item->goalAnimState = LS_STOP;
				item->pos.yPos += coll->Front.Floor + CLICK(3);
				Lara.gunStatus = LG_HANDS_BUSY;
				success = true;
			}
			else if (g_GameFlow->Animations.CrawlExtra && (abs(coll->Front.Ceiling - coll->Front.Floor) < CLICK(1)))
			{
				item->animNumber = LA_VAULT_TO_CROUCH_3CLICK;
				item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
				item->currentAnimState = LS_GRABBING;
				item->goalAnimState = LS_CROUCH_IDLE;
				item->pos.yPos += coll->Front.Floor + CLICK(3);
				Lara.gunStatus = LG_HANDS_BUSY;
				success = true;
			}
		}
		else if (coll->Front.Floor >= -CLICK(7.5f) && coll->Front.Floor <= -CLICK(3.5f))
		{
			item->animNumber = LA_STAND_SOLID;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
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

	if (TestValidLedgeAngle(item, coll) && Lara.climbStatus)
	{
		if (coll->Front.Floor > -CLICK(7.5f) || Lara.waterStatus == LW_WADE || coll->FrontLeft.Floor > -CLICK(7.5f) || coll->FrontRight.Floor > -CLICK(8) || coll->Middle.Ceiling > -1158)
		{
			if ((coll->Front.Floor < -CLICK(4) || coll->Front.Ceiling >= 506) && coll->Middle.Ceiling <= -518)
			{
				if (TestLaraClimbStance(item, coll))
				{
					item->animNumber = LA_STAND_SOLID;
					item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
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

		item->animNumber = LA_STAND_SOLID;
		item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
		item->goalAnimState = LS_JUMP_UP;
		item->currentAnimState = LS_STOP;
		Lara.calcFallSpeed = -116;
		Lara.turnRate = 0;
		
		ShiftItem(item, coll);
		SnapItemToGrid(item, coll); // HACK: until fragile ladder code is refactored, we must exactly snap to grid.
		AnimateLara(item);

		return true;
	}

	if (Lara.canMonkeySwing && g_GameFlow->Animations.MonkeyVault)
	{
		short roomNum = item->roomNumber;
		int ceiling = (GetCeiling(GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNum),
			item->pos.xPos, item->pos.yPos, item->pos.zPos))-(item->pos.yPos);

		if (ceiling > 1792 || ceiling < -1792 || abs(ceiling) == 768)
			return false;

		item->animNumber = LA_STAND_IDLE;
		item->frameNumber = g_Level.Anims[LA_STAND_IDLE].frameBase;
		item->goalAnimState = LS_JUMP_UP;
		item->currentAnimState = LS_MONKEY_VAULT;

		return true;
	}

	return false;
}

bool TestLaraStandUp(COLL_INFO* coll)
{
	return (coll->Middle.Ceiling >= -362);
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

	if (TestHangSwingIn(item, angle))
	{
		if (g_GameFlow->Animations.OscillateHanging)
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
	auto angle = Lara.moveAngle;

	auto climbShift = 0;
	if (Lara.moveAngle == (short)(item->pos.yRot - ANGLE(90.0f)))
		climbShift = -coll->Setup.Radius;
	else if (Lara.moveAngle == (short)(item->pos.yRot + ANGLE(90.0f)))
		climbShift = coll->Setup.Radius;

	// Temporarily move item a bit closer to the wall to get more precise coll results
	auto oldPos = item->pos;
	item->pos.xPos += phd_sin(item->pos.yRot) * coll->Setup.Radius * 0.5f;
	item->pos.zPos += phd_cos(item->pos.yRot) * coll->Setup.Radius * 0.5f;

	// Get height difference with side spaces (left or right, depending on movement direction)
	auto hdif = LaraFloorFront(item, Lara.moveAngle, coll->Setup.Radius * 1.4f);

	// Set stopped flag, if floor height is above footspace which is step size
	auto stopped = hdif < STEP_SIZE / 2;

	// Set stopped flag, if ceiling height is below headspace which is step size
	if (LaraCeilingFront(item, Lara.moveAngle, coll->Setup.Radius * 1.5f, 0) > -950)
		stopped = true;

	// Backup item pos to restore it after coll tests
	item->pos = oldPos;

	// Setup coll info
	Lara.moveAngle = item->pos.yRot;
	coll->Setup.BadHeightDown = NO_BAD_POS;
	coll->Setup.BadHeightUp = -STEPUP_HEIGHT;
	coll->Setup.BadCeilingHeight = 0;
	coll->Setup.ForwardAngle = Lara.moveAngle;

	// When Lara is about to move, use larger embed offset for stabilizing diagonal shimmying)
	auto embedOffset = 4;
	if (TrInput & (IN_LEFT | IN_RIGHT))
		embedOffset = 16;

	item->pos.xPos += phd_sin(item->pos.yRot) * embedOffset;
	item->pos.zPos += phd_cos(item->pos.yRot) * embedOffset;

	GetCollisionInfo(coll, item);

	bool result = false;

	if (Lara.climbStatus) // Ladder case
	{
		if (TrInput & IN_ACTION && item->hitPoints > 0)
		{
			Lara.moveAngle = angle;

			if (!TestLaraHangOnClimbWall(item, coll))
			{
				if (item->animNumber != LA_LADDER_TO_HANG_RIGHT &&
					item->animNumber != LA_LADDER_TO_HANG_LEFT)
				{
					LaraSnapToEdgeOfBlock(item, coll, GetQuadrant(item->pos.yRot));
					item->pos.yPos = coll->Setup.OldPosition.y;
					SetAnimation(item, LA_REACH_TO_HANG, 21);
				}

				result = true;
			}
			else
			{
				if (item->animNumber == LA_REACH_TO_HANG && item->frameNumber == GetFrameNumber(item, 21) &&
					TestLaraClimbStance(item, coll))
				{
					item->goalAnimState = LS_LADDER_IDLE;
				}
			}
		}
		else // Death or action release
		{
			SetAnimation(item, LA_FALL_START);
			item->pos.yPos += 256;
			item->gravityStatus = true;
			item->speed = 2;
			item->fallspeed = 1;
			Lara.gunStatus = LG_NO_ARMS;
		}
	}
	else // Normal case
	{
		if (TrInput & IN_ACTION && item->hitPoints > 0 && coll->Front.Floor <= 0)
		{
			if (stopped && hdif > 0 && climbShift != 0 && (climbShift > 0 == coll->MiddleLeft.Floor > coll->MiddleRight.Floor))
				stopped = false;

			auto verticalShift = coll->Front.Floor - GetBoundsAccurate(item)->Y1;
			auto x = item->pos.xPos;
			auto z = item->pos.zPos;

			Lara.moveAngle = angle;

			if (climbShift != 0)
			{
				auto s = phd_sin(Lara.moveAngle);
				auto c = phd_cos(Lara.moveAngle);
				auto testShift = Vector2(s * climbShift, c * climbShift);

				x += testShift.x;
				z += testShift.y;
			}

			if ((256 << GetQuadrant(item->pos.yRot)) & GetClimbFlags(x, item->pos.yPos, z, item->roomNumber))
			{
				if (!TestLaraHangOnClimbWall(item, coll)) 
					verticalShift = 0; // Ignore vertical shift if ladder is encountered next block
			}
			else if (!TestValidLedge(item, coll, true))
			{
				if ((climbShift < 0 && coll->FrontLeft.Floor  != coll->Front.Floor) ||
					(climbShift > 0 && coll->FrontRight.Floor != coll->Front.Floor))
					stopped = true;
			}

			if (!stopped && 
				coll->Middle.Ceiling < 0 && coll->CollisionType == CT_FRONT && !coll->HitStatic && 
				abs(verticalShift) < SLOPE_DIFFERENCE && TestValidLedgeAngle(item, coll))
			{
				if (item->speed != 0)
					SnapItemToLedge(item, coll);

				item->pos.yPos += verticalShift;
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

				result = true;
			}
		}
		else  // Death, incorrect ledge or action release
		{
			SetAnimation(item, LA_JUMP_UP, 9);
			item->pos.xPos += coll->Shift.x;
			item->pos.yPos += GetBoundsAccurate(item)->Y2;
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
	if (item->animNumber != LA_REACH_TO_HANG)
		return CORNER_RESULT::NONE;

	// Static is in the way, bypass test
	if (coll->HitStatic)
		return CORNER_RESULT::NONE;

	// INNER CORNER TESTS

	// Backup old Lara position and frontal collision
	auto oldPos = item->pos;
	auto oldMoveAngle = Lara.moveAngle;

	// Do further testing only if test angle is equal to resulting edge angle
	if (SnapAndTestItemAtNextCornerPosition(item, coll, testAngle, false))
	{
		// Get bounding box height for further ledge height calculations
		auto bounds = GetBoundsAccurate(item);

		// Store next position
		Lara.nextCornerPos.xPos = item->pos.xPos;
		Lara.nextCornerPos.yPos = LaraCollisionAboveFront(item, item->pos.yRot, coll->Setup.Radius * 2, abs(bounds->Y1) + LARA_HEADROOM).Position.Floor + abs(bounds->Y1);
		Lara.nextCornerPos.zPos = item->pos.zPos;
		Lara.nextCornerPos.yRot = item->pos.yRot;
		Lara.moveAngle = item->pos.yRot;
		
		auto result = TestLaraValidHangPos(item, coll);

		// Restore original item positions
		item->pos = oldPos;
		Lara.moveAngle = oldMoveAngle;

		if (result)
			return CORNER_RESULT::INNER;

		if (Lara.climbStatus)
		{
			auto& angleSet = testAngle > 0 ? LeftExtRightIntTab : LeftIntRightExtTab;
			if (GetClimbFlags(Lara.nextCornerPos.xPos, item->pos.yPos, Lara.nextCornerPos.zPos, item->roomNumber) & (short)angleSet[GetQuadrant(item->pos.yRot)])
			{
				Lara.nextCornerPos.yPos = item->pos.yPos; // Restore original Y pos for ladder tests because we don't snap to ledge height in such case.
				return CORNER_RESULT::INNER;
			}
		}
	}

	// Restore original item positions
	item->pos = oldPos;
	Lara.moveAngle = oldMoveAngle;

	// OUTER CORNER TESTS

	// Test if there's a material obstacles blocking outer corner pathway
	if ((LaraFloorFront(item, item->pos.yRot + ANGLE(testAngle), coll->Setup.Radius + STEP_SIZE) < 0) ||
		(LaraCeilingFront(item, item->pos.yRot + ANGLE(testAngle), coll->Setup.Radius + STEP_SIZE, coll->Setup.Height) > 0))
		return CORNER_RESULT::NONE;

	// Last chance for possible diagonal vs. non-diagonal cases: ray test
	if (!LaraPositionOnLOS(item, item->pos.yRot + ANGLE(testAngle), coll->Setup.Radius + STEP_SIZE))
		return CORNER_RESULT::NONE;

	bool snappable = SnapAndTestItemAtNextCornerPosition(item, coll, testAngle, true);

	// Additional test if there's a material obstacles blocking outer corner pathway
	if ((LaraFloorFront(item, item->pos.yRot, 0) < 0) ||
		(LaraCeilingFront(item, item->pos.yRot, 0, coll->Setup.Height) > 0))
		snappable = false;

	// Do further testing only if test angle is equal to resulting edge angle
	if (snappable)
	{
		// Get bounding box height for further ledge height calculations
		auto bounds = GetBoundsAccurate(item);

		// Store next position
		Lara.nextCornerPos.xPos = item->pos.xPos;
		Lara.nextCornerPos.yPos = LaraCollisionAboveFront(item, item->pos.yRot, coll->Setup.Radius * 2, abs(bounds->Y1) + LARA_HEADROOM).Position.Floor + abs(bounds->Y1);
		Lara.nextCornerPos.zPos = item->pos.zPos;
		Lara.nextCornerPos.yRot = item->pos.yRot;
		Lara.moveAngle = item->pos.yRot;

		auto result = TestLaraValidHangPos(item, coll);

		// Restore original item positions
		item->pos = oldPos;
		Lara.moveAngle = oldMoveAngle;

		if (result)
			return CORNER_RESULT::OUTER;

		if (Lara.climbStatus)
		{
			auto& angleSet = testAngle > 0 ? LeftIntRightExtTab : LeftExtRightIntTab;
			if (GetClimbFlags(Lara.nextCornerPos.xPos, item->pos.yPos, Lara.nextCornerPos.zPos, item->roomNumber) & (short)angleSet[GetQuadrant(item->pos.yRot)])
			{
				Lara.nextCornerPos.yPos = item->pos.yPos; // Restore original Y pos for ladder tests because we don't snap to ledge height in such case.
				return CORNER_RESULT::OUTER;
			}
		}
	}

	// Restore original item positions
	item->pos = oldPos;
	Lara.moveAngle = oldMoveAngle;

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
	coll->Setup.BadHeightUp = -CLICK(2);
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

	if (LaraTestClimbPos(item, coll->Setup.Radius, coll->Setup.Radius + CLICK(0.5f), -700, STOP_SIZE, &shift_r) != 1)
		return false;

	if (LaraTestClimbPos(item, coll->Setup.Radius, -(coll->Setup.Radius + CLICK(0.5f)), -700, STOP_SIZE, &shift_l) != 1)
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

	z += phd_cos(angle) * (STEP_SIZE / 2);
	x += phd_sin(angle) * (STEP_SIZE / 2);

	floor = GetFloor(x, y, z, &roomNum);
	floorHeight = GetFloorHeight(floor, x, y, z);
	ceilingHeight = GetCeiling(floor, x, y, z);

	if (floorHeight != NO_HEIGHT)
	{
		if (g_GameFlow->Animations.OscillateHanging)
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

bool TestLaraHangSideways(ITEM_INFO* item, COLL_INFO* coll, short angle)
{
	auto oldPos = item->pos;

	Lara.moveAngle = item->pos.yRot + angle;

	static constexpr auto sidewayTestDistance = 16;
	item->pos.xPos += phd_sin(Lara.moveAngle) * sidewayTestDistance;
	item->pos.zPos += phd_cos(Lara.moveAngle) * sidewayTestDistance;

	coll->Setup.OldPosition.y = item->pos.yPos;

	auto res = TestLaraHang(item, coll);

	item->pos = oldPos;

	return !res;
}

bool LaraFacingCorner(ITEM_INFO* item, short ang, int dist)
{
	auto angle1 = ang + ANGLE(15);
	auto angle2 = ang - ANGLE(15);

	auto vec1 = GAME_VECTOR(item->pos.xPos + dist * phd_sin(angle1),
							item->pos.yPos - (LARA_HEIGHT / 2),
							item->pos.zPos + dist * phd_cos(angle1), 
							item->roomNumber);

	auto vec2 = GAME_VECTOR(item->pos.xPos + dist * phd_sin(angle2),
							item->pos.yPos - (LARA_HEIGHT / 2),
							item->pos.zPos + dist * phd_cos(angle2),
							item->roomNumber);

	auto pos  = GAME_VECTOR(item->pos.xPos,
							item->pos.yPos - (LARA_HEIGHT / 2),
							item->pos.zPos,
							item->roomNumber);

	auto result1 = LOS(&pos, &vec1);
	auto result2 = LOS(&pos, &vec2);

	return ((result1 == 0) && (result2 == 0));
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
	auto collResult = GetCollisionResult(item, ang, dist, -LARA_HEIGHT);

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
	auto collResult = GetCollisionResult(item, ang, dist, -h);

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
		item->goalAnimState = LS_STOP;
	}

	item->pos.yPos += coll->Middle.Floor + (STOP_SIZE + STEP_SIZE / 2 + STEP_SIZE / 4 - 9);

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
	if (coll->CollisionType != CT_FRONT || !(TrInput & IN_ACTION))
		return false;

	if (Lara.gunStatus &&
		(Lara.gunStatus != LG_READY || Lara.gunType != WEAPON_FLARE))
	{
		return false;
	}

	if (coll->Middle.Ceiling > -STEPUP_HEIGHT)
		return false;

	int frontFloor = coll->Front.Floor + LARA_HEIGHT_SURFSWIM;
	if (frontFloor <= -STOP_SIZE ||
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
			if (g_GameFlow->Animations.CrawlExtra)
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
			if (g_GameFlow->Animations.CrawlExtra)
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
			if (g_GameFlow->Animations.CrawlExtra)
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
	Lara.gunStatus = LG_HANDS_BUSY;
	Lara.waterStatus = LW_ABOVE_WATER;

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

bool TestLaraPoleCollision(ITEM_INFO* item, COLL_INFO* coll, bool up, float offset)
{
	static constexpr auto poleProbeCollRadius = 16.0f;

	bool atLeastOnePoleCollided = false;

	if (GetCollidedObjects(item, WALL_SIZE, true, CollidedItems, nullptr, 0) && CollidedItems[0])
	{
		auto laraBox = TO_DX_BBOX(item->pos, GetBoundsAccurate(item));

		// HACK: because Core implemented upward pole movement as SetPosition command, we can't precisely
		// check her position. So we add a fixed height offset.

		auto sphere = BoundingSphere(laraBox.Center + Vector3(0, (laraBox.Extents.y + poleProbeCollRadius + offset) * (up ? -1 : 1) , 0), poleProbeCollRadius);

		//g_Renderer.addDebugSphere(sphere.Center, 16.0f, Vector4(1, 0, 0, 1), RENDERER_DEBUG_PAGE::LOGIC_STATS);

		int i = 0;
		while (CollidedItems[i] != NULL)
		{
			auto& obj = CollidedItems[i];
			i++;

			if (obj->objectNumber != ID_POLEROPE)
				continue;

			auto poleBox = TO_DX_BBOX(obj->pos, GetBoundsAccurate(obj));
			poleBox.Extents = poleBox.Extents + Vector3(coll->Setup.Radius, 0, coll->Setup.Radius);

			//g_Renderer.addDebugBox(poleBox, Vector4(0, 0, 1, 1), RENDERER_DEBUG_PAGE::LOGIC_STATS);

			if (poleBox.Intersects(sphere))
			{
				atLeastOnePoleCollided = true;
				break;
			}
		}
	}

	return atLeastOnePoleCollided;
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
	else if (waterDepth <= LARA_HEIGHT - LARA_HEADROOM / 2)
	{
		SetAnimation(item, LA_UNDERWATER_TO_STAND);
		item->goalAnimState = LS_STOP;
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
	if(LaraItem->hitPoints <= 0 || LaraItem->hitStatus)
	{
		SetAnimation(LaraItem, LA_TIGHTROPE_FALL_LEFT);
	}

	if(!Lara.tightRopeFall && !(GetRandomControl() & regularity))
		Lara.tightRopeFall = 2 - ((GetRandomControl() & 0xF) != 0);
}
#endif // !NEW_TIGHTROPE

bool TestLaraLean(ITEM_INFO* item, COLL_INFO* coll)
{
#if 0
	// TODO: make it more fine-tuned when new collision is done.
	switch (coll->CollisionType)
	{
	case CT_RIGHT:
		if (TrInput & IN_RIGHT)
			return false;
	case CT_LEFT:
		if (TrInput & IN_LEFT)
			return false;
	}
	return true;
#else
	if (coll->CollisionType == CT_RIGHT || coll->CollisionType == CT_LEFT)
		return false;

	return true;
#endif
}

bool TestLaraSwamp(ITEM_INFO* item)
{
	return (g_Level.Rooms[item->roomNumber].flags & ENV_FLAG_SWAMP) != 0;
}

bool TestLaraWater(ITEM_INFO* item)
{
	return (g_Level.Rooms[item->roomNumber].flags & ENV_FLAG_WATER) != 0;
}
