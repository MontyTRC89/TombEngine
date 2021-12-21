#include "framework.h"
#include "lara_overhang.h"
#include "input.h"
#include "camera.h"
#include "control/control.h"
#include "lara_climb.h"
#include "lara_tests.h"
#include "collision/floordata.h"
#include "collision/collide_room.h"
#include "items.h"
#include "level.h"


#define SLOPE_CHECK (slope.x == goal.x && slope.y == goal.y) // HACK - convert to subfuncs 
#define SLOPE_INVERSE_CHECK  (slope.x == -goal.x && slope.y == -goal.y)

// **************  Utility functions section  *************** //

bool TestMonkey(FLOOR_INFO* floor, int x, int y, int z)
{
	return GetCollisionResult(floor, x, y, z).BottomBlock->Flags.Monkeyswing;
}

/* get the signed diff between two orients */
short DirOrientDiff(short orient1, short orient2)
{
	int diff = orient1 - orient2;

	if (diff > 32767)
		diff -= 65536;
	if (diff < -32768)
		diff += 65536;

	return short(diff);
}


/* test if inside sector strip (0-1023) in
currently faced quadrant, between min and max */
bool InStrip(int x, int z, short facing, int min, int max)
{
	if (min > SECTOR(1) - 1)
		min = SECTOR(1) - 1;
	if (max > SECTOR(1) - 1)
		max = SECTOR(1) - 1;

	int quadrant = GetQuadrant(facing);
	int dx = x & (SECTOR(1) - 1);
	int dz = z & (SECTOR(1) - 1);

	switch (quadrant)
	{
	case NORTH:
		if (dz >= (SECTOR(1) - 1 - max) && dz <= (SECTOR(1) - 1 - min))
			return true;
		break;

	case SOUTH:
		if (dz >= min && dz <= max)
			return true;
		break;

	case EAST:
		if (dx >= (SECTOR(1) - 1 - max) && dx <= (SECTOR(1) - 1 - min))
			return true;
		break;

	case WEST:
		if (dx >= min && dx <= max)
			return true;
		break;
	}

	return false;
}


/* align facing and X/Z pos to sector edge */
void AlignToEdge(ITEM_INFO* lara, WORD edgeDist)
{
	if (edgeDist > SECTOR(1) - 1)
		edgeDist = SECTOR(1) - 1;

	// align to closest cardinal facing
	lara->pos.yRot += 0x2000;
	lara->pos.yRot &= 0xC000;

	switch (lara->pos.yRot) // align to faced edge
	{
	case FACING_NORTH:
		lara->pos.zPos &= ~0x3FF;
		lara->pos.zPos += (SECTOR(1) - 1 - edgeDist);
		break;

	case FACING_SOUTH:
		lara->pos.zPos &= ~0x3FF;
		lara->pos.zPos += edgeDist + 1;
		break;

	case FACING_EAST:
		lara->pos.xPos &= ~0x3FF;
		lara->pos.xPos += (SECTOR(1) - 1 - edgeDist);
		break;

	case FACING_WEST:
		lara->pos.xPos &= ~0x3FF;
		lara->pos.xPos += edgeDist + 1;
		break;
	}
}


/* correct position after grabbing slope */
bool AlignToGrab(ITEM_INFO* lara)
{
	bool legLeft = false;

	lara->pos.yRot += 0x2000;
	lara->pos.yRot &= 0xC000;

	switch (lara->pos.yRot)
	{
	case FACING_NORTH:
		if (((lara->pos.zPos + 0x80) & ~0x3FF) == (lara->pos.zPos & ~0x3FF))
			lara->pos.zPos += 0x80;
		lara->pos.zPos = (lara->pos.zPos & ~0xFF) + 155;
		legLeft = (lara->pos.zPos & 0x100) ? false : true;
		break;

	case FACING_SOUTH:
		if (((lara->pos.zPos - 0x80) & ~0x3FF) == (lara->pos.zPos & ~0x3FF))
			lara->pos.zPos -= 0x80;
		lara->pos.zPos = (lara->pos.zPos & ~0xFF) + 101;
		legLeft = (lara->pos.zPos & 0x100) ? true : false;
		break;

	case FACING_WEST:
		if (((lara->pos.xPos - 0x80) & ~0x3FF) == (lara->pos.xPos & ~0x3FF))
			lara->pos.xPos -= 0x80;
		lara->pos.xPos = (lara->pos.xPos & ~0xFF) + 101;
		legLeft = (lara->pos.xPos & 0x100) ? true : false;
		break;

	case FACING_EAST:
		if (((lara->pos.xPos + 0x80) & ~0x3FF) == (lara->pos.xPos & ~0x3FF))
			lara->pos.xPos += 0x80;
		lara->pos.xPos = (lara->pos.xPos & ~0xFF) + 155;
		legLeft = (lara->pos.xPos & 0x100) ? false : true;
		break;
	}

	return legLeft;
}

void FillSlopeData(short orient, Vector2& goal, short& climbOrient, short& goalOrient, PHD_VECTOR& pos)
{
	int quadrant = (unsigned short)(orient + 0x2000) / 0x4000;

	switch (quadrant)
	{
	case NORTH:
		climbOrient = (short)CLIMB_DIRECTION::North;
		goalOrient = ANGLE(0);
		goal.y = 4;
		pos.z = CLICK(1);
		break;
	case EAST:
		climbOrient = (short)CLIMB_DIRECTION::East;
		goalOrient = ANGLE(90);
		goal.x = -4;
		pos.x = CLICK(1);
		break;
	case SOUTH:
		climbOrient = (short)CLIMB_DIRECTION::South;
		goalOrient = ANGLE(180);
		goal.y = -4;
		pos.z = -CLICK(1);
		break;
	case WEST:
		climbOrient = (short)CLIMB_DIRECTION::West;
		goalOrient = ANGLE(270);
		goal.x = 4;
		pos.x = -CLICK(1);
		break;
	}
}

