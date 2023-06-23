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
#include "Game/Setup.h"
#include "Scripting/Include/Flow/ScriptInterfaceFlowHandler.h"
#include "Specific/Input/Input.h"
#include "Specific/level.h"

using namespace TEN::Input;

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

struct SlopeData
{
	Vector3i Offset;
	Vector2 Goal;
	short ClimbOrient;
	short GoalOrient;
};

// -----------------
// UTILITY FUNCTIONS
// -----------------

inline bool SlopeCheck(Vector2 slope, Vector2 goal)
{
	return (slope.x == goal.x && slope.y == goal.y);
}

inline bool SlopeInvCheck(Vector2 slope, Vector2 goal)
{
	return (slope.x == -goal.x && slope.y == -goal.y);
}

short FindBridge(int tiltGrade, short orient, Vector3i& pos, int* returnHeight, int ceilingMinY = 0, int ceilingMaxY = 0)
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

	int xMin = pos.x & ~WALL_MASK;
	int xMax = xMin + WALL_MASK;
	int zMin = pos.z & ~WALL_MASK;
	int zMax = zMin + WALL_MASK;

	for (int i = 0; i < g_Level.Items.size(); i++)
	{
		auto* bridgeItem = &g_Level.Items[i];
		if (bridgeItem->ObjectNumber != bridgeSlot)
			continue;

		short orientDelta = (short)(bridgeItem->Pose.Orientation.y - orient);

		bool orientCheck = false;
		if (orientDelta == ANGLE(90.0f))
			orientCheck = true;
		else if (bridgeItem->ObjectNumber == ID_BRIDGE_FLAT)
			orientCheck = true;
		else if ((bridgeItem->ObjectNumber == ID_BRIDGE_TILT1 || bridgeItem->ObjectNumber == ID_BRIDGE_TILT2) && abs(orientDelta) == ANGLE(90.0f))
			orientCheck = true;

		if (!orientCheck)
			continue;

		if (bridgeItem->Pose.Position.x >= xMin &&
			bridgeItem->Pose.Position.x <= xMax &&
			bridgeItem->Pose.Position.z >= zMin &&
			bridgeItem->Pose.Position.z <= zMax)
		{
			if (ceilingMinY || ceilingMaxY)
			{
				if (Objects[bridgeItem->ObjectNumber].ceiling == nullptr)
					continue;

				*returnHeight = Objects[bridgeItem->ObjectNumber].ceiling(i, pos.x, pos.y, pos.z).value_or(NO_HEIGHT);
				
				int ceilingDistance = *returnHeight - pos.y;
				if (ceilingDistance >= ceilingMinY && ceilingDistance <= ceilingMaxY)
					return i;
			}
			else
				return i;
		}
	}

	return NO_ITEM;
}

// Get the signed difference between two orientations.
short OrientDelta(short orient1, short orient2)
{
	int deltaOrient = orient1 - orient2;

	if (deltaOrient > ANGLE(180.0f))
		deltaOrient -= ANGLE(360.0f);

	if (deltaOrient < -ANGLE(180.0f))
		deltaOrient += ANGLE(360.0f);

	return short(deltaOrient);
}

// Test if inside sector strip (0-1023) in currently faced quadrant, between min and max.
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
void AlignToEdge(ItemInfo* item, short edgeDist)
{
	if (edgeDist > WALL_MASK)
		edgeDist = WALL_MASK;

	// Align to closest cardinal facing.
	item->Pose.Orientation.y += ANGLE(45.0f);
	item->Pose.Orientation.y &= ANGLE(270.0f);

	// Align to faced edge.
	switch (item->Pose.Orientation.y)
	{
	case FACING_NORTH:
		item->Pose.Position.z &= ~WALL_MASK;
		item->Pose.Position.z += (WALL_MASK - edgeDist);
		break;

	case FACING_SOUTH:
		item->Pose.Position.z &= ~WALL_MASK;
		item->Pose.Position.z += edgeDist + 1;
		break;

	case FACING_EAST:
		item->Pose.Position.x &= ~WALL_MASK;
		item->Pose.Position.x += (WALL_MASK - edgeDist);
		break;

	case FACING_WEST:
		item->Pose.Position.x &= ~WALL_MASK;
		item->Pose.Position.x += edgeDist + 1;
		break;
	}
}

