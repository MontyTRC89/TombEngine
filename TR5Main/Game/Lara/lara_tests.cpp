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

// TODO: Move to lara_test_structs.h after merge of Sezz vaults branch
struct CornerTestResult
{
	bool Success;
	PHD_3DPOS ProbeResult;
	PHD_3DPOS RealPositionResult;
};

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

	if (!(TrInput & IN_ACTION) || !(TrInput & IN_FORWARD) ||
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
		auto vaultResult = TestLaraVault1StepToCrouch(item, coll);
		if (vaultResult.success && !success)
		{
			item->pos.yPos = vaultResult.height + CLICK(1);
			item->animNumber = LA_VAULT_TO_CROUCH_1CLICK;
			item->currentAnimState = LS_GRABBING;
			item->frameNumber = GetFrameNumber(item, 0);
			item->goalAnimState = LS_CROUCH_IDLE;
			info->gunStatus = LG_HANDS_BUSY;
			success = true;
		}
		
		// Vault to stand up two steps.
		vaultResult = TestLaraVault2Steps(item, coll);
		if (vaultResult.success && !success)
		{
			item->pos.yPos = vaultResult.height + CLICK(2);
			item->animNumber = LA_VAULT_TO_STAND_2CLICK_START;
			item->currentAnimState = LS_GRABBING;
			item->frameNumber = GetFrameNumber(item, 0);
			item->goalAnimState = LS_IDLE;
			info->gunStatus = LG_HANDS_BUSY;
			success = true;
		}
		// Vault to crouch up two steps.
		vaultResult = TestLaraVault2StepsToCrouch(item, coll);
		if (vaultResult.success && !success &&
			g_GameFlow->Animations.CrawlExtended)
		{
			item->pos.yPos = vaultResult.height + CLICK(2);
			item->animNumber = LA_VAULT_TO_CROUCH_2CLICK;
			item->frameNumber = GetFrameNumber(item, 0);
			item->currentAnimState = LS_GRABBING;
			item->goalAnimState = LS_CROUCH_IDLE;
			info->gunStatus = LG_HANDS_BUSY;
			success = true;
		}

		// Vault to stand up three steps.
		vaultResult = TestLaraVault3Steps(item, coll);
		if (vaultResult.success && !success)
		{
			item->pos.yPos = vaultResult.height + CLICK(3);
			item->animNumber = LA_VAULT_TO_STAND_3CLICK;
			item->currentAnimState = LS_GRABBING;
			item->frameNumber = GetFrameNumber(item, 0);
			item->goalAnimState = LS_IDLE;
			info->gunStatus = LG_HANDS_BUSY;
			success = true;
		}
		// Vault to crouch up three steps.
		vaultResult = TestLaraVault3StepsToCrouch(item, coll);
		if (vaultResult.success && !success &&
			g_GameFlow->Animations.CrawlExtended)
		{
			item->pos.yPos = vaultResult.height + CLICK(3);
			item->animNumber = LA_VAULT_TO_CROUCH_3CLICK;
			item->frameNumber = GetFrameNumber(item, 0);
			item->currentAnimState = LS_GRABBING;
			item->goalAnimState = LS_CROUCH_IDLE;
			info->gunStatus = LG_HANDS_BUSY;
			success = true;
		}

		// Auto jump to hang.
		vaultResult = TestLaraVaultAutoJump(item, coll);
		if (vaultResult.success && !success)
		{
			info->calcFallSpeed = -3 - sqrt(-9600 - 12 * (vaultResult.height - item->pos.yPos));
			item->animNumber = LA_STAND_SOLID;
			item->frameNumber = GetFrameNumber(item, 0);
			item->goalAnimState = LS_JUMP_UP;
			item->currentAnimState = LS_IDLE;
			AnimateLara(item);
			success = true;
		}

		if (success)
		{
			info->turnRate = 0;
			SnapItemToLedge(item, coll, 0.2f);
			return true;
		}
	}
	
	// Auto jump to ladder.
	auto ladderAutoJumpResult = TestLaraLadderAutoJump(item, coll);
	if (ladderAutoJumpResult.success)
	{
		info->calcFallSpeed = -3 - sqrt(-9600 - 12 * std::max((ladderAutoJumpResult.height - item->pos.yPos + CLICK(0.2f)), -CLICK(7.1f)));
		item->animNumber = LA_STAND_SOLID;
		item->frameNumber = GetFrameNumber(item, 0);
		item->goalAnimState = LS_JUMP_UP;
		item->currentAnimState = LS_IDLE;
		info->gunStatus = LG_HANDS_BUSY;
		info->turnRate = 0;

		ShiftItem(item, coll);
		SnapItemToGrid(item, coll); // HACK: until fragile ladder code is refactored, we must exactly snap to grid.
		AnimateLara(item);

		return true;
	}
	// Mount ladder.
	else if (TestLaraLadderMount(item, coll) &&
		TestLaraClimbStance(item, coll))
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

bool TestLaraKeepLow(ITEM_INFO* item, COLL_INFO* coll)
{
	// TODO: Temporary. coll->Setup.Radius is currently only set to
	// LARA_RAD_CRAWL in the collision function, then reset by LaraAboveWater().
	// For tests called in control functions, then, it will store the wrong radius. @Sezz 2021.11.05
	auto radius = (item->currentAnimState == LS_CROUCH_IDLE ||
		item->currentAnimState == LS_CROUCH_TURN_LEFT ||
		item->currentAnimState == LS_CROUCH_TURN_RIGHT)
		? LARA_RAD : LARA_RAD_CRAWL;

	auto y = item->pos.yPos;
	auto probeFront = GetCollisionResult(item, item->pos.yRot, radius, -coll->Setup.Height);
	auto probeBack = GetCollisionResult(item, item->pos.yRot + ANGLE(180.0f), radius, -coll->Setup.Height);
	auto probeMiddle = GetCollisionResult(item);

	// TODO: Cannot use as a failsafe in standing states; bugged with slanted ceilings reaching the ground.
	// In common setups, Lara may embed on such ceilings, resulting in inappropriate crouch state dispatches. @Sezz 2021.10.15
	if ((probeFront.Position.Ceiling - y) >= -LARA_HEIGHT ||		// Front is not a clamp.
		(probeBack.Position.Ceiling - y) >= -LARA_HEIGHT ||			// Back is not a clamp.
		(probeMiddle.Position.Ceiling - y) >= -LARA_HEIGHT)			// Middle is not a clamp.
	{
		return true;
	}

	return false;
}

