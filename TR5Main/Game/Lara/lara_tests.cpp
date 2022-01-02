#include "framework.h"
#include "Game/Lara/lara_tests.h"

#include "Game/animation.h"
#include "Game/collision/collide_room.h"
#include "Game/collision/collide_item.h"
#include "Game/control/control.h"
#include "Game/control/los.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_climb.h"
#include "Game/Lara/lara_collide.h"
#include "Game/Lara/lara_flare.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/Lara/lara_monkey.h"
#include "Renderer/Renderer11.h"
#include "Scripting/GameFlowScript.h"
#include "Specific/input.h"
#include "Specific/level.h"

using namespace TEN::Renderer;
using namespace TEN::Floordata;

// -----------------------------
// TEST FUNCTIONS
// For State Control & Collision
// -----------------------------

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
	if (heightLimit && (abs(left - y) > CLICK(0.5f) || abs(right - y) > CLICK(0.5f)))
		return false;

	// Determine allowed slope difference for a given collision radius
	auto slopeDelta = ((float)STEPUP_HEIGHT / (float)WALL_SIZE) * (coll->Setup.Radius * 2);

	// Discard if there is a slope beyond tolerance delta
	if (abs(left - right) >= slopeDelta)
		return false;

	// Discard if ledge is not within distance threshold
	if (abs(coll->NearestLedgeDistance) > coll->Setup.Radius * sqrt(2) + 4)
		return false;

	// Discard if ledge is not within angle threshold
	if (!TestValidLedgeAngle(item, coll))
		return false;
	
	if (!ignoreHeadroom)
	{
		auto headroom = (coll->Front.Floor + coll->Setup.Height) - coll->Middle.Ceiling;
		if (headroom < CLICK(1))
			return false;
	}
	
	return true;
}

bool TestValidLedgeAngle(ITEM_INFO* item, COLL_INFO* coll)
{
	return abs((short) (coll->NearestLedgeAngle - item->pos.yRot)) <= LARA_GRAB_THRESHOLD;
}