// Correct position after grabbing slope.
bool AlignToGrab(ItemInfo* item)
{
	bool legLeft = false;

	item->Pose.Orientation.y += ANGLE(45.0f);
	item->Pose.Orientation.y &= ANGLE(270.0f);

	switch (item->Pose.Orientation.y)
	{
	case FACING_NORTH:
		if (((item->Pose.Position.z + (int)CLICK(0.5f)) & ~WALL_MASK) == (item->Pose.Position.z & ~WALL_MASK))
			item->Pose.Position.z += CLICK(0.5f);

		item->Pose.Position.z = (item->Pose.Position.z & ~CLICK(1)) + HORIZONTAL_ALIGN_NORTHEAST;
		legLeft = (item->Pose.Position.z & CLICK(1)) ? false : true;
		break;

	case FACING_SOUTH:
		if (((item->Pose.Position.z - (int)CLICK(0.5f)) & ~WALL_MASK) == (item->Pose.Position.z & ~WALL_MASK))
			item->Pose.Position.z -= CLICK(0.5f);

		item->Pose.Position.z = (item->Pose.Position.z & ~CLICK(1)) + HORIZONTAL_ALIGN_SOUTHWEST;
		legLeft = (item->Pose.Position.z & CLICK(1)) ? true : false;
		break;

	case FACING_WEST:
		if (((item->Pose.Position.x - (int)CLICK(0.5f)) & ~WALL_MASK) == (item->Pose.Position.x & ~WALL_MASK))
			item->Pose.Position.x -= CLICK(0.5f);

		item->Pose.Position.x = (item->Pose.Position.x & ~CLICK(1)) + HORIZONTAL_ALIGN_SOUTHWEST;
		legLeft = (item->Pose.Position.x & CLICK(1)) ? true : false;
		break;

	case FACING_EAST:
		if (((item->Pose.Position.x + (int)CLICK(0.5f)) & ~WALL_MASK) == (item->Pose.Position.x & ~WALL_MASK))
			item->Pose.Position.x += CLICK(0.5f);

		item->Pose.Position.x = (item->Pose.Position.x & ~CLICK(1)) + HORIZONTAL_ALIGN_NORTHEAST;
		legLeft = (item->Pose.Position.x & CLICK(1)) ? false : true;
		break;
	}

	return legLeft;
}

SlopeData GetSlopeData(ItemInfo* item)
{
	SlopeData slopeData;
	switch (GetQuadrant(item->Pose.Orientation.y))
	{
	case NORTH:
		slopeData.Offset.z = CLICK(1);
		slopeData.Goal.y = -4;
		slopeData.ClimbOrient = (short)ClimbDirectionFlags::North;
		slopeData.GoalOrient = 0;
		break;

	case EAST:
		slopeData.Offset.x = CLICK(1);
		slopeData.Goal.x = -4;
		slopeData.ClimbOrient = (short)ClimbDirectionFlags::East;
		slopeData.GoalOrient = ANGLE(90.0f);
		break;

	case SOUTH:
		slopeData.Offset.z = -CLICK(1);
		slopeData.Goal.y = 4;
		slopeData.ClimbOrient = (short)ClimbDirectionFlags::South;
		slopeData.GoalOrient = ANGLE(180.0f);
		break;

	case WEST:
		slopeData.Offset.x = -CLICK(1);
		slopeData.Goal.x = 4;
		slopeData.ClimbOrient = (short)ClimbDirectionFlags::West;
		slopeData.GoalOrient = ANGLE(270.0f);
		break;
	}

	return slopeData;
}

// -----------------------------
// OVERHANG CLIMB
// Control & Collision Functions
// -----------------------------

