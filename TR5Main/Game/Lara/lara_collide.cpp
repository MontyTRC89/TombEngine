#include "framework.h"
#include "Game/Lara/lara_collide.h"

#include "Game/animation.h"
#include "Game/collision/collide_room.h"
#include "Game/collision/collide_item.h"
#include "Game/control/control.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/Lara/lara_swim.h"
#include "Game/Lara/lara_tests.h"
#include "Specific/input.h"
#include "Specific/level.h"
#include "Specific/setup.h"
#include "Scripting/GameFlowScript.h"
#include "Scripting/GameScriptLevel.h"

// -----------------------------
// COLLISION TEST FUNCTIONS
// For State Control & Collision
// -----------------------------

bool LaraDeflectEdge(ITEM_INFO* item, CollisionInfo* coll)
{
	if (coll->CollisionType == CT_FRONT || coll->CollisionType == CT_TOP_FRONT)
	{
		ShiftItem(item, coll);

		item->Animation.TargetState = LS_IDLE;
		item->Animation.Velocity = 0;
		item->Animation.Airborne = false;
		return true;
	}

	if (coll->CollisionType == CT_LEFT)
	{
		ShiftItem(item, coll);
		item->Position.yRot += ANGLE(coll->DiagonalStepAtLeft() ? DEFLECT_DIAGONAL_ANGLE : DEFLECT_STRAIGHT_ANGLE);
	}
	else if (coll->CollisionType == CT_RIGHT)
	{
		ShiftItem(item, coll);
		item->Position.yRot -= ANGLE(coll->DiagonalStepAtRight() ? DEFLECT_DIAGONAL_ANGLE : DEFLECT_STRAIGHT_ANGLE);
	}

	return false;
}

bool LaraDeflectEdgeJump(ITEM_INFO* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	if (coll->CollisionType != CT_NONE)
		ShiftItem(item, coll);

	if (coll->CollisionType == CT_FRONT || coll->CollisionType == CT_TOP_FRONT)
	{
		if (!lara->Control.CanClimbLadder || item->Animation.Velocity != 2)
		{
			if (coll->Middle.Floor <= CLICK(1))
			{
				SetAnimation(item, LA_LAND);
				LaraSnapToHeight(item, coll);
			}
			else if (abs(item->Animation.Velocity) > 50) // TODO: Tune and demagic this value.
				SetAnimation(item, LA_JUMP_WALL_SMASH_START, 1);

			item->Animation.Velocity /= 4;
			lara->Control.MoveAngle += ANGLE(180.0f);

			if (item->Animation.VerticalVelocity <= 0)
				item->Animation.VerticalVelocity = 1;
		}

		return true;
	}

	switch (coll->CollisionType)
	{
	case CT_LEFT:
		item->Position.yRot += ANGLE(DEFLECT_STRAIGHT_ANGLE);
		break;

	case CT_RIGHT:
		item->Position.yRot -= ANGLE(DEFLECT_STRAIGHT_ANGLE);
		break;

	case CT_TOP:
	case CT_TOP_FRONT:
		if (item->Animation.VerticalVelocity <= 0)
			item->Animation.VerticalVelocity = 1;

		break;

	case CT_CLAMP:
		item->Position.zPos -= 400 * phd_cos(coll->Setup.ForwardAngle);
		item->Position.xPos -= 400 * phd_sin(coll->Setup.ForwardAngle);
		item->Animation.Velocity = 0;
		coll->Middle.Floor = 0;

		if (item->Animation.VerticalVelocity <= 0)
			item->Animation.VerticalVelocity = 16;

		break;
	}

	return false;
}

void LaraSlideEdgeJump(ITEM_INFO* item, CollisionInfo* coll)
{
	ShiftItem(item, coll);

	switch (coll->CollisionType)
	{
	case CT_LEFT:
		item->Position.yRot += ANGLE(DEFLECT_STRAIGHT_ANGLE);
		break;

	case CT_RIGHT:
		item->Position.yRot -= ANGLE(DEFLECT_STRAIGHT_ANGLE);
		break;

	case CT_TOP:
	case CT_TOP_FRONT:
		if (item->Animation.VerticalVelocity <= 0)
			item->Animation.VerticalVelocity = 1;

		break;

	case CT_CLAMP:
		item->Position.zPos -= 400 * phd_cos(coll->Setup.ForwardAngle);
		item->Position.xPos -= 400 * phd_sin(coll->Setup.ForwardAngle);
		item->Animation.Velocity = 0;
		coll->Middle.Floor = 0;

		if (item->Animation.VerticalVelocity <= 0)
			item->Animation.VerticalVelocity = 16;

		break;
	}
}