void lara_col_slopeclimb(ITEM_INFO* lara, COLL_INFO* coll)
{
	GetCollisionInfo(coll, lara);

	PHD_VECTOR now = { lara->pos.xPos, lara->pos.yPos, lara->pos.zPos };
	PHD_VECTOR offset = { 0, 0, 0 };
	Vector2 slope = { 0, 0 };
	Vector2 goal = { 0, 0 };
	short climbOrient = 0;
	short goalOrient = 0;

	FillSlopeData(lara->pos.yRot, goal, climbOrient, goalOrient, offset);
	PHD_VECTOR up = { lara->pos.xPos - offset.x, lara->pos.yPos - CLICK(1), lara->pos.zPos - offset.z };
	PHD_VECTOR down = { lara->pos.xPos + offset.x, lara->pos.yPos + CLICK(1), lara->pos.zPos + offset.z };

	auto collResultUp = GetCollisionResult(up.x, up.y, up.z, lara->roomNumber);
	auto collResultDown = GetCollisionResult(down.x, down.y, down.z, lara->roomNumber);

	short tempRoom = 0;

	if (lara->animNumber == LA_OVERHANG_LADDER_SLOPE_CONCAVE)
		return;

	FLOOR_INFO* floorNow = (FLOOR_INFO*)GetFloor(now.x, now.y, now.z, &(tempRoom = lara->roomNumber));
	int ceiling = GetCeiling(floorNow, now.x, now.y, now.z);
	lara->pos.yPos = ceiling + HEIGHT_ADJUST;

	// drop down if action not pressed
	if (!(TrInput & IN_ACTION))
	{
		SetAnimation(lara, lara->animNumber == LA_OVERHANG_IDLE_LEFT ? LA_OVERHANG_DROP_LEFT : LA_OVERHANG_DROP_RIGHT);
		return;
	}

	// engage shimmy mode if left (sidestep) or right (sidestep) key is pressed
	if (TrInput & IN_LEFT || TrInput & IN_RIGHT)
	{
		auto info = (LaraInfo*&)lara->data;
		info->nextCornerPos.zRot = (lara->animNumber == LA_OVERHANG_IDLE_LEFT) ? true : false; // HACK!
		SetAnimation(lara, lara->animNumber == LA_OVERHANG_IDLE_LEFT ? LA_OVERHANG_IDLE_2_HANG_LEFT : LA_OVERHANG_IDLE_2_HANG_RIGHT);
		return;
	}

	if (TrInput & IN_FORWARD) // UP key pressed
	{
		// test for ledge over slope
		tempRoom = collResultUp.Block->RoomAbove(up.x, up.z).value_or(NO_ROOM);
		if (tempRoom != NO_ROOM)
		{
			short oldRoomCam = Camera.pos.roomNumber;     //Trng.pGlobTomb4->pAdr->Camera.pCameraSrc->Room;
			short oldRoomTarg = Camera.target.roomNumber; // Trng.pGlobTomb4->pAdr->Camera.pCameraTarget->Room;

			FLOOR_INFO* testLedge = (FLOOR_INFO*)GetFloor(now.x, now.y - 3 * CLICK(1), now.z, &tempRoom);
			int ledgeCeiling = GetCeiling(testLedge, now.x, now.y - 3 * CLICK(1), now.z);
			int ledgeHeight = GetFloorHeight(testLedge, now.x, now.y - 3 * CLICK(1), now.z);
			if ((ledgeHeight - ledgeCeiling >= 3 * CLICK(1)) && abs((lara->pos.yPos - 688) - ledgeHeight) < 64)
			{
				AlignToEdge(lara, 868);
				SetAnimation(lara, LA_OVERHANG_LEDGE_VAULT_START); // ledge climb-up from slope
			}
		}

		// test for slope to overhead ladder transition (convex)
		if (GetClimbFlags(collResultUp.BottomBlock) & climbOrient &&
			InStrip(lara->pos.xPos, lara->pos.zPos, lara->pos.yRot, 3 * CLICK(1), 4 * CLICK(1)))
		{
			if (TestLaraWall(lara, 0, 0, -4 * CLICK(1)) != SPLAT_COLL::NONE &&
				GetCeiling(collResultUp.Block, up.x, up.y, up.z) - lara->pos.yPos <= 1456)  // check if a wall is actually there...
			{
				AlignToEdge(lara, 868);
				SetAnimation(lara, LA_OVERHANG_SLOPE_LADDER_CONVEX_START);
			}
		}

		// test for monkey at next position
		if (collResultUp.BottomBlock->Flags.Monkeyswing)
		{
			int yDiff = collResultUp.Position.Ceiling - ceiling;

			slope = collResultUp.Block->TiltXZ(up.x, up.z, false);

			int height; // height variable for bridge ceiling functions

			// test for upwards slope to climb
			short bridge1 = -1; // FindBridge(4, lara->pos.yRot, up, &height, -5 * CLICK(1) / 2, -3 * CLICK(1) / 2); FIXME
			if (yDiff >= -5 * CLICK(1) / 4 && yDiff <= -3 * CLICK(1) / 4 && (SLOPE_CHECK || bridge1 >= 0))
			{
				// do one more check for wall/ceiling step 2*offX/Z further to avoid lara sinking her head in wall/step
				FLOOR_INFO* testWall = (FLOOR_INFO*)GetFloor((up.x - offset.x), (up.y - CLICK(1)), (up.z - offset.z),
					&(tempRoom = lara->roomNumber));
				int testCeiling = GetCeiling(testWall, (up.x - offset.x), (up.y - CLICK(1)), (up.z - offset.z));

				if (!testWall->IsWall((up.x - offset.x), (up.z - offset.z)) && (ceiling - testCeiling) > 0x40) // no wall or downwards ceiling step
				{
					TranslateItem(lara, 0, -256, -256);
					SetAnimation(lara, lara->animNumber == LA_OVERHANG_IDLE_LEFT ? LA_OVERHANG_CLIMB_UP_LEFT : LA_OVERHANG_CLIMB_UP_RIGHT);
					//lara->goalAnimState = 62;
				}
			}

			// test for flat monkey (abs(slope) < 2)
			bridge1 = -1; // FindBridge(0, goalOrient, up, &height, -9 * CLICK(1) / 4, -5 * CLICK(1) / 4); FIXME
			if (bridge1 < 0)
				bridge1 = -1; // FindBridge(1, goalOrient, up, &height, -9 * CLICK(1) / 4, -5 * CLICK(1) / 4); FIXME

			if (yDiff > -CLICK(1) && yDiff <= -CLICK(1) / 2 &&
				((abs(slope.x) <= 2 && abs(slope.y) <= 2) || bridge1 >= 0))
				SetAnimation(lara, LA_OVERHANG_SLOPE_MONKEY_CONCAVE); // slope to overhead monkey transition (concave)
		}
	}
	else if (TrInput & IN_BACK)
	{
		// get floor_info 256 downstream of Lara
		FLOOR_INFO* floorNext = (FLOOR_INFO*)GetFloor(down.x, down.y, down.z, &(tempRoom = lara->roomNumber));

		if ((GetClimbFlags(GetCollisionResult(floorNow, lara->pos.xPos, lara->pos.yPos, lara->pos.zPos).BottomBlock) & climbOrient) &&
			InStrip(lara->pos.xPos, lara->pos.zPos, lara->pos.yRot, 0, CLICK(1)))
		{
			AlignToEdge(lara, 100);
			SetAnimation(lara, LA_OVERHANG_SLOPE_LADDER_CONCAVE); // slope to underlying ladder transition (concave)
			return;
		}

		if (TestMonkey(floorNext, down.x, down.y, down.z))
		{
			slope = floorNext->TiltXZ(down.x, down.z, false);

			int height;
			int yDiff = GetCeiling(floorNext, down.x, down.y, down.z) - ceiling;

			// test for flat monkey (abs(slope) < 2)
			short bridge1 = -1; // FindBridge(0, goalOrient, down, &height, -3 * CLICK(1), -2 * CLICK(1)); FIXME
			if (bridge1 < 0)
				bridge1 = -1; // FindBridge(1, goalOrient, down, &height, -3 * CLICK(1), -2 * CLICK(1)); FIXME

			if ((abs(yDiff) < CLICK(1) && abs(slope.x) <= 2 && abs(slope.y) <= 2) || bridge1 >= 0)
				SetAnimation(lara, LA_OVERHANG_SLOPE_MONKEY_CONVEX); // Force slope to underlying monkey transition (convex)

			// test for downwards slope to climb
			bridge1 = -1; // FindBridge(4, goalOrient, down, &height, -5 * CLICK(1) / 2, -3 * CLICK(1) / 2); FIXME
			if (yDiff >= 3 * CLICK(1) / 4 && yDiff <= 5 * CLICK(1) / 4 && (SLOPE_CHECK || bridge1 >= 0))
			{
				SetAnimation(lara, lara->animNumber == LA_OVERHANG_IDLE_LEFT ? LA_OVERHANG_CLIMB_DOWN_LEFT : LA_OVERHANG_CLIMB_DOWN_RIGHT);
				return;
			}
		}
	}
}