void lara_col_slopeclimb(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	GetCollisionInfo(coll, item);

	auto slopeData = GetSlopeData(item);

	auto now = item->Pose.Position;
	auto up = Vector3i(item->Pose.Position.x - slopeData.Offset.x, item->Pose.Position.y - CLICK(1), item->Pose.Position.z - slopeData.Offset.z);
	auto down = Vector3i(item->Pose.Position.x + slopeData.Offset.x, item->Pose.Position.y + CLICK(1), item->Pose.Position.z + slopeData.Offset.z);

	auto probeNow = GetCollision(now.x, now.y, now.z, item->RoomNumber);
	auto probeUp = GetCollision(up.x, up.y, up.z, item->RoomNumber);
	auto probeDown = GetCollision(down.x, down.y, down.z, item->RoomNumber);

	if (item->Animation.AnimNumber == LA_OVERHANG_LADDER_SLOPE_CONCAVE)
		return;

	item->Pose.Position.y = probeNow.Position.Ceiling + HEIGHT_ADJUST;

	// Drop down if action not pressed.
	if (!(TrInput & IN_ACTION))
	{
		SetAnimation(item, item->Animation.AnimNumber == LA_OVERHANG_IDLE_LEFT ? LA_OVERHANG_DROP_LEFT : LA_OVERHANG_DROP_RIGHT);
		return;
	}

	// Engage shimmy mode if LEFT/LSTEP or RIGHT/RSTEP are pressed.
	if (TrInput & IN_LEFT || TrInput & IN_RIGHT)
	{
		lara->Context.NextCornerPos.Orientation.z = (item->Animation.AnimNumber == LA_OVERHANG_IDLE_LEFT) ? true : false; // HACK.
		SetAnimation(item, item->Animation.AnimNumber == LA_OVERHANG_IDLE_LEFT ? LA_OVERHANG_IDLE_2_HANG_LEFT : LA_OVERHANG_IDLE_2_HANG_RIGHT);
		return;
	}

	if (TrInput & IN_FORWARD)
	{
		// Test for ledge over slope.
		short tempRoom = probeUp.Block->GetRoomNumberAbove(up.x, up.z).value_or(NO_ROOM);
		if (tempRoom != NO_ROOM)
		{
			auto probeLedge = GetCollision(now.x, now.y - CLICK(3), now.z, tempRoom);

			if ((probeLedge.Position.Floor - probeLedge.Position.Ceiling) >= CLICK(3) &&
				abs((item->Pose.Position.y - (CLICK(2.5f) + 48)) - probeLedge.Position.Floor) < 64)
			{
				AlignToEdge(item, FORWARD_ALIGNMENT);
				SetAnimation(item, LA_OVERHANG_LEDGE_VAULT_START); // Ledge climb-up from slope.
			}
		}

		// Test for slope to overhead ladder transition (convex).
		if (GetClimbFlags(probeUp.BottomBlock) & slopeData.ClimbOrient &&
			InStrip(item->Pose.Position.x, item->Pose.Position.z, item->Pose.Orientation.y, CLICK(3), CLICK(4)))
		{
			if (GetCollision(probeUp.Block, up.x, up.y, up.z).Position.Ceiling - item->Pose.Position.y <= (BLOCK(1.5f) - 80))  // Check if a wall is actually there.
			{
				AlignToEdge(item, FORWARD_ALIGNMENT);
				SetAnimation(item, LA_OVERHANG_SLOPE_LADDER_CONVEX_START);
			}
		}

		// Test for monkey at next position.
		if (probeUp.BottomBlock->Flags.Monkeyswing)
		{
			int yDelta = probeUp.Position.Ceiling - probeNow.Position.Ceiling;

			int height; // Height variable for bridge ceiling functions.

			// Test for upwards slope to climb.
			short bridge = FindBridge(4, item->Pose.Orientation.y, up, &height, -CLICK(2.5f), -CLICK(1.5f));
			if (yDelta >= -CLICK(1.25f) && yDelta <= -CLICK(0.75f) && (SlopeCheck(probeUp.CeilingTilt, slopeData.Goal) || bridge >= 0))
			{
				// Do one more check for wall/ceiling step 2 * offX / Z further to avoid lara sinking her head in wall/step.
				auto probeWall = GetCollision((up.x - slopeData.Offset.x), (up.y - CLICK(1)), (up.z - slopeData.Offset.z), item->RoomNumber);

				if (!probeWall.Block->IsWall((up.x - slopeData.Offset.x), (up.z - slopeData.Offset.z)) &&
					(probeNow.Position.Ceiling - probeWall.Position.Ceiling) > CLICK(0.5f)) // No wall or downward ceiling step.
				{
					TranslateItem(item, 0, -CLICK(1), -CLICK(1));
					SetAnimation(item, item->Animation.AnimNumber == LA_OVERHANG_IDLE_LEFT ? LA_OVERHANG_CLIMB_UP_LEFT : LA_OVERHANG_CLIMB_UP_RIGHT);
					//item->TargetState = 62;
				}
			}

			// Test for flat monkey (abs(slope) < 2).
			bridge = FindBridge(0, slopeData.GoalOrient, up, &height, -CLICK(2.25f), -CLICK(1.25f));
			if (bridge < 0)
				bridge = FindBridge(1, slopeData.GoalOrient, up, &height, -CLICK(2.25f), -CLICK(1.25f));

			// HACK: because of the different calculations of bridge height in TR4 and TEN, we need to lower yDiff tolerance to 0.9f.
			if (yDelta > -CLICK(0.9f) && yDelta <= -CLICK(0.5f) &&
				((abs(probeUp.CeilingTilt.x) <= 2 && abs(probeUp.CeilingTilt.y) <= 2) || bridge >= 0))
			{
				SetAnimation(item, LA_OVERHANG_SLOPE_MONKEY_CONCAVE); // Slope to overhead monkey transition (concave).
			}
		}
	}
	else if (TrInput & IN_BACK)
	{
		if ((GetClimbFlags(GetCollision(probeNow.Block, item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z).BottomBlock) & slopeData.ClimbOrient) &&
			InStrip(item->Pose.Position.x, item->Pose.Position.z, item->Pose.Orientation.y, 0, CLICK(1)))
		{
			AlignToEdge(item, BACKWARD_ALIGNMENT);
			SetAnimation(item, LA_OVERHANG_SLOPE_LADDER_CONCAVE); // Slope to underlying ladder transition (concave).
			return;
		}

		if (probeDown.BottomBlock->Flags.Monkeyswing)
		{
			int height;
			int yDiff = probeDown.Position.Ceiling - probeNow.Position.Ceiling;

			// Test for flat monkey (abs(slope) < 2).
			short bridge = FindBridge(0, slopeData.GoalOrient, down, &height, -CLICK(3), -CLICK(2));
			if (bridge < 0)
				bridge = FindBridge(1, slopeData.GoalOrient, down, &height, -CLICK(3), -CLICK(2));

			if ((abs(yDiff) < CLICK(1) && abs(probeDown.CeilingTilt.x) <= 2 && abs(probeDown.CeilingTilt.y) <= 2) || bridge >= 0)
				SetAnimation(item, LA_OVERHANG_SLOPE_MONKEY_CONVEX); // Force slope to underlying monkey transition (convex)

			// Test for downward slope to climb.
			bridge = FindBridge(4, slopeData.GoalOrient, down, &height, -CLICK(2.5f), -CLICK(1.5f));
			if (yDiff >= CLICK(0.75f) && yDiff <= CLICK(1.25f) && (SlopeCheck(probeDown.CeilingTilt, slopeData.Goal) || bridge >= 0))
			{
				SetAnimation(item, item->Animation.AnimNumber == LA_OVERHANG_IDLE_LEFT ? LA_OVERHANG_CLIMB_DOWN_LEFT : LA_OVERHANG_CLIMB_DOWN_RIGHT);
				return;
			}
		}
	}
}