bool TestLaraVault(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	if (!(TrInput & IN_ACTION) ||
		info->gunStatus != LG_HANDS_FREE)
	{
		return false;
	}

	if (TestLaraSwamp(item) && info->waterSurfaceDist < -CLICK(3))
		return false;
	
	if (TestValidLedge(item, coll))
	{
		bool success = false;

		// Vault to crouch up one step.
		if (TestLaraVault1StepToCrouch(item, coll))
		{
			item->animNumber = LA_VAULT_TO_CROUCH_1CLICK;
			item->currentAnimState = LS_GRABBING;
			item->frameNumber = GetFrameNumber(item, 0);
			item->goalAnimState = LS_CROUCH_IDLE;
			info->gunStatus = LG_HANDS_BUSY;
			success = true;
		}
		
		// Vault to stand up two steps.
		else if (TestLaraVault2Steps(item, coll))
		{
			item->animNumber = LA_VAULT_TO_STAND_2CLICK_START;
			item->currentAnimState = LS_GRABBING;
			item->frameNumber = GetFrameNumber(item, 0);
			item->goalAnimState = LS_IDLE;
			info->gunStatus = LG_HANDS_BUSY;
			success = true;
		}
		// Vault to crouch up two steps.
		else if (TestLaraVault2StepsToCrouch(item, coll) &&
			g_GameFlow->Animations.CrawlExtended)
		{
			item->animNumber = LA_VAULT_TO_CROUCH_2CLICK;
			item->frameNumber = GetFrameNumber(item, 0);
			item->currentAnimState = LS_GRABBING;
			item->goalAnimState = LS_CROUCH_IDLE;
			info->gunStatus = LG_HANDS_BUSY;
			success = true;
		}

		// Vault to stand up three steps.
		else if (TestLaraVault3Steps(item, coll))
		{
			item->animNumber = LA_VAULT_TO_STAND_3CLICK;
			item->currentAnimState = LS_GRABBING;
			item->frameNumber = GetFrameNumber(item, 0);
			item->goalAnimState = LS_IDLE;
			info->gunStatus = LG_HANDS_BUSY;
			success = true;
		}
		// Vault to crouch up three steps.
		else if (TestLaraVault3StepsToCrouch(item, coll) &&
				g_GameFlow->Animations.CrawlExtended)
		{
			item->animNumber = LA_VAULT_TO_CROUCH_3CLICK;
			item->frameNumber = GetFrameNumber(item, 0);
			item->currentAnimState = LS_GRABBING;
			item->goalAnimState = LS_CROUCH_IDLE;
			info->gunStatus = LG_HANDS_BUSY;
			success = true;
		}

		// Auto jump.
		else if (TestLaraAutoJump(item, coll))
		{
			item->animNumber = LA_STAND_SOLID;
			item->frameNumber = GetFrameNumber(item, 0);
			item->goalAnimState = LS_JUMP_UP;
			item->currentAnimState = LS_IDLE;
			AnimateLara(item);
			success = true;
		}

		if (success)
		{
			SnapItemToLedge(item, coll, 0.2f);
			return true;
		}
	}

	int y = item->pos.yPos;
	auto probeFront = GetCollisionResult(item, coll->NearestLedgeAngle, coll->Setup.Radius * sqrt(2) + 4, -coll->Setup.Height);
	auto probeFrontLeft = GetCollisionResult(item, coll->NearestLedgeAngle, coll->Setup.Radius * sqrt(2) + 4, -coll->Setup.Height, -coll->Setup.Radius);
	auto probeFrontRight = GetCollisionResult(item, coll->NearestLedgeAngle, coll->Setup.Radius * sqrt(2) + 4, -coll->Setup.Height, coll->Setup.Radius);

	// Begin ladder climb.
	// TODO: Broken. Lara will always perform a ladder mount.
	if (info->climbStatus && TestValidLedgeAngle(item, coll))
	{
		if ((probeFront.Position.Floor - y) > -CLICK(7.5f) ||			// Upper front floor bound.
			(probeFrontLeft.Position.Floor - y) > -CLICK(7.5f) ||		// Upper left floor bound.
			(probeFrontRight.Position.Floor - y) > -CLICK(2) ||			// Upper right floor bound.
			coll->Middle.Ceiling > -CLICK(4.5f) + 6 ||					// Upper ceiling bound.
			info->waterStatus == LW_WADE)
		{
			if (((probeFront.Position.Floor - y) < -CLICK(4) || probeFront.Position.Ceiling >= -coll->Setup.Height - CLICK(2) - 6) &&
				coll->Middle.Ceiling <= -(CLICK(2) + 6))
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
	if (TestLaraMonkeyAutoJump(item, coll) &&
		g_GameFlow->Animations.MonkeyAutoJump)
	{
		item->animNumber = LA_STAND_IDLE;
		item->frameNumber = GetFrameNumber(item, 0);
		item->goalAnimState = LS_JUMP_UP;
		item->currentAnimState = LS_MONKEY_VAULT;
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
	auto probeBack = GetCollisionResult(item, item->pos.yRot + ANGLE(180.0f), radius, -coll->Setup.Height);

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
	LaraInfo*& info = item->data;

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

	info->moveAngle = angle;
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

	auto floor = GetFloor(x, y, z, &roomNum);
	auto floorHeight = GetFloorHeight(floor, x, y, z);
	auto ceilHeight = GetCeiling(floor, x, y, z);

	if (floorHeight == NO_HEIGHT)
		return SPLAT_COLL::WALL;

	if (y >= floorHeight || y <= ceilHeight)
		return SPLAT_COLL::STEP;

	return SPLAT_COLL::NONE;
}

bool TestLaraHangJumpUp(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	if (!(TrInput & IN_ACTION) || (info->gunStatus != LG_HANDS_FREE) || (coll->HitStatic))
		return false;

	if (info->canMonkeySwing && coll->CollisionType == CT_TOP)
	{
		SetAnimation(item, LA_JUMP_UP_TO_MONKEYSWING);
		item->gravityStatus = false;
		item->speed = 0;
		item->fallspeed = 0;

		info->gunStatus = LG_HANDS_BUSY;

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

	info->gunStatus = LG_HANDS_BUSY;
	info->torsoYrot = 0;
	info->torsoXrot = 0;

	return true;
}

bool TestLaraHangJump(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	if (!(TrInput & IN_ACTION) || (info->gunStatus != LG_HANDS_FREE) || (coll->HitStatic))
		return false;

	if (info->canMonkeySwing && coll->CollisionType == CT_TOP)
	{
		ResetLaraFlex(item);
		info->gunStatus = LG_HANDS_BUSY;

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
		if (g_GameFlow->Animations.OscillateHang)
		{
			ResetLaraFlex(item);
			SetAnimation(item, LA_REACH_TO_HANG_OSCILLATE);
		}
		else
		{
			ResetLaraFlex(item);
			SetAnimation(item, LA_REACH_TO_MONKEYSWING);
		}
	}
	else
		SetAnimation(item, LA_REACH_TO_HANG);

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

	info->gunStatus = LG_HANDS_BUSY;

	return true;
}

bool TestLaraHang(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	auto angle = info->moveAngle;

	auto climbShift = 0;
	if (info->moveAngle == (short)(item->pos.yRot - ANGLE(90.0f)))
		climbShift = -coll->Setup.Radius;
	else if (info->moveAngle == (short)(item->pos.yRot + ANGLE(90.0f)))
		climbShift = coll->Setup.Radius;

	// Temporarily move item a bit closer to the wall to get more precise coll results
	auto oldPos = item->pos;
	item->pos.xPos += phd_sin(item->pos.yRot) * coll->Setup.Radius * 0.5f;
	item->pos.zPos += phd_cos(item->pos.yRot) * coll->Setup.Radius * 0.5f;

	// Get height difference with side spaces (left or right, depending on movement direction)
	auto hdif = LaraFloorFront(item, info->moveAngle, coll->Setup.Radius * 1.4f);

	// Set stopped flag, if floor height is above footspace which is step size
	auto stopped = hdif < CLICK(0.5f);

	// Set stopped flag, if ceiling height is below headspace which is step size
	if (LaraCeilingFront(item, info->moveAngle, coll->Setup.Radius * 1.5f, 0) > -950)
		stopped = true;

	// Backup item pos to restore it after coll tests
	item->pos = oldPos;

	// Setup coll info
	info->moveAngle = item->pos.yRot;
	coll->Setup.BadHeightDown = NO_BAD_POS;
	coll->Setup.BadHeightUp = -STEPUP_HEIGHT;
	coll->Setup.BadCeilingHeight = 0;
	coll->Setup.ForwardAngle = info->moveAngle;

	// When Lara is about to move, use larger embed offset for stabilizing diagonal shimmying)
	auto embedOffset = 4;
	if (TrInput & (IN_LEFT | IN_RIGHT))
		embedOffset = 16;

	item->pos.xPos += phd_sin(item->pos.yRot) * embedOffset;
	item->pos.zPos += phd_cos(item->pos.yRot) * embedOffset;

	GetCollisionInfo(coll, item);

	bool result = false;

	if (info->climbStatus) // Ladder case
	{
		if (TrInput & IN_ACTION && item->hitPoints > 0)
		{
			info->moveAngle = angle;

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
			info->gunStatus = LG_HANDS_FREE;
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

			info->moveAngle = angle;

			if (climbShift != 0)
			{
				auto s = phd_sin(info->moveAngle);
				auto c = phd_cos(info->moveAngle);
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
			item->pos.yPos += GetBoundsAccurate(item)->Y2 * 1.8f;
			item->pos.zPos += coll->Shift.z;
			item->gravityStatus = true;
			item->speed = 2;
			item->fallspeed = 1;
			info->gunStatus = LG_HANDS_FREE;
		}
	}

	return result;
}

CORNER_RESULT TestLaraHangCorner(ITEM_INFO* item, COLL_INFO* coll, float testAngle)
{
	LaraInfo*& info = item->data;

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
		info->nextCornerPos.xPos = item->pos.xPos;
		info->nextCornerPos.yPos = LaraCollisionAboveFront(item, item->pos.yRot, coll->Setup.Radius * 2, abs(bounds->Y1) + LARA_HEADROOM).Position.Floor + abs(bounds->Y1);
		info->nextCornerPos.zPos = item->pos.zPos;
		info->nextCornerPos.yRot = item->pos.yRot;
		info->moveAngle = item->pos.yRot;
		
		auto result = TestLaraValidHangPos(item, coll);

		// Restore original item positions
		item->pos = oldPos;
		info->moveAngle = oldMoveAngle;

		if (result)
			return CORNER_RESULT::INNER;

		if (info->climbStatus)
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
	info->moveAngle = oldMoveAngle;

	// OUTER CORNER TESTS

	// Test if there's a material obstacles blocking outer corner pathway
	if ((LaraFloorFront(item, item->pos.yRot + ANGLE(testAngle), coll->Setup.Radius + CLICK(1)) < 0) ||
		(LaraCeilingFront(item, item->pos.yRot + ANGLE(testAngle), coll->Setup.Radius + CLICK(1), coll->Setup.Height) > 0))
		return CORNER_RESULT::NONE;

	// Last chance for possible diagonal vs. non-diagonal cases: ray test
	if (!LaraPositionOnLOS(item, item->pos.yRot + ANGLE(testAngle), coll->Setup.Radius + CLICK(1)))
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
		info->nextCornerPos.xPos = item->pos.xPos;
		info->nextCornerPos.yPos = LaraCollisionAboveFront(item, item->pos.yRot, coll->Setup.Radius * 2, abs(bounds->Y1) + LARA_HEADROOM).Position.Floor + abs(bounds->Y1);
		info->nextCornerPos.zPos = item->pos.zPos;
		info->nextCornerPos.yRot = item->pos.yRot;
		info->moveAngle = item->pos.yRot;

		auto result = TestLaraValidHangPos(item, coll);

		// Restore original item positions
		item->pos = oldPos;
		info->moveAngle = oldMoveAngle;

		if (result)
			return CORNER_RESULT::OUTER;

		if (info->climbStatus)
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
	info->moveAngle = oldMoveAngle;

	return CORNER_RESULT::NONE;
}

bool TestLaraValidHangPos(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	// Get incoming ledge height and own Lara's upper bound.
	// First one will be negative while first one is positive.
	// Difference between two indicates difference in height between ledges.
	auto frontFloor = LaraCollisionAboveFront(item, info->moveAngle, coll->Setup.Radius + CLICK(0.5f), LARA_HEIGHT).Position.Floor;
	auto laraUpperBound = item->pos.yPos - coll->Setup.Height;

	// If difference is above 1/2 click, return false (ledge is out of reach).
	if (abs(frontFloor - laraUpperBound) > CLICK(0.5f))
 		return false;

	// Embed Lara into wall to make collision test succeed
	item->pos.xPos += phd_sin(item->pos.yRot) * 8;
	item->pos.zPos += phd_cos(item->pos.yRot) * 8;

	// Setup new GCI call
	info->moveAngle = item->pos.yRot;
	coll->Setup.BadHeightDown = NO_BAD_POS;
	coll->Setup.BadHeightUp = -CLICK(2);
	coll->Setup.BadCeilingHeight = 0;
	coll->Setup.Mode = COLL_PROBE_MODE::FREE_FLAT;
	coll->Setup.ForwardAngle = info->moveAngle;

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

	if (LaraTestClimbPos(item, coll->Setup.Radius, coll->Setup.Radius + CLICK(0.5f), -700, CLICK(2), &shift_r) != 1)
		return false;

	if (LaraTestClimbPos(item, coll->Setup.Radius, -(coll->Setup.Radius + CLICK(0.5f)), -700, CLICK(2), &shift_l) != 1)
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
	LaraInfo*& info = item->data;
	int shift, result;

	if (info->climbStatus == 0)
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

	if (info->moveAngle != item->pos.yRot)
	{
		short l = LaraCeilingFront(item, item->pos.yRot, 0, 0);
		short r = LaraCeilingFront(item, info->moveAngle, CLICK(0.5f), 0);

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
	LaraInfo*& info = item->data;
	int x = item->pos.xPos;
	int y = item->pos.yPos;
	int z = item->pos.zPos;
	short roomNum = item->roomNumber;
	FLOOR_INFO* floor;
	int floorHeight, ceilingHeight;

	z += phd_cos(angle) * CLICK(0.5f);
	x += phd_sin(angle) * CLICK(0.5f);

	floor = GetFloor(x, y, z, &roomNum);
	floorHeight = GetFloorHeight(floor, x, y, z);
	ceilingHeight = GetCeiling(floor, x, y, z);

	if (floorHeight != NO_HEIGHT)
	{
		if (g_GameFlow->Animations.OscillateHang)
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
	LaraInfo*& info = item->data;
	auto oldPos = item->pos;

	Lara.moveAngle = item->pos.yRot + angle;

	static constexpr auto sidewayTestDistance = 16;
	item->pos.xPos += phd_sin(info->moveAngle) * sidewayTestDistance;
	item->pos.zPos += phd_cos(info->moveAngle) * sidewayTestDistance;

	coll->Setup.OldPosition.y = item->pos.yPos;

	auto res = TestLaraHang(item, coll);

	item->pos = oldPos;

	return !res;
}

bool TestLaraStandingJump(ITEM_INFO* item, COLL_INFO* coll, short angle)
{
	auto y = item->pos.yPos;
	auto probe = GetCollisionResult(item, angle, CLICK(1), -coll->Setup.Height);
	
	// TODO: Ceiling test interfered with bridges. For now, behaves as original.
	if (!TestLaraFacingCorner(item, angle, CLICK(1)) &&
		probe.Position.Floor - y >= -STEPUP_HEIGHT &&									// Highest floor bound.
		probe.Position.Floor != NO_HEIGHT)
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
	LaraInfo*& info = item->data;

	if (coll->CollisionType == CT_FRONT ||
		coll->Middle.Slope ||
		coll->Middle.Floor >= 0)
	{
		return false;
	}

	if (coll->Middle.Floor >= -CLICK(0.5f))
	{
		SetAnimation(item, LA_STAND_IDLE);
	}
	else
	{
		SetAnimation(item, LA_ONWATER_TO_WADE_1CLICK);
		item->goalAnimState = LS_IDLE;
	}

	item->pos.yPos += coll->Middle.Floor + CLICK(2.75f) - 9;

	UpdateItemRoom(item, -(STEPUP_HEIGHT - 3));

	item->pos.zRot = 0;
	item->pos.xRot = 0;
	item->gravityStatus = false;
	item->speed = 0;
	item->fallspeed = 0;
	info->waterStatus = LW_WADE;

	return true;
}

bool TestLaraWaterClimbOut(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	if (coll->CollisionType != CT_FRONT || !(TrInput & IN_ACTION))
		return false;

	if (info->gunStatus &&
		(info->gunStatus != LG_READY || info->gunType != WEAPON_FLARE))
	{
		return false;
	}

	if (coll->Middle.Ceiling > -STEPUP_HEIGHT)
		return false;

	int frontFloor = coll->Front.Floor + LARA_HEIGHT_SURFSWIM;
	if (frontFloor <= -CLICK(2) ||
		frontFloor > CLICK(1.25f) - 4)
	{
		return false;
	}

	if (!TestValidLedge(item, coll))
		return false;

	auto surface = LaraCollisionAboveFront(item, coll->Setup.ForwardAngle, CLICK(2), CLICK(1));
	auto headroom = surface.Position.Floor - surface.Position.Ceiling;

	if (frontFloor <= -CLICK(1))
	{
		if (headroom < LARA_HEIGHT)
		{
			if (g_GameFlow->Animations.CrawlExtended)
				SetAnimation(item, LA_ONWATER_TO_CROUCH_1CLICK);
			else
				return false;
		}
		else
			SetAnimation(item, LA_ONWATER_TO_STAND_1CLICK);
	}
	else if (frontFloor > CLICK(0.5f))
	{
		if (headroom < LARA_HEIGHT)
		{
			if (g_GameFlow->Animations.CrawlExtended)
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
			if (g_GameFlow->Animations.CrawlExtended)
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
	LaraInfo*& info = item->data;

	if (!(TrInput & IN_ACTION) ||
		!info->climbStatus ||
		coll->CollisionType != CT_FRONT)
	{
		return false;
	}

	if (info->gunStatus &&
		(info->gunStatus != LG_READY || info->gunType != WEAPON_FLARE))
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
	info->gunStatus = LG_HANDS_BUSY;
	info->waterStatus = LW_ABOVE_WATER;

	return true;
}

void TestLaraWaterDepth(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

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
	// Height check was at CLICK(2) before but changed to this 
	// because now Lara surfaces on a head level, not mid-body level.
	else if (waterDepth <= LARA_HEIGHT - LARA_HEADROOM / 2)
	{
		SetAnimation(item, LA_UNDERWATER_TO_STAND);
		item->goalAnimState = LS_IDLE;
		item->pos.zRot = 0;
		item->pos.xRot = 0;
		item->speed = 0;
		item->fallspeed = 0;
		item->gravityStatus = false;
		info->waterStatus = LW_WADE;
		item->pos.yPos = GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);
	}
}

#ifndef NEW_TIGHTROPE
void GetTighRopeFallOff(int regularity) {
	if (LaraItem->hitPoints <= 0 || LaraItem->hitStatus)
		SetAnimation(LaraItem, LA_TIGHTROPE_FALL_LEFT);

	if (!info->tightRopeFall && !(GetRandomControl() & regularity))
		info->tightRopeFall = 2 - ((GetRandomControl() & 0xF) != 0);
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
	if (coll->Middle.Floor < -CLICK(0.5f) &&				// Lower floor bound.
		coll->Middle.Floor >= -STEPUP_HEIGHT &&				// Upper floor bound.
		item->currentAnimState != LS_CRAWL_IDLE &&			// Crawl step up handled differently.
		item->currentAnimState != LS_CRAWL_FORWARD)
	{
		return true;
	}

	return false;
}

bool TestLaraStepDown(ITEM_INFO* item, COLL_INFO* coll)
{
	if (coll->Middle.Floor > CLICK(0.5f) &&				// Upper floor bound.
		coll->Middle.Floor <= STEPUP_HEIGHT &&			// Lower floor bound.
		item->currentAnimState != LS_CRAWL_IDLE &&		// Crawl step down handled differently.
		item->currentAnimState != LS_CRAWL_FORWARD)
	{
		return true;
	}

	return false;
}

bool TestLaraVaultTolerance(ITEM_INFO* item, COLL_INFO* coll, int lowerBound, int upperBound, int clampMin, int clampMax, int gapMin, int snapHeight, bool checkSwampDepth)
{
	LaraInfo*& info = item->data;

	int y = item->pos.yPos;
	auto probeFront = GetCollisionResult(item, coll->NearestLedgeAngle, coll->Setup.Radius * sqrt(2) + 4, -coll->Setup.Height);
	auto probeMiddle = GetCollisionResult(item);
	bool swampTooDeep = checkSwampDepth ? (TestLaraSwamp(item) && info->waterSurfaceDist < -CLICK(3)) : TestLaraSwamp(item);

	// "Floor" ahead may be formed by ceiling; raise y position of probe point to find potential vault candidate location.
	int yOffset = lowerBound;  // May fail?
	while (((probeFront.Position.Floor - y) > 0 || abs(probeFront.Position.Ceiling - probeFront.Position.Floor) <= clampMin) &&
			yOffset > upperBound)
	{
		probeFront = GetCollisionResult(item, coll->NearestLedgeAngle, coll->Setup.Radius * sqrt(2) + 4, yOffset - coll->Setup.Height);
		yOffset -= CLICK(0.5f);
	}
	
	// Assess vault candidate location.
	if ((probeFront.Position.Floor - y) < lowerBound &&									// Lower floor bound.
		(probeFront.Position.Floor - y) >= upperBound &&								// Upper floor bound.
		abs(probeFront.Position.Ceiling - probeFront.Position.Floor) > clampMin &&		// Lower clamp limit.
		abs(probeFront.Position.Ceiling - probeFront.Position.Floor) <= clampMax &&		// Upper clamp limit.
		abs(probeMiddle.Position.Ceiling - probeFront.Position.Floor) >= gapMin &&		// Gap is optically permissive.
		!swampTooDeep &&																// Swamp depth is permissive.
		probeFront.Position.Floor != NO_HEIGHT)
	{
		// TODO: Command query separation please.
		// Calculate auto jump velocity.
		if (snapHeight == 0)
			info->calcFallSpeed = -3 - sqrt(-9600 - 12 * (probeFront.Position.Floor - y));
		// Snap y position to align vault animation.
		else
			item->pos.yPos = probeFront.Position.Floor + snapHeight;

		return true;
	}

	return false;
}

bool TestLaraVault2Steps(ITEM_INFO* item, COLL_INFO* coll)
{
	return TestLaraVaultTolerance(item, coll, -STEPUP_HEIGHT, -CLICK(2.5f), LARA_HEIGHT, -MAX_HEIGHT, CLICK(1), CLICK(2));		// Floor range: (-STEPUP_HEIGHT, -CLICK(2.5f)]
}

bool TestLaraVault3Steps(ITEM_INFO* item, COLL_INFO* coll)
{
	return TestLaraVaultTolerance(item, coll, -CLICK(2.5f), -CLICK(3.5f), LARA_HEIGHT, -MAX_HEIGHT, CLICK(1), CLICK(3));		// Floor range: (-CLICK(2.5f), -CLICK(3.5f)]
}

bool TestLaraAutoJump(ITEM_INFO* item, COLL_INFO* coll)
{
	return TestLaraVaultTolerance(item, coll, -CLICK(3.5f), -CLICK(7.5f), CLICK(0.1f) /* TODO: How much hand room?*/, -MAX_HEIGHT, CLICK(0.1f), false);		// Floor range: (-CLICK(3.5f), -CLICK(7.5f)]
}

bool TestLaraVault1StepToCrouch(ITEM_INFO* item, COLL_INFO* coll)
{
	return TestLaraVaultTolerance(item, coll, 0, -STEPUP_HEIGHT, LARA_HEIGHT_CRAWL, LARA_HEIGHT, CLICK(1), CLICK(1));		// Floor range: (0, -STEPUP_HEIGHT]
}

bool TestLaraVault2StepsToCrouch(ITEM_INFO* item, COLL_INFO* coll)
{
	return TestLaraVaultTolerance(item, coll, -STEPUP_HEIGHT, -CLICK(2.5f), LARA_HEIGHT_CRAWL, LARA_HEIGHT, CLICK(1), CLICK(2));		// Floor range: (-STEPUP_HEIGHT, -CLICK(2.5f)]
}

bool TestLaraVault3StepsToCrouch(ITEM_INFO* item, COLL_INFO* coll)
{
	return TestLaraVaultTolerance(item, coll, -CLICK(2.5f), -CLICK(3.5f), LARA_HEIGHT_CRAWL, LARA_HEIGHT, CLICK(1), CLICK(3));		// Floor range: (-CLICK(2.5f), -CLICK(3.5f)]
}

bool TestLaraMonkeyAutoJump(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	int y = item->pos.yPos;
	auto probe = GetCollisionResult(item);

	if (info->canMonkeySwing &&										// Monkey swing sector flag set. (TODO: Is canMonkeySwing member necessary? Could simply check for sector flag here.)
		(probe.Position.Ceiling - y) < -LARA_HEIGHT_STRETCH &&		// Lower ceiling bound.
		(probe.Position.Ceiling - y) >= -CLICK(7) &&				// Upper ceiling bound.
		!TestLaraSwamp(item))										// No swamp.
	{
		return true;
	}

	return false;
}

// TODO: This function and its clone below should become obsolete with more accurate and accessible collision detection in the future.
// For now, it supercedes old probes and is used alongside COLL_INFO. @Sezz 2021.10.24
bool TestLaraMove(ITEM_INFO* item, COLL_INFO* coll, short angle, int lowerBound, int upperBound, bool checkSlopeDown, bool checkSlopeUp, bool checkDeath)
{
	int y = item->pos.yPos;
	auto probe = GetCollisionResult(item, angle, coll->Setup.Radius * sqrt(2) + 4, -coll->Setup.Height);		// Offset required to account for gap between Lara and the wall. Results in slight overshoot, but avoids oscillation.
	bool isSlopeDown = checkSlopeDown ? (probe.Position.Slope && probe.Position.Floor >= y) : false;
	bool isSlopeUp = checkSlopeUp ? (probe.Position.Slope && probe.Position.Floor < y) : false;
	bool isDeath = checkDeath ? probe.Block->Flags.Death : false;

	if ((probe.Position.Floor - y) <= lowerBound &&										// Lower floor bound.
		(probe.Position.Floor - y) >= upperBound &&										// Upper floor bound.
		(probe.Position.Ceiling - y) < -coll->Setup.Height &&							// Lowest ceiling bound.
		abs(probe.Position.Ceiling - probe.Position.Floor) > coll->Setup.Height &&		// Space is not a clamp.
		!isSlopeDown && !isSlopeUp && !isDeath &&										// No slope or death sector (if applicable).
		probe.Position.Floor != NO_HEIGHT)
	{
		return true;
	}

	return false;
}

// HACK: coll->Setup.Radius and coll->Setup.Height are only set
// in COLLISION functions, then reset by LaraAboveWater() to defaults.
// This means they will store the wrong values for tests called in crawl CONTROL functions.
// When states become objects, collision setup should occur at the beginning of each state, eliminating the need
// for this clone function. @Sezz 2021.12.05
bool TestLaraMoveCrawl(ITEM_INFO* item, COLL_INFO* coll, short angle, int lowerBound, int upperBound, bool checkSlope, bool checkDeath)
{
	int y = item->pos.yPos;
	auto probe = GetCollisionResult(item, angle, LARA_RAD_CRAWL * sqrt(2) + 4, -LARA_HEIGHT_CRAWL);		// Offset required to account for gap between Lara and the wall. Results in slight overshoot, but avoids oscillation.
	bool isSlope = checkSlope ? probe.Position.Slope : false;
	bool isDeath = checkDeath ? probe.Block->Flags.Death : false;

	if ((probe.Position.Floor - y) <= lowerBound &&										// Lower floor bound.
		(probe.Position.Floor - y) >= upperBound &&										// Upper floor bound.
		(probe.Position.Ceiling - y) < -LARA_HEIGHT_CRAWL &&							// Lowest ceiling bound.
		abs(probe.Position.Ceiling - probe.Position.Floor) > LARA_HEIGHT_CRAWL &&		// Space is not a clamp.
		!isSlope && !isDeath &&															// No slope or death sector (if applicable).
		probe.Position.Floor != NO_HEIGHT)
	{
		return true;
	}

	return false;
}

bool TestLaraRunForward(ITEM_INFO* item, COLL_INFO* coll)
{
	return TestLaraMove(item, coll, item->pos.yRot, NO_BAD_POS, -STEPUP_HEIGHT, false, true, false);		// Using BadHeightUp/Down defined in walk and run state collision functions.
}

bool TestLaraWalkForward(ITEM_INFO* item, COLL_INFO* coll)
{
	return TestLaraMove(item, coll, item->pos.yRot, STEPUP_HEIGHT, -STEPUP_HEIGHT);		// Using BadHeightUp/Down defined in walk and run state collision functions.
}

bool TestLaraWalkBack(ITEM_INFO* item, COLL_INFO* coll)
{
	return TestLaraMove(item, coll, item->pos.yRot + ANGLE(180.0f), STEPUP_HEIGHT, -STEPUP_HEIGHT);		// Using BadHeightUp/Down defined in walk back state collision function.
}

bool TestLaraRunBack(ITEM_INFO* item, COLL_INFO* coll)
{
	return TestLaraMove(item, coll, item->pos.yRot + ANGLE(180.0f), NO_BAD_POS, -STEPUP_HEIGHT, false, false, false);		// Using BadHeightUp/Down defined in hop back state collision function.
}

bool TestLaraStepLeft(ITEM_INFO* item, COLL_INFO* coll)
{
	return TestLaraMove(item, coll, item->pos.yRot - ANGLE(90.0f), CLICK(0.8f), -CLICK(0.8f));		// Using BadHeightUp/Down defined in step left state collision function.
}

bool TestLaraStepRight(ITEM_INFO* item, COLL_INFO* coll)
{
	return TestLaraMove(item, coll, item->pos.yRot + ANGLE(90.0f), CLICK(0.8f), -CLICK(0.8f));		// Using BadHeightUp/Down defined in step right state collision function.
}

bool TestLaraWadeForwardSwamp(ITEM_INFO* item, COLL_INFO* coll)
{
	return TestLaraMove(item, coll, item->pos.yRot, NO_BAD_POS, -STEPUP_HEIGHT, false, false, false);		// Using BadHeightUp/Down defined in wade forward state collision function.
}

bool TestLaraWalkBackSwamp(ITEM_INFO* item, COLL_INFO* coll)
{
	return TestLaraMove(item, coll, item->pos.yRot + ANGLE(180.0f), NO_BAD_POS, -STEPUP_HEIGHT, false, false, false);		// Using BadHeightUp defined in walk back state collision function.
}

bool TestLaraStepLeftSwamp(ITEM_INFO* item, COLL_INFO* coll)
{
	return TestLaraMove(item, coll, item->pos.yRot - ANGLE(90.0f), NO_BAD_POS, -CLICK(0.8f), false, false, false);			// Using BadHeightUp defined in step left state collision function.
}

bool TestLaraStepRightSwamp(ITEM_INFO* item, COLL_INFO* coll)
{
	return TestLaraMove(item, coll, item->pos.yRot + ANGLE(90.0f), NO_BAD_POS, -CLICK(0.8f), false, false, false);			// Using BadHeightUp defined in step right state collision function.
}

bool TestLaraCrawlForward(ITEM_INFO* item, COLL_INFO* coll)
{
	return TestLaraMoveCrawl(item, coll, item->pos.yRot, CLICK(1) - 1, -(CLICK(1) - 1));				// Using BadHeightUp/Down defined in crawl state collision functions.
}

bool TestLaraCrawlBack(ITEM_INFO* item, COLL_INFO* coll)
{
	return TestLaraMoveCrawl(item, coll, item->pos.yRot + ANGLE(180.0f), CLICK(1) - 1, -(CLICK(1) - 1));		// Using BadHeightUp/Down defined in crawl state collision functions.
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

	int y = item->pos.yPos;
	auto probe = GetCollisionResult(item, item->pos.yRot, WALL_SIZE / 2, -LARA_HEIGHT_CRAWL);

	if ((probe.Position.Floor - y) <= (CLICK(1) - 1) &&				// Lower floor bound.
		(probe.Position.Floor - y) >= -(CLICK(1) - 1) &&			// Upper floor bound.
		(probe.Position.Ceiling - y) < -LARA_HEIGHT_CRAWL &&		// Lowest ceiling bound.
		!probe.Position.Slope &&									// Not a slope.
		info->waterSurfaceDist >= -CLICK(1) &&						// Water depth is optically permissive.
		!(TrInput & (IN_FLARE | IN_DRAW)) &&						// Avoid unsightly concurrent actions.
		(info->gunType != WEAPON_FLARE || info->flareAge > 0))		// Not handling flare.
	{
		return true;
	}

	return false;
}

bool TestLaraCrawlStepTolerance(ITEM_INFO* item, COLL_INFO* coll, int lowerBound, int upperBound, int clampMin, int gapMin, int crossDist, int destDist, int probeDeltaMax, bool checkSlope, bool checkDeath)
{
	int y = item->pos.yPos;
	auto probeA = GetCollisionResult(item, item->pos.yRot, crossDist, -LARA_HEIGHT_CRAWL);		// Crossing.
	auto probeB = GetCollisionResult(item, item->pos.yRot, destDist, -LARA_HEIGHT_CRAWL);		// Approximate destination.
	auto probeMiddle = GetCollisionResult(item);
	bool isSlope = checkSlope ? probeB.Position.Slope : false;
	bool isDeath = checkDeath ? probeB.Block->Flags.Death : false;

	if ((probeA.Position.Floor - y) <= lowerBound &&								// Lower floor bound.
		(probeA.Position.Floor - y) >= upperBound &&								// Upper floor bound.
		abs(probeA.Position.Ceiling - probeA.Position.Floor) > clampMin &&			// Crossing clamp limit.
		abs(probeB.Position.Ceiling - probeB.Position.Floor) > clampMin &&			// Destination clamp limit.
		abs(probeMiddle.Position.Ceiling - probeA.Position.Floor) >= gapMin &&		// Gap is optically permissive (going up).
		abs(probeA.Position.Ceiling - probeMiddle.Position.Floor) >= gapMin &&		// Gap is optically permissive (going down).
		abs(probeA.Position.Floor - probeB.Position.Floor) <= probeDeltaMax &&		// Destination height is within crawl states' BadHeightUp/Down threshold.
		!isSlope && !isDeath &&														// No slope or death sector.
		probeA.Position.Floor != NO_HEIGHT && probeB.Position.Floor != NO_HEIGHT)
	{
		return true;
	}

	return false;
}

bool TestLaraCrawlUpStep(ITEM_INFO* item, COLL_INFO* coll)
{
	return TestLaraCrawlStepTolerance(item, coll, -CLICK(1), -STEPUP_HEIGHT, LARA_HEIGHT_CRAWL, CLICK(0.6f), CLICK(1), CLICK(2), CLICK(1) - 1);		// Floor range: [-CLICK(1), -STEPUP_HEIGHT]
}

bool TestLaraCrawlDownStep(ITEM_INFO* item, COLL_INFO* coll)
{
	return TestLaraCrawlStepTolerance(item, coll, STEPUP_HEIGHT, CLICK(1), LARA_HEIGHT_CRAWL, CLICK(0.6f), CLICK(1), CLICK(2), CLICK(1) - 1);		// Floor range: [STEPUP_HEIGHT, CLICK(1)]
}

bool TestLaraCrawlExitDownStep(ITEM_INFO* item, COLL_INFO* coll)
{
	return TestLaraCrawlStepTolerance(item, coll, STEPUP_HEIGHT, CLICK(1), LARA_HEIGHT, CLICK(1.25f), CLICK(1), CLICK(1.5f), -MAX_HEIGHT, false);		// Floor range: [STEPUP_HEIGHT, CLICK(1)]
}

bool TestLaraCrawlExitJump(ITEM_INFO* item, COLL_INFO* coll)
{
	return TestLaraCrawlStepTolerance(item, coll, -MAX_HEIGHT, STEPUP_HEIGHT + 1, LARA_HEIGHT, CLICK(1.25f), CLICK(1), CLICK(1.5f), -MAX_HEIGHT, false);		// Floor range: [-MAX_HEIGHT, STEPUP_HEIGHT)
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
	int y = item->pos.yPos;
	auto probe = GetCollisionResult(item, item->pos.yRot + ANGLE(180.0f), CLICK(1), -LARA_HEIGHT_CRAWL);
	bool objectCollided = TestLaraObjectCollision(item, item->pos.yRot + ANGLE(180.0f), CLICK(1), -LARA_HEIGHT_CRAWL);

	if (!objectCollided &&											// No obstruction.
		(probe.Position.Floor - y) >= LARA_HEIGHT_STRETCH &&		// Highest floor bound.
		(probe.Position.Ceiling - y) <= -CLICK(0.75f) &&			// Gap is optically permissive.
		probe.Position.Floor != NO_HEIGHT)
	{
		return true;
	}

	return false;
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

		auto sphere = BoundingSphere(laraBox.Center + Vector3(0, (laraBox.Extents.y + poleProbeCollRadius + offset) * (up ? -1 : 1), 0), poleProbeCollRadius);

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

bool TestLaraPoleUp(ITEM_INFO* item, COLL_INFO* coll)
{
	if (!TestLaraPoleCollision(item, coll, true, CLICK(1)))
		return false;

	// TODO: Accuracy.
	return (coll->Middle.Ceiling < -CLICK(1));
}

bool TestLaraPoleDown(ITEM_INFO* item, COLL_INFO* coll)
{
	if (!TestLaraPoleCollision(item, coll, false))
		return false;

	return (coll->Middle.Floor > 0);
}