bool LaraDeflectEdgeCrawl(ITEM_INFO* item, CollisionInfo* coll)
{
	// Useless in the best case; Lara does not have to embed in order to perform climbing actions in crawl states. Keeping for security. @Sezz 2021.11.26
	if (coll->CollisionType == CT_FRONT || coll->CollisionType == CT_TOP_FRONT)
	{
		ShiftItem(item, coll);

		item->Animation.Velocity = 0;
		item->Animation.Airborne = false;
		return true;
	}

	if (coll->CollisionType == CT_LEFT)
	{
		ShiftItem(item, coll);
		item->Position.yRot += ANGLE(coll->DiagonalStepAtLeft() ? DEFLECT_DIAGONAL_ANGLE_CRAWL : DEFLECT_STRAIGHT_ANGLE_CRAWL);
	}
	else if (coll->CollisionType == CT_RIGHT)
	{
		ShiftItem(item, coll);
		item->Position.yRot -= ANGLE(coll->DiagonalStepAtRight() ? DEFLECT_DIAGONAL_ANGLE_CRAWL : DEFLECT_STRAIGHT_ANGLE_CRAWL);
	}

	return false;
}

bool LaraDeflectEdgeMonkey(ITEM_INFO* item, CollisionInfo* coll)
{
	if (coll->CollisionType == CT_FRONT || coll->CollisionType == CT_TOP_FRONT ||
		coll->HitTallObject)
	{
		ShiftItem(item, coll);

		item->Animation.TargetState = LS_MONKEY_IDLE;
		item->Animation.Velocity = 0;
		item->Animation.Airborne = false;
		return true;
	}

	if (coll->CollisionType == CT_LEFT)
	{
		ShiftItem(item, coll);
		item->Position.yRot += ANGLE(coll->DiagonalStepAtLeft() ? DEFLECT_DIAGONAL_ANGLE : DEFLECT_STRAIGHT_ANGLE);
	}
	else if (coll->CollisionType == CT_RIGHT)
	{
		ShiftItem(item, coll);
		item->Position.yRot -= ANGLE(coll->DiagonalStepAtRight() ? DEFLECT_DIAGONAL_ANGLE : DEFLECT_STRAIGHT_ANGLE);
	}

	return false;
}

void LaraCollideStop(ITEM_INFO* item, CollisionInfo* coll)
{
	switch (coll->Setup.OldState)
	{
	case LS_IDLE:
	case LS_TURN_RIGHT_SLOW:
	case LS_TURN_LEFT_SLOW:
	case LS_TURN_RIGHT_FAST:
	case LS_TURN_LEFT_FAST:
		item->Animation.ActiveState = coll->Setup.OldState;
		item->Animation.AnimNumber = coll->Setup.OldAnimNumber;
		item->Animation.FrameNumber = coll->Setup.OldFrameNumber;

		if (TrInput & IN_LEFT)
		{
			// Prevent turn lock against walls.
			if (item->Animation.ActiveState == LS_TURN_RIGHT_SLOW ||
				item->Animation.ActiveState == LS_TURN_RIGHT_FAST)
			{
				item->Animation.TargetState = LS_IDLE;
			}
			else
				item->Animation.TargetState = LS_TURN_LEFT_SLOW;
		}
		else if (TrInput & IN_RIGHT)
		{
			if (item->Animation.ActiveState == LS_TURN_LEFT_SLOW ||
				item->Animation.ActiveState == LS_TURN_LEFT_FAST)
			{
				item->Animation.TargetState = LS_IDLE;
			}
			else
				item->Animation.TargetState = LS_TURN_RIGHT_SLOW;
		}
		else
			item->Animation.TargetState = LS_IDLE;

		AnimateLara(item);

		break;

	default:
		item->Animation.TargetState = LS_IDLE;

		if (item->Animation.AnimNumber != LA_STAND_SOLID)
			SetAnimation(item, LA_STAND_SOLID);

		break;
	}
}