void lara_as_slopeclimb(ItemInfo* item, CollisionInfo* coll)
{
	if (GlobalCounter % 2)
		item->Pose.Orientation.x--;
	else
		item->Pose.Orientation.x++;

	Camera.flags = CF_FOLLOW_CENTER;

	if (Camera.type != CameraType::Chase)
		return;

	Camera.targetElevation = -ANGLE(16.75f);
	Camera.targetDistance = BLOCK(1.75f);
	Camera.speed = 15;
}

void lara_as_slopefall(ItemInfo* item, CollisionInfo* coll)
{
	item->Animation.IsAirborne = true;

	if (GlobalCounter % 2)
		item->Pose.Orientation.x--;
	else
		item->Pose.Orientation.x++;

	Camera.flags = CF_FOLLOW_CENTER;

	if (Camera.type != CameraType::Chase)
		return;

	Camera.targetElevation = -ANGLE(16.75f);
	Camera.targetDistance = BLOCK(1.75f);
	Camera.speed = 15;
}

void lara_col_slopehang(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	GetCollisionInfo(coll, item);

	auto slopeData = GetSlopeData(item);

	auto now = item->Pose.Position;

	auto probeNow = GetCollision(now.x, now.y, now.z, item->RoomNumber);
	
	item->Pose.Position.y = probeNow.Position.Ceiling + HEIGHT_ADJUST;

	// Drop down if action not pressed.
	if (!(TrInput & IN_ACTION))
	{
		SetAnimation(item, LA_OVERHANG_HANG_DROP);
		item->Animation.IsAirborne = true;
		return;
	}

	if (item->Animation.AnimNumber != LA_OVERHANG_HANG_SWING)
	{
		// Return to climbing mode.
		if (TrInput & IN_FORWARD || TrInput & IN_BACK)
			SetAnimation(item, lara->Context.NextCornerPos.Orientation.z ? LA_OVERHANG_HANG_2_IDLE_LEFT : LA_OVERHANG_HANG_2_IDLE_RIGHT); // HACK.

		// Shimmy control.
		if (TrInput & IN_LEFT || TrInput & IN_RIGHT)
		{
			auto shimmy = now;
			short direction = 0;

			if (TrInput & IN_LEFT)
			{
				shimmy.x -= slopeData.Offset.z / 2;
				shimmy.z += slopeData.Offset.x / 2;
				direction = -ANGLE(90.0f);
			}
			else if (TrInput & IN_RIGHT)
			{
				shimmy.x += slopeData.Offset.z / 2;
				shimmy.z -= slopeData.Offset.x / 2;
				direction = ANGLE(90.0f);
			}

			auto probeShimmy = GetCollision(shimmy.x, shimmy.y, shimmy.z, item->RoomNumber);

			if (probeShimmy.BottomBlock->Flags.Monkeyswing)
			{
				int yDiff = probeShimmy.Position.Ceiling - probeNow.Position.Ceiling;

				int height;
				short bridge = FindBridge(4, slopeData.GoalOrient, shimmy, &height, -CLICK(2.5f), -CLICK(1.5f));

				if ((SlopeCheck(probeShimmy.CeilingTilt, slopeData.Goal) && abs(yDiff) < 64) || bridge >= 0)
					SetAnimation(item, direction < 0 ? LA_OVERHANG_SHIMMY_LEFT : LA_OVERHANG_SHIMMY_RIGHT);
			}
		}
	}
}

void lara_as_slopehang(ItemInfo* item, CollisionInfo* coll)
{
	if (GlobalCounter % 2)
		item->Pose.Orientation.x--;
	else
		item->Pose.Orientation.x++;

	if (Camera.type != CameraType::Chase)
		return;

	Camera.targetElevation = -BLOCK(1);
	Camera.targetDistance = CLICK(6.5f);
	Camera.speed = 15;
}

void lara_col_slopeshimmy(ItemInfo* item, CollisionInfo* coll)
{
	GetCollisionInfo(coll, item);

	auto slopeData = GetSlopeData(item);

	auto now = item->Pose.Position;

	auto probeNow = GetCollision(now.x, now.y, now.z, item->RoomNumber);
	
	item->Pose.Position.y = probeNow.Position.Ceiling + HEIGHT_ADJUST;

	auto shimmy = item->Pose.Position;
	if (item->Animation.AnimNumber == LA_OVERHANG_SHIMMY_LEFT)
	{
		shimmy.x -= slopeData.Offset.z / 2;
		shimmy.z += slopeData.Offset.x / 2;
	}
	else
	{
		shimmy.x += slopeData.Offset.z / 2;
		shimmy.z -= slopeData.Offset.x / 2;
	}

	auto probeShimmy = GetCollision(shimmy.x, shimmy.y, shimmy.z, item->RoomNumber);

	bool cancelShimmy = true;
	if (probeShimmy.BottomBlock->Flags.Monkeyswing)
	{
		int yDiff = probeShimmy.Position.Ceiling - probeNow.Position.Ceiling;

		int height;
		short bridge = FindBridge(4, slopeData.GoalOrient, shimmy, &height, -CLICK(2.5f), -CLICK(1.5f));

		if ((SlopeCheck(probeShimmy.CeilingTilt, slopeData.Goal) && abs(yDiff) < 64) || bridge >= 0)
			cancelShimmy = false;
	}

	if (cancelShimmy)
		SetAnimation(item, (item->Animation.AnimNumber == LA_OVERHANG_SHIMMY_LEFT) ? LA_OVERHANG_SHIMMY_LEFT_STOP : LA_OVERHANG_SHIMMY_RIGHT_STOP);
}

