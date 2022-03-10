#include "framework.h"
#include "Game/Lara/lara_overhang.h"

#include "Game/camera.h"
#include "Game/collision/floordata.h"
#include "Game/collision/collide_room.h"
#include "Game/control/control.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_climb.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/Lara/lara_tests.h"
#include "Game/items.h"
#include "Specific/input.h"
#include "Specific/setup.h"
#include "Scripting/GameFlowScript.h"

constexpr auto HORIZONTAL_ALIGN_NORTHEAST = 155;
constexpr auto HORIZONTAL_ALIGN_SOUTHWEST = 101;
constexpr auto FORWARD_ALIGNMENT = 868;
constexpr auto BACKWARD_ALIGNMENT = 100;
constexpr auto SLOPE_ALIGNMENT = 154;

constexpr short FACING_NORTH = 0;
constexpr short FACING_EAST = 16384;
constexpr short FACING_SOUTH = -32768;
constexpr short FACING_WEST = -16384;

constexpr auto HEIGHT_ADJUST = CLICK(2) + 20;

// **************  Utility functions section  *************** //

inline bool SlopeCheck(Vector2 slope, Vector2 goal)
{
	return (slope.x == goal.x && slope.y == goal.y);
}

inline bool SlopeInvCheck(Vector2 slope, Vector2 goal)
{
	return (slope.x == -goal.x && slope.y == -goal.y);
}

bool TestMonkey(FLOOR_INFO* floor, int x, int y, int z)
{
	return GetCollisionResult(floor, x, y, z).BottomBlock->Flags.Monkeyswing;
}

short FindBridge(int tiltGrade, short facing, PHD_VECTOR& pos, int* returnHeight, int minCeilY = 0, int maxCeilY = 0)
{
	short bridgeSlot;

	switch (tiltGrade)
	{
	case 0:
		bridgeSlot = ID_BRIDGE_FLAT;
		break;
	case 1:
		bridgeSlot = ID_BRIDGE_TILT1;
		break;
	case 2:
		bridgeSlot = ID_BRIDGE_TILT2;
		break;
	case 3:
		bridgeSlot = ID_BRIDGE_TILT3;
		break;
	case 4:
		bridgeSlot = ID_BRIDGE_TILT4;
		break;
	default:
		bridgeSlot = ID_BRIDGE_CUSTOM;
		break;
	}

	int xmin = pos.x & ~WALL_MASK;
	int xmax = xmin + WALL_MASK;
	int zmin = pos.z & ~WALL_MASK;
	int zmax = zmin + WALL_MASK;

	for (int i = 0; i < g_Level.Items.size(); i++)
	{
		auto obj = &g_Level.Items[i];

		if (obj->ObjectNumber != bridgeSlot)
			continue;

		short facingDiff = (short)(obj->Position.yRot - facing);

		bool facingCheck = false;
		if (facingDiff == ANGLE(90))
			facingCheck = true;
		else if (obj->ObjectNumber == ID_BRIDGE_FLAT)
			facingCheck = true;
		else if ((obj->ObjectNumber == ID_BRIDGE_TILT1 || obj->ObjectNumber == ID_BRIDGE_TILT2) && abs(facingDiff) == ANGLE(90.0f))
			facingCheck = true;

		if (!facingCheck)
			continue;

		if (obj->Position.xPos >= xmin &&
			obj->Position.xPos <= xmax &&
			obj->Position.zPos >= zmin &&
			obj->Position.zPos <= zmax)
		{
			if (minCeilY || maxCeilY)
			{
				if (Objects[obj->ObjectNumber].ceiling == nullptr)
					continue;

				*returnHeight = Objects[obj->ObjectNumber].ceiling(i, pos.x, pos.y, pos.z).value_or(NO_HEIGHT);
				
				int ceilingDist = *returnHeight - pos.y;
				if (ceilingDist >= minCeilY && ceilingDist <= maxCeilY)
					return i;
			}
			else
				return i;
		}
	}

	return NO_ITEM;
}

// Get the signed diff between two orients
short DirOrientDiff(short orient1, short orient2)
{
	int diff = orient1 - orient2;

	if (diff > ANGLE(180.0f))
		diff -= ANGLE(360.0f);
	if (diff < -ANGLE(180.0f))
		diff += ANGLE(360.0f);

	return short(diff);
}

// Test if inside sector strip (0-1023) in currently faced quadrant, between min and max
bool InStrip(int x, int z, short facing, int min, int max)
{
	if (min > WALL_MASK)
		min = WALL_MASK;
	if (max > WALL_MASK)
		max = WALL_MASK;

	int quadrant = GetQuadrant(facing);
	int dx = x & WALL_MASK;
	int dz = z & WALL_MASK;

	switch (quadrant)
	{
	case NORTH:
		if (dz >= (WALL_MASK - max) && dz <= (WALL_MASK - min))
			return true;

		break;

	case SOUTH:
		if (dz >= min && dz <= max)
			return true;

		break;

	case EAST:
		if (dx >= (WALL_MASK - max) && dx <= (WALL_MASK - min))
			return true;

		break;

	case WEST:
		if (dx >= min && dx <= max)
			return true;

		break;
	}

	return false;
}

// Align facing and X/Z pos to sector edge.
void AlignToEdge(ITEM_INFO* item, short edgeDist)
{
	if (edgeDist > WALL_MASK)
		edgeDist = WALL_MASK;

	// Align to closest cardinal facing
	item->Position.yRot += ANGLE(45.0f);
	item->Position.yRot &= 0xC000;

	switch (item->Position.yRot) // Align to faced edge
	{
	case FACING_NORTH:
		item->Position.zPos &= ~WALL_MASK;
		item->Position.zPos += (WALL_MASK - edgeDist);
		break;

	case FACING_SOUTH:
		item->Position.zPos &= ~WALL_MASK;
		item->Position.zPos += edgeDist + 1;
		break;

	case FACING_EAST:
		item->Position.xPos &= ~WALL_MASK;
		item->Position.xPos += (WALL_MASK - edgeDist);
		break;

	case FACING_WEST:
		item->Position.xPos &= ~WALL_MASK;
		item->Position.xPos += edgeDist + 1;
		break;
	}
}