bool TestLaraSlide(ITEM_INFO* item, COLL_INFO* coll)
{
	if ((abs(coll->TiltX) > 2 || abs(coll->TiltZ) > 2) &&
		!TestLaraSwamp(item))
	{
		return true;
	}

	return false;
}

bool TestLaraSwamp(ITEM_INFO* item)
{
	return (g_Level.Rooms[item->roomNumber].flags & ENV_FLAG_SWAMP);
}

bool TestLaraWater(ITEM_INFO* item)
{
	return (g_Level.Rooms[item->roomNumber].flags & ENV_FLAG_WATER);
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

		DoLaraMonkeySnap(item, coll);

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

		SetAnimation(item, LA_REACH_TO_HANG, 12);

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

	if (!(TrInput & IN_ACTION) || info->gunStatus != LG_HANDS_FREE || coll->HitStatic)
		return false;

	if (info->canMonkeySwing && coll->CollisionType == CT_TOP)
	{
		SetAnimation(item, LA_REACH_TO_MONKEYSWING);
		ResetLaraFlex(item);
		info->gunStatus = LG_HANDS_BUSY;
		item->gravityStatus = false;
		item->speed = 0;
		item->fallspeed = 0;

		return true;
	}

	if (coll->Middle.Ceiling > -STEPUP_HEIGHT ||
		coll->Middle.Floor < 200 ||
		coll->CollisionType != CT_FRONT)
	{
		return false;
	}

	int edge;
	auto edgeCatch = TestLaraEdgeCatch(item, coll, &edge);
	if (!edgeCatch)
		return false;

	bool ladder = TestLaraHangOnClimbWall(item, coll);

	if (!(ladder && edgeCatch) &&
		!(TestValidLedge(item, coll, true, true) && edgeCatch > 0))
	{
		return false;
	}

	if (TestHangSwingIn(item))
	{
		SetAnimation(item, LA_REACH_TO_HANG_OSCILLATE);
		ResetLaraFlex(item);
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

	info->gunStatus = LG_HANDS_BUSY;
	item->gravityStatus = true;
	item->speed = 2;
	item->fallspeed = 1;

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
	coll->Setup.BadFloorHeightDown = NO_BAD_POS;
	coll->Setup.BadFloorHeightUp = -STEPUP_HEIGHT;
	coll->Setup.BadCeilingHeightDown = 0;
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

CornerTestResult TestItemAtNextCornerPosition(ITEM_INFO* item, COLL_INFO* coll, float angle, bool outer)
{
	auto result = CornerTestResult();

	// Determine real turning angle
	auto turnAngle = outer ? angle : -angle;

	// Backup previous position into array
	PHD_3DPOS pos[3] = { item->pos, item->pos, item->pos };

	// Do a two-step rotation check. First step is real resulting position, and second step is probing
	// position. We need this because checking at exact ending position does not always return
	// correct results with nearest ledge angle.

	for (int i = 0; i < 2; i++)
	{
		// Determine collision box anchor point and rotate collision box around this anchor point.
		// Then determine new test position from centerpoint of new collision box position.

		// Push back item a bit to compensate for possible edge ledge cases
		pos[i].xPos -= round((coll->Setup.Radius * (outer ? -0.2f : 0.2f)) * phd_sin(pos[i].yRot));
		pos[i].zPos -= round((coll->Setup.Radius * (outer ? -0.2f : 0.2f)) * phd_cos(pos[i].yRot));

		// Move item at the distance of full collision diameter plus half-radius margin to movement direction 
		pos[i].xPos += round((coll->Setup.Radius * (i == 0 ? 2.0f : 2.5f)) * phd_sin(Lara.moveAngle));
		pos[i].zPos += round((coll->Setup.Radius * (i == 0 ? 2.0f : 2.5f)) * phd_cos(Lara.moveAngle));

		// Determine anchor point
		auto cX = pos[i].xPos + round(coll->Setup.Radius * phd_sin(pos[i].yRot));
		auto cZ = pos[i].zPos + round(coll->Setup.Radius * phd_cos(pos[i].yRot));
		cX += (coll->Setup.Radius * phd_sin(pos[i].yRot + ANGLE(90.0f * -std::copysign(1.0f, angle))));
		cZ += (coll->Setup.Radius * phd_cos(pos[i].yRot + ANGLE(90.0f * -std::copysign(1.0f, angle))));

		// Determine distance from anchor point to new item position
		auto dist = Vector2(pos[i].xPos, pos[i].zPos) - Vector2(cX, cZ);
		auto s = phd_sin(ANGLE(turnAngle));
		auto c = phd_cos(ANGLE(turnAngle));

		// Shift item to a new anchor point
		pos[i].xPos = dist.x * c - dist.y * s + cX;
		pos[i].zPos = dist.x * s + dist.y * c + cZ;

		// Virtually rotate item to new angle
		short newAngle = pos[i].yRot - ANGLE(turnAngle);
		pos[i].yRot = newAngle;

		// Snap to nearest ledge, if any.
		item->pos = pos[i];
		SnapItemToLedge(item, coll, item->pos.yRot);

		// Copy resulting position to an array and restore original item position.
		pos[i] = item->pos;
		item->pos = pos[2];

		if (i == 1) // Both passes finished, construct the result.
		{
			result.RealPositionResult = pos[0];
			result.ProbeResult = pos[1];
			result.Success = newAngle == pos[i].yRot;
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

	auto cornerResult = TestItemAtNextCornerPosition(item, coll, testAngle, false);

	// Do further testing only if test angle is equal to resulting edge angle
	if (cornerResult.Success)
	{
		// Get bounding box height for further ledge height calculations
		auto bounds = GetBoundsAccurate(item);

		// Store next position
		item->pos = cornerResult.RealPositionResult;
		info->nextCornerPos.xPos = item->pos.xPos;
		info->nextCornerPos.yPos = LaraCollisionAboveFront(item, item->pos.yRot, coll->Setup.Radius * 2, abs(bounds->Y1) + LARA_HEADROOM).Position.Floor + abs(bounds->Y1);
		info->nextCornerPos.zPos = item->pos.zPos;
		info->nextCornerPos.yRot = item->pos.yRot;
		info->moveAngle = item->pos.yRot;
		
		item->pos = cornerResult.ProbeResult;
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

	cornerResult = TestItemAtNextCornerPosition(item, coll, testAngle, true);

	// Additional test if there's a material obstacles blocking outer corner pathway
	if ((LaraFloorFront(item, item->pos.yRot, 0) < 0) ||
		(LaraCeilingFront(item, item->pos.yRot, 0, coll->Setup.Height) > 0))
		cornerResult.Success = false;

	// Do further testing only if test angle is equal to resulting edge angle
	if (cornerResult.Success)
	{
		// Get bounding box height for further ledge height calculations
		auto bounds = GetBoundsAccurate(item);

		// Store next position
		item->pos = cornerResult.RealPositionResult;
		info->nextCornerPos.xPos = item->pos.xPos;
		info->nextCornerPos.yPos = LaraCollisionAboveFront(item, item->pos.yRot, coll->Setup.Radius * 2, abs(bounds->Y1) + LARA_HEADROOM).Position.Floor + abs(bounds->Y1);
		info->nextCornerPos.zPos = item->pos.zPos;
		info->nextCornerPos.yRot = item->pos.yRot;
		info->moveAngle = item->pos.yRot;

		item->pos = cornerResult.ProbeResult;
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
	coll->Setup.BadFloorHeightDown = NO_BAD_POS;
	coll->Setup.BadFloorHeightUp = -CLICK(2);
	coll->Setup.BadCeilingHeightDown = 0;
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
	int shiftRight, shiftLeft;

	if (LaraTestClimbPos(item, coll->Setup.Radius, coll->Setup.Radius + CLICK(0.5f), -700, CLICK(2), &shiftRight) != 1)
		return false;

	if (LaraTestClimbPos(item, coll->Setup.Radius, -(coll->Setup.Radius + CLICK(0.5f)), -700, CLICK(2), &shiftLeft) != 1)
		return false;

	if (shiftRight)
	{
		if (shiftLeft)
		{
			if (shiftRight < 0 != shiftLeft < 0)
				return false;

			if ((shiftRight < 0 && shiftLeft < shiftRight) ||
				(shiftRight > 0 && shiftLeft > shiftRight))
			{
				item->pos.yPos += shiftLeft;
				return true;
			}
		}

		item->pos.yPos += shiftRight;
	}
	else if (shiftLeft)
		item->pos.yPos += shiftLeft;

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

bool TestHangSwingIn(ITEM_INFO* item)
{
	LaraInfo*& info = item->data;

	int y = item->pos.yPos;
	auto probe = GetCollisionResult(item, item->pos.yRot, CLICK(0.5f));

	if ((probe.Position.Floor - y) > 0 &&
		(probe.Position.Ceiling - y) < -400 &&
		probe.Position.Floor != NO_HEIGHT)
	{
		return true;
	}

	return false;
}

bool TestLaraHangSideways(ITEM_INFO* item, COLL_INFO* coll, short angle)
{
	LaraInfo*& info = item->data;

	auto oldPos = item->pos;

	info->moveAngle = item->pos.yRot + angle;

	static constexpr auto sidewayTestDistance = 16;
	item->pos.xPos += phd_sin(info->moveAngle) * sidewayTestDistance;
	item->pos.zPos += phd_cos(info->moveAngle) * sidewayTestDistance;

	coll->Setup.OldPosition.y = item->pos.yPos;

	bool res = TestLaraHang(item, coll);

	item->pos = oldPos;

	return !res;
}

bool TestLaraFacingCorner(ITEM_INFO* item, short angle, int dist)
{
	short angle1 = angle + ANGLE(15.0f);
	short angle2 = angle - ANGLE(15.0f);

	auto start = GAME_VECTOR(item->pos.xPos,
		item->pos.yPos - STEPUP_HEIGHT,
		item->pos.zPos,
		item->roomNumber);

	auto end1 = GAME_VECTOR(item->pos.xPos + dist * phd_sin(angle1),
		item->pos.yPos - STEPUP_HEIGHT,
		item->pos.zPos + dist * phd_cos(angle1),
		item->roomNumber);

	auto end2 = GAME_VECTOR(item->pos.xPos + dist * phd_sin(angle2),
		item->pos.yPos - STEPUP_HEIGHT,
		item->pos.zPos + dist * phd_cos(angle2),
		item->roomNumber);

	bool result1 = LOS(&start, &end1);
	bool result2 = LOS(&start, &end2);

	return (!result1 && !result2);
}

bool TestLaraSplat(ITEM_INFO* item, int dist, int height, int side)
{
	auto start = GAME_VECTOR(
		item->pos.xPos + (phd_cos(item->pos.yRot) * side),
		item->pos.yPos + height,
		item->pos.zPos + (phd_sin(item->pos.yRot) * -side),
		item->roomNumber);

	auto end = GAME_VECTOR(
		item->pos.xPos + (phd_sin(item->pos.yRot) * dist) + (phd_cos(item->pos.yRot) * side),
		item->pos.yPos + height,
		item->pos.zPos + (phd_cos(item->pos.yRot) * dist) + (phd_sin(item->pos.yRot) * -side),
		item->roomNumber);

	return !LOS(&start, &end);
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

bool TestLaraMonkeyFall(ITEM_INFO* item, COLL_INFO* coll)
{
	int y = item->pos.yPos - LARA_HEIGHT_MONKEY;
	auto probe = GetCollisionResult(item);

	if (!probe.BottomBlock->Flags.Monkeyswing ||			// Monkey swing sector not set.
		(probe.Position.Ceiling - y) > CLICK(1.25f) ||		// Lower bound.
		(probe.Position.Ceiling - y) < -CLICK(1.25f) ||		// Upper bound.
		probe.Position.Ceiling == NO_HEIGHT)
	{
		return true;
	}

	return false;
}

bool TestLaraLand(ITEM_INFO* item, COLL_INFO* coll)
{
	if (item->gravityStatus &&
		item->fallspeed >= 0 &&
		(coll->Middle.Floor <= 0 || TestLaraSwamp(item)))
	{
		return true;
	}

	return false;
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

bool TestLaraMonkeyStep(ITEM_INFO* item, COLL_INFO* coll)
{
	int y = item->pos.yPos - LARA_HEIGHT_MONKEY;
	auto probe = GetCollisionResult(item);

	if ((probe.Position.Ceiling - y) <= CLICK(1.25f) ||			// Lower bound.
		(probe.Position.Ceiling - y) >= -CLICK(1.25f) ||		// Upper bound.
		probe.Position.Ceiling == NO_HEIGHT)
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

// TODO: This function and its clone TestLaraCrawlMoveTolerance() should become obsolete with more accurate and accessible collision detection in the future.
// For now, it supercedes old probes and is used alongside COLL_INFO. @Sezz 2021.10.24
bool TestLaraMoveTolerance(ITEM_INFO* item, COLL_INFO* coll, MoveTestData testData)
{
	int y = item->pos.yPos;
	auto probe = GetCollisionResult(item, testData.angle, coll->Setup.Radius * sqrt(2) + 4, -coll->Setup.Height);		// Offset required to account for gap between Lara and the wall. Results in slight overshoot, but avoids oscillation.
	bool isSlopeDown = testData.checkSlopeDown ? (probe.Position.Slope && probe.Position.Floor > y) : false;
	bool isSlopeUp = testData.checkSlopeUp ? (probe.Position.Slope && probe.Position.Floor < y) : false;
	bool isDeath = testData.checkDeath ? probe.Block->Flags.Death : false;

	if ((probe.Position.Floor - y) <= testData.lowerBound &&							// Lower floor bound.
		(probe.Position.Floor - y) >= testData.upperBound &&							// Upper floor bound.
		(probe.Position.Ceiling - y) < -coll->Setup.Height &&							// Lowest ceiling bound.
		abs(probe.Position.Ceiling - probe.Position.Floor) > coll->Setup.Height &&		// Space is not a clamp.
		!isSlopeDown && !isSlopeUp && !isDeath &&										// No slope or death sector (if applicable).
		probe.Position.Floor != NO_HEIGHT)
	{
		return true;
	}

	return false;
}

bool TestLaraRunForward(ITEM_INFO* item, COLL_INFO* coll)
{
	// Using BadFloorHeightUp/Down defined in run state collision function.

	MoveTestData testData
	{
		item->pos.yRot,
		NO_BAD_POS,
		-STEPUP_HEIGHT,
		false, true, false
	};

	return TestLaraMoveTolerance(item, coll, testData);
}

bool TestLaraWalkForward(ITEM_INFO* item, COLL_INFO* coll)
{
	// Using BadFloorHeightUp/Down defined in walk state collision function.

	MoveTestData testData
	{
		item->pos.yRot,
		STEPUP_HEIGHT,
		-STEPUP_HEIGHT
	};

	return TestLaraMoveTolerance(item, coll, testData);
}

bool TestLaraWalkBack(ITEM_INFO* item, COLL_INFO* coll)
{
	// Using BadFloorHeightUp/Down defined in walk back state collision function.

	MoveTestData testData
	{
		item->pos.yRot + ANGLE(180.0f),
		STEPUP_HEIGHT,
		-STEPUP_HEIGHT
	};

	return TestLaraMoveTolerance(item, coll, testData);	
}

bool TestLaraRunBack(ITEM_INFO* item, COLL_INFO* coll)
{
	// Using BadFloorHeightUp/Down defined in hop back state collision function.

	MoveTestData testData
	{
		item->pos.yRot + ANGLE(180.0f),
		NO_BAD_POS, -STEPUP_HEIGHT,
		false, false, false
	};

	return TestLaraMoveTolerance(item, coll, testData);
}

bool TestLaraStepLeft(ITEM_INFO* item, COLL_INFO* coll)
{
	// Using BadFloorHeightUp/Down defined in step left state collision function.

	MoveTestData testData
	{
		item->pos.yRot - ANGLE(90.0f),
		CLICK(0.8f),
		-CLICK(0.8f)
	};

	return TestLaraMoveTolerance(item, coll, testData);
}

bool TestLaraStepRight(ITEM_INFO* item, COLL_INFO* coll)
{
	// Using BadFloorHeightUp/Down defined in step right state collision function.

	MoveTestData testData
	{
		item->pos.yRot + ANGLE(90.0f),
		CLICK(0.8f),
		-CLICK(0.8f)
	};

	return TestLaraMoveTolerance(item, coll, testData);
}

bool TestLaraWadeForwardSwamp(ITEM_INFO* item, COLL_INFO* coll)
{
	// Using BadFloorHeightUp/Down defined in wade forward state collision function.

	MoveTestData testData
	{
		item->pos.yRot,
		NO_BAD_POS,
		-STEPUP_HEIGHT,
		false, false, false
	};

	return TestLaraMoveTolerance(item, coll, testData);
}

bool TestLaraWalkBackSwamp(ITEM_INFO* item, COLL_INFO* coll)
{
	// Using BadFloorHeightUp defined in walk back state collision function.

	MoveTestData testData
	{
		item->pos.yRot + ANGLE(180.0f),
		NO_BAD_POS,
		-STEPUP_HEIGHT,
		false, false, false
	};

	return TestLaraMoveTolerance(item, coll, testData);
}

bool TestLaraStepLeftSwamp(ITEM_INFO* item, COLL_INFO* coll)
{
	// Using BadFloorHeightUp defined in step left state collision function.

	MoveTestData testData
	{
		item->pos.yRot - ANGLE(90.0f),
		NO_BAD_POS,
		-CLICK(0.8f),
		false, false, false
	};

	return TestLaraMoveTolerance(item, coll, testData);
}

bool TestLaraStepRightSwamp(ITEM_INFO* item, COLL_INFO* coll)
{
	// Using BadFloorHeightUp defined in step right state collision function.

	MoveTestData testData
	{
		item->pos.yRot + ANGLE(90.0f),
		NO_BAD_POS,
		-CLICK(0.8f),
		false, false, false
	};

	return TestLaraMoveTolerance(item, coll, testData);
}

// HACK: coll->Setup.Radius and coll->Setup.Height are only set
// in COLLISION functions, then reset by LaraAboveWater() to defaults.
// This means they will store the wrong values for tests called in crawl CONTROL functions.
// When states become objects, collision setup should occur at the beginning of each state, eliminating the need
// for this clone function. @Sezz 2021.12.05
bool TestLaraCrawlMoveTolerance(ITEM_INFO* item, COLL_INFO* coll, MoveTestData testData)
{
	int y = item->pos.yPos;
	auto probe = GetCollisionResult(item, testData.angle, LARA_RAD_CRAWL * sqrt(2) + 4, -LARA_HEIGHT_CRAWL);
	bool isSlopeDown = testData.checkSlopeDown ? (probe.Position.Slope && probe.Position.Floor > y) : false;
	bool isSlopeUp = testData.checkSlopeUp ? (probe.Position.Slope && probe.Position.Floor < y) : false;
	bool isDeath = testData.checkDeath ? probe.Block->Flags.Death : false;

	if ((probe.Position.Floor - y) <= testData.lowerBound &&							// Lower floor bound.
		(probe.Position.Floor - y) >= testData.upperBound &&							// Upper floor bound.
		(probe.Position.Ceiling - y) < -LARA_HEIGHT_CRAWL &&							// Lowest ceiling bound.
		abs(probe.Position.Ceiling - probe.Position.Floor) > LARA_HEIGHT_CRAWL &&		// Space is not a clamp.
		!isSlopeDown && !isSlopeUp && !isDeath &&										// No slope or death sector (if applicable).
		probe.Position.Floor != NO_HEIGHT)
	{
		return true;
	}

	return false;
}

bool TestLaraCrawlForward(ITEM_INFO* item, COLL_INFO* coll)
{
	// Using BadFloorHeightUp/Down defined in crawl state collision functions.

	MoveTestData testData
	{
		item->pos.yRot,
		CLICK(1) - 1,
		-(CLICK(1) - 1)
	};

	return TestLaraCrawlMoveTolerance(item, coll, testData);
}

bool TestLaraCrawlBack(ITEM_INFO* item, COLL_INFO* coll)
{
	// Using BadFloorHeightUp/Down defined in crawl state collision functions.

	MoveTestData testData
	{
		item->pos.yRot + ANGLE(180.0f),
		CLICK(1) - 1,
		-(CLICK(1) - 1)
	};

	return TestLaraCrawlMoveTolerance(item, coll, testData);
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
	auto probe = GetCollisionResult(item, item->pos.yRot, CLICK(2), -LARA_HEIGHT_CRAWL);

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

bool TestLaraMonkeyMoveTolerance(ITEM_INFO* item, COLL_INFO* coll, MonkeyMoveTestData testData)
{
	int y = item->pos.yPos - LARA_HEIGHT_MONKEY;
	auto probe = GetCollisionResult(item, testData.angle, coll->Setup.Radius * sqrt(2) + 4);

	// TODO: Determine the vector of the ceiling triangle instead.
	bool isSlope;
	/*if (abs(coll->Middle.Ceiling - coll->Front.Ceiling) >= SLOPE_DIFFERENCE &&
		abs(coll->MiddleLeft.Ceiling - coll->MiddleRight.Ceiling) >= (SLOPE_DIFFERENCE * 2))
	{
		isSlope = true;
	}
	else*/
		isSlope = false;

	if (probe.BottomBlock->Flags.Monkeyswing &&
		(probe.Position.Floor - y - LARA_HEIGHT_MONKEY) > 0 &&						// Upper floor boundary.
		(probe.Position.Ceiling - y) <= testData.lowerBound &&						// Lower ceiling boundary.
		(probe.Position.Ceiling - y) >= testData.upperBound &&						// Lower ceiling boundary. TODO: Not working??
		!isSlope &&
		probe.Position.Ceiling != NO_HEIGHT)
	{
		return true;
	}

	return false;
}

bool TestLaraMonkeyForward(ITEM_INFO* item, COLL_INFO* coll)
{
	MonkeyMoveTestData testData
	{
		coll->Setup.ForwardAngle,
		CLICK(1.25f),
		-CLICK(1.25f)
	};

	return TestLaraMonkeyMoveTolerance(item, coll, testData);
}

bool TestLaraMonkeyBack(ITEM_INFO* item, COLL_INFO* coll)
{
	MonkeyMoveTestData testData
	{
		coll->Setup.ForwardAngle + ANGLE(180.0f),
		CLICK(1.25f),
		-CLICK(1.25f)
	};

	return TestLaraMonkeyMoveTolerance(item, coll, testData);
}

bool TestLaraMonkeyShimmyLeft(ITEM_INFO* item, COLL_INFO* coll)
{
	MonkeyMoveTestData testData
	{
		coll->Setup.ForwardAngle - ANGLE(90.0f),
		CLICK(0.75f),
		-CLICK(0.75f)
	};

	return TestLaraMonkeyMoveTolerance(item, coll, testData);
}

bool TestLaraMonkeyShimmyRight(ITEM_INFO* item, COLL_INFO* coll)
{
	MonkeyMoveTestData testData
	{
		coll->Setup.ForwardAngle + ANGLE(90.0f),
		CLICK(0.75f),
		-CLICK(0.75f)
	};

	return TestLaraMonkeyMoveTolerance(item, coll, testData);
}

VaultTestResultData TestLaraVaultTolerance(ITEM_INFO* item, COLL_INFO* coll, VaultTestData testData)
{
	LaraInfo*& info = item->data;

	int y = item->pos.yPos;
	auto probeFront = GetCollisionResult(item, coll->NearestLedgeAngle, coll->Setup.Radius * sqrt(2) + 4, -coll->Setup.Height);
	auto probeMiddle = GetCollisionResult(item);
	bool swampTooDeep = testData.checkSwampDepth ? (TestLaraSwamp(item) && info->waterSurfaceDist < -CLICK(3)) : TestLaraSwamp(item);

	// "Floor" ahead may be formed by ceiling; raise y position of probe point to find potential vault candidate location.
	int yOffset = testData.lowerBound;
	while (((probeFront.Position.Ceiling - y) > -coll->Setup.Height ||									// Ceiling is below Lara's height.
			abs(probeFront.Position.Ceiling - probeFront.Position.Floor) <= testData.clampMin ||		// Clamp is too small.
			abs(probeFront.Position.Ceiling - probeFront.Position.Floor) > testData.clampMax) &&		// Clamp is too large.
		yOffset > (testData.upperBound - coll->Setup.Height))											// Offset is not too high.
	{
		probeFront = GetCollisionResult(item, coll->NearestLedgeAngle, coll->Setup.Radius * sqrt(2) + 4, yOffset);
		yOffset -= std::max((int)CLICK(0.5f), testData.clampMin);
	}

	// Assess vault candidate location.
	if ((probeFront.Position.Floor - y) < testData.lowerBound &&									// Lower floor bound.
		(probeFront.Position.Floor - y) >= testData.upperBound &&									// Upper floor bound.
		abs(probeFront.Position.Ceiling - probeFront.Position.Floor) > testData.clampMin &&			// Lower clamp limit.
		abs(probeFront.Position.Ceiling - probeFront.Position.Floor) <= testData.clampMax &&		// Upper clamp limit.
		abs(probeMiddle.Position.Ceiling - probeFront.Position.Floor) >= testData.gapMin &&			// Gap is optically permissive.
		!swampTooDeep &&																			// Swamp depth is permissive.
		probeFront.Position.Floor != NO_HEIGHT)
	{

		return VaultTestResultData { true, probeFront.Position.Floor };
	}

	return VaultTestResultData { false, NO_HEIGHT };
}

VaultTestResultData TestLaraVault2Steps(ITEM_INFO* item, COLL_INFO* coll)
{
	// Floor range: (-STEPUP_HEIGHT, -CLICK(2.5f)]
	// Clamp range: (-LARA_HEIGHT, MAX_HEIGHT]

	VaultTestData testData
	{
		-STEPUP_HEIGHT,
		-CLICK(2.5f),
		LARA_HEIGHT,
		-MAX_HEIGHT,
		CLICK(1)
	};

	return TestLaraVaultTolerance(item, coll, testData);
}

VaultTestResultData TestLaraVault3Steps(ITEM_INFO* item, COLL_INFO* coll)
{
	// Floor range: (-CLICK(2.5f), -CLICK(3.5f)]
	// Clamp range: (-LARA_HEIGHT, MAX_HEIGHT]

	VaultTestData testData
	{
		-CLICK(2.5f),
		-CLICK(3.5f),
		LARA_HEIGHT,
		-MAX_HEIGHT,
		CLICK(1),
	};

	return TestLaraVaultTolerance(item, coll, testData);
}

VaultTestResultData TestLaraVaultAutoJump(ITEM_INFO* item, COLL_INFO* coll)
{
	// Floor range: (-CLICK(3.5f), -CLICK(7.5f)]
	// Clamp range: (-CLICK(0.1f), MAX_HEIGHT]

	VaultTestData testData
	{
		-CLICK(3.5f),
		-CLICK(7.5f),
		CLICK(0.1f), // TODO: Is this enough hand room?
		-MAX_HEIGHT,
		CLICK(0.1f),
		false
	};

	return TestLaraVaultTolerance(item, coll, testData);
}

VaultTestResultData TestLaraVault1StepToCrouch(ITEM_INFO* item, COLL_INFO* coll)
{
	// Floor range: (0, -STEPUP_HEIGHT]
	// Clamp range: (-LARA_HEIGHT_CRAWL, -LARA_HEIGHT]

	VaultTestData testData
	{
		0,
		-STEPUP_HEIGHT,
		LARA_HEIGHT_CRAWL,
		LARA_HEIGHT,
		CLICK(1),
	};

	return TestLaraVaultTolerance(item, coll, testData);
}

VaultTestResultData TestLaraVault2StepsToCrouch(ITEM_INFO* item, COLL_INFO* coll)
{
	// Floor range: (-STEPUP_HEIGHT, -CLICK(2.5f)]
	// Clamp range: (-LARA_HEIGHT_CRAWL, -LARA_HEIGHT]

	VaultTestData testData
	{
		-STEPUP_HEIGHT,
		-CLICK(2.5f),
		LARA_HEIGHT_CRAWL,
		LARA_HEIGHT,
		CLICK(1),
	};

	return TestLaraVaultTolerance(item, coll, testData);
}

VaultTestResultData TestLaraVault3StepsToCrouch(ITEM_INFO* item, COLL_INFO* coll)
{
	// Floor range: (-CLICK(2.5f), -CLICK(3.5f)]
	// Clamp range: (-LARA_HEIGHT_CRAWL, -LARA_HEIGHT]

	VaultTestData testData
	{
		-CLICK(2.5f),
		-CLICK(3.5f),
		LARA_HEIGHT_CRAWL,
		LARA_HEIGHT,
		CLICK(1),
	};

	return TestLaraVaultTolerance(item, coll, testData);
}

VaultTestResultData TestLaraLadderAutoJump(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	int y = item->pos.yPos;
	auto probeFront = GetCollisionResult(item, coll->NearestLedgeAngle, coll->Setup.Radius * sqrt(2) + 4, -coll->Setup.Height);
	auto probeMiddle = GetCollisionResult(item);

	if (TestValidLedgeAngle(item, coll) &&
		!TestLaraSwamp(item) &&										// No swamp.
		info->climbStatus &&										// Ladder sector flag set.
		(probeMiddle.Position.Ceiling - y) <= -CLICK(6.5f) &&		// Lower middle ceiling bound.
		coll->NearestLedgeDistance <= coll->Setup.Radius)			// Appropriate distance from wall.
	{
		return VaultTestResultData{ true, probeMiddle.Position.Ceiling };
	}

	return VaultTestResultData{ false, NO_HEIGHT };
}

bool TestLaraLadderMount(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	int y = item->pos.yPos;
	auto probeFront = GetCollisionResult(item, coll->NearestLedgeAngle, coll->Setup.Radius * sqrt(2) + 4, -coll->Setup.Height);
	auto probeMiddle = GetCollisionResult(item);

	if (TestValidLedgeAngle(item, coll) &&
		info->climbStatus &&										// Ladder sector flag set.
		(probeMiddle.Position.Ceiling - y) <= -CLICK(4.5f) &&		// Lower middle ceiling bound.
		(probeMiddle.Position.Floor - y) > -CLICK(6.5f) &&			// Upper middle floor bound.
		(probeFront.Position.Ceiling - y) <= -CLICK(4.5f) &&		// Lower front ceiling bound.
		coll->NearestLedgeDistance <= coll->Setup.Radius)			// Appropriate distance from wall.
	{
		return true;
	}

	return false;
}

bool TestLaraMonkeyAutoJump(ITEM_INFO* item, COLL_INFO* coll)
{
	LaraInfo*& info = item->data;

	int y = item->pos.yPos;
	auto probe = GetCollisionResult(item);

	if (!TestLaraSwamp(item) &&										// No swamp.
		info->canMonkeySwing &&										// Monkey swing sector flag set.
		(probe.Position.Ceiling - y) < -LARA_HEIGHT_MONKEY &&		// Lower ceiling bound.
		(probe.Position.Ceiling - y) >= -CLICK(7))					// Upper ceiling bound.
	{
		return true;
	}

	return false;
}

bool TestLaraCrawlVaultTolerance(ITEM_INFO* item, COLL_INFO* coll, CrawlVaultTestData testData)
{
	int y = item->pos.yPos;
	auto probeA = GetCollisionResult(item, item->pos.yRot, testData.crossDist, -LARA_HEIGHT_CRAWL);		// Crossing.
	auto probeB = GetCollisionResult(item, item->pos.yRot, testData.destDist, -LARA_HEIGHT_CRAWL);		// Approximate destination.
	auto probeMiddle = GetCollisionResult(item);
	bool isSlope = testData.checkSlope ? probeB.Position.Slope : false;
	bool isDeath = testData.checkDeath ? probeB.Block->Flags.Death : false;

	if ((probeA.Position.Floor - y) <= testData.lowerBound &&								// Lower floor bound.
		(probeA.Position.Floor - y) >= testData.upperBound &&								// Upper floor bound.
		abs(probeA.Position.Ceiling - probeA.Position.Floor) > testData.clampMin &&			// Crossing clamp limit.
		abs(probeB.Position.Ceiling - probeB.Position.Floor) > testData.clampMin &&			// Destination clamp limit.
		abs(probeMiddle.Position.Ceiling - probeA.Position.Floor) >= testData.gapMin &&		// Gap is optically permissive (going up).
		abs(probeA.Position.Ceiling - probeMiddle.Position.Floor) >= testData.gapMin &&		// Gap is optically permissive (going down).
		abs(probeA.Position.Floor - probeB.Position.Floor) <= testData.probeDeltaMax &&		// Crossing and destination floor height difference suggests continuous crawl surface.
		(probeA.Position.Ceiling - y) < -testData.gapMin &&									// Ceiling height is permissive.
		!isSlope && !isDeath &&																// No slope or death sector.
		probeA.Position.Floor != NO_HEIGHT && probeB.Position.Floor != NO_HEIGHT)
	{
		return true;
	}

	return false;
}

bool TestLaraCrawlUpStep(ITEM_INFO* item, COLL_INFO* coll)
{
	// Floor range: [-CLICK(1), -STEPUP_HEIGHT]

	CrawlVaultTestData testData
	{
		-CLICK(1),
		-STEPUP_HEIGHT,
		LARA_HEIGHT_CRAWL,
		CLICK(0.6f),
		CLICK(1.2f),
		CLICK(2),
		CLICK(1) - 1
	};

	return TestLaraCrawlVaultTolerance(item, coll, testData);
}

bool TestLaraCrawlDownStep(ITEM_INFO* item, COLL_INFO* coll)
{
	// Floor range: [STEPUP_HEIGHT, CLICK(1)]

	CrawlVaultTestData testData
	{
		STEPUP_HEIGHT,
		CLICK(1),
		LARA_HEIGHT_CRAWL,
		CLICK(0.6f),
		CLICK(1.2f),
		CLICK(2),
		CLICK(1) - 1
	};

	return TestLaraCrawlVaultTolerance(item, coll, testData);
}

bool TestLaraCrawlExitDownStep(ITEM_INFO* item, COLL_INFO* coll)
{
	// Floor range: [STEPUP_HEIGHT, CLICK(1)]

	CrawlVaultTestData testData
	{
		STEPUP_HEIGHT,
		CLICK(1),
		LARA_HEIGHT,
		CLICK(1.25f),
		CLICK(1.2f),
		CLICK(1.5f),
		-MAX_HEIGHT,
		false
	};

	return TestLaraCrawlVaultTolerance(item, coll, testData);
}

bool TestLaraCrawlExitJump(ITEM_INFO* item, COLL_INFO* coll)
{
	// Floor range: [-MAX_HEIGHT, STEPUP_HEIGHT)

	CrawlVaultTestData testData
	{
		-MAX_HEIGHT,
		STEPUP_HEIGHT + 1,
		LARA_HEIGHT,
		CLICK(1.25f),
		CLICK(1.2f),
		CLICK(1.5f),
		-MAX_HEIGHT,
		false
	};

	return TestLaraCrawlVaultTolerance(item, coll, testData);
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
	auto probe = GetCollisionResult(item, item->pos.yRot + ANGLE(180.0f), CLICK(1.2f), -LARA_HEIGHT_CRAWL);
	bool objectCollided = TestLaraObjectCollision(item, item->pos.yRot + ANGLE(180.0f), CLICK(1.2f), -LARA_HEIGHT_CRAWL);

	if (!objectCollided &&											// No obstruction.
		(probe.Position.Floor - y) >= LARA_HEIGHT_STRETCH &&		// Highest floor bound.
		(probe.Position.Ceiling - y) <= -CLICK(0.75f) &&			// Gap is optically permissive.
		probe.Position.Floor != NO_HEIGHT)
	{
		return true;
	}

	return false;
}

bool TestLaraJumpTolerance(ITEM_INFO* item, COLL_INFO* coll, JumpTestData testData)
{
	LaraInfo*& info = item->data;

	int y = item->pos.yPos;
	auto probe = GetCollisionResult(item, testData.angle, testData.dist, -coll->Setup.Height);
	bool isWading = testData.checkWadeStatus ? (info->waterStatus == LW_WADE) : false;

	if (((probe.Position.Floor - y) >= -STEPUP_HEIGHT ||										// Highest floor bound...
			probe.Position.Slope) &&																// OR surface is a slope.
		((probe.Position.Ceiling - y) < -(coll->Setup.Height + (LARA_HEADROOM * 0.7f)) ||		// Ceiling height is permissive... 
			((probe.Position.Ceiling - y) < -coll->Setup.Height &&									// OR ceiling is level with Lara's head
				(probe.Position.Floor - y) >= CLICK(0.5f))) &&										// AND there is a drop below.
		!isWading &&																			// Not wading in water (if applicable).
		!TestLaraSwamp(item) &&																	// No swamp.
		!TestLaraFacingCorner(item, testData.angle, testData.dist) &&							// Avoid jumping through corners.
		probe.Position.Floor != NO_HEIGHT)
	{
		return true;
	}

	return false;
}

bool TestLaraRunJumpForward(ITEM_INFO* item, COLL_INFO* coll)
{
	JumpTestData testData
	{
		item->pos.yRot,
		CLICK(1.5f)
	};

	return TestLaraJumpTolerance(item, coll, testData);
}

bool TestLaraJumpForward(ITEM_INFO* item, COLL_INFO* coll)
{
	JumpTestData testData
	{
		item->pos.yRot
	};

	return TestLaraJumpTolerance(item, coll, testData);
}

bool TestLaraJumpBack(ITEM_INFO* item, COLL_INFO* coll)
{
	JumpTestData testData
	{
		item->pos.yRot + ANGLE(180.0f)
	};

	return TestLaraJumpTolerance(item, coll, testData);
}

bool TestLaraJumpLeft(ITEM_INFO* item, COLL_INFO* coll)
{
	JumpTestData testData
	{
		item->pos.yRot - ANGLE(90.0f)
	};

	return TestLaraJumpTolerance(item, coll, testData);
}

bool TestLaraJumpRight(ITEM_INFO* item, COLL_INFO* coll)
{
	JumpTestData testData
	{
		item->pos.yRot + ANGLE(90.0f)
	};

	return TestLaraJumpTolerance(item, coll, testData);
}

bool TestLaraJumpUp(ITEM_INFO* item, COLL_INFO* coll)
{
	JumpTestData testData
	{
		0,
		0,
		false
	};

	return TestLaraJumpTolerance(item, coll, testData);
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