void lara_as_slopeshimmy(ItemInfo* item, CollisionInfo* coll)
{
	if (GlobalCounter % 2)
		item->Pose.Orientation.x--;
	else
		item->Pose.Orientation.x++;

	if (Camera.type != CameraType::Chase)
		return;

	Camera.targetElevation = -BLOCK(1);
	Camera.targetDistance = CLICK(6.5f);
	Camera.speed = 15;

	auto* lara = GetLaraInfo(item);

	if (item->Animation.AnimNumber == LA_OVERHANG_SHIMMY_LEFT)
	{
		lara->Control.MoveAngle = item->Pose.Orientation.y - ANGLE(90.0f);
		Camera.targetAngle = -ANGLE(22.5f);
	}
	else
	{
		lara->Control.MoveAngle = item->Pose.Orientation.y + ANGLE(90.0f);
		Camera.targetAngle = ANGLE(22.5f);
	}
}

void lara_as_slopeclimbup(ItemInfo* item, CollisionInfo* coll)
{
	if (GlobalCounter % 2)
		item->Pose.Orientation.x--;
	else
		item->Pose.Orientation.x++;

	Camera.flags = CF_FOLLOW_CENTER;

	if (Camera.type != CameraType::Chase)
		return; // If camera mode isn't chase (0) then don't change camera angles.

	Camera.targetElevation = BLOCK(2);
	Camera.targetDistance = CLICK(7);
	Camera.speed = 15;


	if (!(TrInput & IN_ACTION))
	{
		int frame = GetFrameNumber(item);
		int length = GetFrameCount(item->Animation.AnimNumber);
		int dPos = CLICK(1) - (frame * CLICK(1) / length);

		TranslateItem(item, 0, dPos, dPos);
		if (item->Animation.AnimNumber == LA_OVERHANG_CLIMB_UP_LEFT)
			SetAnimation(item, frame <= 2 * length / 3 ? LA_OVERHANG_DROP_LEFT : LA_OVERHANG_DROP_RIGHT);
		else
			SetAnimation(item, frame <= 2 * length / 3 ? LA_OVERHANG_DROP_RIGHT : LA_OVERHANG_DROP_LEFT);
	}
}

void lara_as_slopeclimbdown(ItemInfo* item, CollisionInfo* coll)
{
	if (GlobalCounter % 2)
		item->Pose.Orientation.x--;
	else
		item->Pose.Orientation.x++;

	Camera.flags = CF_FOLLOW_CENTER;

	if (Camera.type != CameraType::Chase)
		return;

	Camera.targetElevation = -3072;
	Camera.targetDistance = 1664;
	Camera.speed = 15;

	if (!(TrInput & IN_ACTION))
	{
		int frame = GetFrameNumber(item);
		int length = GetFrameCount(item->Animation.AnimNumber);
		int dPos = frame * CLICK(1) / length;

		TranslateItem(item, 0, dPos, dPos);
		if (item->Animation.AnimNumber == LA_OVERHANG_CLIMB_DOWN_LEFT)
			SetAnimation(item, frame <= length / 2 ? LA_OVERHANG_DROP_LEFT : LA_OVERHANG_DROP_RIGHT);
		else
			SetAnimation(item, frame <= length / 2 ? LA_OVERHANG_DROP_RIGHT : LA_OVERHANG_DROP_LEFT);
	}
}

void lara_as_sclimbstart(ItemInfo* item, CollisionInfo* coll)
{
	// Rotating camera effect during monkey to overhead slope transition.
	if (item->Animation.AnimNumber == LA_OVERHANG_MONKEY_SLOPE_CONVEX)
	{
		int frame = GetFrameNumber(item);
		int numFrames = GetFrameCount(item->Animation.AnimNumber);

		float frac = (frame * 1.5f) / (float)(numFrames);
		if (frac > 1.0f)
			frac = 1.0f;

		Camera.flags = CF_FOLLOW_CENTER;

		int distance = TestLaraWall(item, 0, BLOCK(1.5f), 0) ? BLOCK(1) : CLICK(6.5f);

		if (item->Animation.FrameNumber < GetAnimData(item).frameEnd)
		{
			Camera.targetDistance = distance;
			Camera.targetElevation = int(BLOCK(3) * frac);
			Camera.targetAngle = int(-ANGLE(180.0f) * frac);
			Camera.targetspeed = 15;
		}
		else
		{

			Camera.targetDistance = distance;
			Camera.targetElevation = BLOCK(3);
			Camera.targetAngle = 0;
			Camera.targetspeed = 15;
		}
	}
	else// if (item->animNumber == LA_OVERHANG_MONKEY_SLOPE_CONCAVE)
	{
		//Camera.flags = CF_FOLLOW_CENTER;
		Camera.targetElevation = -ANGLE(11.25f);
		Camera.targetDistance = CLICK(6.5f);
		Camera.speed = 15;
	}

	if (GlobalCounter % 2)
		item->Pose.Orientation.x++;
	else
		item->Pose.Orientation.x--;
}