// Correct position after grabbing slope
bool AlignToGrab(ITEM_INFO* item)
{
	bool legLeft = false;

	item->Position.yRot += ANGLE(45.0f);
	item->Position.yRot &= 0xC000;

	switch (item->Position.yRot)
	{
	case FACING_NORTH:
		if (((item->Position.zPos + (int)CLICK(0.5f)) & ~WALL_MASK) == (item->Position.zPos & ~WALL_MASK))
			item->Position.zPos += CLICK(0.5f);

		item->Position.zPos = (item->Position.zPos & ~CLICK(1)) + HORIZONTAL_ALIGN_NORTHEAST;
		legLeft = (item->Position.zPos & CLICK(1)) ? false : true;
		break;

	case FACING_SOUTH:
		if (((item->Position.zPos - (int)CLICK(0.5f)) & ~WALL_MASK) == (item->Position.zPos & ~WALL_MASK))
			item->Position.zPos -= CLICK(0.5f);

		item->Position.zPos = (item->Position.zPos & ~CLICK(1)) + HORIZONTAL_ALIGN_SOUTHWEST;
		legLeft = (item->Position.zPos & CLICK(1)) ? true : false;
		break;

	case FACING_WEST:
		if (((item->Position.xPos - (int)CLICK(0.5f)) & ~WALL_MASK) == (item->Position.xPos & ~WALL_MASK))
			item->Position.xPos -= CLICK(0.5f);

		item->Position.xPos = (item->Position.xPos & ~CLICK(1)) + HORIZONTAL_ALIGN_SOUTHWEST;
		legLeft = (item->Position.xPos & CLICK(1)) ? true : false;
		break;

	case FACING_EAST:
		if (((item->Position.xPos + (int)CLICK(0.5f)) & ~WALL_MASK) == (item->Position.xPos & ~WALL_MASK))
			item->Position.xPos += CLICK(0.5f);

		item->Position.xPos = (item->Position.xPos & ~CLICK(1)) + HORIZONTAL_ALIGN_NORTHEAST;
		legLeft = (item->Position.xPos & CLICK(1)) ? false : true;
		break;
	}

	return legLeft;
}

void FillSlopeData(short orient, Vector2& goal, short& climbOrient, short& goalOrient, PHD_VECTOR& pos)
{
	switch (GetQuadrant(orient))
	{
	case NORTH:
		climbOrient = (short)CLIMB_DIRECTION::North;
		goalOrient = 0;
		goal.y = -4;
		pos.z = CLICK(1);
		break;

	case EAST:
		climbOrient = (short)CLIMB_DIRECTION::East;
		goalOrient = ANGLE(90.0f);
		goal.x = -4;
		pos.x = CLICK(1);
		break;

	case SOUTH:
		climbOrient = (short)CLIMB_DIRECTION::South;
		goalOrient = ANGLE(180.0f);
		goal.y = 4;
		pos.z = -CLICK(1);
		break;

	case WEST:
		climbOrient = (short)CLIMB_DIRECTION::West;
		goalOrient = ANGLE(270.0f);
		goal.x = 4;
		pos.x = -CLICK(1);
		break;
	}
}

// ************  Climbing logic (control & collision routines) ************* //