void lara_as_slopeclimb(ITEM_INFO* lara, COLL_INFO* coll)
{
	if (GlobalCounter % 2)
		lara->pos.xRot--;
	else
		lara->pos.xRot++;

	Camera.flags = 1;

	if (Camera.type != CAMERA_TYPE::CHASE_CAMERA)
		return;

	Camera.targetElevation = -3072;
	Camera.targetDistance = 1792;
	Camera.speed = 15;
}


void lara_as_slopefall(ITEM_INFO* lara, COLL_INFO* coll)
{
	lara->gravityStatus = true;

	if (GlobalCounter % 2)
		lara->pos.xRot--;
	else
		lara->pos.xRot++;

	Camera.flags = 1;

	if (Camera.type != CAMERA_TYPE::CHASE_CAMERA)
		return;

	Camera.targetElevation = -3072;
	Camera.targetDistance = 1792;
	Camera.speed = 15;

}


void lara_col_slopehang(ITEM_INFO* lara, COLL_INFO* coll)
{
	GetCollisionInfo(coll, lara);

	PHD_VECTOR now = { lara->pos.xPos, lara->pos.yPos, lara->pos.zPos };
	PHD_VECTOR offset = { 0, 0, 0 };
	Vector2 slope = { 0, 0 };
	Vector2 goal = { 0, 0 };
	short climbOrient = 0;
	short goalOrient = 0;

	FillSlopeData(lara->pos.yRot, goal, climbOrient, goalOrient, offset);

	short tempRoom = 0;

	FLOOR_INFO* floorNow = (FLOOR_INFO*)GetFloor(now.x, now.y, now.z, &(tempRoom = lara->roomNumber));
	int ceiling = GetCeiling(floorNow, now.x, now.y, now.z);
	lara->pos.yPos = ceiling + HEIGHT_ADJUST;

	// drop down if action not pressed 
	if (!(TrInput & IN_ACTION))
	{
		SetAnimation(lara, LA_OVERHANG_HANG_DROP);
		return;
	}

	if (lara->animNumber != LA_OVERHANG_HANG_SWING)
	{
		// return to climbing mode
		if (TrInput & IN_FORWARD || TrInput & IN_BACK)
		{
			auto info = (LaraInfo*&)lara->data;
			SetAnimation(lara, info->nextCornerPos.zRot ? LA_OVERHANG_HANG_2_IDLE_LEFT : LA_OVERHANG_HANG_2_IDLE_RIGHT); // HACK!
		}

		// shimmy control
		if (TrInput & IN_LEFT || TrInput & IN_RIGHT)
		{
			PHD_VECTOR shimmy = { now.x, now.y, now.z };
			short dir = 0;
			if (TrInput & IN_LEFT)
			{
				shimmy.x -= offset.z / 2;
				shimmy.z += offset.x / 2;
				dir = -16384;
			}
			else if (TrInput & IN_RIGHT)
			{
				shimmy.x += offset.z / 2;
				shimmy.z -= offset.x / 2;
				dir = 16384;
			}

			FLOOR_INFO* floorNext = (FLOOR_INFO*)GetFloor(shimmy.x, shimmy.y, shimmy.z, &(tempRoom = lara->roomNumber));

			if (TestMonkey(floorNext, shimmy.x, shimmy.y, shimmy.z))
			{
				slope = floorNext->TiltXZ(shimmy.x, shimmy.z, false);

				int yDiff = GetCeiling(floorNext, shimmy.x, shimmy.y, shimmy.z) - ceiling;

				int height;
				short bridge1 = -1; // FindBridge(4, goalOrient, shimmy, &height, -5 * CLICK(1) / 2, -3 * CLICK(1) / 2); FIXME

				if ((SLOPE_CHECK && abs(yDiff) < 64) || bridge1 >= 0)
					SetAnimation(lara, dir < 0 ? LA_OVERHANG_SHIMMY_LEFT : LA_OVERHANG_SHIMMY_RIGHT);
			}
		}
	}

}