void lara_as_sclimbstop(ItemInfo* item, CollisionInfo* coll)
{
	// Rotating camera effect during monkey to overhead slope transition.

	// Following camera effect during the slope to underlying monkey transition.
	if (item->Animation.AnimNumber == LA_OVERHANG_SLOPE_MONKEY_CONVEX)
	{
		Camera.flags = CF_FOLLOW_CENTER;
		Camera.targetDistance = CLICK(6.5f);
		Camera.targetElevation = ANGLE(11.25f);
		Camera.targetspeed = 15;
	}
	// Rotating camera effect during concave slope to monkey transition.
	else if (item->Animation.AnimNumber == LA_OVERHANG_SLOPE_MONKEY_CONCAVE)
	{
		int frame = GetFrameNumber(item);
		int numFrames = GetFrameCount(item->Animation.AnimNumber);

		float frac = (frame * 1.25f) / (float)(numFrames);
		if (frac > 1.0f)
			frac = 1.0f;

		Camera.flags = CF_FOLLOW_CENTER;

		if (item->Animation.FrameNumber < GetAnimData(item).frameEnd)
		{
			
			Camera.targetAngle = (short)(-ANGLE(90.0f) * frac);
			Camera.targetDistance = BLOCK(1.75f) - int(CLICK(2) * frac);
			Camera.targetspeed = 15;
		}
		else
		{
			Camera.targetAngle = ANGLE(90.0f);
			Camera.targetDistance = BLOCK(1.25f);
			Camera.targetspeed = 15;
		}
	}
	else
	{
		Camera.targetDistance = CLICK(6.5f);
		Camera.targetElevation = -ANGLE(11.25f);
		Camera.targetspeed = 15;
	}


	if (GlobalCounter % 2)
		item->Pose.Orientation.x++;
	else
		item->Pose.Orientation.x--;
}

void lara_as_sclimbend(ItemInfo* item, CollisionInfo* coll)
{
	switch (item->Animation.AnimNumber)
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

	GetLaraInfo(item)->Context.NextCornerPos.Orientation.z = 0;
	item->Pose.Orientation.x = 0;
}

// ----------------------------------------
// EXTENSION FUNCTIONS
// For Existing State Control and Collision
// ----------------------------------------

// Extends LS_HANG (10)
void SlopeHangExtra(ItemInfo* item, CollisionInfo* coll)
{
	if (!g_GameFlow->HasOverhangClimb())
		return;

	auto slopeData = GetSlopeData(item);

	auto down = Vector3i(item->Pose.Position.x + slopeData.Offset.x, item->Pose.Position.y + CLICK(1), item->Pose.Position.z + slopeData.Offset.z);
	
	auto probeDown = GetCollision(down.x, down.y, down.z, item->RoomNumber);

	int ceilDist = item->Pose.Position.y - probeDown.Position.Ceiling;

	if (item->Animation.TargetState == LS_LADDER_IDLE) // Prevent going from hang to climb mode if slope is under ladder.
	{
		if (ceilDist >= CLICK(1) && ceilDist < CLICK(2))
		{
			if ((probeDown.CeilingTilt.x / 3) == (slopeData.Goal.x / 3) ||
				(probeDown.CeilingTilt.y / 3) == (slopeData.Goal.y / 3))
			{
				item->Animation.TargetState = LS_HANG;
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
				if ((probeDown.CeilingTilt.x / 3) == (goal.x / 3) ||
					(probeDown.CeilingTilt.z / 3) == (goal.y / 3))
				{
					SetAnimation(item, LA_REACH_TO_HANG, 21);
				}
			}
		}
	}*/
}

// Extends LS_REACH (11)
void SlopeReachExtra(ItemInfo* item, CollisionInfo* coll)
{
	if (!g_GameFlow->HasOverhangClimb())
		return;

	auto slopeData = GetSlopeData(item);

	auto now = item->Pose.Position;

	auto probeNow = GetCollision(now.x, now.y, now.z, item->RoomNumber);
	
	int ceilDist = item->Pose.Position.y - probeNow.Position.Ceiling;

	if (probeNow.BottomBlock->Flags.Monkeyswing && ceilDist <= CLICK(3.5f))
	{
		int height;
		short bridge = FindBridge(4, slopeData.GoalOrient, now, &height, -CLICK(4), -CLICK(2.5f));

		if (abs(probeNow.CeilingTilt.x) > 2 || abs(probeNow.CeilingTilt.y) > 2 || bridge >= 0)
		{
			bool disableGrab = true;
			if (SlopeCheck(probeNow.CeilingTilt, slopeData.Goal) || bridge >= 0)
			{
				if (abs(OrientDelta(item->Pose.Orientation.y, slopeData.GoalOrient)) < ANGLE(33.75f))
					disableGrab = false;
			}

			if (disableGrab)
				ClearAction(In::Action); // HACK.
		}
	}
}