void lara_col_slopeclimb(ITEM_INFO* item, COLL_INFO* coll)
{
	GetCollisionInfo(coll, item);

	PHD_VECTOR now = { item->Position.xPos, item->Position.yPos, item->Position.zPos };
	PHD_VECTOR offset = { 0, 0, 0 };
	Vector2 slope = { 0, 0 };
	Vector2 goal = { 0, 0 };
	short climbOrient = 0;
	short goalOrient = 0;

	FillSlopeData(item->Position.yRot, goal, climbOrient, goalOrient, offset);
	PHD_VECTOR up = { item->Position.xPos - offset.x, item->Position.yPos - CLICK(1), item->Position.zPos - offset.z };
	PHD_VECTOR down = { item->Position.xPos + offset.x, item->Position.yPos + CLICK(1), item->Position.zPos + offset.z };

	auto collResultUp = GetCollisionResult(up.x, up.y, up.z, item->RoomNumber);
	auto collResultDown = GetCollisionResult(down.x, down.y, down.z, item->RoomNumber);

	short tempRoom = 0;

	if (item->AnimNumber == LA_OVERHANG_LADDER_SLOPE_CONCAVE)
		return;

	auto floorNow = GetFloor(now.x, now.y, now.z, &(tempRoom = item->RoomNumber));
	int ceiling = GetCeiling(floorNow, now.x, now.y, now.z);
	item->Position.yPos = ceiling + HEIGHT_ADJUST;

	// Drop down if action not pressed
	if (!(TrInput & IN_ACTION))
	{
		SetAnimation(item, item->AnimNumber == LA_OVERHANG_IDLE_LEFT ? LA_OVERHANG_DROP_LEFT : LA_OVERHANG_DROP_RIGHT);
		return;
	}

	// Engage shimmy mode if left (sidestep) or right (sidestep) key is pressed
	if (TrInput & IN_LEFT || TrInput & IN_RIGHT)
	{
		auto* lara = GetLaraInfo(item);
		lara->NextCornerPos.zRot = (item->AnimNumber == LA_OVERHANG_IDLE_LEFT) ? true : false; // HACK!
		SetAnimation(item, item->AnimNumber == LA_OVERHANG_IDLE_LEFT ? LA_OVERHANG_IDLE_2_HANG_LEFT : LA_OVERHANG_IDLE_2_HANG_RIGHT);
		return;
	}

	if (TrInput & IN_FORWARD)
	{
		// Test for ledge over slope
		tempRoom = collResultUp.Block->RoomAbove(up.x, up.z).value_or(NO_ROOM);
		if (tempRoom != NO_ROOM)
		{
			short oldRoomCam = Camera.pos.roomNumber;
			short oldRoomTarg = Camera.target.roomNumber; 

			auto testLedge = GetFloor(now.x, now.y - CLICK(3), now.z, &tempRoom);
			int ledgeCeiling = GetCeiling(testLedge, now.x, now.y - CLICK(3), now.z);
			int ledgeHeight = GetFloorHeight(testLedge, now.x, now.y - CLICK(3), now.z);
			if ((ledgeHeight - ledgeCeiling >= CLICK(3)) && abs((item->Position.yPos - 688) - ledgeHeight) < 64)
			{
				AlignToEdge(item, FORWARD_ALIGNMENT);
				SetAnimation(item, LA_OVERHANG_LEDGE_VAULT_START); // Ledge climb-up from slope
			}
		}

		// Test for slope to overhead ladder transition (convex)
		if (GetClimbFlags(collResultUp.BottomBlock) & climbOrient &&
			InStrip(item->Position.xPos, item->Position.zPos, item->Position.yRot, CLICK(3), CLICK(4)))
		{
			if (TestLaraSplat(item, 0, 0, -CLICK(4)) &&
				GetCeiling(collResultUp.Block, up.x, up.y, up.z) - item->Position.yPos <= 1456)  // Check if a wall is actually there...
			{
				AlignToEdge(item, FORWARD_ALIGNMENT);
				SetAnimation(item, LA_OVERHANG_SLOPE_LADDER_CONVEX_START);
			}
		}

		// Test for monkey at next position
		if (collResultUp.BottomBlock->Flags.Monkeyswing)
		{
			int yDiff = collResultUp.Position.Ceiling - ceiling;

			slope = collResultUp.Block->TiltXZ(up.x, up.z, false);

			int height; // Height variable for bridge ceiling functions

			// Test for upwards slope to climb
			short bridge1 = FindBridge(4, item->Position.yRot, up, &height, -CLICK(5) / 2, -CLICK(3) / 2);
			if (yDiff >= -CLICK(5) / 4 && yDiff <= -CLICK(3) / 4 && (SlopeCheck(slope, goal) || bridge1 >= 0))
			{
				// Do one more check for wall/ceiling step 2*offX/Z further to avoid lara sinking her head in wall/step
				auto testWall = (FLOOR_INFO*)GetFloor((up.x - offset.x), (up.y - CLICK(1)), (up.z - offset.z),
					&(tempRoom = item->RoomNumber));
				int testCeiling = GetCeiling(testWall, (up.x - offset.x), (up.y - CLICK(1)), (up.z - offset.z));

				if (!testWall->IsWall((up.x - offset.x), (up.z - offset.z)) && (ceiling - testCeiling) > CLICK(0.5f)) // No wall or downwards ceiling step
				{
					TranslateItem(item, 0, -CLICK(1), -CLICK(1));
					SetAnimation(item, item->AnimNumber == LA_OVERHANG_IDLE_LEFT ? LA_OVERHANG_CLIMB_UP_LEFT : LA_OVERHANG_CLIMB_UP_RIGHT);
					//item->TargetState = 62;
				}
			}

			// Test for flat monkey (abs(slope) < 2)
			bridge1 = FindBridge(0, goalOrient, up, &height, -CLICK(9) / 4, -CLICK(5) / 4);
			if (bridge1 < 0)
				bridge1 = FindBridge(1, goalOrient, up, &height, -CLICK(9) / 4, -CLICK(5) / 4);

			// HACK: because of the different calculations of bridge height in TR4 and TEN, we need to lower yDiff tolerance to 0.9f.

			if (yDiff > -CLICK(0.9f) && yDiff <= -CLICK(0.5f) && ((abs(slope.x) <= 2 && abs(slope.y) <= 2) || bridge1 >= 0))
				SetAnimation(item, LA_OVERHANG_SLOPE_MONKEY_CONCAVE); // Slope to overhead monkey transition (concave)
		}
	}
	else if (TrInput & IN_BACK)
	{
		// Get floor_lara 256 downstream of Lara
		auto floorNext = (FLOOR_INFO*)GetFloor(down.x, down.y, down.z, &(tempRoom = item->RoomNumber));

		if ((GetClimbFlags(GetCollisionResult(floorNow, item->Position.xPos, item->Position.yPos, item->Position.zPos).BottomBlock) & climbOrient) &&
			InStrip(item->Position.xPos, item->Position.zPos, item->Position.yRot, 0, CLICK(1)))
		{
			AlignToEdge(item, BACKWARD_ALIGNMENT);
			SetAnimation(item, LA_OVERHANG_SLOPE_LADDER_CONCAVE); // Slope to underlying ladder transition (concave)
			return;
		}

		if (TestMonkey(floorNext, down.x, down.y, down.z))
		{
			slope = floorNext->TiltXZ(down.x, down.z, false);

			int height;
			int yDiff = GetCeiling(floorNext, down.x, down.y, down.z) - ceiling;

			// Test for flat monkey (abs(slope) < 2)
			short bridge1 = FindBridge(0, goalOrient, down, &height, -CLICK(3), -CLICK(2));
			if (bridge1 < 0)
				bridge1 = FindBridge(1, goalOrient, down, &height, -CLICK(3), -CLICK(2));

			if ((abs(yDiff) < CLICK(1) && abs(slope.x) <= 2 && abs(slope.y) <= 2) || bridge1 >= 0)
				SetAnimation(item, LA_OVERHANG_SLOPE_MONKEY_CONVEX); // Force slope to underlying monkey transition (convex)

			// Test for downwards slope to climb
			bridge1 = FindBridge(4, goalOrient, down, &height, -CLICK(5) / 2, -CLICK(3) / 2);
			if (yDiff >= CLICK(3) / 4 && yDiff <= CLICK(5) / 4 && (SlopeCheck(slope, goal) || bridge1 >= 0))
			{
				SetAnimation(item, item->AnimNumber == LA_OVERHANG_IDLE_LEFT ? LA_OVERHANG_CLIMB_DOWN_LEFT : LA_OVERHANG_CLIMB_DOWN_RIGHT);
				return;
			}
		}
	}
}

