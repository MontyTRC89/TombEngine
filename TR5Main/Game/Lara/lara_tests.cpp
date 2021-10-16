#include "framework.h"
#include "lara.h"
#include "lara_tests.h"
#include "input.h"
#include "level.h"
#include "animation.h"
#include "lara_climb.h"
#include "lara_collide.h"
#include "lara_flare.h"
#include "control/control.h"
#include "control/los.h"
#include "items.h"
#include "Renderer11.h"

using namespace TEN::Renderer;
using namespace TEN::Floordata;

static short LeftClimbTab[4] = // offset 0xA0638
{
	0x0200, 0x0400, 0x0800, 0x0100
};

static short RightClimbTab[4] = // offset 0xA0640
{
	0x0800, 0x0100, 0x0200, 0x0400
};

/*this file has all the generic test functions called in lara's state code*/

// Test if a ledge in front of item is valid to climb.
bool TestValidLedge(ITEM_INFO* item, COLL_INFO* coll)
{
	// Determine probe base point.
	// We use double-radius here for two purposes. First - we can't guarantee that
	// shifts weren't already applied and misfire may occur. Second - it guarantees
	// that Lara won't land on a very thin edge of diagonal geometry.

	int xf = phd_sin(coll->NearestLedgeAngle) * (coll->Setup.Radius * 2);
	int zf = phd_cos(coll->NearestLedgeAngle) * (coll->Setup.Radius * 2);

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

	if (abs((short)(coll->NearestLedgeAngle - coll->Setup.ForwardAngle)) > LARA_GRAB_THRESHOLD)
		return false; 
	
	auto headroom = (coll->Front.Floor + coll->Setup.Height) - coll->Middle.Ceiling;
	if (headroom < STEP_SIZE)
		return false;
	
	return (coll->CollisionType == CT_FRONT);
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

	if (TestValidLedge(item, coll))
	{
		if (coll->Front.Floor < 0 && coll->Front.Floor >= -256 && Lara.NewAnims.CrawlVault1click)
		{
			if (Lara.NewAnims.CrawlVault1click && (abs(coll->Front.Ceiling - coll->Front.Floor) < 256))
			{
				item->animNumber = LA_VAULT_TO_CROUCH_1CLICK;
				item->currentAnimState = LS_GRABBING;
				item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
				item->goalAnimState = LS_CROUCH_IDLE;
				item->pos.yPos += coll->Front.Floor + 256;
				Lara.gunStatus = LG_HANDS_BUSY;
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
			}
			else if (Lara.NewAnims.CrawlVault2click && (abs(coll->Front.Ceiling - coll->Front.Floor) < 256))
			{
				item->animNumber = LA_VAULT_TO_CROUCH_2CLICK;
				item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
				item->currentAnimState = LS_GRABBING;
				item->goalAnimState = LS_CROUCH_IDLE;
				item->pos.yPos += coll->Front.Floor + 512;
				Lara.gunStatus = LG_HANDS_BUSY;
			}
			else
			{
				return false;
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
			}
			else if (Lara.NewAnims.CrawlVault3click && (abs(coll->Front.Ceiling - coll->Front.Floor) < 256))
			{
				item->animNumber = LA_VAULT_TO_CROUCH_3CLICK;
				item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
				item->currentAnimState = LS_GRABBING;
				item->goalAnimState = LS_CROUCH_IDLE;
				item->pos.yPos += coll->Front.Floor + 768;
				Lara.gunStatus = LG_HANDS_BUSY;
			}
			else
			{
				return false;
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
		}

		item->pos.yRot = coll->NearestLedgeAngle;
		ShiftItem(item, coll);

		item->pos.xPos += phd_sin(coll->NearestLedgeAngle) * coll->NearestLedgeDistance;
		item->pos.zPos += phd_cos(coll->NearestLedgeAngle) * coll->NearestLedgeDistance;

		return true;
	}
	else if (Lara.climbStatus)
	{
		if (coll->Front.Floor > -1920 || Lara.waterStatus == LW_WADE || coll->FrontLeft.Floor > -1920 || coll->FrontRight.Floor > -2048 || coll->Middle.Ceiling > -1158)
		{
			if ((coll->Front.Floor < -1024 || coll->Front.Ceiling >= 506) && coll->Middle.Ceiling <= -518)
			{
				ShiftItem(item, coll);

				if (TestLaraClimbStance(item, coll))
				{
					item->animNumber = LA_STAND_SOLID;
					item->frameNumber = g_Level.Anims[item->animNumber].frameBase;
					item->goalAnimState = LS_LADDER_IDLE;
					item->currentAnimState = LS_STOP;
					AnimateLara(item);
					item->pos.yRot = coll->NearestLedgeAngle;
					Lara.gunStatus = LG_HANDS_BUSY;
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
		AnimateLara(item);

		item->pos.yRot = coll->NearestLedgeAngle;
		ShiftItem(item, coll);

		item->pos.xPos += phd_sin(coll->NearestLedgeAngle) * coll->NearestLedgeDistance;
		item->pos.zPos += phd_cos(coll->NearestLedgeAngle) * coll->NearestLedgeDistance;

		return true;
	}
	else if (Lara.NewAnims.MonkeyVault && Lara.canMonkeySwing)
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

bool TestLaraKeepDucked(COLL_INFO* coll)
{
	// TODO: Cannot use as a failsafe; this is bugged with slanted ceilings reaching the ground. @Sezz 2021.10.15
	if (coll->Middle.Ceiling >= -LARA_HEIGHT_CRAWL) // Was -362.
		return true;

	return false;
}

// LEGACY
// TODO: Gradually replace calls with new TestLaraSlide() (currently TestLaraSlideNew()) and SetLaraSlideState(). @Sezz 2021.09.27
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
		return SPLAT_COLL::SPLAT_WALL;

	if (y >= h || y <= c)
		return SPLAT_COLL::SPLAT_STEP;

	return SPLAT_COLL::SPLAT_NONE;
}

bool TestLaraHang(ITEM_INFO* item, COLL_INFO* coll)
{
	ANIM_FRAME* frame;

	auto delta = 0;
	auto flag = 0;
	auto angle = Lara.moveAngle;

	if (angle == (short) (item->pos.yRot - ANGLE(90)))
	{
		delta = -100;
	}
	else if (angle == (short) (item->pos.yRot + ANGLE(90)))
	{
		delta = 100;
	}

	auto hdif = LaraFloorFront(item, angle, 100);

	if (hdif < 200)
		flag = 1;

	auto cdif = LaraCeilingFront(item, angle, 100, 0);
	auto dir = GetQuadrant(item->pos.yRot);

	switch (dir)
	{
	case NORTH:
		item->pos.zPos += 4;
		break;
	case EAST:
		item->pos.xPos += 4;
		break;
	case SOUTH:
		item->pos.zPos -= 4;
		break;
	case WEST:
		item->pos.xPos -= 4;
		break;
	}

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

			switch (dir)
			{
			case NORTH:
				x += delta;
				break;
			case EAST:
				z -= delta;
				break;
			case SOUTH:
				x -= delta;
				break;
			case WEST:
				z += delta;
				break;
			}

			Lara.moveAngle = angle;

			if (256 << dir & GetClimbFlags(x, item->pos.yPos, z, item->roomNumber))
			{
				if (!TestLaraHangOnClimbWall(item, coll))
					dfront = 0;
			}
			else if (abs(coll->FrontLeft.Floor - coll->FrontRight.Floor) >= 60)
			{
				if (delta < 0 && coll->FrontLeft.Floor != coll->Front.Floor || delta > 0 && coll->FrontRight.Floor != coll->Front.Floor)
					flag2 = 1;
			}

			coll->Front.Floor = front;

			if (!flag2 && coll->Middle.Ceiling < 0 && coll->CollisionType == CT_FRONT && !flag && !coll->HitStatic && cdif <= -950 && dfront >= -60 && dfront <= 60)
			{
				switch (dir)
				{
				case NORTH:
				case SOUTH:
					item->pos.zPos += coll->Shift.z;
					break;
				case EAST:
				case WEST:
					item->pos.xPos += coll->Shift.x;
					break;
				}

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

int TestLaraHangLeftCorner(ITEM_INFO* item, COLL_INFO* coll)
{
	if (item->animNumber != LA_REACH_TO_HANG && item->animNumber != LA_HANG_FEET_IDLE)
		return 0;

	if (coll->HitStatic)
		return 0;

	int x;
	int z;

	int oldXpos = item->pos.xPos;
	int oldZpos = item->pos.zPos;
	short oldYrot = item->pos.yRot;
	int oldFrontFloor = coll->Front.Floor;

	short angle = GetQuadrant(item->pos.yRot);
	if (angle != NORTH && angle != SOUTH)
	{
		x = item->pos.xPos ^ (item->pos.xPos ^ item->pos.zPos) & 0x3FF;
		z = item->pos.zPos ^ (item->pos.xPos ^ item->pos.zPos) & 0x3FF;
	}
	else
	{
		x = (item->pos.xPos & 0xFFFFFC00) - (item->pos.zPos & 0x3FF) + SECTOR(1);
		z = (item->pos.zPos & 0xFFFFFC00) - (item->pos.xPos & 0x3FF) + SECTOR(1);
	}

	item->pos.xPos = x;
	Lara.cornerX = x;
	item->pos.zPos = z;
	Lara.cornerZ = z;
	item->pos.yRot -= ANGLE(90.0f);

	auto result = -TestLaraValidHangPos(item, coll);
	if (result)
	{
		if (Lara.climbStatus)
		{
			if (GetClimbFlags(x, item->pos.yPos, z, item->roomNumber) & RightClimbTab[angle])
			{
				item->pos.xPos = oldXpos;
				item->pos.zPos = oldZpos;
				item->pos.yRot = oldYrot;
				Lara.moveAngle = oldYrot;
				return result;
			}
		}
		else
		{
			if (abs(oldFrontFloor - coll->Front.Floor) <= 60)
			{
				item->pos.xPos = oldXpos;
				item->pos.zPos = oldZpos;
				item->pos.yRot = oldYrot;
				Lara.moveAngle = oldYrot;
				return result;
			}
		}
	}

	item->pos.xPos = oldXpos;
	item->pos.zPos = oldZpos;
	item->pos.yRot = oldYrot;
	Lara.moveAngle = oldYrot;

	if (LaraFloorFront(item, oldYrot - ANGLE(90.0f), 116) < 0)
		return 0;

	switch (angle)
	{
	case NORTH:
		x = ((item->pos.xPos ^ item->pos.zPos) & 0x3FF) ^ item->pos.xPos - SECTOR(1);
		z = ((item->pos.xPos ^ item->pos.zPos) & 0x3FF) ^ item->pos.zPos + SECTOR(1);
		break;

	case SOUTH:
		x = ((item->pos.xPos ^ item->pos.zPos) & 0x3FF) ^ (item->pos.xPos + SECTOR(1));
		z = ((item->pos.xPos ^ item->pos.zPos) & 0x3FF) ^ (item->pos.zPos - SECTOR(1));
		break;

	case WEST:
		x = (item->pos.xPos & 0xFFFFFC00) - (item->pos.zPos & 0x3FF);
		z = (item->pos.zPos & 0xFFFFFC00) - (item->pos.xPos & 0x3FF);
		break;

	default:
		x = ((item->pos.xPos + SECTOR(1)) & 0xFFFFFC00) - (item->pos.zPos & 0x3FF) + SECTOR(1);
		z = ((item->pos.zPos + SECTOR(1)) & 0xFFFFFC00) - (item->pos.xPos & 0x3FF) + SECTOR(1);
		break;

	}

	item->pos.xPos = x;
	Lara.cornerX = x;
	item->pos.zPos = z;
	Lara.cornerZ = z;
	item->pos.yRot += ANGLE(90.0f);

	result = TestLaraValidHangPos(item, coll);
	if (!result)
	{
		item->pos.xPos = oldXpos;
		item->pos.zPos = oldZpos;
		item->pos.yRot = oldYrot;
		Lara.moveAngle = oldYrot;
		return result;
	}

	item->pos.xPos = oldXpos;
	item->pos.zPos = oldZpos;
	item->pos.yRot = oldYrot;
	Lara.moveAngle = oldYrot;

	if (!Lara.climbStatus)
	{
		if (abs(oldFrontFloor - coll->Front.Floor) <= 60)
		{
			switch (angle)
			{
			case NORTH:
				if ((oldXpos & 0x3FF) > 512)
					result = 0;
				break;
			case EAST:
				if ((oldZpos & 0x3FF) < 512)
					result = 0;
				break;
			case SOUTH:
				if ((oldXpos & 0x3FF) < 512)
					result = 0;
				break;
			case WEST:
				if ((oldZpos & 0x3FF) > 512)
					result = 0;
				break;
			}
			return result;
		}
		return 0;
	}

	if (GetClimbFlags(x, item->pos.yPos, z, item->roomNumber) & LeftClimbTab[angle])
		return result;

	short front = LaraFloorFront(item, item->pos.yRot, 116);
	if (abs(front - coll->Front.Floor) > 60)
		return 0;

	if (front < -768)
		return 0;

	return result;
}

int TestLaraHangRightCorner(ITEM_INFO* item, COLL_INFO* coll)
{
	if (item->animNumber != LA_REACH_TO_HANG && item->animNumber != LA_HANG_FEET_IDLE)
		return 0;

	if (coll->HitStatic)
		return 0;

	int x;
	int z;

	int oldXpos = item->pos.xPos;
	int oldZpos = item->pos.zPos;
	short oldYrot = item->pos.yRot;
	int oldFrontFloor = coll->Front.Floor;

	short angle = GetQuadrant(item->pos.yRot);
	if (angle != NORTH && angle != SOUTH)
	{
		x = (item->pos.xPos & 0xFFFFFC00) - (item->pos.zPos & 0x3FF) + SECTOR(1);
		z = (item->pos.zPos & 0xFFFFFC00) - (item->pos.xPos & 0x3FF) + SECTOR(1);
	}
	else
	{
		x = item->pos.xPos ^ (item->pos.xPos ^ item->pos.zPos) & 0x3FF;
		z = item->pos.zPos ^ (item->pos.xPos ^ item->pos.zPos) & 0x3FF;
	}

	item->pos.xPos = x;
	Lara.cornerX = x;
	item->pos.zPos = z;
	Lara.cornerZ = z;
	item->pos.yRot += ANGLE(90.0f);

	auto result = -TestLaraValidHangPos(item, coll);
	if (result)
	{
		if (Lara.climbStatus)
		{
			if (GetClimbFlags(x, item->pos.yPos, z, item->roomNumber) & LeftClimbTab[angle])
			{
				item->pos.xPos = oldXpos;
				item->pos.zPos = oldZpos;
				item->pos.yRot = oldYrot;
				Lara.moveAngle = oldYrot;
				return result;
			}
		}
		else
		{
			if (abs(oldFrontFloor - coll->Front.Floor) <= 60)
			{
				item->pos.xPos = oldXpos;
				item->pos.zPos = oldZpos;
				item->pos.yRot = oldYrot;
				Lara.moveAngle = oldYrot;
				return result;
			}
		}
	}

	item->pos.xPos = oldXpos;
	item->pos.zPos = oldZpos;
	item->pos.yRot = oldYrot;
	Lara.moveAngle = oldYrot;

	if (LaraFloorFront(item, oldYrot + ANGLE(90.0f), 116) < 0)
		return 0;

	switch (angle)
	{
	case NORTH:
		x = ((item->pos.xPos + SECTOR(1)) & 0xFFFFFC00) - (item->pos.zPos & 0x3FF) + SECTOR(1);
		z = ((item->pos.zPos + SECTOR(1)) & 0xFFFFFC00) - (item->pos.xPos & 0x3FF) + SECTOR(1);
		break;

	case SOUTH:
		x = ((item->pos.xPos - SECTOR(1)) & 0xFFFFFC00) - (item->pos.zPos & 0x3FF) + SECTOR(1);
		z = ((item->pos.zPos - SECTOR(1)) & 0xFFFFFC00) - (item->pos.xPos & 0x3FF) + SECTOR(1);
		break;

	case WEST:
		x = (item->pos.xPos ^ item->pos.zPos) & 0x3FF ^ (item->pos.xPos - SECTOR(1));
		z = (item->pos.xPos ^ item->pos.zPos) & 0x3FF ^ (item->pos.zPos + SECTOR(1));
		break;

	default:
		x = ((item->pos.xPos ^ item->pos.zPos) & 0x3FF) ^ (item->pos.xPos + SECTOR(1));
		z = ((item->pos.xPos ^ item->pos.zPos) & 0x3FF) ^ (item->pos.zPos - SECTOR(1));
		break;

	}

	item->pos.xPos = x;
	Lara.cornerX = x;
	item->pos.zPos = z;
	Lara.cornerZ = z;
	item->pos.yRot -= ANGLE(90.0f);

	result = TestLaraValidHangPos(item, coll);
	if (!result)
	{
		item->pos.xPos = oldXpos;
		item->pos.zPos = oldZpos;
		item->pos.yRot = oldYrot;
		Lara.moveAngle = oldYrot;
		return result;
	}

	item->pos.xPos = oldXpos;
	item->pos.zPos = oldZpos;
	item->pos.yRot = oldYrot;
	Lara.moveAngle = oldYrot;

	if (!Lara.climbStatus)
	{
		if (abs(oldFrontFloor - coll->Front.Floor) <= 60)
		{
			switch (angle)
			{
			case NORTH:
				if ((oldXpos & 0x3FF) < 512)
					result = 0;
				break;
			case EAST:
				if ((oldZpos & 0x3FF) > 512)
					result = 0;
				break;
			case SOUTH:
				if ((oldXpos & 0x3FF) > 512)
					result = 0;
				break;
			case WEST:
				if ((oldZpos & 0x3FF) < 512)
					result = 0;
				break;
			}
			return result;
		}
		return false;
	}

	if (GetClimbFlags(x, item->pos.yPos, z, item->roomNumber) & RightClimbTab[angle])
		return result;

	short front = LaraFloorFront(item, item->pos.yRot, 116);
	if (abs(front - coll->Front.Floor) > 60)
		return 0;

	if (front < -768)
		return 0;

	return result;
}

int TestLaraValidHangPos(ITEM_INFO* item, COLL_INFO* coll)
{
	if (LaraFloorFront(item, Lara.moveAngle, 100) < 200)
		return 0;

	short angle = GetQuadrant(item->pos.yRot);
	switch (angle)
	{
	case NORTH:
		item->pos.zPos += 4;
		break;
	case EAST:
		item->pos.xPos += 4;
		break;
	case SOUTH:
		item->pos.zPos -= 4;
		break;
	case WEST:
		item->pos.xPos -= 4;
		break;
	default:
		break;
	}

	coll->Setup.BadHeightDown = NO_BAD_POS;
	coll->Setup.BadHeightUp = -512;
	coll->Setup.BadCeilingHeight = 0;

	Lara.moveAngle = item->pos.yRot;

	coll->Setup.ForwardAngle = Lara.moveAngle;
	GetCollisionInfo(coll, item);

	if (coll->Middle.Ceiling >= 0 || coll->CollisionType != CT_FRONT || coll->HitStatic)
		return 0;

	return abs(coll->Front.Floor - coll->FrontRight.Floor) < 60;
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

	if (LaraTestClimbPos(item, coll->Setup.Radius, coll->Setup.Radius, bounds->Y1, bounds->Y2 - bounds->Y1, &shift) &&
		LaraTestClimbPos(item, coll->Setup.Radius, -coll->Setup.Radius, bounds->Y1, bounds->Y2 - bounds->Y1, &shift))
	{
		result = LaraTestClimbPos(item, coll->Setup.Radius, 0, bounds->Y1, bounds->Y2 - bounds->Y1, &shift);
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

	if (abs(coll->FrontLeft.Floor - coll->FrontRight.Floor) >= SLOPE_DIFFERENCE)
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

	z += phd_cos(angle) * STEP_SIZE;
	x += phd_sin(angle) * STEP_SIZE;

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

	z += phd_cos(angle) * STEP_SIZE;
	x += phd_sin(angle) * STEP_SIZE;

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

	switch (GetQuadrant(Lara.moveAngle))
	{
	case 0:
		z += 16;
		break;
	case 1:
		x += 16;
		break;
	case 2:
		z -= 16;
		break;
	case 3:
		x -= 16;
		break;
	}

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

bool TestLaraFacingCorner(ITEM_INFO* item, short ang, int dist)
{
	// TODO: Objects? Lara will attempt to jump against them.
	// TODO: Check for ceilings! @Sezz 2021.10.16

	auto x = item->pos.xPos;
	auto y = item->pos.yPos;
	auto z = item->pos.zPos;

	auto angleA = ang + ANGLE(15.0f);
	auto angleB = ang - ANGLE(15.0f);

	auto probeA = GetCollisionResult(item, angleA, dist, 0);
	auto probeB = GetCollisionResult(item, angleB, dist, 0);

	if (probeA.Position.Floor - y < -STEPUP_HEIGHT ||
		probeB.Position.Floor - y < -STEPUP_HEIGHT)
	{
		return true;
	}

	return false;
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
	if (coll->CollisionType == CT_RIGHT || coll->CollisionType == CT_LEFT)
		return false;

return true;
}

bool IsStandingWeapon(LARA_WEAPON_TYPE gunType)
{
	if (gunType == WEAPON_SHOTGUN ||
		gunType == WEAPON_HK ||
		gunType == WEAPON_CROSSBOW ||
		gunType == WEAPON_TORCH ||
		gunType == WEAPON_GRENADE_LAUNCHER ||
		gunType == WEAPON_HARPOON_GUN ||
		gunType == WEAPON_ROCKET_LAUNCHER ||
		gunType == WEAPON_SNOWMOBILE)
	{
		return true;
	}

	return false;
}

// TODO: Try using each state's BadStep up/down.  @Sezz 2021.10.11
bool TestLaraStep(COLL_INFO* coll)
{
	if (coll->Middle.Floor >= -STEPUP_HEIGHT &&
		coll->Middle.Floor <= STEPUP_HEIGHT)
	{
		return true;
	}

	return false;
}

bool TestLaraStepUp(ITEM_INFO* item, COLL_INFO* coll)
{
	if (coll->Middle.Floor < -STEP_SIZE / 2 &&
		coll->Middle.Floor >= -STEPUP_HEIGHT &&
		coll->Middle.Floor != NO_HEIGHT &&
		item->currentAnimState != LS_WALK_BACK &&
		item->currentAnimState != LS_HOP_BACK &&
		item->currentAnimState != LS_SPRINT)
	{
		return true;
	}

	return false;
}

bool TestLaraStepDown(ITEM_INFO* item, COLL_INFO* coll)
{
	if (coll->Middle.Floor > STEP_SIZE / 2 &&
		coll->Middle.Floor <= STEPUP_HEIGHT &&
		coll->Middle.Floor != NO_HEIGHT &&
		item->currentAnimState != LS_RUN_FORWARD &&
		item->currentAnimState != LS_HOP_BACK &&
		item->currentAnimState != LS_SPRINT)
	{
		return true;
	}

	return false;
}

bool TestLaraFall(COLL_INFO* coll)
{
	if (coll->Middle.Floor <= STEPUP_HEIGHT ||
		Lara.waterStatus == LW_WADE)	// TODO: This causes a legacy floor snap BUG when lara wades off a ledge into a dry room. @Sezz 2021.09.26
	{
		return false;
	}

	return true;
}

// TODO: Remane when legacy TestLaraSlide() is removed.
bool TestLaraSlideNew(COLL_INFO* coll)
{
	if (abs(coll->TiltX) <= 2 && abs(coll->TiltZ) <= 2)
		return false;

	return true;
}

bool TestLaraStepLeft(ITEM_INFO* item)
{
	auto collFloorResult = LaraCollisionFront(item, item->pos.yRot - ANGLE(90.0f), LARA_RAD + 48);
	auto collCeilingResult = LaraCeilingCollisionFront(item, item->pos.yRot - ANGLE(90.0f), LARA_RAD + 48, LARA_HEIGHT);

	if ((collFloorResult.Position.Floor < STEP_SIZE / 2 && collFloorResult.Position.Floor > -STEP_SIZE / 2) &&
		collCeilingResult.Position.Ceiling <= 0 &&
		!collFloorResult.Position.Slope)
	{
		return true;
	}

	return false;
}

bool TestLaraStepRight(ITEM_INFO* item)
{
	auto collFloorResult = LaraCollisionFront(item, item->pos.yRot + ANGLE(90.0f), LARA_RAD + 48);
	auto collCeilingResult = LaraCeilingCollisionFront(item, item->pos.yRot + ANGLE(90.0f), LARA_RAD + 48, LARA_HEIGHT);

	if ((collFloorResult.Position.Floor < STEP_SIZE / 2 && collFloorResult.Position.Floor > -STEP_SIZE / 2) &&
		collCeilingResult.Position.Ceiling <= 0 &&
		!collFloorResult.Position.Slope)
	{
		return true;
	}

	return false;
}

bool TestLaraCrawl(ITEM_INFO* item)
{
	if (Lara.gunStatus == LG_NO_ARMS &&
		/*Lara.waterSurfaceDist >= -STEP_SIZE &&*/
		!(TrInput & (IN_FLARE | IN_DRAW)) &&					// Avoid unsightly concurrent actions.
		(Lara.gunType != WEAPON_FLARE || Lara.flareAge > 0))	// Flare is not being handled.
	{
		return true;
	}

	return false;
}

bool TestLaraCrouchRoll(ITEM_INFO* item, COLL_INFO* coll)
{
	// This is a discrete probe and fails in many cases. Perhaps we need a ray for these kinds of tests. @Sezz 2021.10.14
	// Instances of failure:
	// - Facing thin geometry
	// - Facing drop at the top of an incline
	// - Facing small step to the side of the bottom of an incline
	// - Facing step from an incline, beyond which is a flat descent of one step

	// Ceiling?
	auto y = item->pos.yPos;
	auto angle = coll->Setup.ForwardAngle;
	auto probe = GetCollisionResult(item, angle, WALL_SIZE, 0);

	if (Lara.gunStatus == LG_NO_ARMS &&
		Lara.waterSurfaceDist >= -STEP_SIZE &&					// Water depth is optically feasible for action.
		probe.Position.Floor - y < STEP_SIZE &&					// No drop.
		probe.Position.Floor - y > -STEP_SIZE &&				// No wall.
		!probe.Position.Slope &&								// No slope.
		!(TrInput & (IN_FLARE | IN_DRAW)) &&					// Avoid unsightly concurrent actions.
		(Lara.gunType != WEAPON_FLARE || Lara.flareAge > 0))	// Not handling flare.
	{
		return true;
	}

	return false;
}

// BUG: If Lara crawls up/down into a lower area under a slanted ceiling, she will sometimes teleport back. @Sezz 2021.10.16
bool TestLaraCrawlUpStep(ITEM_INFO* item, COLL_INFO* coll)
{
	auto y = item->pos.yPos;
	auto angle = coll->Setup.ForwardAngle;
	auto probe = GetCollisionResult(item, angle, STEP_SIZE, 0);

	if (probe.Position.Floor - y == -STEP_SIZE &&										// TODO: floor boundary
		abs(probe.Position.Ceiling - probe.Position.Floor) >= LARA_HEIGHT_CRAWL &&		// Space is not a clamp. TODO: coll->Setup.Height not working?
		probe.Position.Ceiling - y != NO_HEIGHT)
	{
		return true;
	}

	return false;
}

bool TestLaraCrawlDownStep(ITEM_INFO* item, COLL_INFO* coll)
{
	auto y = item->pos.yPos;
	auto angle = coll->Setup.ForwardAngle;
	auto probe = GetCollisionResult(item, angle, STEP_SIZE, 0);

	if (probe.Position.Floor - y == STEP_SIZE &&										// TODO: floor boundary.
		probe.Position.Ceiling - y <= -(STEP_SIZE / 2) &&								// Ceiling lower boundary.
		abs(probe.Position.Ceiling - probe.Position.Floor) >= LARA_HEIGHT_CRAWL &&		// Space is not a clamp. TODO: coll->Setup.Height not working?
		probe.Position.Ceiling - y != NO_HEIGHT)
	{
		return true;
	}

	return false;
}

bool TestLaraCrawlExitDownStep(ITEM_INFO* item, COLL_INFO* coll)
{
	auto y = item->pos.yPos;
	auto angle = coll->Setup.ForwardAngle;
	auto probe = GetCollisionResult(item, angle, STEP_SIZE, 0);

	// TODO: Consider height of ceiling directly above. Lara could potentially exit where a very, very steep ceiling meets the crawlspace exit at a slant.
	if (probe.Position.Floor - y == STEP_SIZE &&										// TODO: floor boundary.
		probe.Position.Ceiling - y <= STEP_SIZE &&										// Ceiling lower boundary. Necessary?
		abs(probe.Position.Ceiling - probe.Position.Floor) >= LARA_HEIGHT &&			// Space is not a clamp. TODO: Consider headroom?
		probe.Position.Ceiling - y != NO_HEIGHT)
	{
		return true;
	}

	return false;
}

bool TestLaraCrawlExitJump(ITEM_INFO* item, COLL_INFO* coll)
{
	auto y = item->pos.yPos;
	auto angle = coll->Setup.ForwardAngle;
	auto probe = GetCollisionResult(item, angle, STEP_SIZE, 0);

	if (probe.Position.Floor - y > STEPUP_HEIGHT && // TODO: Harmonise with 1 step exit.
		probe.Position.Ceiling - y < LARA_HEIGHT && // Consider headroom?
		probe.Position.Floor - y != NO_HEIGHT)
	{
		return true;
	}

	return false;
}

bool TestLaraCrawlVault(ITEM_INFO* item, COLL_INFO* coll)
{
	auto y = item->pos.yPos;
	auto angle = coll->Setup.ForwardAngle;
	auto probe = GetCollisionResult(item, angle, STEP_SIZE, 0);

	if (abs(probe.Position.Floor - y) >= STEP_SIZE &&		// Upper/lower floor boundary.
		probe.Position.Floor - y != NO_HEIGHT)
	{
		return true;
	}

	return false;
}

// Entirely temporary. @Sezz 2021.10.16
bool TestLaraDrawWeaponsFromCrawlIdle(ITEM_INFO* item)
{
	if (item->animNumber == LA_CRAWL_IDLE ||
		(item->animNumber == LA_CROUCH_TO_CRAWL_START &&
			item->frameNumber >= g_Level.Anims[item->animNumber].frameBase + 8))
		return true;

	return false;
}