// Extends LS_CLIMB_IDLE (56)
void SlopeClimbExtra(ItemInfo* item, CollisionInfo* coll)
{
	if (!g_GameFlow->HasOverhangClimb())
		return;

	auto slopeData = GetSlopeData(item);

	auto now = item->Pose.Position;
	auto down = Vector3i(item->Pose.Position.x + slopeData.Offset.x, item->Pose.Position.y + CLICK(1), item->Pose.Position.z + slopeData.Offset.z);

	auto probeNow = GetCollision(now.x, now.y, now.z, item->RoomNumber);
	auto probeDown = GetCollision(down.x, down.y, down.z, item->RoomNumber);

	// Block for ladder to overhead slope transition.
	if (item->Animation.AnimNumber == LA_LADDER_IDLE)
	{
		if (TrInput & IN_FORWARD)
		{
			int ceilDist = probeNow.Position.Ceiling - item->Pose.Position.y;

			if (probeNow.BottomBlock->Flags.Monkeyswing && ceilDist >= -CLICK(4) && ceilDist <= -CLICK(3))
			{
				short facing = item->Pose.Orientation.y + ANGLE(45.0f);
				facing &= ANGLE(270.0f);

				int height;
				short bridge = FindBridge(4, facing, now, &height, -CLICK(4), -CLICK(3)); 

				if (SlopeCheck(probeNow.CeilingTilt, slopeData.Goal) || bridge >= 0)
				{
					item->Pose.Position.y = probeNow.Position.Ceiling + 900;
					SetAnimation(item, LA_OVERHANG_LADDER_SLOPE_CONCAVE); // Ladder to overhead slope transition (concave).
				}
			}
		}

		if (TrInput & IN_BACK)
		{
			int ceilDist = probeDown.Position.Ceiling - item->Pose.Position.y;

			if (probeDown.BottomBlock->Flags.Monkeyswing && ceilDist >= 0 && ceilDist <= CLICK(1))
			{
				short facing = item->Pose.Orientation.y + ANGLE(45.0f);
				facing &= ANGLE(270.0f);

				int height;
				short bridge = FindBridge(4, facing, down, &height, -CLICK(0.5f), -CLICK(0.25f)); 

				if (SlopeCheck(probeDown.CeilingTilt, slopeData.Goal) || bridge >= 0)
				{
					item->Pose.Position.y = probeDown.Position.Ceiling - 156;
					SetAnimation(item, LA_OVERHANG_LADDER_SLOPE_CONVEX); // Ladder to underlying slope transition (convex).
				}
			}
		}
	}
}

// Extends LS_LADDER_IDLE (56)
bool LadderMonkeyExtra(ItemInfo* item, CollisionInfo* coll)
{
	auto probe = GetCollision(item);

	if (probe.Position.CeilingSlope)
		return false;

	if (probe.BottomBlock->Flags.Monkeyswing && (item->Pose.Position.y - coll->Setup.Height - CLICK(0.5f) <= probe.Position.Ceiling))
	{
		item->Animation.TargetState = LS_MONKEY_IDLE;
		return true;
	}

	return false;
}

// Extends LS_LADDER_DOWN (61)
void SlopeClimbDownExtra(ItemInfo* item, CollisionInfo* coll)
{
	if (!g_GameFlow->HasOverhangClimb())
		return;

	auto slopeData = GetSlopeData(item);

	auto down = Vector3i(item->Pose.Position.x + slopeData.Offset.x, item->Pose.Position.y + CLICK(1), item->Pose.Position.z + slopeData.Offset.z);
	
	auto probeDown = GetCollision(down.x, down.y, down.z, item->RoomNumber);

	if (item->Animation.AnimNumber == LA_LADDER_DOWN) // Make Lara stop before underlying slope ceiling at correct height.
	{
		if (TrInput & IN_BACK)
		{
			int ceilDist = probeDown.Position.Ceiling - item->Pose.Position.y;

			if (probeDown.BottomBlock->Flags.Monkeyswing && ceilDist >= 0 && ceilDist <= CLICK(1))
			{
				short facing = item->Pose.Orientation.y + ANGLE(45.0f);
				facing &= ANGLE(270.0f);

				int height;
				short bridge = FindBridge(4, facing, down, &height, -CLICK(0.5f), -CLICK(0.25f));

				if (SlopeCheck(probeDown.CeilingTilt, slopeData.Goal) || bridge >= 0)
				{
					item->Pose.Position.y = probeDown.Position.Ceiling - 156;
					item->Animation.TargetState = LS_LADDER_IDLE;
				}
			}

			// Old block.
			/*if (probeDown.BottomBlock->Flags.Monkeyswing)
			{
				int midpoint = 29; // HACK: lara_col_climb_down func, case for frame 29, dehardcode later.
				
				//down.y += 256;
				int height;
				if (!GetFrameNumber(item, 0))
				{
					short bridge = FindBridge(4, slopeData.GoalOrient, down, &height, -CLICK(3), CLICK(4));
					if (ceilDist < CLICK(1) && (bridge >= 0 || SlopeCheck(probeDown.CeilingTilt, slopeData.Goal)))
						item->Animation.TargetState = LS_LADDER_IDLE;
				}
				else if (GetFrameNumber(item, 0) == midpoint)
				{
					short bridge = FindBridge(4, slopeData.GoalOrient, down, &height, -CLICK(2), CLICK(5));
					if (ceilDist < CLICK(1) * 2 && (bridge >= 0 || SlopeCheck(probeDown.CeilingTilt, slopeData.Goal)))
					{
						item->Pose.Position.y += CLICK(1); // Do midpoint Y translation.
						item->Animation.TargetState = LS_LADDER_IDLE;
					}
				}
			}*/
		}
	}
}