void lara_as_slopeclimb(ITEM_INFO* item, COLL_INFO* coll)
{
	if (GlobalCounter % 2)
		item->Position.xRot--;
	else
		item->Position.xRot++;

	Camera.flags = 1;

	if (Camera.type != CameraType::Chase)
		return;

	Camera.targetElevation = -3072;
	Camera.targetDistance = 1792;
	Camera.speed = 15;
}

void lara_as_slopefall(ITEM_INFO* item, COLL_INFO* coll)
{
	item->Airborne = true;

	if (GlobalCounter % 2)
		item->Position.xRot--;
	else
		item->Position.xRot++;

	Camera.flags = 1;

	if (Camera.type != CameraType::Chase)
		return;

	Camera.targetElevation = -3072;
	Camera.targetDistance = 1792;
	Camera.speed = 15;
}

void lara_col_slopehang(ITEM_INFO* item, COLL_INFO* coll)
{
	GetCollisionInfo(coll, item);

	PHD_VECTOR now = { item->Position.xPos, item->Position.yPos, item->Position.zPos };
	PHD_VECTOR offset = { 0, 0, 0 };
	Vector2 slope = { 0, 0 };
	Vector2 goal = { 0, 0 };
	short climbOrient = 0;
	short goalOrient = 0;

	FillSlopeData(item->Position.yRot, goal, climbOrient, goalOrient, offset);

	short tempRoom = 0;

	auto floorNow = GetFloor(now.x, now.y, now.z, &(tempRoom = item->RoomNumber));
	int ceiling = GetCeiling(floorNow, now.x, now.y, now.z);
	item->Position.yPos = ceiling + HEIGHT_ADJUST;

	// Drop down if action not pressed 
	if (!(TrInput & IN_ACTION))
	{
		SetAnimation(item, LA_OVERHANG_HANG_DROP);
		return;
	}

	if (item->AnimNumber != LA_OVERHANG_HANG_SWING)
	{
		// Return to climbing mode
		if (TrInput & IN_FORWARD || TrInput & IN_BACK)
		{
			auto* lara = GetLaraInfo(item);
			SetAnimation(item, lara->NextCornerPos.zRot ? LA_OVERHANG_HANG_2_IDLE_LEFT : LA_OVERHANG_HANG_2_IDLE_RIGHT); // HACK!
		}

		// Shimmy control
		if (TrInput & IN_LEFT || TrInput & IN_RIGHT)
		{
			PHD_VECTOR shimmy = { now.x, now.y, now.z };
			short dir = 0;

			if (TrInput & IN_LEFT)
			{
				shimmy.x -= offset.z / 2;
				shimmy.z += offset.x / 2;
				dir = -ANGLE(90.0f);
			}
			else if (TrInput & IN_RIGHT)
			{
				shimmy.x += offset.z / 2;
				shimmy.z -= offset.x / 2;
				dir = ANGLE(90.0f);
			}

			FLOOR_INFO* floorNext = (FLOOR_INFO*)GetFloor(shimmy.x, shimmy.y, shimmy.z, &(tempRoom = item->RoomNumber));

			if (TestMonkey(floorNext, shimmy.x, shimmy.y, shimmy.z))
			{
				slope = floorNext->TiltXZ(shimmy.x, shimmy.z, false);

				int yDiff = GetCeiling(floorNext, shimmy.x, shimmy.y, shimmy.z) - ceiling;

				int height;
				short bridge1 = FindBridge(4, goalOrient, shimmy, &height, -CLICK(2.5f), -CLICK(1.5f));

				if ((SlopeCheck(slope, goal) && abs(yDiff) < 64) || bridge1 >= 0)
					SetAnimation(item, dir < 0 ? LA_OVERHANG_SHIMMY_LEFT : LA_OVERHANG_SHIMMY_RIGHT);
			}
		}
	}
}

void lara_as_slopehang(ITEM_INFO* item, COLL_INFO* coll)
{
	if (GlobalCounter % 2)
		item->Position.xRot--;
	else
		item->Position.xRot++;

	if (Camera.type != CameraType::Chase)
		return;

	Camera.targetElevation = -SECTOR(1);
	Camera.targetDistance = CLICK(6.5f);
	Camera.speed = 15;
}

void lara_col_slopeshimmy(ITEM_INFO* item, COLL_INFO* coll)
{
	GetCollisionInfo(coll, item);

	PHD_VECTOR now = { item->Position.xPos, item->Position.yPos, item->Position.zPos };
	PHD_VECTOR offset = { 0, 0, 0 };
	Vector2 slope = { 0, 0 };
	Vector2 goal = { 0, 0 };
	short climbOrient = 0;
	short goalOrient = 0;

	FillSlopeData(item->Position.yRot, goal, climbOrient, goalOrient, offset);

	short tempRoom = 0;

	auto floorNow = GetFloor(now.x, now.y, now.z, &(tempRoom = item->RoomNumber));
	int ceiling = GetCeiling(floorNow, now.x, now.y, now.z);
	item->Position.yPos = ceiling + HEIGHT_ADJUST;

	PHD_VECTOR shimmy = { item->Position.xPos, item->Position.yPos, item->Position.zPos };
	if (item->AnimNumber == LA_OVERHANG_SHIMMY_LEFT)
	{
		shimmy.x -= offset.z / 2;
		shimmy.z += offset.x / 2;
	}
	else
	{
		shimmy.x += offset.z / 2;
		shimmy.z -= offset.x / 2;
	}

	auto floorNext = GetFloor(shimmy.x, shimmy.y, shimmy.z, &(tempRoom = item->RoomNumber));

	bool cancelShimmy = true;
	if (TestMonkey(floorNext, shimmy.x, shimmy.y, shimmy.z))
	{
		slope = floorNext->TiltXZ(shimmy.x, shimmy.z, false);

		int yDiff = GetCeiling(floorNext, shimmy.x, shimmy.y, shimmy.z) - ceiling;

		int height;
		short bridge1 = FindBridge(4, goalOrient, shimmy, &height, -CLICK(2.5f), -CLICK(1.5f)); 

		if ((SlopeCheck(slope, goal) && abs(yDiff) < 64) || bridge1 >= 0)
			cancelShimmy = false;
	}

	if (cancelShimmy)
		SetAnimation(item, (item->AnimNumber == LA_OVERHANG_SHIMMY_LEFT) ? LA_OVERHANG_SHIMMY_LEFT_STOP : LA_OVERHANG_SHIMMY_RIGHT_STOP);
}