void lara_as_slopehang(ITEM_INFO* lara, COLL_INFO* coll)
{
	if (GlobalCounter % 2)
		lara->pos.xRot--;
	else
		lara->pos.xRot++;

	if (Camera.type != CAMERA_TYPE::CHASE_CAMERA)
		return;

	Camera.targetElevation = -1024;
	Camera.targetDistance = 1664;
	Camera.speed = 15;
}


void lara_col_slopeshimmy(ITEM_INFO* lara, COLL_INFO* coll)
{
	GetCollisionInfo(coll, lara);

	PHD_VECTOR now = { lara->pos.xPos, lara->pos.yPos, lara->pos.zPos };
	PHD_VECTOR offset = { 0, 0, 0 };
	Vector2 slope = { 0, 0 };
	Vector2 goal = { 0, 0 };
	short climbOrient = 0;
	short goalOrient = 0;

	FillSlopeData(lara->pos.yRot, goal, climbOrient, goalOrient, offset);

	short tempRoom = 0;

	FLOOR_INFO* floorNow = (FLOOR_INFO*)GetFloor(now.x, now.y, now.z, &(tempRoom = lara->roomNumber));
	int ceiling = GetCeiling(floorNow, now.x, now.y, now.z);
	lara->pos.yPos = ceiling + HEIGHT_ADJUST;

	PHD_VECTOR shimmy = { lara->pos.xPos, lara->pos.yPos, lara->pos.zPos };
	if (lara->animNumber == LA_OVERHANG_SHIMMY_LEFT)
	{
		shimmy.x -= offset.z / 2;
		shimmy.z += offset.x / 2;
	}
	else
	{
		shimmy.x += offset.z / 2;
		shimmy.z -= offset.x / 2;
	}

	FLOOR_INFO* floorNext = (FLOOR_INFO*)GetFloor(shimmy.x, shimmy.y, shimmy.z, &(tempRoom = lara->roomNumber));

	bool cancelShimmy = true;
	if (TestMonkey(floorNext, shimmy.x, shimmy.y, shimmy.z))
	{
		slope = floorNext->TiltXZ(shimmy.x, shimmy.z, false);

		int yDiff = GetCeiling(floorNext, shimmy.x, shimmy.y, shimmy.z) - ceiling;

		int height;
		short bridge1 = -1; // FindBridge(4, goalOrient, shimmy, &height, -5 * CLICK(1) / 2, -3 * CLICK(1) / 2); fixme

		if ((SLOPE_CHECK && abs(yDiff) < 64) || bridge1 >= 0)
			cancelShimmy = false;
	}

	if (cancelShimmy)
		SetAnimation(lara, (lara->animNumber == LA_OVERHANG_SHIMMY_LEFT) ? LA_OVERHANG_SHIMMY_LEFT_STOP : LA_OVERHANG_SHIMMY_RIGHT_STOP);
}