void LaraCollideStopCrawl(ITEM_INFO* item, CollisionInfo* coll)
{
	switch (coll->Setup.OldState)
	{
	case LS_CRAWL_IDLE:
	case LS_CRAWL_TURN_LEFT:
	case LS_CRAWL_TURN_RIGHT:
		item->Animation.ActiveState = coll->Setup.OldState;
		item->Animation.AnimNumber = coll->Setup.OldAnimNumber;
		item->Animation.FrameNumber = coll->Setup.OldFrameNumber;

		if (TrInput & IN_LEFT)
			item->Animation.TargetState = LS_CRAWL_TURN_LEFT;
		else if (TrInput & IN_RIGHT)
			item->Animation.TargetState = LS_CRAWL_TURN_RIGHT;
		else
			item->Animation.TargetState = LS_CRAWL_IDLE;

		AnimateLara(item);
		break;

	default:
		item->Animation.ActiveState = LS_CRAWL_IDLE;
		item->Animation.TargetState = LS_CRAWL_IDLE;

		if (item->Animation.AnimNumber != LA_CRAWL_IDLE)
		{
			item->Animation.AnimNumber = LA_CRAWL_IDLE;
			item->Animation.FrameNumber = GetFrameNumber(item, 0);
		}

		break;
	}
}

void LaraCollideStopMonkey(ITEM_INFO* item, CollisionInfo* coll)
{
	switch (coll->Setup.OldState)
	{
	case LS_MONKEY_IDLE:
	case LS_MONKEY_TURN_LEFT:
	case LS_MONKEY_TURN_RIGHT:
		item->Animation.ActiveState = coll->Setup.OldState;
		item->Animation.AnimNumber = coll->Setup.OldAnimNumber;
		item->Animation.FrameNumber = coll->Setup.OldFrameNumber;

		if (TrInput & IN_LEFT)
			item->Animation.TargetState = LS_MONKEY_TURN_LEFT;
		else if (TrInput & IN_RIGHT)
			item->Animation.TargetState = LS_MONKEY_TURN_RIGHT;
		else
			item->Animation.TargetState = LS_MONKEY_IDLE;

		AnimateLara(item);
		break;

	default:
		item->Animation.ActiveState = LS_MONKEY_IDLE;
		item->Animation.TargetState = LS_MONKEY_IDLE;

		if (item->Animation.AnimNumber != LA_MONKEY_IDLE)
		{
			item->Animation.AnimNumber = LA_MONKEY_IDLE;
			item->Animation.FrameNumber = GetFrameNumber(item, 0);
		}

		break;
	}
}

void LaraSnapToEdgeOfBlock(ITEM_INFO* item, CollisionInfo* coll, short angle)
{
	if (item->Animation.ActiveState == LS_SHIMMY_RIGHT)
	{
		switch (angle)
		{
		case NORTH:
			item->Position.xPos = coll->Setup.OldPosition.x & 0xFFFFFF90 | 0x390;
			return;

		case EAST:
			item->Position.zPos = coll->Setup.OldPosition.z & 0xFFFFFC70 | 0x70;
			return;

		case SOUTH:
			item->Position.xPos = coll->Setup.OldPosition.x & 0xFFFFFC70 | 0x70;
			return;

		case WEST:
		default:
			item->Position.zPos = coll->Setup.OldPosition.z & 0xFFFFFF90 | 0x390;
			return;
		}
	}

	if (item->Animation.ActiveState == LS_SHIMMY_LEFT)
	{
		switch (angle)
		{
		case NORTH:
			item->Position.xPos = coll->Setup.OldPosition.x & 0xFFFFFC70 | 0x70;
			return;

		case EAST:
			item->Position.zPos = coll->Setup.OldPosition.z & 0xFFFFFF90 | 0x390;
			return;

		case SOUTH:
			item->Position.xPos = coll->Setup.OldPosition.x & 0xFFFFFF90 | 0x390;
			return;

		case WEST:
		default:
			item->Position.zPos = coll->Setup.OldPosition.z & 0xFFFFFC70 | 0x70;
			return;
		}
	}
}