void lara_as_slopeshimmy(ITEM_INFO* item, COLL_INFO* coll)
{
	if (GlobalCounter % 2)
		item->Position.xRot--;
	else
		item->Position.xRot++;

	if (Camera.type != CameraType::Chase)
		return;

	Camera.targetElevation = -SECTOR(1);
	Camera.targetDistance = CLICK(6.5f);
	Camera.speed = 15;

	auto* lara = GetLaraInfo(item);

	if (item->AnimNumber == LA_OVERHANG_SHIMMY_LEFT)
	{
		lara->Control.MoveAngle = item->Position.yRot - ANGLE(90.0f);
		Camera.targetAngle = -ANGLE(22.5f);
	}
	else
	{
		lara->Control.MoveAngle = item->Position.yRot + ANGLE(90.0f);
		Camera.targetAngle = ANGLE(22.5f);
	}
}

void lara_as_slopeclimbup(ITEM_INFO* item, COLL_INFO* coll)
{
	if (GlobalCounter % 2)
		item->Position.xRot--;
	else
		item->Position.xRot++;

	Camera.flags = 1;

	if (Camera.type != CameraType::Chase)
		return; // If camera mode isn't chase (0) then don't change camera angles

	Camera.targetElevation = SECTOR(2);
	Camera.targetDistance = CLICK(7);
	Camera.speed = 15;


	if (!(TrInput & IN_ACTION))
	{
		int frame = GetCurrentRelativeFrameNumber(item);
		int length = GetFrameCount(item->AnimNumber);
		int dPos = CLICK(1) - (frame * CLICK(1) / length);

		TranslateItem(item, 0, dPos, dPos);
		if (item->AnimNumber == LA_OVERHANG_CLIMB_UP_LEFT)
			SetAnimation(item, frame <= 2 * length / 3 ? LA_OVERHANG_DROP_LEFT : LA_OVERHANG_DROP_RIGHT);
		else
			SetAnimation(item, frame <= 2 * length / 3 ? LA_OVERHANG_DROP_RIGHT : LA_OVERHANG_DROP_LEFT);
	}
}

void lara_as_slopeclimbdown(ITEM_INFO* item, COLL_INFO* coll)
{
	if (GlobalCounter % 2)
		item->Position.xRot--;
	else
		item->Position.xRot++;

	Camera.flags = 1;

	if (Camera.type != CameraType::Chase)
		return;

	Camera.targetElevation = -3072;
	Camera.targetDistance = 1664;
	Camera.speed = 15;

	if (!(TrInput & IN_ACTION))
	{
		int frame = GetCurrentRelativeFrameNumber(item);
		int length = GetFrameCount(item->AnimNumber);
		int dPos = frame * CLICK(1) / length;

		TranslateItem(item, 0, dPos, dPos);
		if (item->AnimNumber == LA_OVERHANG_CLIMB_DOWN_LEFT)
			SetAnimation(item, frame <= length / 2 ? LA_OVERHANG_DROP_LEFT : LA_OVERHANG_DROP_RIGHT);
		else
			SetAnimation(item, frame <= length / 2 ? LA_OVERHANG_DROP_RIGHT : LA_OVERHANG_DROP_LEFT);
	}
}

void lara_as_sclimbstart(ITEM_INFO* item, COLL_INFO* coll)
{
	// Rotating camera effect during monkey to overhead slope transition
	if (item->AnimNumber == LA_OVERHANG_MONKEY_SLOPE_CONVEX)
	{
		int frame = GetCurrentRelativeFrameNumber(item);
		int numFrames = GetFrameCount(item->AnimNumber);

		float frac = (frame * 1.5f) / (float)(numFrames);
		if (frac > 1.0f)
			frac = 1.0f;

		Camera.flags = 1;

		int distance = 1664;
		if (TestLaraSplat(item, 0, 1536, 0))
		{
			distance = 1024;
		}

		if (item->FrameNumber < g_Level.Anims[item->AnimNumber].frameEnd)
		{
			Camera.targetDistance = distance;
			Camera.targetElevation = int(3072 * frac);
			Camera.targetAngle = int(-32767 * frac);
			Camera.targetspeed = 15;
		}
		else
		{

			Camera.targetDistance = distance;
			Camera.targetElevation = SECTOR(3);
			Camera.targetAngle = 0;
			Camera.targetspeed = 15;
		}
	}
	else// if (item->animNumber == LA_OVERHANG_MONKEY_SLOPE_CONCAVE)
	{
		//Camera.flags = 1;
		Camera.targetElevation = -2048;
		Camera.targetDistance = 1664;
		Camera.speed = 15;
	}

	if (GlobalCounter % 2)
		item->Position.xRot++;
	else
		item->Position.xRot--;
}