void lara_as_slopeshimmy(ITEM_INFO* lara, COLL_INFO* coll)
{
	if (GlobalCounter % 2)
		lara->pos.xRot--;
	else
		lara->pos.xRot++;

	if (Camera.type != CAMERA_TYPE::CHASE_CAMERA)
		return;

	Camera.targetElevation = -1024;
	Camera.targetDistance = 1664;
	Camera.speed = 15;

	auto info = (LaraInfo*&)lara->data;

	if (lara->animNumber == LA_OVERHANG_SHIMMY_LEFT)
	{
		info->moveAngle = lara->pos.yRot - 16384;
		Camera.targetAngle = -4096;
	}
	else
	{
		info->moveAngle = lara->pos.yRot + 16384;
		Camera.targetAngle = 4096;
	}
}


void lara_as_slopeclimbup(ITEM_INFO* lara, COLL_INFO* coll)
{
	if (GlobalCounter % 2)
		lara->pos.xRot--;
	else
		lara->pos.xRot++;

	Camera.flags = 1;

	if (Camera.type != CAMERA_TYPE::CHASE_CAMERA)
		return; // if camera mode isn't chase (0) then don't change camera angles

	Camera.targetElevation = 2048;
	Camera.targetDistance = 1792;
	Camera.speed = 15;


	if (!(TrInput & IN_ACTION))
	{
		int frame = GetFrameNumber(lara, 0);
		int length = g_Level.Anims[lara->animNumber].frameEnd - g_Level.Anims[lara->animNumber].frameBase;
		int dPos = CLICK(1) - (frame * CLICK(1) / length);

		TranslateItem(lara, 0, dPos, dPos);
		if (lara->animNumber == LA_OVERHANG_CLIMB_UP_LEFT)
			SetAnimation(lara, frame <= 2 * length / 3 ? LA_OVERHANG_DROP_LEFT : LA_OVERHANG_DROP_RIGHT);
		else
			SetAnimation(lara, frame <= 2 * length / 3 ? LA_OVERHANG_DROP_RIGHT : LA_OVERHANG_DROP_LEFT);
	}
}


void lara_as_slopeclimbdown(ITEM_INFO* lara, COLL_INFO* coll)
{
	if (GlobalCounter % 2)
		lara->pos.xRot--;
	else
		lara->pos.xRot++;

	Camera.flags = 1;

	if (Camera.type != CAMERA_TYPE::CHASE_CAMERA)
		return;

	Camera.targetElevation = -3072;
	Camera.targetDistance = 1664;
	Camera.speed = 15;

	if (!(TrInput & IN_ACTION))
	{
		int frame = GetFrameNumber(lara, 0);
		int length = g_Level.Anims[lara->animNumber].frameEnd - g_Level.Anims[lara->animNumber].frameBase;
		int dPos = frame * CLICK(1) / length;

		TranslateItem(lara, 0, dPos, dPos);
		if (lara->animNumber == LA_OVERHANG_CLIMB_DOWN_LEFT)
			SetAnimation(lara, frame <= length / 2 ? LA_OVERHANG_DROP_LEFT : LA_OVERHANG_DROP_RIGHT);
		else
			SetAnimation(lara, frame <= length / 2 ? LA_OVERHANG_DROP_RIGHT : LA_OVERHANG_DROP_LEFT);
	}
}


void lara_as_sclimbstart(ITEM_INFO* lara, COLL_INFO* coll)
{
	// rotating camera effect during monkey to overhead slope transition
	if (lara->animNumber == LA_OVERHANG_MONKEY_SLOPE_CONVEX)
	{
		int frame = GetFrameNumber(lara, 0);
		float frac = 0.0f;
		int numFrames = g_Level.Anims[lara->animNumber].frameEnd - g_Level.Anims[lara->animNumber].frameBase;

		frac = (frame * 1.5f) / (float)(numFrames);
		if (frac > 1.0f)
			frac = 1.0f;

		Camera.flags = 1;

		int distance = 1664;
		if (TestLaraWall(lara, 0, 1536, 0) != SPLAT_COLL::NONE)
		{
			distance = 1024;
		}

		if (lara->frameNumber < g_Level.Anims[lara->animNumber].frameEnd)
		{
			Camera.targetDistance = distance;
			Camera.targetElevation = int(3072 * frac);
			Camera.targetAngle = int(-32767 * frac);
			Camera.targetspeed = 15;
		}
		else
		{

			Camera.targetDistance = distance;
			Camera.targetElevation = 3072;
			Camera.targetAngle = 0;
			Camera.targetspeed = 15;
		}
	}
	else// if (lara->animNumber == LA_OVERHANG_MONKEY_SLOPE_CONCAVE)
	{
		//Camera.flags = 1;
		Camera.targetElevation = -2048;
		Camera.targetDistance = 1664;
		Camera.speed = 15;
	}

	if (GlobalCounter % 2)
		lara->pos.xRot++;
	else
		lara->pos.xRot--;
}


