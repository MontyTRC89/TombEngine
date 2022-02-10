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

bool LaraDeflectEdge(ITEM_INFO* item, COLL_INFO* coll)
{
	if (coll->CollisionType == CT_FRONT || coll->CollisionType == CT_TOP_FRONT)
	{
		ShiftItem(item, coll);

		item->TargetState = LS_IDLE;
		item->Velocity = 0;
		item->Airborne = false;

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

bool LaraDeflectEdgeJump(ITEM_INFO* item, COLL_INFO* coll)
{
	if (coll->CollisionType != CT_NONE)
		ShiftItem(item, coll);

	if (coll->CollisionType == CT_FRONT || coll->CollisionType == CT_TOP_FRONT)
	{
		if (!Lara.Control.CanClimbLadder || item->Velocity != 2)
		{
			if (coll->Middle.Floor <= CLICK(1))
			{
				SetAnimation(item, LA_LAND);
				LaraSnapToHeight(item, coll);
			}
			else if (abs(item->Velocity) > 50) // TODO: Tune and demagic value.
				SetAnimation(item, LA_JUMP_WALL_SMASH_START, 1);

			item->Velocity /= 4;
			Lara.Control.MoveAngle += ANGLE(180.0f);

			if (item->VerticalVelocity <= 0)
				item->VerticalVelocity = 1;
		}

		return true;
	}

	if (coll->CollisionType == CT_TOP || coll->Middle.Ceiling >= 0)
	{
		if (item->VerticalVelocity <= 0)
			item->VerticalVelocity = 1;
	}
	else if (coll->CollisionType == CT_LEFT)
		item->Position.yRot += ANGLE(DEFLECT_STRAIGHT_ANGLE);
	else if (coll->CollisionType == CT_RIGHT)
		item->Position.yRot -= ANGLE(DEFLECT_STRAIGHT_ANGLE);
	else if (coll->CollisionType == CT_CLAMP)
	{
		item->Position.xPos -= 400 * phd_sin(coll->Setup.ForwardAngle);
		item->Position.zPos -= 400 * phd_cos(coll->Setup.ForwardAngle);

		item->Velocity = 0;
		coll->Middle.Floor = 0;

		if (item->VerticalVelocity <= 0)
			item->VerticalVelocity = 16;
	}

	return false;
}

void LaraSlideEdgeJump(ITEM_INFO* item, COLL_INFO* coll)
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
		if (item->VerticalVelocity <= 0)
			item->VerticalVelocity = 1;
		break;

	case CT_CLAMP:
		item->Position.zPos -= 400 * phd_cos(coll->Setup.ForwardAngle);
		item->Position.xPos -= 400 * phd_sin(coll->Setup.ForwardAngle);

		item->Velocity = 0;
		coll->Middle.Floor = 0;

		if (item->VerticalVelocity <= 0)
			item->VerticalVelocity = 16;

		break;
	}
}