void lara_as_sclimbstop(ITEM_INFO* item, COLL_INFO* coll)
{
	// Rotating camera effect during monkey to overhead slope transition

	// Following camera effect during the slope to underlying monkey transition
	if (item->AnimNumber == LA_OVERHANG_SLOPE_MONKEY_CONVEX)
	{
		Camera.flags = 1;
		Camera.targetDistance = 1664;
		Camera.targetElevation = 2048;
		Camera.targetspeed = 15;
	}
	// Rotating camera effect during concave slope to monkey transition
	else if (item->AnimNumber == LA_OVERHANG_SLOPE_MONKEY_CONCAVE)
	{
		int frame = GetCurrentRelativeFrameNumber(item);
		int numFrames = GetFrameCount(item->AnimNumber);

		float frac = (frame * 1.25f) / (float)(numFrames);
		if (frac > 1.0f)
			frac = 1.0f;

		Camera.flags = 1;

		if (item->FrameNumber < g_Level.Anims[item->AnimNumber].frameEnd)
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
		item->Position.xRot++;
	else
		item->Position.xRot--;
}

void lara_as_sclimbend(ITEM_INFO* item, COLL_INFO* coll)
{
	switch (item->AnimNumber)
	{
	case LA_OVERHANG_EXIT_MONKEY_FORWARD:
		SetAnimation(item, LA_MONKEY_FORWARD);
		break;
	case LA_OVERHANG_EXIT_MONKEY_IDLE:
		SetAnimation(item, LA_MONKEY_IDLE);
		break;
	case LA_OVERHANG_EXIT_LADDER:
		SetAnimation(item, LA_LADDER_IDLE);
		break;
	case LA_OVERHANG_EXIT_VAULT:
		SetAnimation(item, LA_HANG_TO_STAND_END);
		break;
	case LA_OVERHANG_EXIT_DROP:
		SetAnimation(item, LA_FALL);
		break;
	case LA_OVERHANG_EXIT_HANG:
		SetAnimation(item, LA_JUMP_UP);
		break;
	}

	((LaraInfo*&)item->Data)->NextCornerPos.zRot = 0;
	item->Position.xRot = 0;
}

// ********  Extending existing state routines  ********

// Extends state 10 (AS_HANG)
void SlopeHangExtra(ITEM_INFO* item, COLL_INFO* coll)
{
	if (!g_GameFlow->Animations.HasOverhangClimb)
		return;

	PHD_VECTOR offset = { 0, 0, 0 };
	Vector2 goal = { 0, 0 };
	short climbOrient = 0;
	short goalOrient = 0;

	FillSlopeData(item->Position.yRot, goal, climbOrient, goalOrient, offset);

	short tempRoom = 0;
	PHD_VECTOR down = { item->Position.xPos + offset.x, item->Position.yPos + CLICK(1), item->Position.zPos + offset.z };
	auto floorNext = GetFloor(down.x, down.y, down.z, &(tempRoom = item->RoomNumber));
	int ceilDist = item->Position.yPos - GetCeiling(floorNext, down.x, down.y, down.z);

	if (item->TargetState == LS_LADDER_IDLE) // Prevent going from hang to climb mode if slope is under ladder
	{
		if (ceilDist >= CLICK(1) && ceilDist < CLICK(2))
		{
			auto slope = floorNext->TiltXZ(down.x, down.z, false);

			if ((slope.x / 3) == (goal.x / 3) || (slope.y / 3) == (goal.y / 3))
			{
				item->TargetState = LS_HANG;
				if (TrInput & IN_FORWARD)
					SetAnimation(item, LA_LADDER_SHIMMY_UP);
				/*else if (TrInput & IN_BACK)
					SetAnimation(item, LA_LADDER_SHIMMY_DOWN);*/
			}
		}
	}
	/*else if (item->TargetState == AS_HANG)
	{
		if (item->animNumber == LA_LADDER_SHIMMY_DOWN)
		{
			if (ceilDist < CLICK(1))
			{
				auto slope = floorNext->TiltXZ(down.x, down.z, false);
				if ((slope.tiltX / 3) == (goal.tiltX / 3) || (slope.tiltZ / 3) == (goal.tiltZ / 3))
					SetAnimation(item, LA_REACH_TO_HANG, 21);
			}
		}
	}*/
}

// Extends state 11 (AS_REACH)
void SlopeReachExtra(ITEM_INFO* item, COLL_INFO* coll)
{
	if (!g_GameFlow->Animations.HasOverhangClimb)
		return;

	PHD_VECTOR now = { item->Position.xPos, item->Position.yPos, item->Position.zPos };
	PHD_VECTOR offset = { 0, 0, 0 };
	Vector2 slope = { 0, 0 };
	Vector2 goal = { 0, 0 };
	short climbOrient = 0;
	short goalOrient = 0;

	FillSlopeData(item->Position.yRot, goal, climbOrient, goalOrient, offset);
	PHD_VECTOR down = { item->Position.xPos + offset.x, item->Position.yPos + CLICK(1), item->Position.zPos + offset.z };

	short tempRoom = 0;

	auto floorNow = GetFloor(now.x, now.y, now.z, &(tempRoom = item->RoomNumber));
	int ceilDist = item->Position.yPos - GetCeiling(floorNow, now.x, now.y, now.z);

	if (TestMonkey(floorNow, now.x, now.y, now.z) && ceilDist <= CLICK(3.5f))
	{
		slope = floorNow->TiltXZ(now.x, now.z, false);

		int height;
		short bridge1 = FindBridge(4, goalOrient, now, &height, -CLICK(4), -5 * (CLICK(0.5f))); 

		if (abs(slope.x) > 2 || abs(slope.y) > 2 || bridge1 >= 0)
		{
			bool disableGrab = true;
			if (SlopeCheck(slope, goal) || bridge1 >= 0)
			{
				if (abs(DirOrientDiff(item->Position.yRot, goalOrient)) < 0x1800)
					disableGrab = false;
			}

			if (disableGrab)
				TrInput &= ~IN_ACTION; // HACK!
		}
	}
}

// Extends state 56 (AS_CLIMBSTNC)
void SlopeClimbExtra(ITEM_INFO* item, COLL_INFO* coll)
{
	if (!g_GameFlow->Animations.HasOverhangClimb)
		return;

	PHD_VECTOR now = { item->Position.xPos, item->Position.yPos, item->Position.zPos };
	PHD_VECTOR offset = { 0, 0, 0 };
	Vector2 slope = { 0, 0 };
	Vector2 goal = { 0, 0 };
	short climbOrient = 0;
	short goalOrient = 0;

	FillSlopeData(item->Position.yRot, goal, climbOrient, goalOrient, offset);
	PHD_VECTOR down = { item->Position.xPos + offset.x, item->Position.yPos + CLICK(1), item->Position.zPos + offset.z };

	short tempRoom = 0;

	auto floorNow = GetFloor(now.x, now.y, now.z, &(tempRoom = item->RoomNumber));
	int ceiling = GetCeiling(floorNow, now.x, now.y, now.z);

	// Block for ladder to overhead slope transition
	if (item->AnimNumber == LA_LADDER_IDLE)
	{
		if (TrInput & IN_FORWARD)
		{
			int ceiling = GetCeiling(floorNow, now.x, now.y, now.z);
			int ceilDist = ceiling - item->Position.yPos;

			if (TestMonkey(floorNow, now.x, now.y, now.z) && ceilDist >= -CLICK(4) && ceilDist <= -CLICK(3))
			{
				slope = floorNow->TiltXZ(item->Position.xPos, item->Position.zPos, false);

				int height;

				short facing = item->Position.yRot + ANGLE(45);
				facing &= 0xC000;

				short bridge1 = FindBridge(4, facing, now, &height, -CLICK(4), -CLICK(3)); 

				if (SlopeCheck(slope, goal) || bridge1 >= 0)
				{
					item->Position.yPos = ceiling + 900;
					SetAnimation(item, LA_OVERHANG_LADDER_SLOPE_CONCAVE); // Ladder to overhead slope transition (concave)
				}
			}
		}

		if (TrInput & IN_BACK)
		{
			auto floorNext = GetFloor(down.x, down.y, down.z, &(tempRoom = item->RoomNumber));
			int ceiling = GetCeiling(floorNext, down.x, down.y, down.z);
			int ceilDist = ceiling - item->Position.yPos;

			if (TestMonkey(floorNext, down.x, down.y, down.z) && ceilDist >= 0 && ceilDist <= CLICK(1))
			{
				slope = floorNext->TiltXZ(down.x, down.z, false);

				int height;

				short facing = item->Position.yRot + ANGLE(45);
				facing &= 0xC000;

				short bridge1 = FindBridge(4, facing, down, &height, -CLICK(0.5f), -CLICK(0.25f)); 

				if (SlopeCheck(slope, goal) || bridge1 >= 0)
				{
					item->Position.yPos = ceiling - 156;
					SetAnimation(item, LA_OVERHANG_LADDER_SLOPE_CONVEX); // Ladder to underlying slope transition (convex)
				}
			}
		}
	}
}

void LadderMonkeyExtra(ITEM_INFO* item, COLL_INFO* coll)
{
	if (!g_GameFlow->Animations.HasOverhangClimb)
		return;

	auto result = GetCollisionResult(item);
	auto ceilingTilt = result.Block->TiltXZ(item->Position.xPos, item->Position.yPos, false);

	if (abs(ceilingTilt.x) > 2 || abs(ceilingTilt.y) > 2)
		return;

	if (result.BottomBlock->Flags.Monkeyswing && (item->Position.yPos - coll->Setup.Height - CLICK(0.5f) <= result.Position.Ceiling))
		item->TargetState = LS_MONKEY_IDLE;
}

// Extends state 61 (AS_CLIMBDOWN)
void SlopeClimbDownExtra(ITEM_INFO* item, COLL_INFO* coll)
{
	if (!g_GameFlow->Animations.HasOverhangClimb)
		return;

	PHD_VECTOR now = { item->Position.xPos, item->Position.yPos, item->Position.zPos };
	PHD_VECTOR offset = { 0, 0, 0 };
	Vector2 slope = { 0, 0 };
	Vector2 goal = { 0, 0 };
	short climbOrient = 0;
	short goalOrient = 0;

	FillSlopeData(item->Position.yRot, goal, climbOrient, goalOrient, offset);
	PHD_VECTOR down = { item->Position.xPos + offset.x, item->Position.yPos + CLICK(1), item->Position.zPos + offset.z };

	short tempRoom = 0;

	auto floorNow = GetFloor(now.x, now.y, now.z, &(tempRoom = item->RoomNumber));
	int ceiling = GetCeiling(floorNow, now.x, now.y, now.z);

	if (item->AnimNumber == LA_LADDER_DOWN) // Make Lara stop before underlying slope ceiling at correct height
	{
		if (TrInput & IN_BACK)
		{
			auto floorNext = GetFloor(down.x, down.y, down.z, &(tempRoom = item->RoomNumber));
			int ceiling = GetCeiling(floorNext, down.x, down.y, down.z);
			int ceilDist = ceiling - item->Position.yPos;

			if (TestMonkey(floorNext, down.x, down.y, down.z))
			{
				slope = floorNext->TiltXZ(down.x, down.z, false);

				int midpoint = 29; // HACK: lara_col_climbdown func, case for frame 29, dehardcode later.
				
				//down.y += 256;
				int height;
				if (!GetFrameNumber(item, 0))
				{
					short bridge1 = FindBridge(4, goalOrient, down, &height, -CLICK(3), CLICK(4)); 
					if (ceilDist < CLICK(1) && (bridge1 >= 0 || SlopeCheck(slope, goal)))
						item->TargetState = LS_LADDER_IDLE;
				}
				else if (GetFrameNumber(item, 0) == midpoint)
				{
					short bridge1 = FindBridge(4, goalOrient, down, &height, -CLICK(2), CLICK(5)); 
					if (ceilDist < CLICK(1) * 2 && (bridge1 >= 0 || SlopeCheck(slope, goal)))
					{
						item->Position.yPos += CLICK(1); // Do midpoint Y translation
						item->TargetState = LS_LADDER_IDLE;
					}
				}
			}
		}
	}
}

// Extends state 75 (AS_HANG2)
void SlopeMonkeyExtra(ITEM_INFO* item, COLL_INFO* coll)
{
	auto* lara = GetLaraInfo(item);

	if (!g_GameFlow->Animations.HasOverhangClimb)
		return;

	PHD_VECTOR now = { item->Position.xPos, item->Position.yPos, item->Position.zPos };
	PHD_VECTOR offset = { 0, 0, 0 };
	Vector2 slope = { 0, 0 };
	Vector2 goal = { 0, 0 };
	short climbOrient = 0;
	short goalOrient = 0;

	FillSlopeData(item->Position.yRot, goal, climbOrient, goalOrient, offset);
	PHD_VECTOR down = { item->Position.xPos + offset.x, item->Position.yPos + CLICK(1), item->Position.zPos + offset.z };

	short tempRoom = 0;

	auto floorNow = GetFloor(now.x, now.y, now.z, &(tempRoom = item->RoomNumber));
	int ceiling = GetCeiling(floorNow, now.x, now.y, now.z);

	if (item->AnimNumber == LA_REACH_TO_MONKEY && !GetFrameNumber(item, 0)) // Manage proper grabbing of monkey slope on forward jump
	{
		int ceiling = GetCeiling(floorNow, now.x, now.y, now.z);
		int ceilDist = item->Position.yPos - ceiling;

		if (TestMonkey(floorNow, now.x, now.y, now.z) && ceilDist <= CLICK(3.5f))
		{
			slope = floorNow->TiltXZ(now.x, now.z, false);

			int height;

			short facing = item->Position.yRot + ANGLE(45);
			facing &= 0xC000;

			short bridge1 = FindBridge(4, facing, now, &height, -CLICK(3.5f), -CLICK(2.5f)); 

			if (SlopeCheck(slope, goal) || bridge1 >= 0)
			{
				lara->NextCornerPos.zRot = AlignToGrab(item);

				int ceiling2 = GetCeiling(floorNow, item->Position.xPos, item->Position.yPos, item->Position.zPos);
				item->Position.yPos = ceiling2 + HEIGHT_ADJUST;

				SetAnimation(item, LA_OVERHANG_HANG_SWING);
			}
		}
	}

	if (TrInput & IN_FORWARD) // Monkey to slope transitions
	{
		if (TestMonkey(floorNow, now.x, now.y, now.z) && ((item->AnimNumber == LA_REACH_TO_MONKEY && GetFrameNumber(item, 0) >= 54) || item->AnimNumber == LA_MONKEY_IDLE))
		{
			if (abs(DirOrientDiff(goalOrient, item->Position.yRot)) <= ANGLE(30) &&
				InStrip(item->Position.xPos, item->Position.zPos, item->Position.yRot, 0, CLICK(1) / 2))
			{
				auto floorNext = GetFloor(down.x, down.y, down.z, &(tempRoom = item->RoomNumber));

				if (TestMonkey(floorNext, down.x, down.y, down.z))
				{
					int ceiling = GetCeiling(floorNext, down.x, now.y, down.z);
					int yDiff = ceiling - GetCeiling(floorNow, now.x, now.y, now.z);

					slope = floorNext->TiltXZ(down.x, down.z, false);

					int height;

					short bridge1 = FindBridge(4, goalOrient, down, &height, -CLICK(7) >> 1, -CLICK(5) >> 1);
					if ((SlopeCheck(slope, goal) && yDiff > 0 && yDiff < CLICK(1)) || bridge1 >= 0)
					{
						AlignToEdge(item, SLOPE_ALIGNMENT);
						SetAnimation(item, LA_OVERHANG_MONKEY_SLOPE_CONCAVE); // Transition from monkey to underlying slope (concave)
						return;
						//item->pos.yPos = ceiling + 496;
						//PerformFlipeffect(NULL, 51, 1, 2); // Disable the UP key command for 2 sec // HACK!!!
					}

					bridge1 = FindBridge(4, goalOrient + ANGLE(180.0f), down, &height, -CLICK(5), -CLICK(4));
					if ((SlopeInvCheck(slope, goal) && yDiff > -CLICK(1) && yDiff < 0) || bridge1 >= 0)
					{
						AlignToEdge(item, SLOPE_ALIGNMENT);
						SetAnimation(item, LA_OVERHANG_MONKEY_SLOPE_CONVEX); // Transition from monkey to overhanging slope (convex)
						return;
						//item->pos.yPos = ceiling + 914;
					}
				}
			}
		}

		if (Lara.Control.CanMonkeySwing)
		{
			// Additional overhang ladder tests

			auto collResult = GetCollisionResult(down.x, item->Position.yPos - coll->Setup.Height, down.z, item->RoomNumber);
			auto topSide = item->Position.yPos - coll->Setup.Height;

			if (collResult.BottomBlock->Flags.ClimbPossible(GetClimbDirection(item->Position.yRot + ANGLE(180.0f))) &&
				collResult.Position.Floor >= item->Position.yPos - CLICK(1) &&
				collResult.Position.Ceiling <= topSide - CLICK(1))
			{
				// Primary checks succeeded, now do C-shaped secondary probing
				collResult = GetCollisionResult(down.x, topSide, down.z, collResult.RoomNumber);
				collResult = GetCollisionResult(down.x, topSide - CLICK(2), down.z, collResult.RoomNumber);
				collResult = GetCollisionResult(now.x, topSide - CLICK(2), now.z, collResult.RoomNumber);

				if ((collResult.Position.Floor <= topSide - CLICK(1)) || (collResult.Position.Ceiling >= topSide - CLICK(1)))
				{
					if (item->TargetState != LS_LADDER_IDLE)
					{
						SnapItemToLedge(item, coll);
						item->TargetState = LS_LADDER_IDLE;
					}
				}
			}
		}
	}
}