void lara_as_sclimbstop(ITEM_INFO* lara, COLL_INFO* coll)
{
	// rotating camera effect during monkey to overhead slope transition

	// following camera effect during the slope to underlying monkey transition
	if (lara->animNumber == LA_OVERHANG_SLOPE_MONKEY_CONVEX)
	{
		Camera.flags = 1;
		Camera.targetDistance = 1664;
		Camera.targetElevation = 2048;
		Camera.targetspeed = 15;
	}
	// rotating camera effect during concave slope to monkey transition
	else if (lara->animNumber == LA_OVERHANG_SLOPE_MONKEY_CONCAVE)
	{
		int frame = GetFrameNumber(lara, 0);
		int numFrames = g_Level.Anims[lara->animNumber].frameEnd - g_Level.Anims[lara->animNumber].frameBase;

		float frac = (frame * 1.25f) / (float)(numFrames);
		if (frac > 1.0f)
			frac = 1.0f;

		Camera.flags = 1;

		if (lara->frameNumber < g_Level.Anims[lara->animNumber].frameEnd)
		{
			
			Camera.targetAngle = (short)(-16384 * frac);
			Camera.targetDistance = 1792 - int(512 * frac);
			Camera.targetspeed = 15;
		}
		else
		{
			Camera.targetAngle = 16384;
			Camera.targetDistance = 1280;
			Camera.targetspeed = 15;
		}
	}
	else
	{
		Camera.targetDistance = 1664;
		Camera.targetElevation = -2048;
		Camera.targetspeed = 15;
	}


	if (GlobalCounter % 2)
		lara->pos.xRot++;
	else
		lara->pos.xRot--;
}


void lara_as_sclimbend(ITEM_INFO* lara, COLL_INFO* coll)
{
	switch (lara->animNumber)
	{
	case LA_OVERHANG_EXIT_MONKEY_FORWARD:
		SetAnimation(lara, LA_MONKEYSWING_FORWARD);
		break;
	case LA_OVERHANG_EXIT_MONKEY_IDLE:
		SetAnimation(lara, LA_MONKEYSWING_IDLE);
		break;
	case LA_OVERHANG_EXIT_LADDER:
		SetAnimation(lara, LA_LADDER_IDLE);
		break;
	case LA_OVERHANG_EXIT_VAULT:
		SetAnimation(lara, LA_HANG_TO_STAND_END);
		break;
	case LA_OVERHANG_EXIT_DROP:
		SetAnimation(lara, LA_FALL);
		break;
	case LA_OVERHANG_EXIT_HANG:
		SetAnimation(lara, LA_JUMP_UP);
		break;
	}

	lara->pos.xRot = 0;
}


// ********  Callback procedures for extending existing state routines  ********
// added via RequireMyCallbacks() (scroll down below)

/* extends state 10 (AS_HANG) */
void hang_slope_extra(ITEM_INFO* lara, COLL_INFO* col)
{
	PHD_VECTOR offset = { 0, 0, 0 };
	Vector2 goal = { 0, 0 };
	short climbOrient = 0;
	short goalOrient = 0;

	FillSlopeData(lara->pos.yRot, goal, climbOrient, goalOrient, offset);

	short tempRoom = 0;
	PHD_VECTOR down = { lara->pos.xPos + offset.x, lara->pos.yPos + CLICK(1), lara->pos.zPos + offset.z };
	FLOOR_INFO* floorNext = (FLOOR_INFO*)GetFloor(down.x, down.y, down.z, &(tempRoom = lara->roomNumber));
	int ceilDist = lara->pos.yPos - GetCeiling(floorNext, down.x, down.y, down.z);

	if (lara->goalAnimState == LS_LADDER_IDLE) // prevent going from hang to climb mode if slope is under ladder
	{
		if (ceilDist >= CLICK(1) && ceilDist < 2 * CLICK(1))
		{
			auto slope = floorNext->TiltXZ(down.x, down.z, false);

			if ((slope.x / 3) == (goal.x / 3) || (slope.y / 3) == (goal.y / 3))
			{
				lara->goalAnimState = LS_HANG;
				if (TrInput & IN_FORWARD)
					SetAnimation(lara, LA_LADDER_SHIMMY_UP);
				/*else if (TrInput & IN_BACK)
					SetAnimation(lara, LA_LADDER_SHIMMY_DOWN);*/
			}
		}
	}
	/*else if (lara->goalAnimState == AS_HANG)
	{
		if (lara->animNumber == LA_LADDER_SHIMMY_DOWN)
		{
			if (ceilDist < CLICK(1))
			{
				auto slope = floorNext->TiltXZ(down.x, down.z, false);
				if ((slope.tiltX / 3) == (goal.tiltX / 3) || (slope.tiltZ / 3) == (goal.tiltZ / 3))
					SetAnimation(lara, LA_REACH_TO_HANG, 21);
			}
		}
	}*/
}

/* extends state 11 (AS_REACH) */
void reach_slope_extra(ITEM_INFO* lara, COLL_INFO* col)
{
	PHD_VECTOR now = { lara->pos.xPos, lara->pos.yPos, lara->pos.zPos };
	PHD_VECTOR offset = { 0, 0, 0 };
	Vector2 slope = { 0, 0 };
	Vector2 goal = { 0, 0 };
	short climbOrient = 0;
	short goalOrient = 0;

	FillSlopeData(lara->pos.yRot, goal, climbOrient, goalOrient, offset);
	PHD_VECTOR down = { lara->pos.xPos + offset.x, lara->pos.yPos + CLICK(1), lara->pos.zPos + offset.z };

	short tempRoom = 0;

	FLOOR_INFO* floorNow = (FLOOR_INFO*)GetFloor(now.x, now.y, now.z, &(tempRoom = lara->roomNumber));
	int ceilDist = lara->pos.yPos - GetCeiling(floorNow, now.x, now.y, now.z);

	if (TestMonkey(floorNow, now.x, now.y, now.z) && ceilDist <= 7 * CLICK(1) / 2)
	{

		slope = floorNow->TiltXZ(now.x, now.z, false);

		int height;
		short bridge1 = -1; // FindBridge(4, goalOrient, now, &height, -4 * CLICK(1), -5 * (CLICK(1) / 2)); fixme

		if (abs(slope.x) > 2 || abs(slope.y) > 2 || bridge1 >= 0)
		{
			bool disableGrab = true;
			if (SLOPE_CHECK || bridge1 >= 0)
			{
				if (abs(DirOrientDiff(lara->pos.yRot, goalOrient)) < 0x1800)
					disableGrab = false;
			}

			if (disableGrab)
				TrInput &= ~IN_ACTION; // HACK!
		}
	}
}