void LaraResetGravityStatus(ITEM_INFO* item, CollisionInfo* coll)
{
	// This routine cleans gravity status flag and VerticalVelocity, making it
	// impossible to perform bugs such as QWOP and flare jump. Found by Troye -- Lwmte, 25.09.2021

	if (coll->Middle.Floor <= STEPUP_HEIGHT)
	{
		item->Animation.VerticalVelocity = 0;
		item->Animation.Airborne = false;
	}
}

void LaraSnapToHeight(ITEM_INFO* item, CollisionInfo* coll)
{
	if (TestEnvironment(ENV_FLAG_SWAMP, item) && coll->Middle.Floor > 0)
		item->Position.yPos += SWAMP_GRAVITY;
	else if (coll->Middle.Floor != NO_HEIGHT)
		item->Position.yPos += coll->Middle.Floor;
}

void GetLaraDeadlyBounds()
{
	BOUNDING_BOX tBounds;
	auto* bounds = GetBoundsAccurate(LaraItem);
	phd_RotBoundingBoxNoPersp(&LaraItem->Position, bounds, &tBounds);

	DeadlyBounds[0] = LaraItem->Position.xPos + tBounds.X1;
	DeadlyBounds[1] = LaraItem->Position.xPos + tBounds.X2;
	DeadlyBounds[2] = LaraItem->Position.yPos + tBounds.Y1;
	DeadlyBounds[3] = LaraItem->Position.yPos + tBounds.Y2;
	DeadlyBounds[4] = LaraItem->Position.zPos + tBounds.Z1;
	DeadlyBounds[5] = LaraItem->Position.zPos + tBounds.Z2;
}

void LaraJumpCollision(ITEM_INFO* item, CollisionInfo* coll, short moveAngle)
{
	auto* lara = GetLaraInfo(item);

	lara->Control.MoveAngle = moveAngle;
	coll->Setup.LowerFloorBound = NO_LOWER_BOUND;
	coll->Setup.UpperFloorBound = -STEPUP_HEIGHT;
	coll->Setup.LowerCeilingBound = BAD_JUMP_CEILING;
	coll->Setup.ForwardAngle = lara->Control.MoveAngle;
	GetCollisionInfo(coll, item);

	LaraDeflectEdgeJump(item, coll);
}

void LaraSurfaceCollision(ITEM_INFO* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	coll->Setup.ForwardAngle = lara->Control.MoveAngle;

	GetCollisionInfo(coll, item, PHD_VECTOR(0, LARA_HEIGHT_SURFSWIM, 0));
	ShiftItem(item, coll);

	if (coll->CollisionType & (CT_FRONT | CT_TOP | CT_TOP_FRONT | CT_CLAMP) ||
		coll->Middle.Floor < 0 && coll->Middle.FloorSlope)
	{
		item->Animation.VerticalVelocity = 0;
		item->Position.xPos = coll->Setup.OldPosition.x;
		item->Position.yPos = coll->Setup.OldPosition.y;
		item->Position.zPos = coll->Setup.OldPosition.z;
	}
	else if (coll->CollisionType == CT_LEFT)
		item->Position.yRot += ANGLE(5.0f);
	else if (coll->CollisionType == CT_RIGHT)
		item->Position.yRot -= ANGLE(5.0f);

	if (GetWaterHeight(item->Position.xPos, item->Position.yPos, item->Position.zPos, item->RoomNumber) - item->Position.yPos > -100)
		TestLaraWaterStepOut(item, coll);
	else
		SetLaraSwimDiveAnimation(item);
}