bool LaraDeflectEdgeCrawl(ITEM_INFO* item, COLL_INFO* coll)
{
	// Useless in the best case; Lara does not have to embed in order to perform climbing actions in crawl states. Keeping for security. @Sezz 2021.11.26
	if (coll->CollisionType == CT_FRONT || coll->CollisionType == CT_TOP_FRONT)
	{
		ShiftItem(item, coll);

		item->Airborne = false;
		item->Velocity = 0;

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

bool LaraDeflectEdgeMonkey(ITEM_INFO* item, COLL_INFO* coll)
{
	if (coll->CollisionType == CT_FRONT || coll->CollisionType == CT_TOP_FRONT
		|| coll->HitTallObject)
	{
		ShiftItem(item, coll);

		item->TargetState = LS_MONKEY_IDLE;
		item->Velocity = 0;
		item->Airborne = false;

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

bool TestLaraHitCeiling(COLL_INFO* coll)
{
	if (coll->CollisionType == CT_TOP ||
		coll->CollisionType == CT_CLAMP)
	{
		return true;
	}

	return false;
}

void SetLaraHitCeiling(ITEM_INFO* item, COLL_INFO* coll)
{
	item->Position.xPos = coll->Setup.OldPosition.x;
	item->Position.yPos = coll->Setup.OldPosition.y;
	item->Position.zPos = coll->Setup.OldPosition.z;

	item->Velocity = 0;
	item->VerticalVelocity = 0;
	item->Airborne = false;
}

void LaraCollideStop(ITEM_INFO* item, COLL_INFO* coll)
{
	switch (coll->Setup.OldState)
	{
	case LS_IDLE:
	case LS_TURN_RIGHT_SLOW:
	case LS_TURN_LEFT_SLOW:
	case LS_TURN_RIGHT_FAST:
	case LS_TURN_LEFT_FAST:
		item->ActiveState = coll->Setup.OldState;
		item->AnimNumber = coll->Setup.OldAnimNumber;
		item->FrameNumber = coll->Setup.OldFrameNumber;

		if (TrInput & IN_LEFT)
		{
			// Prevent turn lock against walls.
			if (item->ActiveState == LS_TURN_RIGHT_SLOW ||
				item->ActiveState == LS_TURN_RIGHT_FAST)
			{
				item->TargetState = LS_IDLE;
			}
			else
				item->TargetState = LS_TURN_LEFT_SLOW;
		}
		else if (TrInput & IN_RIGHT)
		{
			if (item->ActiveState == LS_TURN_LEFT_SLOW ||
				item->ActiveState == LS_TURN_LEFT_FAST)
			{
				item->TargetState = LS_IDLE;
			}
			else
				item->TargetState = LS_TURN_RIGHT_SLOW;
		}
		else
			item->TargetState = LS_IDLE;

		AnimateLara(item);

		break;

	default:
		item->TargetState = LS_IDLE;

		if (item->AnimNumber != LA_STAND_SOLID)
			SetAnimation(item, LA_STAND_SOLID);

		break;
	}
}

void LaraCollideStopCrawl(ITEM_INFO* item, COLL_INFO* coll)
{
	switch (coll->Setup.OldState)
	{
	case LS_CRAWL_IDLE:
	case LS_CRAWL_TURN_LEFT:
	case LS_CRAWL_TURN_RIGHT:
		item->ActiveState = coll->Setup.OldState;
		item->AnimNumber = coll->Setup.OldAnimNumber;
		item->FrameNumber = coll->Setup.OldFrameNumber;

		if (TrInput & IN_LEFT)
			item->TargetState = LS_CRAWL_TURN_LEFT;
		else if (TrInput & IN_RIGHT)
			item->TargetState = LS_CRAWL_TURN_RIGHT;
		else
			item->TargetState = LS_CRAWL_IDLE;

		AnimateLara(item);
		break;

	default:
		item->ActiveState = LS_CRAWL_IDLE;
		item->TargetState = LS_CRAWL_IDLE;

		if (item->AnimNumber != LA_CRAWL_IDLE)
		{
			item->AnimNumber = LA_CRAWL_IDLE;
			item->FrameNumber = GetFrameNumber(item, 0);
		}

		break;
	}
}

void LaraCollideStopMonkey(ITEM_INFO* item, COLL_INFO* coll)
{
	switch (coll->Setup.OldState)
	{
	case LS_MONKEY_IDLE:
	case LS_MONKEY_TURN_LEFT:
	case LS_MONKEY_TURN_RIGHT:
		item->ActiveState = coll->Setup.OldState;
		item->AnimNumber = coll->Setup.OldAnimNumber;
		item->FrameNumber = coll->Setup.OldFrameNumber;

		if (TrInput & IN_LEFT)
			item->TargetState = LS_MONKEY_TURN_LEFT;
		else if (TrInput & IN_RIGHT)
			item->TargetState = LS_MONKEY_TURN_RIGHT;
		else
			item->TargetState = LS_MONKEY_IDLE;

		AnimateLara(item);
		break;

	default:
		item->ActiveState = LS_MONKEY_IDLE;
		item->TargetState = LS_MONKEY_IDLE;

		if (item->AnimNumber != LA_MONKEY_IDLE)
		{
			item->AnimNumber = LA_MONKEY_IDLE;
			item->FrameNumber = GetFrameNumber(item, 0);
		}

		break;
	}
}

void LaraSnapToEdgeOfBlock(ITEM_INFO* item, COLL_INFO* coll, short angle)
{
	if (item->ActiveState == LS_SHIMMY_RIGHT)
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

	if (item->ActiveState == LS_SHIMMY_LEFT)
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

void LaraResetGravityStatus(ITEM_INFO* item, COLL_INFO* coll)
{
	// This routine cleans gravity status flag and fallspeed, making it
	// impossible to perform bugs such as QWOP and flare jump. Found by Troye -- Lwmte, 25.09.2021

	if (coll->Middle.Floor <= STEPUP_HEIGHT)
	{
		item->Airborne = false;
		item->VerticalVelocity = 0;
	}
}

void LaraSnapToHeight(ITEM_INFO* item, COLL_INFO* coll)
{
	if (TestEnvironment(ENV_FLAG_SWAMP, item) && coll->Middle.Floor > 0)
		item->Position.yPos += SWAMP_GRAVITY;
	else if (coll->Middle.Floor != NO_HEIGHT)
		item->Position.yPos += coll->Middle.Floor;
}

void GetLaraDeadlyBounds()
{
	BOUNDING_BOX* bounds;
	BOUNDING_BOX tbounds;

	bounds = GetBoundsAccurate(LaraItem);
	phd_RotBoundingBoxNoPersp(&LaraItem->Position, bounds, &tbounds);

	DeadlyBounds[0] = LaraItem->Position.xPos + tbounds.X1;
	DeadlyBounds[1] = LaraItem->Position.xPos + tbounds.X2;
	DeadlyBounds[2] = LaraItem->Position.yPos + tbounds.Y1;
	DeadlyBounds[3] = LaraItem->Position.yPos + tbounds.Y2;
	DeadlyBounds[4] = LaraItem->Position.zPos + tbounds.Z1;
	DeadlyBounds[5] = LaraItem->Position.zPos + tbounds.Z2;
}

void LaraJumpCollision(ITEM_INFO* item, COLL_INFO* coll, short moveAngle)
{
	LaraInfo*& info = item->Data;

	info->Control.MoveAngle = moveAngle;
	coll->Setup.LowerFloorBound = NO_LOWER_BOUND;
	coll->Setup.UpperFloorBound = -STEPUP_HEIGHT;
	coll->Setup.LowerCeilingBound = BAD_JUMP_CEILING;
	coll->Setup.ForwardAngle = info->Control.MoveAngle;
	GetCollisionInfo(coll, item);

	if (TestLaraSlide(item, coll) && TestLaraLand(item, coll))
	{
		SetLaraSlideState(item, coll);
		SetLaraLand(item, coll);
		return;
	}

	LaraDeflectEdgeJump(item, coll);
}

void LaraSurfaceCollision(ITEM_INFO* item, COLL_INFO* coll)
{
	coll->Setup.ForwardAngle = Lara.Control.MoveAngle;

	GetCollisionInfo(coll, item, PHD_VECTOR(0, LARA_HEIGHT_SURFSWIM, 0));
	ShiftItem(item, coll);

	if (coll->CollisionType & (CT_FRONT | CT_TOP | CT_TOP_FRONT | CT_CLAMP) ||
		coll->Middle.Floor < 0 && coll->Middle.FloorSlope)
	{
		item->VerticalVelocity = 0;
		item->Position.xPos = coll->Setup.OldPosition.x;
		item->Position.yPos = coll->Setup.OldPosition.y;
		item->Position.zPos = coll->Setup.OldPosition.z;
	}
	else if (coll->CollisionType == CT_LEFT)
	{
		item->Position.yRot += ANGLE(5);
	}
	else if (coll->CollisionType == CT_RIGHT)
	{
		item->Position.yRot -= ANGLE(5);
	}

	if (GetWaterHeight(item->Position.xPos, item->Position.yPos, item->Position.zPos, item->RoomNumber) - item->Position.yPos > -100)
	{
		TestLaraWaterStepOut(item, coll);
	}
	else
		SwimDive(item);
}

void LaraSwimCollision(ITEM_INFO* item, COLL_INFO* coll)
{
	int oldX = item->Position.xPos;
	int oldY = item->Position.yPos;
	int oldZ = item->Position.zPos;
	short oldXrot = item->Position.xRot;
	short oldYrot = item->Position.yRot;
	short oldZrot = item->Position.zRot;

	if (item->Position.xRot < -ANGLE(90.0f) ||
		item->Position.xRot > ANGLE(90.0f))
	{
		Lara.Control.MoveAngle = item->Position.yRot + ANGLE(180.0f);
		coll->Setup.ForwardAngle = item->Position.yRot - ANGLE(180.0f);
	}
	else
	{
		Lara.Control.MoveAngle = item->Position.yRot;
		coll->Setup.ForwardAngle = item->Position.yRot;
	}

	int height = LARA_HEIGHT * phd_sin(item->Position.xRot);
	height = abs(height);

	auto level = g_GameFlow->GetLevel(CurrentLevel);
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
					item->VerticalVelocity = 0;
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
		if (item->Position.xRot >= -8190)
		{
			flag = 1;
			item->Position.xRot -= ANGLE(1.0f);
		}

		break;

	case CT_TOP_FRONT:
		item->VerticalVelocity = 0;
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
		flag = 2;
		item->Position.xPos = coll->Setup.OldPosition.x;
		item->Position.yPos = coll->Setup.OldPosition.y;
		item->Position.zPos = coll->Setup.OldPosition.z;
		item->VerticalVelocity = 0;

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

	if (Lara.Control.WaterStatus != WaterStatus::FlyCheat && Lara.ExtraAnim == NO_ITEM)
		TestLaraWaterDepth(item, coll);
}

bool TestLaraObjectCollision(ITEM_INFO* item, short angle, int dist, int height, int side)
{
	auto oldPos = item->Position;
	int sideSign = copysign(1, side);

	item->Position.xPos += phd_sin(item->Position.yRot + angle) * dist + phd_cos(angle + ANGLE(90.0f) * sideSign) * abs(side);
	item->Position.yPos += height;
	item->Position.zPos += phd_cos(item->Position.yRot + angle) * dist + phd_sin(angle + ANGLE(90.0f) * sideSign) * abs(side);

	auto result = GetCollidedObjects(item, LARA_RAD, true, CollidedItems, CollidedMeshes, 0);

	item->Position = oldPos;
	return result;
}