/* extends state 56 (AS_CLIMBSTNC) */
void climb_slope_extra(ITEM_INFO* lara, COLL_INFO* coll)
{
	PHD_VECTOR now = { lara->pos.xPos, lara->pos.yPos, lara->pos.zPos };
	PHD_VECTOR offset = { 0, 0, 0 };
	Vector2 slope = { 0, 0 };
	Vector2 goal = { 0, 0 };
	short climbOrient = 0;
	short goalOrient = 0;

	FillSlopeData(lara->pos.yRot, goal, climbOrient, goalOrient, offset);
	PHD_VECTOR down = { lara->pos.xPos + offset.x, lara->pos.yPos + CLICK(1), lara->pos.zPos + offset.z };

	short tempRoom = 0;

	FLOOR_INFO* floorNow = (FLOOR_INFO*)GetFloor(now.x, now.y, now.z, &(tempRoom = lara->roomNumber));
	int ceiling = GetCeiling(floorNow, now.x, now.y, now.z);

	// block for ladder to overhead slope transition
	if (lara->animNumber == 164)
	{
		if (TrInput & IN_FORWARD)
		{
			int ceiling = GetCeiling(floorNow, now.x, now.y, now.z);
			int ceilDist = ceiling - lara->pos.yPos;

			if (TestMonkey(floorNow, now.x, now.y, now.z) && ceilDist >= -4 * CLICK(1) && ceilDist <= -3 * CLICK(1))
			{
				slope = floorNow->TiltXZ(lara->pos.xPos, lara->pos.zPos, false);

				int height;

				short facing = lara->pos.yRot + 0x2000;
				facing &= 0xC000;

				short bridge1 = -1; // FindBridge(4, facing, now, &height, -4 * CLICK(1), -3 * CLICK(1)); FIXME

				if (SLOPE_CHECK || bridge1 >= 0)
				{
					lara->pos.yPos = ceiling + 900;
					SetAnimation(lara, LA_OVERHANG_LADDER_SLOPE_CONCAVE); // ladder to overhead slope transition (concave)
				}
			}
		}

		if (TrInput & IN_BACK)
		{
			FLOOR_INFO* floorNext = (FLOOR_INFO*)GetFloor(down.x, down.y, down.z, &(tempRoom = lara->roomNumber));
			int ceiling = GetCeiling(floorNext, down.x, down.y, down.z);
			int ceilDist = ceiling - lara->pos.yPos;

			if (TestMonkey(floorNext, down.x, down.y, down.z) && ceilDist >= 0 && ceilDist <= CLICK(1))
			{
				slope = floorNext->TiltXZ(down.x, down.z, false);

				int height;

				short facing = lara->pos.yRot + 0x2000;
				facing &= 0xC000;

				short bridge1 = -1; // FindBridge(4, facing, down, &height, -CLICK(1) / 2, -CLICK(1) / 4); FIXME

				if (SLOPE_CHECK || bridge1 >= 0)
				{
					lara->pos.yPos = ceiling - 156;
					SetAnimation(lara, LA_OVERHANG_LADDER_SLOPE_CONCAVE); // ladder to underlying slope transition (convex)
				}
			}
		}
	}
}

/* extends state 61 (AS_CLIMBDOWN) */
void climbdown_slope_extra(ITEM_INFO* lara, COLL_INFO* coll)
{
	PHD_VECTOR now = { lara->pos.xPos, lara->pos.yPos, lara->pos.zPos };
	PHD_VECTOR offset = { 0, 0, 0 };
	Vector2 slope = { 0, 0 };
	Vector2 goal = { 0, 0 };
	short climbOrient = 0;
	short goalOrient = 0;

	FillSlopeData(lara->pos.yRot, goal, climbOrient, goalOrient, offset);
	PHD_VECTOR down = { lara->pos.xPos + offset.x, lara->pos.yPos + CLICK(1), lara->pos.zPos + offset.z };

	short tempRoom = 0;

	FLOOR_INFO* floorNow = (FLOOR_INFO*)GetFloor(now.x, now.y, now.z, &(tempRoom = lara->roomNumber));
	int ceiling = GetCeiling(floorNow, now.x, now.y, now.z);

	if (lara->animNumber == 168) // make lara stop before underlying slope ceiling at correct height
	{
		if (TrInput & IN_BACK)
		{
			FLOOR_INFO* floorNext = (FLOOR_INFO*)GetFloor(down.x, down.y, down.z, &(tempRoom = lara->roomNumber));
			int ceiling = GetCeiling(floorNext, down.x, down.y, down.z);
			int ceilDist = ceiling - lara->pos.yPos;

			if (TestMonkey(floorNext, down.x, down.y, down.z))
			{
				slope = floorNext->TiltXZ(down.x, down.z, false);

				int midpoint = 29; // HACK: lara_col_climbdown func, case for frame 29, dehardcode later.
				
				//down.y += 256;
				int height;
				if (!GetFrameNumber(lara, 0))
				{
					short bridge1 = -1; // FindBridge(4, goalOrient, down, &height, -CLICK(1) * 3, CLICK(1) * 4); fixme
					if (ceilDist < CLICK(1) && (bridge1 >= 0 || SLOPE_CHECK))
						lara->goalAnimState = 56;
				}
				else if (GetFrameNumber(lara, 0) == midpoint)
				{
					short bridge1 = -1; // FindBridge(4, goalOrient, down, &height, -CLICK(1) * 2, CLICK(1) * 5); fixme
					if (ceilDist < CLICK(1) * 2 && (bridge1 >= 0 || SLOPE_CHECK))
					{
						lara->pos.yPos += CLICK(1); // do midpoint Y translation
						lara->goalAnimState = 56;
					}
				}
			}
		}
	}
}