void LaraSwimCollision(ITEM_INFO* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	int oldX = item->Position.xPos;
	int oldY = item->Position.yPos;
	int oldZ = item->Position.zPos;
	short oldXrot = item->Position.xRot;
	short oldYrot = item->Position.yRot;
	short oldZrot = item->Position.zRot;

	if (item->Position.xRot < -ANGLE(90.0f) ||
		item->Position.xRot > ANGLE(90.0f))
	{
		lara->Control.MoveAngle = item->Position.yRot + ANGLE(180.0f);
		coll->Setup.ForwardAngle = item->Position.yRot - ANGLE(180.0f);
	}
	else
	{
		lara->Control.MoveAngle = item->Position.yRot;
		coll->Setup.ForwardAngle = item->Position.yRot;
	}

	int height = LARA_HEIGHT * phd_sin(item->Position.xRot);
	height = abs(height);

	auto* level = g_GameFlow->GetLevel(CurrentLevel);
	if (height < ((level->LaraType == LaraType::Divesuit) << 6) + 200)
		height = ((level->LaraType == LaraType::Divesuit) << 6) + 200;

	coll->Setup.UpperFloorBound = -CLICK(0.25f);
	coll->Setup.Height = height;

	GetCollisionInfo(coll, item, PHD_VECTOR(0, height / 2, 0));

	auto c1 = *coll;
	c1.Setup.ForwardAngle += ANGLE(45.0f);
	GetCollisionInfo(&c1, item, PHD_VECTOR(0, height / 2, 0));

	auto c2 = *coll;
	c2.Setup.ForwardAngle -= ANGLE(45.0f);
	GetCollisionInfo(&c2, item, PHD_VECTOR(0, height / 2, 0));

	ShiftItem(item, coll);

	int flag = 0;

	switch (coll->CollisionType)
	{
	case CT_FRONT:
		if (item->Position.xRot <= ANGLE(25.0f))
		{
			if (item->Position.xRot >= -ANGLE(25.0f))
			{
				if (item->Position.xRot > ANGLE(5.0f))
					item->Position.xRot += ANGLE(0.5f);
				else if (item->Position.xRot < -ANGLE(5.0f))
					item->Position.xRot -= ANGLE(0.5f);
				else if (item->Position.xRot > 0)
					item->Position.xRot += 45;
				else if (item->Position.xRot < 0)
					item->Position.xRot -= 45;
				else
				{
					item->Animation.VerticalVelocity = 0;
					flag = 1;
				}
			}
			else
			{
				item->Position.xRot -= ANGLE(1.0f);
				flag = 1;
			}
		}
		else
		{
			item->Position.xRot += ANGLE(1.0f);
			flag = 1;
		}

		if (c1.CollisionType == CT_LEFT)
			item->Position.yRot += ANGLE(2.0f);
		else if (c1.CollisionType == CT_RIGHT)
			item->Position.yRot -= ANGLE(2.0f);
		else if (c2.CollisionType == CT_LEFT)
			item->Position.yRot += ANGLE(2.0f);
		else if (c2.CollisionType == CT_RIGHT)
			item->Position.yRot -= ANGLE(2.0f);

		break;

	case CT_TOP:
		if (item->Position.xRot >= -ANGLE(45.0f))
		{
			item->Position.xRot -= ANGLE(1.0f);
			flag = 1;
		}

		break;

	case CT_TOP_FRONT:
		item->Animation.VerticalVelocity = 0;
		flag = 1;
		break;

	case CT_LEFT:
		item->Position.yRot += ANGLE(2.0f);
		flag = 1;
		break;

	case CT_RIGHT:
		item->Position.yRot -= ANGLE(2.0f);
		flag = 1;
		break;

	case CT_CLAMP:
		item->Position.xPos = coll->Setup.OldPosition.x;
		item->Position.yPos = coll->Setup.OldPosition.y;
		item->Position.zPos = coll->Setup.OldPosition.z;
		item->Animation.VerticalVelocity = 0;
		flag = 2;
		break;
	}

	if (coll->Middle.Floor < 0 && coll->Middle.Floor != NO_HEIGHT)
	{
		flag = 1;
		item->Position.xRot += ANGLE(1.0f);
		item->Position.yPos += coll->Middle.Floor;
	}

	if (oldX == item->Position.xPos &&
		oldY == item->Position.yPos &&
		oldZ == item->Position.zPos &&
		oldXrot == item->Position.xRot &&
		oldYrot == item->Position.yRot ||
		flag != 1)
	{
		if (flag == 2)
			return;
	}

	if (lara->Control.WaterStatus != WaterStatus::FlyCheat && lara->ExtraAnim == NO_ITEM)
		TestLaraWaterDepth(item, coll);
}

