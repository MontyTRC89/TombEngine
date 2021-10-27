#include "framework.h"
#include "lara.h"
#include "lara_tests.h"
#include "input.h"
#include "level.h"
#include "animation.h"
#include "lara_climb.h"
#include "lara_collide.h"
#include "control/control.h"
#include "control/los.h"
#include "items.h"
#include "Renderer11.h"

using namespace TEN::Renderer;
using namespace TEN::Floordata;

/*this file has all the generic test functions called in lara's state code*/

// Test if a ledge in front of item is valid to climb.
bool TestValidLedge(ITEM_INFO* item, COLL_INFO* coll, bool ignoreHeadroom)
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

	// Discard if there is a slope beyond tolerance delta
	if (abs(left - right) >= slopeDelta)
		return false;

	if (!TestValidLedgeAngle(coll))
		return false; 
	
	if (!ignoreHeadroom)
	{
		auto headroom = (coll->Front.Floor + coll->Setup.Height) - coll->Middle.Ceiling;
		if (headroom < STEP_SIZE)
			return false;
	}
	
	return (coll->CollisionType == CT_FRONT);
}

bool TestValidLedgeAngle(COLL_INFO* coll)
{
	return (abs((short)(coll->NearestLedgeAngle - coll->Setup.ForwardAngle)) <= LARA_GRAB_THRESHOLD);
}