// Extends LS_MONKEY_IDLE (75)
void SlopeMonkeyExtra(ItemInfo* item, CollisionInfo* coll)
{
	auto* lara = GetLaraInfo(item);

	if (!g_GameFlow->HasOverhangClimb())
		return;

	auto slopeData = GetSlopeData(item);

	auto now = item->Pose.Position;
	auto down = Vector3i(item->Pose.Position.x + slopeData.Offset.x, item->Pose.Position.y + CLICK(1), item->Pose.Position.z + slopeData.Offset.z);

	auto probeNow = GetCollision(now.x, now.y, now.z, item->RoomNumber);
	auto probeDown = GetCollision(down.x, down.y, down.z, item->RoomNumber);

	if (item->Animation.AnimNumber == LA_REACH_TO_MONKEY && !GetFrameIndex(item, 0)) // Manage proper grabbing of monkey slope on forward jump.
	{
		int ceilDist = item->Pose.Position.y - probeNow.Position.Ceiling;

		if (probeNow.BottomBlock->Flags.Monkeyswing && ceilDist <= CLICK(3.5f))
		{
			short facing = item->Pose.Orientation.y + ANGLE(45.0f);
			facing &= 0xC000;

			int height;
			short bridge = FindBridge(4, facing, now, &height, -CLICK(3.5f), -CLICK(2.5f)); 

			if (SlopeCheck(probeNow.CeilingTilt, slopeData.Goal) || bridge >= 0)
			{
				lara->Context.NextCornerPos.Orientation.z = AlignToGrab(item);

				int ceiling = GetCollision(probeNow.Block, item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z).Position.Ceiling;
				item->Pose.Position.y = ceiling + HEIGHT_ADJUST;

				SetAnimation(item, LA_OVERHANG_HANG_SWING);
			}
		}
	}

	if (TrInput & IN_FORWARD) // Monkey to slope transitions.
	{
		if (probeNow.BottomBlock->Flags.Monkeyswing &&
			((item->Animation.AnimNumber == LA_REACH_TO_MONKEY && GetFrameIndex(item, 0) >= 54) || item->Animation.AnimNumber == LA_MONKEY_IDLE))
		{
			if (abs(OrientDelta(slopeData.GoalOrient, item->Pose.Orientation.y)) <= ANGLE(30.0f) &&
				InStrip(item->Pose.Position.x, item->Pose.Position.z, item->Pose.Orientation.y, 0, CLICK(0.5f)))
			{
				if (probeDown.BottomBlock->Flags.Monkeyswing)
				{
					int ceiling = GetCollision(probeDown.Block, down.x, now.y, down.z).Position.Ceiling;
					int yDiff = ceiling - probeNow.Position.Ceiling;

					int height;
					short bridge = FindBridge(4, slopeData.GoalOrient, down, &height, -CLICK(7) >> 1, -CLICK(5) >> 1);
					if ((SlopeCheck(probeDown.CeilingTilt, slopeData.Goal) && yDiff > 0 && yDiff < CLICK(1)) || bridge >= 0)
					{
						AlignToEdge(item, SLOPE_ALIGNMENT);
						SetAnimation(item, LA_OVERHANG_MONKEY_SLOPE_CONCAVE); // Transition from monkey to underlying slope (concave).
						return;
						//item->Pose.Position.y = ceiling + 496;
						//PerformFlipeffect(NULL, 51, 1, 2); // Disable the UP key command for 2 sec // HACK!!!
					}

					bridge = FindBridge(4, slopeData.GoalOrient + ANGLE(180.0f), down, &height, -CLICK(5), -CLICK(4));
					if ((SlopeInvCheck(probeDown.CeilingTilt, slopeData.Goal) && yDiff > -CLICK(1) && yDiff < 0) || bridge >= 0)
					{
						AlignToEdge(item, SLOPE_ALIGNMENT);
						SetAnimation(item, LA_OVERHANG_MONKEY_SLOPE_CONVEX); // Transition from monkey to overhanging slope (convex).
						return;
						//item->Pose.Position.y = ceiling + 914;
					}
				}
			}
		}

		if (lara->Control.CanMonkeySwing)
		{
			// Additional overhang ladder tests.

			int y = item->Pose.Position.y - coll->Setup.Height;
			auto probe = GetCollision(down.x, item->Pose.Position.y - coll->Setup.Height, down.z, item->RoomNumber);

			if (probe.BottomBlock->Flags.IsWallClimbable(GetClimbDirectionFlags(item->Pose.Orientation.y + ANGLE(180.0f))) &&
				probe.Position.Floor >= (item->Pose.Position.y - CLICK(1)) &&
				probe.Position.Ceiling <= (y - CLICK(1)))
			{
				// Primary checks succeeded, now do C-shaped secondary probing.
				probe = GetCollision(down.x, y, down.z, probe.RoomNumber);
				probe = GetCollision(down.x, y - CLICK(2), down.z, probe.RoomNumber);
				probe = GetCollision(now.x, y - CLICK(2), now.z, probe.RoomNumber);

				if (probe.Position.Floor <= (y - CLICK(1)) ||
					probe.Position.Ceiling >= (y - CLICK(1)))
				{
					if (item->Animation.TargetState != LS_LADDER_IDLE)
					{
						SnapItemToLedge(item, coll);
						item->Animation.TargetState = LS_LADDER_IDLE;
					}
				}
			}
		}
	}
}