void LaraWaterCurrent(ITEM_INFO* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	if (lara->WaterCurrentActive)
	{
		auto* sink = &g_Level.Sinks[lara->WaterCurrentActive - 1];

		short angle = mGetAngle(sink->x, sink->z, item->Position.xPos, item->Position.zPos);
		lara->WaterCurrentPull.x += (sink->strength * SECTOR(1) * phd_sin(angle - ANGLE(90.0f)) - lara->WaterCurrentPull.x) / 16;
		lara->WaterCurrentPull.z += (sink->strength * SECTOR(1) * phd_cos(angle - ANGLE(90.0f)) - lara->WaterCurrentPull.z) / 16;

		item->Position.yPos += (sink->y - item->Position.yPos) >> 4;
	}
	else
	{
		int shift = 0;

		if (abs(lara->WaterCurrentPull.x) <= 16)
			shift = (abs(lara->WaterCurrentPull.x) > 8) + 2;
		else
			shift = 4;
		lara->WaterCurrentPull.x -= lara->WaterCurrentPull.x >> shift;

		if (abs(lara->WaterCurrentPull.x) < 4)
			lara->WaterCurrentPull.x = 0;

		if (abs(lara->WaterCurrentPull.z) <= 16)
			shift = (abs(lara->WaterCurrentPull.z) > 8) + 2;
		else
			shift = 4;
		lara->WaterCurrentPull.z -= lara->WaterCurrentPull.z >> shift;

		if (abs(lara->WaterCurrentPull.z) < 4)
			lara->WaterCurrentPull.z = 0;

		if (!lara->WaterCurrentPull.x && !lara->WaterCurrentPull.z)
			return;
	}

	item->Position.xPos += lara->WaterCurrentPull.x >> 8;
	item->Position.zPos += lara->WaterCurrentPull.z >> 8;
	lara->WaterCurrentActive = 0;

	coll->Setup.ForwardAngle = phd_atan(item->Position.zPos - coll->Setup.OldPosition.z, item->Position.xPos - coll->Setup.OldPosition.x);
	coll->Setup.Height = LARA_HEIGHT_CRAWL;
	GetCollisionInfo(coll, item, PHD_VECTOR(0, 200, 0));

	if (coll->CollisionType == CT_FRONT)
	{
		if (item->Position.xRot > ANGLE(35.0f))
			item->Position.xRot += ANGLE(1.0f);
		else if (item->Position.xRot < -ANGLE(35.0f))
			item->Position.xRot -= ANGLE(1.0f);
		else
			item->Animation.VerticalVelocity = 0;
	}
	else if (coll->CollisionType == CT_TOP)
		item->Position.xRot -= ANGLE(1.0f);
	else if (coll->CollisionType == CT_TOP_FRONT)
		item->Animation.VerticalVelocity = 0;
	else if (coll->CollisionType == CT_LEFT)
		item->Position.yRot += ANGLE(5.0f);
	else if (coll->CollisionType == CT_RIGHT)
		item->Position.yRot -= ANGLE(5.0f);

	if (coll->Middle.Floor < 0 && coll->Middle.Floor != NO_HEIGHT)
		item->Position.yPos += coll->Middle.Floor;

	ShiftItem(item, coll);

	coll->Setup.OldPosition.x = item->Position.xPos;
	coll->Setup.OldPosition.y = item->Position.yPos;
	coll->Setup.OldPosition.z = item->Position.zPos;
}

bool TestLaraHitCeiling(CollisionInfo* coll)
{
	if (coll->CollisionType == CT_TOP ||
		coll->CollisionType == CT_CLAMP)
	{
		return true;
	}

	return false;
}

void SetLaraHitCeiling(ITEM_INFO* item, CollisionInfo* coll)
{
	item->Position.xPos = coll->Setup.OldPosition.x;
	item->Position.yPos = coll->Setup.OldPosition.y;
	item->Position.zPos = coll->Setup.OldPosition.z;

	item->Animation.Velocity = 0;
	item->Animation.VerticalVelocity = 0;
	item->Animation.Airborne = false;
}

bool TestLaraObjectCollision(ITEM_INFO* item, short angle, int distance, int height, int side)
{
	auto oldPos = item->Position;
	int sideSign = copysign(1, side);

	item->Position.xPos += phd_sin(item->Position.yRot + angle) * distance + phd_cos(angle + ANGLE(90.0f) * sideSign) * abs(side);
	item->Position.yPos += height;
	item->Position.zPos += phd_cos(item->Position.yRot + angle) * distance + phd_sin(angle + ANGLE(90.0f) * sideSign) * abs(side);

	auto result = GetCollidedObjects(item, LARA_RAD, true, CollidedItems, CollidedMeshes, 0);

	item->Position = oldPos;
	return result;
}