bool TestLaraVault(ITEM_INFO* item, COLL_INFO* coll)
{
	if (!(TrInput & IN_ACTION) || Lara.gunStatus != LG_NO_ARMS)
		return false;

	// TODO: Enable with lua!
	Lara.NewAnims.CrawlVault1click = 1;
	Lara.NewAnims.CrawlVault2click = 1;
	Lara.NewAnims.CrawlVault3click = 1;
	Lara.NewAnims.MonkeyVault = 1;

	float pushOffset = 0.5f;

	if (TestValidLedge(item, coll))
	{
		bool success = false;

		if (coll->Front.Floor < 0 && coll->Front.Floor >= -256)
		{
			if (Lara.NewAnims.CrawlVault1click && (abs(coll->Front.Ceiling - coll->Front.Floor) < 256))
			{
				item->animNumber = LA_VAULT_TO_CROUCH_1CLICK;
				item->currentAnimState = LS_GRABBING;
				item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
				item->goalAnimState = LS_CROUCH_IDLE;
				item->pos.yPos += coll->Front.Floor + 256;
				Lara.gunStatus = LG_HANDS_BUSY;
				success = true;
			}
		}
		else if (coll->Front.Floor >= -640 && coll->Front.Floor <= -384)
		{
			if (coll->Front.Floor - coll->Front.Ceiling >= 0 &&
				coll->FrontLeft.Floor - coll->FrontLeft.Ceiling >= 0 &&
				coll->FrontRight.Floor - coll->FrontRight.Ceiling >= 0)
			{
#if 0
				if (g_Level.Rooms[item->roomNumber].flags & ENV_FLAG_SWAMP && Lara.waterSurfaceDist < -768)
					return false;
#endif

				item->animNumber = LA_VAULT_TO_STAND_2CLICK_START;
				item->currentAnimState = LS_GRABBING;
				item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
				item->goalAnimState = LS_STOP;
				item->pos.yPos += coll->Front.Floor + 512;
				Lara.gunStatus = LG_HANDS_BUSY;
				pushOffset = 0.7f;
				success = true;
			}
			else if (Lara.NewAnims.CrawlVault2click && (abs(coll->Front.Ceiling - coll->Front.Floor) < 256))
			{
				item->animNumber = LA_VAULT_TO_CROUCH_2CLICK;
				item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
				item->currentAnimState = LS_GRABBING;
				item->goalAnimState = LS_CROUCH_IDLE;
				item->pos.yPos += coll->Front.Floor + 512;
				Lara.gunStatus = LG_HANDS_BUSY;
				success = true;
			}
		}
		else if (coll->Front.Floor >= -896 && coll->Front.Floor <= -640)
		{
			if (coll->Front.Floor - coll->Front.Ceiling >= 0 &&
				coll->FrontLeft.Floor - coll->FrontLeft.Ceiling >= 0 &&
				coll->FrontRight.Floor - coll->FrontRight.Ceiling >= 0)
			{
#if 0
				if (g_Level.Rooms[item->roomNumber].flags & ENV_FLAG_SWAMP && Lara.waterSurfaceDist < -768)
					return 0;
#endif

				item->animNumber = LA_VAULT_TO_STAND_3CLICK;
				item->currentAnimState = LS_GRABBING;
				item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
				item->goalAnimState = LS_STOP;
				item->pos.yPos += coll->Front.Floor + 768;
				Lara.gunStatus = LG_HANDS_BUSY;
				pushOffset = 0.7f;
				success = true;
			}
			else if (Lara.NewAnims.CrawlVault3click && (abs(coll->Front.Ceiling - coll->Front.Floor) < 256))
			{
				item->animNumber = LA_VAULT_TO_CROUCH_3CLICK;
				item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
				item->currentAnimState = LS_GRABBING;
				item->goalAnimState = LS_CROUCH_IDLE;
				item->pos.yPos += coll->Front.Floor + 768;
				Lara.gunStatus = LG_HANDS_BUSY;
				success = true;
			}
		}
		else if (coll->Front.Floor >= -1920 && coll->Front.Floor <= -896)
		{
#if 0
			if (g_Level.Rooms[item->roomNumber].flags & ENV_FLAG_SWAMP)
				return false;
#endif

			item->animNumber = LA_STAND_SOLID;
			item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
			item->goalAnimState = LS_JUMP_UP;
			item->currentAnimState = LS_STOP;
			Lara.calcFallSpeed = -3 - sqrt(-9600 - 12 * coll->Front.Floor);
			AnimateLara(item);
			pushOffset = item->speed ? 0.5f : 0.15f; // While on the run, Lara tends to embed less
			success = true;
		}

		if (success)
		{
			ShiftItem(item, coll);
			SnapItemToLedge(item, coll, pushOffset);
		}
	}

	if (TestValidLedgeAngle(coll) && Lara.climbStatus)
	{
		if (coll->Front.Floor > -1920 || Lara.waterStatus == LW_WADE || coll->FrontLeft.Floor > -1920 || coll->FrontRight.Floor > -2048 || coll->Middle.Ceiling > -1158)
		{
			if ((coll->Front.Floor < -1024 || coll->Front.Ceiling >= 506) && coll->Middle.Ceiling <= -518)
			{
				if (TestLaraClimbStance(item, coll))
				{
					item->animNumber = LA_STAND_SOLID;
					item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
					item->goalAnimState = LS_LADDER_IDLE;
					item->currentAnimState = LS_STOP;
					Lara.gunStatus = LG_HANDS_BUSY;
					Lara.turnRate = 0;

					AnimateLara(item);
					ShiftItem(item, coll);
					SnapItemToLedge(item, coll, 0.1f);

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

		AnimateLara(item);
		ShiftItem(item, coll);
		SnapItemToLedge(item, coll, 0.1f);

		return true;
	}

	if (Lara.canMonkeySwing && Lara.NewAnims.MonkeyVault)
	{
		short roomNum = item->roomNumber;
		int ceiling = (GetCeiling(GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNum),
			item->pos.xPos, item->pos.yPos, item->pos.zPos))-(item->pos.yPos);

		if (ceiling > 1792 || ceiling < -1792 || abs(ceiling) == 768)
			return false;

		item->animNumber = LA_STAND_IDLE;
		item->frameNumber = g_Level.Anims[LA_STAND_IDLE].frameBase;
		item->goalAnimState = LS_JUMP_UP;
		item->currentAnimState = LS_TEST_1;

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

			if (!flag2 && coll->Middle.Ceiling < 0 && coll->CollisionType == CT_FRONT && !flag && !coll->HitStatic && cdif <= -950 && dfront >= -60 && dfront <= 60)
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
	item->pos.yRot += ANGLE(testAngle);
	SnapItemToLedge(item, coll, item->pos.yRot);

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

	// Restore original item positions
	item->pos = oldPos;
	Lara.moveAngle = oldPos.yRot;

	// OUTER CORNER TESTS

	// Test if there's a material obstacles blocking outer corner pathway
	if (LaraFloorFront(item, item->pos.yRot + ANGLE(testAngle), coll->Setup.Radius + STEP_SIZE) < 0)
		return CORNER_RESULT::NONE;
	if (LaraCeilingFront(item, item->pos.yRot + ANGLE(testAngle), coll->Setup.Radius + STEP_SIZE, coll->Setup.Height) > 0)
		return CORNER_RESULT::NONE;

	// Push Lara diagonally to other side of corner at distance of 1/2 wall size
	c = phd_cos(item->pos.yRot + ANGLE(testAngle / 2));
	s = phd_sin(item->pos.yRot + ANGLE(testAngle / 2));
	item->pos.xPos += s * WALL_SIZE / 3;
	item->pos.zPos += c * WALL_SIZE / 3;

	// Virtually rotate Lara 90 degrees to the left and snap to nearest ledge, if any.
	item->pos.yRot -= ANGLE(testAngle);
	Lara.moveAngle = item->pos.yRot;
	SnapItemToLedge(item, coll, item->pos.yRot);

	// FIXME? Those hacky fields are still used somewhere to align her...
	Lara.cornerX = item->pos.xPos;
	Lara.cornerZ = item->pos.zPos;

	result = TestLaraValidHangPos(item, coll);
	
	if (result)
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
	coll->Setup.Mode == COLL_PROBE_MODE::FREE_FLAT;
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
	BOUNDING_BOX* bounds;
	int shift, result;

	if (Lara.climbStatus == 0)
		return false;

	if (item->fallspeed < 0)
		return false;

	switch (GetQuadrant(item->pos.yRot))
	{
	case NORTH:
	case SOUTH:
		item->pos.zPos += coll->Shift.z;
		break;

	case EAST:
	case WEST:
		item->pos.xPos += coll->Shift.x;
		break;

	default:
		break;
	}

	bounds = GetBoundsAccurate(item);

	if (Lara.moveAngle != item->pos.yRot)
	{
		short l = LaraCeilingFront(item, item->pos.yRot, 0, 0);
		short r = LaraCeilingFront(item, Lara.moveAngle, 128, 0);

		if (abs(l - r) > 60)
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
	Lara.NewAnims.FeetHanging = 0;

	if (Lara.climbStatus || !Lara.NewAnims.FeetHanging)
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
							item->pos.yPos,
							item->pos.zPos,
							item->roomNumber);

	auto result1 = LOS(&pos, &vec1);
	auto result2 = LOS(&pos, &vec2);

	return (result1 == 0 && result2 == 0);
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