/* extends state 75 (AS_HANG2) */
void monkey_slope_extra(ITEM_INFO* lara, COLL_INFO* coll)
{
	PHD_VECTOR now = { lara->pos.xPos, lara->pos.yPos, lara->pos.zPos };
	PHD_VECTOR offset = { 0, 0, 0 };
	Vector2 slope = { 0, 0 };
	Vector2 goal = { 0, 0 };
	short climbOrient = 0;
	short goalOrient = 0;

	FillSlopeData(lara->pos.yRot, goal, climbOrient, goalOrient, offset);
	PHD_VECTOR down = { lara->pos.xPos + offset.x, lara->pos.yPos + CLICK(1), lara->pos.zPos + offset.z };

	short tempRoom = 0;

	FLOOR_INFO* floorNow = (FLOOR_INFO*)GetFloor(now.x, now.y, now.z, &(tempRoom = lara->roomNumber));
	int ceiling = GetCeiling(floorNow, now.x, now.y, now.z);

	if (lara->animNumber == 150 && !GetFrameNumber(lara, 0)) // manage proper grabbing of monkey slope on forward jump
	{
		int ceiling = GetCeiling(floorNow, now.x, now.y, now.z);
		int ceilDist = lara->pos.yPos - ceiling;

		if (TestMonkey(floorNow, now.x, now.y, now.z) && ceilDist <= 7 * CLICK(1) / 2)
		{
			slope = floorNow->TiltXZ(now.x, now.z, false);

			int height;

			short facing = lara->pos.yRot + 0x2000;
			facing &= 0xC000;

			short bridge1 = -1; // FindBridge(4, facing, now, &height, -7 * CLICK(1) / 2, -5 * CLICK(1) / 2); fixme

			if (SLOPE_CHECK || bridge1 >= 0)
			{

				auto info = (LaraInfo*&)lara->data;
				info->nextCornerPos.zRot = AlignToGrab(lara);

				int ceiling2 = GetCeiling(floorNow, lara->pos.xPos, lara->pos.yPos, lara->pos.zPos);
				lara->pos.yPos = ceiling2 + HEIGHT_ADJUST;

				SetAnimation(lara, LA_OVERHANG_HANG_SWING);
			}
		}
	}

	if (TrInput & IN_FORWARD) // monkey to slope transitions
	{
		if (TestMonkey(floorNow, now.x, now.y, now.z) && ((lara->animNumber == 150 && GetFrameNumber(lara, 0) >= 54) || lara->animNumber == 234))
		{
			if (abs(DirOrientDiff(goalOrient, lara->pos.yRot)) <= 0x1400 &&
				InStrip(lara->pos.xPos, lara->pos.zPos, lara->pos.yRot, 0, CLICK(1) / 2))
			{
				FLOOR_INFO* floorNext = (FLOOR_INFO*)GetFloor(down.x, down.y, down.z, &(tempRoom = lara->roomNumber));

				if (TestMonkey(floorNext, down.x, down.y, down.z))
				{
					int ceiling = GetCeiling(floorNext, down.x, now.y, down.z);
					int yDiff = ceiling - GetCeiling(floorNow, now.x, now.y, now.z);

					slope = floorNext->TiltXZ(down.x, down.z, false);

					int height;

					short bridge1 = -1; // FindBridge(4, goalOrient, down, &height, -7 * CLICK(1) >> 1, -5 * CLICK(1) >> 1); fixme
					if ((SLOPE_CHECK && yDiff > 0 && yDiff < CLICK(1)) || bridge1 >= 0)
					{
						AlignToEdge(lara, 154);
						SetAnimation(lara, LA_OVERHANG_MONKEY_SLOPE_CONCAVE); // transition from monkey to underlying slope (concave)
						//lara->pos.yPos = ceiling + 496;
						//PerformFlipeffect(NULL, 51, 1, 2); // disable the UP key command for 2 sec // HACK!!!
					}

					bridge1 = -1; // FindBridge(4, goalOrient + 0x8000, down, &height, -5 * CLICK(1), -4 * CLICK(1)); fixme
					if ((SLOPE_INVERSE_CHECK && yDiff > -CLICK(1) && yDiff < 0) || bridge1 >= 0)
					{
						AlignToEdge(lara, 154);
						SetAnimation(lara, LA_OVERHANG_MONKEY_SLOPE_CONVEX); // transition from monkey to overhanging slope (convex)
						//lara->pos.yPos = ceiling + 914;
					}
				}
			}
		}
	}
}