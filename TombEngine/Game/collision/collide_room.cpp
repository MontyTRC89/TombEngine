#include "framework.h"
#include "Game/collision/collide_room.h"

#include "Game/control/box.h"
#include "Game/control/los.h"
#include "Game/collision/collide_item.h"
#include "Game/animation.h"
#include "Game/Lara/lara.h"
#include "Game/items.h"
#include "Game/room.h"
#include "Sound/sound.h"
#include "Specific/trmath.h"
#include "Renderer/Renderer11.h"

using namespace TEN::Floordata;
using namespace TEN::Renderer;

void ShiftItem(ItemInfo* item, CollisionInfo* coll)
{
	item->Pose.Position += coll->Shift;
	coll->Shift = Vector3Int();
}

void SnapItemToLedge(ItemInfo* item, CollisionInfo* coll, float offsetMultiplier, bool snapToAngle)
{
	TranslateItem(item, coll->NearestLedgeAngle, coll->NearestLedgeDistance + (coll->Setup.Radius * offsetMultiplier));
	item->Pose.Orientation = Vector3Shrt(
		0,
		snapToAngle ? coll->NearestLedgeAngle : item->Pose.Orientation.y,
		0
	);
}

void SnapItemToLedge(ItemInfo* item, CollisionInfo* coll, short angle, float offsetMultiplier)
{
	short backup = coll->Setup.ForwardAngle;
	coll->Setup.ForwardAngle = angle;

	float distance;
	auto ledgeAngle = GetNearestLedgeAngle(item, coll, distance);

	coll->Setup.ForwardAngle = backup;

	TranslateItem(item, ledgeAngle, distance + (coll->Setup.Radius * offsetMultiplier));
	item->Pose.Orientation = Vector3Shrt(0, ledgeAngle, 0);
}

void SnapItemToGrid(ItemInfo* item, CollisionInfo* coll)
{
	SnapItemToLedge(item, coll);

	int direction = (unsigned short)(item->Pose.Orientation.y + ANGLE(45.0f)) / ANGLE(90.0f);

	switch (direction)
	{
	case NORTH:
		item->Pose.Position.z = (item->Pose.Position.z | (SECTOR(1) - 1)) - coll->Setup.Radius;
		break;

	case EAST:
		item->Pose.Position.x = (item->Pose.Position.x | (SECTOR(1) - 1)) - coll->Setup.Radius;
		break;

	case SOUTH:
		item->Pose.Position.z = (item->Pose.Position.z & ~(SECTOR(1) - 1)) + coll->Setup.Radius;
		break;

	case WEST:
		item->Pose.Position.x = (item->Pose.Position.x & ~(SECTOR(1) - 1)) + coll->Setup.Radius;
		break;
	}
}

int FindGridShift(int x, int z)
{
	if ((x / SECTOR(1)) == (z / SECTOR(1)))
		return 0;

	if ((z / SECTOR(1)) <= (x / SECTOR(1)))
		return (-1 - (x & (WALL_SIZE - 1)));
	else
		return ((SECTOR(1) + 1) - (x & (SECTOR(1) - 1)));
}

// Test if the axis-aligned bounding box collides with geometry at all.

bool TestItemRoomCollisionAABB(ItemInfo* item)
{
	auto* framePtr = GetBestFrame(item);
	auto box = framePtr->boundingBox + item->Pose;
	short maxY = std::min(box.Y1, box.Y2);
	short minY = std::max(box.Y1, box.Y2);

	auto test = [item](short x, short y, short z, bool floor)
	{
		auto collPos = GetCollision(x, y, z, item->RoomNumber).Position;
		if (floor) return y > collPos.Floor;
		return y < collPos.Ceiling;
	};

	bool collided =
		test(box.X1, minY, box.Z1, true) ||
		test(box.X2, minY, box.Z1, true) ||
		test(box.X1, minY, box.Z2, true) ||
		test(box.X2, minY, box.Z2, true) ||
		test(box.X1, maxY, box.Z1, false) ||
		test(box.X2, maxY, box.Z1, false) ||
		test(box.X1, maxY, box.Z2, false) ||
		test(box.X2, maxY, box.Z2, false) ;

	return collided;
}

// Overload used to quickly get point/room collision parameters at a given item's position.
CollisionResult GetCollision(ItemInfo* item)
{
	auto newRoomNumber = item->RoomNumber;
	auto floor = GetFloor(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, &newRoomNumber);
	auto probe = GetCollision(floor, item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z);

	probe.RoomNumber = newRoomNumber;
	return probe;
}

// Overload used to probe point/room collision parameters from a given item's position.
CollisionResult GetCollision(ItemInfo* item, short angle, float forward, float up, float right)
{
	short tempRoomNumber = item->RoomNumber;

	// TODO: Find cleaner solution. Constructing a Location for Lara on the spot can result in a stumble when climbing onto thin platforms. @Sezz 2022.06.14
	auto location =
		item->IsLara() ?
		item->Location :
		ROOM_VECTOR{ GetFloor(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, &tempRoomNumber)->Room, item->Pose.Position.y };

	auto point = TranslateVector(item->Pose.Position, angle, forward, up, right);
	int adjacentRoomNumber = GetRoom(location, item->Pose.Position.x, point.y, item->Pose.Position.z).roomNumber;
	return GetCollision(point.x, point.y, point.z, adjacentRoomNumber);
}

// Overload used to probe point/room collision parameters from a given position.
CollisionResult GetCollision(Vector3Int pos, int roomNumber, short angle, float forward, float up, float right)
{
	short tempRoomNumber = roomNumber;
	auto location = ROOM_VECTOR{ GetFloor(pos.x, pos.y, pos.z, &tempRoomNumber)->Room, pos.y };

	auto point = TranslateVector(pos, angle, forward, up, right);
	int adjacentRoomNumber = GetRoom(location, pos.x, point.y, pos.z).roomNumber;
	return GetCollision(point.x, point.y, point.z, adjacentRoomNumber);
}

// Overload used as a universal wrapper across collisional code to replace
// triads of roomNumber-GetFloor()-GetFloorHeight() operations.
// The advantage is that it does NOT modify the incoming roomNumber argument,
// instead storing one modified by GetFloor() within the returned CollisionResult struct.
// This way, no external variables are modified as output arguments.
CollisionResult GetCollision(int x, int y, int z, short roomNumber)
{
	auto room = roomNumber;
	auto floor = GetFloor(x, y, z, &room);
	auto result = GetCollision(floor, x, y, z);

	result.RoomNumber = room;
	return result;
}

// A reworked legacy GetFloorHeight() function which writes data
// into a special CollisionResult struct instead of global variables.
// It writes for both floor and ceiling heights at the same coordinates, meaning it should be used
// in place of successive GetFloorHeight() and GetCeilingHeight() calls to increase readability.
CollisionResult GetCollision(FloorInfo* floor, int x, int y, int z)
{
	CollisionResult result = {};

	// Record coordinates.
	result.Coordinates = Vector3Int(x, y, z);

	// Return provided block into result as itself.
	result.Block = floor;

	// Floor and ceiling heights are directly borrowed from new floordata.
	result.Position.Floor = GetFloorHeight(ROOM_VECTOR{ floor->Room, y }, x, z).value_or(NO_HEIGHT);
	result.Position.Ceiling = GetCeilingHeight(ROOM_VECTOR{ floor->Room, y }, x, z).value_or(NO_HEIGHT);

	// Probe bottom block through portals.
	while (floor->RoomBelow(x, y, z).value_or(NO_ROOM) != NO_ROOM)
	{
		auto* room = &g_Level.Rooms[floor->RoomBelow(x, y, z).value_or(floor->Room)];
		floor = GetSector(room, x - room->x, z - room->z);
	}

	// Return probed bottom block into result.
	result.BottomBlock = floor;

	// Get tilts.
	result.FloorTilt = floor->TiltXZ(x, z, true);
	result.CeilingTilt = floor->TiltXZ(x, z, false);

	// Split, bridge and slope data
	result.Position.DiagonalStep = floor->FloorIsDiagonalStep();
	result.Position.SplitAngle = floor->FloorCollision.SplitAngle;
	result.Position.Bridge = result.BottomBlock->InsideBridge(x, result.Position.Floor, z, true, false);
	result.Position.FloorSlope = result.Position.Bridge < 0 && (abs(result.FloorTilt.x) >= 3 || (abs(result.FloorTilt.y) >= 3));
	result.Position.CeilingSlope = abs(result.CeilingTilt.x) >= 4 || abs(result.CeilingTilt.y) >= 4; // TODO: Fix on bridges placed beneath ceiling slopes. @Sezz 2022.01.29

	// TODO: check if we need to keep here this slope vs. bridge check from legacy GetTiltType
	/*if ((y + CLICK(2)) < (floor->FloorHeight(x, z)))
		result.FloorTilt = Vector2::Zero;*/

	return result;
}

void GetCollisionInfo(CollisionInfo* coll, ItemInfo* item, bool resetRoom)
{
	GetCollisionInfo(coll, item, Vector3Int(), resetRoom);
}

void GetCollisionInfo(CollisionInfo* coll, ItemInfo* item, Vector3Int offset, bool resetRoom)
{
	// Player collision has several more precise checks for bridge collisions.
	// Therefore, we should differentiate these code paths.
	bool playerCollision = item->IsLara();

	// Reset collision parameters.
	coll->CollisionType = CT_NONE;
	coll->Shift.x = 0;
	coll->Shift.y = 0;
	coll->Shift.z = 0;

	// Offset base probe position by provided offset, if any.
	int xPos = item->Pose.Position.x + offset.x;
	int yPos = item->Pose.Position.y + offset.y;
	int zPos = item->Pose.Position.z + offset.z;

	// Specify base probe position, Y position being bounds top side.
	int x = xPos;
	int y = yPos - coll->Setup.Height;
	int z = zPos;

	// Define side probe offsets.
	int xfront, xright, xleft, zfront, zright, zleft;

	// Get nearest 90-degree snapped angle (quadrant).
	auto quadrant = GetQuadrant(coll->Setup.ForwardAngle);

	// Get side probe offsets depending on quadrant.
	// If unconstrained mode is specified, don't use quadrant.
	switch (coll->Setup.Mode == CollisionProbeMode::Quadrants ? quadrant : -1)
	{
	case 0:
		xfront =  phd_sin(coll->Setup.ForwardAngle) * coll->Setup.Radius;
		zfront =  coll->Setup.Radius;
		xleft  = -coll->Setup.Radius;
		zleft  =  coll->Setup.Radius;
		xright =  coll->Setup.Radius;
		zright =  coll->Setup.Radius;
		break;

	case 1:
		xfront =  coll->Setup.Radius;
		zfront =  phd_cos(coll->Setup.ForwardAngle) * coll->Setup.Radius;
		xleft  =  coll->Setup.Radius;
		zleft  =  coll->Setup.Radius;
		xright =  coll->Setup.Radius;
		zright = -coll->Setup.Radius;
		break;

	case 2:
		xfront =  phd_sin(coll->Setup.ForwardAngle) * coll->Setup.Radius;
		zfront = -coll->Setup.Radius;
		xleft  =  coll->Setup.Radius;
		zleft  = -coll->Setup.Radius;
		xright = -coll->Setup.Radius;
		zright = -coll->Setup.Radius;
		break;

	case 3:
		xfront = -coll->Setup.Radius;
		zfront =  phd_cos(coll->Setup.ForwardAngle) * coll->Setup.Radius;
		xleft  = -coll->Setup.Radius;
		zleft  = -coll->Setup.Radius;
		xright = -coll->Setup.Radius;
		zright =  coll->Setup.Radius;
		break;

	default: 
		// No valid quadrant, return true probe offsets from object rotation.
		xfront = phd_sin(coll->Setup.ForwardAngle) * coll->Setup.Radius;
		zfront = phd_cos(coll->Setup.ForwardAngle) * coll->Setup.Radius;
		xleft  = (xfront * (coll->Setup.Mode == CollisionProbeMode::FreeForward ? 0.5f : 1.0f)) + phd_sin(coll->Setup.ForwardAngle - ANGLE(90)) * coll->Setup.Radius;
		zleft  = (zfront * (coll->Setup.Mode == CollisionProbeMode::FreeForward ? 0.5f : 1.0f)) + phd_cos(coll->Setup.ForwardAngle - ANGLE(90)) * coll->Setup.Radius;
		xright = (xfront * (coll->Setup.Mode == CollisionProbeMode::FreeForward ? 0.5f : 1.0f)) + phd_sin(coll->Setup.ForwardAngle + ANGLE(90)) * coll->Setup.Radius;
		zright = (zfront * (coll->Setup.Mode == CollisionProbeMode::FreeForward ? 0.5f : 1.0f)) + phd_cos(coll->Setup.ForwardAngle + ANGLE(90)) * coll->Setup.Radius;
		break;
	}

	// Define generic variables used for later object-specific position test shifts.
	ROOM_VECTOR tfLocation{}, tcLocation{}, lrfLocation{}, lrcLocation{};
	int height, ceiling;

	// Parameter definition ends here, now process to actual collision tests...
	
	// TEST 1: TILT AND NEAREST LEDGE CALCULATION

	auto collResult = GetCollision(x, item->Pose.Position.y, z, item->RoomNumber);
	coll->FloorTilt = collResult.FloorTilt;
	coll->CeilingTilt = collResult.CeilingTilt;
	coll->NearestLedgeAngle = GetNearestLedgeAngle(item, coll, coll->NearestLedgeDistance);

	// Debug angle and distance
	// g_Renderer.PrintDebugMessage("Nearest angle: %d", coll->NearestLedgeAngle);
	// g_Renderer.PrintDebugMessage("Nearest dist:  %f", coll->NearestLedgeDistance);
	
	// TEST 2: CENTERPOINT PROBE

	collResult = GetCollision(x, y, z, item->RoomNumber);
	auto topRoomNumber = collResult.RoomNumber; // Keep top room number as we need it to re-probe from origin room

	if (playerCollision)
	{
		tfLocation = GetRoom(item->Location, x, y, z);
		height = GetFloorHeight(tfLocation, x, z).value_or(NO_HEIGHT);

		tcLocation = GetRoom(item->Location, x, y - item->Animation.VerticalVelocity, z);
		ceiling = GetCeilingHeight(tcLocation, x, z).value_or(NO_HEIGHT);
	}
	else
	{
		height = collResult.Position.Floor;
		ceiling = GetCeiling(collResult.Block, x, y - item->Animation.VerticalVelocity, z);
	}
	if (height  != NO_HEIGHT) height -= (playerCollision ? yPos : y);
	if (ceiling != NO_HEIGHT) ceiling -= y;

	coll->Middle = collResult.Position;
	coll->Middle.Floor = height;
	coll->Middle.Ceiling = ceiling;

	// TEST 3: FRONTAL PROBE

	x = xPos + xfront;
	z = zPos + zfront;

	g_Renderer.AddDebugSphere(Vector3(x, y, z), 32, Vector4(1, 0, 0, 1), RENDERER_DEBUG_PAGE::LOGIC_STATS);

	collResult = GetCollision(x, y, z, topRoomNumber);

	if (playerCollision)
	{
		if (resetRoom)
		{
			tfLocation = item->Location;
			tcLocation = item->Location;
			topRoomNumber = item->RoomNumber;
		}

		tfLocation = GetRoom(tfLocation, x, y, z);
		height = GetFloorHeight(tfLocation, x, z).value_or(NO_HEIGHT);

		tcLocation = GetRoom(tcLocation, x, y - item->Animation.VerticalVelocity, z);
		ceiling = GetCeilingHeight(tcLocation, x, z).value_or(NO_HEIGHT);
	}
	else
	{
		height = collResult.Position.Floor;
		ceiling = GetCeiling(collResult.Block, x, y - item->Animation.VerticalVelocity, z);
	}
	if (height  != NO_HEIGHT) height -= (playerCollision ? yPos : y);
	if (ceiling != NO_HEIGHT) ceiling -= y;

	coll->Front = collResult.Position;
	coll->Front.Floor = height;
	coll->Front.Ceiling = ceiling;

	if (playerCollision)
	{
		tfLocation = GetRoom(tfLocation, x + xfront, y, z + zfront);
		height = GetFloorHeight(tfLocation, x + xfront, z + zfront).value_or(NO_HEIGHT);
	}
	else
	{
		height = GetCollision(x + xfront, y, z + zfront, topRoomNumber).Position.Floor;
	}
	if (height != NO_HEIGHT) height -= (playerCollision ? yPos : y);

	if (coll->Setup.BlockFloorSlopeUp && 
		coll->Front.FloorSlope && 
		coll->Front.Floor < coll->Middle.Floor && 
		coll->Front.Floor < 0 &&
		height < coll->Front.Floor)
	{
		coll->Front.Floor = MAX_HEIGHT;
	}
	else if (coll->Setup.BlockFloorSlopeDown && 
			 coll->Front.FloorSlope && 
			 coll->Front.Floor > coll->Middle.Floor)
	{
		coll->Front.Floor = STOP_SIZE;
	}
	else if (coll->Setup.BlockCeilingSlope &&
			 coll->Front.CeilingSlope)
	{
		coll->Front.Floor = MAX_HEIGHT;
	}
	else if (coll->Setup.BlockDeathFloorDown && 
			 coll->Front.Floor >= CLICK(0.5f) &&
			 collResult.BottomBlock->Flags.Death)
	{
		coll->Front.Floor = STOP_SIZE;
	}
	else if (coll->Setup.BlockMonkeySwingEdge &&
		!GetCollision(x, y + coll->Setup.Height, z, item->RoomNumber).BottomBlock->Flags.Monkeyswing)
	{
		coll->Front.Floor = MAX_HEIGHT;
	}

	// TEST 4: MIDDLE-LEFT PROBE

	x = xPos + xleft;
	z = zPos + zleft;

	g_Renderer.AddDebugSphere(Vector3(x, y, z), 32, Vector4(0, 0, 1, 1), RENDERER_DEBUG_PAGE::LOGIC_STATS);

	collResult = GetCollision(x, y, z, item->RoomNumber);

	if (playerCollision)
	{
		lrfLocation = GetRoom(item->Location, x, y, z);
		height = GetFloorHeight(lrfLocation, x, z).value_or(NO_HEIGHT);

		lrcLocation = GetRoom(item->Location, x, y - item->Animation.VerticalVelocity, z);
		ceiling = GetCeilingHeight(lrcLocation, x, z).value_or(NO_HEIGHT);
	}
	else
	{
		height = collResult.Position.Floor;
		ceiling = GetCeiling(collResult.Block, x, y - item->Animation.VerticalVelocity, z);
	}
	if (height  != NO_HEIGHT) height -= (playerCollision ? yPos : y);
	if (ceiling != NO_HEIGHT) ceiling -= y;

	coll->MiddleLeft = collResult.Position;
	coll->MiddleLeft.Floor = height;
	coll->MiddleLeft.Ceiling = ceiling;

	if (coll->Setup.BlockFloorSlopeUp && 
		coll->MiddleLeft.FloorSlope && 
		coll->MiddleLeft.Floor < 0)
	{
		coll->MiddleLeft.Floor = MAX_HEIGHT;
	}
	else if (coll->Setup.BlockFloorSlopeDown && 
			 coll->MiddleLeft.FloorSlope && 
			 coll->MiddleLeft.Floor > 0)
	{
		coll->MiddleLeft.Floor = STOP_SIZE;
	}
	else if (coll->Setup.BlockCeilingSlope &&
			 coll->MiddleLeft.CeilingSlope)
	{
		coll->MiddleLeft.Floor = MAX_HEIGHT;
	}
	else if (coll->Setup.BlockDeathFloorDown && 
			 coll->MiddleLeft.Floor >= CLICK(0.5f) &&
			 collResult.BottomBlock->Flags.Death)
	{
		coll->MiddleLeft.Floor = STOP_SIZE;
	}
	else if (coll->Setup.BlockMonkeySwingEdge &&
		!GetCollision(x, y + coll->Setup.Height, z, item->RoomNumber).BottomBlock->Flags.Monkeyswing)
	{
		coll->MiddleLeft.Floor = MAX_HEIGHT;
	}

	// TEST 5: FRONT-LEFT PROBE

	collResult = GetCollision(x, y, z, topRoomNumber); // We use plain x/z values here, proposed by Choco

	if (playerCollision)
	{
		tfLocation = GetRoom(tfLocation, x, y, z);
		height = GetFloorHeight(tfLocation, x, z).value_or(NO_HEIGHT);

		tcLocation = GetRoom(tcLocation, x, y - item->Animation.VerticalVelocity, z);
		ceiling = GetCeilingHeight(tcLocation, x, z).value_or(NO_HEIGHT);
	}
	else
	{
		height = collResult.Position.Floor;
		ceiling = GetCeiling(collResult.Block, x, y - item->Animation.VerticalVelocity, z);
	}
	if (height  != NO_HEIGHT) height -= (playerCollision ? yPos : y);
	if (ceiling != NO_HEIGHT) ceiling -= y;

	coll->FrontLeft = collResult.Position;
	coll->FrontLeft.Floor = height;
	coll->FrontLeft.Ceiling = ceiling;

	if (coll->Setup.BlockFloorSlopeUp && 
		coll->FrontLeft.FloorSlope && 
		coll->FrontLeft.Floor < 0)
	{
		coll->FrontLeft.Floor = MAX_HEIGHT;
	}
	else if (coll->Setup.BlockFloorSlopeDown && 
			 coll->FrontLeft.FloorSlope && 
			 coll->FrontLeft.Floor > 0)
	{
		coll->FrontLeft.Floor = STOP_SIZE;
	}
	else if (coll->Setup.BlockCeilingSlope &&
			 coll->FrontLeft.CeilingSlope)
	{
		coll->FrontLeft.Floor = MAX_HEIGHT;
	}
	else if (coll->Setup.BlockDeathFloorDown && 
			 coll->FrontLeft.Floor >= CLICK(0.5f) &&
			 collResult.BottomBlock->Flags.Death)
	{
		coll->FrontLeft.Floor = STOP_SIZE;
	}
	else if (coll->Setup.BlockMonkeySwingEdge &&
		!GetCollision(x, y + coll->Setup.Height, z, item->RoomNumber).BottomBlock->Flags.Monkeyswing)
	{
		coll->FrontLeft.Floor = MAX_HEIGHT;
	}

	// TEST 6: MIDDLE-RIGHT PROBE

	x = xPos + xright;
	z = zPos + zright;

	g_Renderer.AddDebugSphere(Vector3(x, y, z), 32, Vector4(0, 1, 0, 1), RENDERER_DEBUG_PAGE::LOGIC_STATS);

	collResult = GetCollision(x, y, z, item->RoomNumber);

	if (playerCollision)
	{
		lrfLocation = GetRoom(item->Location, x, y, z);
		height = GetFloorHeight(lrfLocation, x, z).value_or(NO_HEIGHT);

		lrcLocation = GetRoom(item->Location, x, y - item->Animation.VerticalVelocity, z);
		ceiling = GetCeilingHeight(lrcLocation, x, z).value_or(NO_HEIGHT);
	}
	else
	{
		height = collResult.Position.Floor;
		ceiling = GetCeiling(collResult.Block, x, y - item->Animation.VerticalVelocity, z);
	}
	if (height  != NO_HEIGHT) height -= (playerCollision ? yPos : y);
	if (ceiling != NO_HEIGHT) ceiling -= y;

	coll->MiddleRight = collResult.Position;
	coll->MiddleRight.Floor = height;
	coll->MiddleRight.Ceiling = ceiling;

	if (coll->Setup.BlockFloorSlopeUp && 
		coll->MiddleRight.FloorSlope && 
		coll->MiddleRight.Floor < 0)
	{
		coll->MiddleRight.Floor = MAX_HEIGHT;
	}
	else if (coll->Setup.BlockFloorSlopeDown && 
			 coll->MiddleRight.FloorSlope && 
			 coll->MiddleRight.Floor > 0)
	{
		coll->MiddleRight.Floor = STOP_SIZE;
	}
	else if (coll->Setup.BlockCeilingSlope &&
			 coll->MiddleRight.CeilingSlope)
	{
		coll->MiddleRight.Floor = MAX_HEIGHT;
	}
	else if (coll->Setup.BlockDeathFloorDown && 
			 coll->MiddleRight.Floor >= CLICK(0.5f) &&
			 collResult.BottomBlock->Flags.Death)
	{
		coll->MiddleRight.Floor = STOP_SIZE;
	}
	else if (coll->Setup.BlockMonkeySwingEdge &&
		!GetCollision(x, y + coll->Setup.Height, z, item->RoomNumber).BottomBlock->Flags.Monkeyswing)
	{
		coll->MiddleRight.Floor = MAX_HEIGHT;
	}

	// TEST 7: FRONT-RIGHT PROBE

	collResult = GetCollision(x, y, z, topRoomNumber);

	if (playerCollision)
	{
		tfLocation = GetRoom(tfLocation, x, y, z);
		height = GetFloorHeight(tfLocation, x, z).value_or(NO_HEIGHT);

		tcLocation = GetRoom(tcLocation, x, y - item->Animation.VerticalVelocity, z);
		ceiling = GetCeilingHeight(tcLocation, x, z).value_or(NO_HEIGHT);
	}
	else
	{
		height = collResult.Position.Floor;
		ceiling = GetCeiling(collResult.Block, x, y - item->Animation.VerticalVelocity, z);
	}
	if (height  != NO_HEIGHT) height -= (playerCollision ? yPos : y);
	if (ceiling != NO_HEIGHT) ceiling -= y;

	coll->FrontRight = collResult.Position;
	coll->FrontRight.Floor = height;
	coll->FrontRight.Ceiling = ceiling;

	if (coll->Setup.BlockFloorSlopeUp && 
		coll->FrontRight.FloorSlope && 
		coll->FrontRight.Floor < 0)
	{
		coll->FrontRight.Floor = MAX_HEIGHT;
	}
	else if (coll->Setup.BlockFloorSlopeDown && 
			 coll->FrontRight.FloorSlope && 
			 coll->FrontRight.Floor > 0)
	{
		coll->FrontRight.Floor = STOP_SIZE;
	}
	else if (coll->Setup.BlockCeilingSlope &&
			 coll->FrontRight.CeilingSlope)
	{
		coll->FrontRight.Floor = MAX_HEIGHT;
	}
	else if (coll->Setup.BlockDeathFloorDown && 
			 coll->FrontRight.Floor >= CLICK(0.5f) &&
			 collResult.BottomBlock->Flags.Death)
	{
		coll->FrontRight.Floor = STOP_SIZE;
	}
	else if (coll->Setup.BlockMonkeySwingEdge &&
		!GetCollision(x, y + coll->Setup.Height, z, item->RoomNumber).BottomBlock->Flags.Monkeyswing)
	{
		coll->FrontRight.Floor = MAX_HEIGHT;
	}

	// TEST 8: SOLID STATIC MESHES

	CollideSolidStatics(item, coll);

	// Collision tests now end.
	// Get to calculation of collision side and shifts.

	if (coll->Middle.Floor == NO_HEIGHT)
	{
		coll->Shift.x = coll->Setup.OldPosition.x - xPos;
		coll->Shift.y = coll->Setup.OldPosition.y - yPos;
		coll->Shift.z = coll->Setup.OldPosition.z - zPos;
		coll->CollisionType = CT_FRONT;
		return;
	}

	if (coll->Middle.Floor - coll->Middle.Ceiling <= 0)
	{
		coll->Shift.x = coll->Setup.OldPosition.x - xPos;
		coll->Shift.y = coll->Setup.OldPosition.y - yPos;
		coll->Shift.z = coll->Setup.OldPosition.z - zPos;
		coll->CollisionType = CT_CLAMP;
		return;
	}

	if (coll->Middle.Ceiling >= 0)
	{
		coll->Shift.y = coll->Middle.Ceiling;
		coll->CollisionType = CT_TOP;
	}

	if (coll->Front.Floor > coll->Setup.LowerFloorBound ||
		coll->Front.Floor < coll->Setup.UpperFloorBound ||
		coll->Front.Ceiling > coll->Setup.LowerCeilingBound ||
		coll->Front.Ceiling < coll->Setup.UpperCeilingBound ||
		coll->Front.Floor - coll->Front.Ceiling <= 0)
	{
		if (coll->Front.HasDiagonalSplit())
		{
			coll->Shift.x = coll->Setup.OldPosition.x - xPos;
			coll->Shift.z = coll->Setup.OldPosition.z - zPos;
		}
		else
		{
			switch (quadrant)
			{
			case 0:
			case 2:
				coll->Shift.x = coll->Setup.OldPosition.x - xPos;
				coll->Shift.z = FindGridShift(zPos + zfront, zPos);
				break;

			case 1:
			case 3:
				coll->Shift.x = FindGridShift(xPos + xfront, xPos);
				coll->Shift.z = coll->Setup.OldPosition.z - zPos;
				break;

			}
		}
		coll->CollisionType = (coll->CollisionType == CT_TOP ? CT_TOP_FRONT : CT_FRONT);
		return;
	}

	if (coll->Front.Ceiling > coll->Setup.LowerCeilingBound ||
		coll->Front.Ceiling < coll->Setup.UpperCeilingBound)
	{
		coll->Shift.x = coll->Setup.OldPosition.x - xPos;
		coll->Shift.y = coll->Setup.OldPosition.y - yPos;
		coll->Shift.z = coll->Setup.OldPosition.z - zPos;
		coll->CollisionType = CT_TOP_FRONT;
		return;
	}

	if (coll->MiddleLeft.Floor > coll->Setup.LowerFloorBound ||
		coll->MiddleLeft.Floor < coll->Setup.UpperFloorBound ||
		coll->MiddleLeft.Ceiling > coll->Setup.LowerCeilingBound ||
		coll->MiddleLeft.Ceiling < coll->Setup.UpperCeilingBound ||
		coll->MiddleLeft.Floor - coll->MiddleLeft.Ceiling <= 0)
	{
		if (coll->TriangleAtLeft() && !coll->MiddleLeft.FloorSlope)
		{
			// HACK: Force slight push-out to the left side to avoid stucking
			TranslateItem(item, coll->Setup.ForwardAngle + ANGLE(8.0f), item->Animation.Velocity);

			coll->Shift.x = coll->Setup.OldPosition.x - xPos;
			coll->Shift.z = coll->Setup.OldPosition.z - zPos;
		}
		else
		{
			switch (quadrant)
			{
			case 0:
			case 2:
				coll->Shift.x = FindGridShift(xPos + xleft, xPos + xfront);
				break;

			case 1:
			case 3:
				coll->Shift.z = FindGridShift(zPos + zleft, zPos + zfront);
				break;
			}
		}

		if (coll->DiagonalStepAtLeft())
		{
			int quarter = (unsigned short)(coll->Setup.ForwardAngle) / ANGLE(90); // different from quadrant!
			quarter %= 2;

			if (coll->MiddleLeft.HasFlippedDiagonalSplit())
			{
				if (quarter) coll->CollisionType = CT_LEFT;
			}
			else
			{
				if (!quarter) coll->CollisionType = CT_LEFT;
			}
		}
		else
		{
			coll->CollisionType = CT_LEFT;
		}

		return;
	}

	if (coll->MiddleRight.Floor > coll->Setup.LowerFloorBound ||
		coll->MiddleRight.Floor < coll->Setup.UpperFloorBound ||
		coll->MiddleRight.Ceiling > coll->Setup.LowerCeilingBound ||
		coll->MiddleRight.Ceiling < coll->Setup.UpperCeilingBound ||
		coll->MiddleRight.Floor - coll->MiddleRight.Ceiling <= 0)
	{
		if (coll->TriangleAtRight() && !coll->MiddleRight.FloorSlope)
		{
			// HACK: Force slight push-out to the right side to avoid stucking
			TranslateItem(item, coll->Setup.ForwardAngle - ANGLE(8.0f), item->Animation.Velocity);

			coll->Shift.x = coll->Setup.OldPosition.x - xPos;
			coll->Shift.z = coll->Setup.OldPosition.z - zPos;
		}
		else
		{
			switch (quadrant)
			{
			case 0:
			case 2:
				coll->Shift.x = FindGridShift(xPos + xright, xPos + xfront);
				break;

			case 1:
			case 3:
				coll->Shift.z = FindGridShift(zPos + zright, zPos + zfront);
				break;
			}
		}

		if (coll->DiagonalStepAtRight())
		{
			int quarter = (unsigned short)(coll->Setup.ForwardAngle) / ANGLE(90); // different from quadrant!
			quarter %= 2;

			if (coll->MiddleRight.HasFlippedDiagonalSplit())
			{
				if (quarter) coll->CollisionType = CT_RIGHT;
			}
			else
			{
				if (!quarter) coll->CollisionType = CT_RIGHT;
			}
		}
		else
		{
			coll->CollisionType = CT_RIGHT;
		}

		return;
	}
}

// New function for rotating item along XZ slopes.
// (int radiusDivide) is for radiusZ, else the MaxZ is too high and cause rotation problem !
// Dont need to set a value in radiusDivisor if you dont need it (radiusDivisor is set to 1 by default).
// Warning: dont set it to 0 !!!!
void CalculateItemRotationToSurface(ItemInfo* item, float radiusDivisor, short xOffset, short zOffset)
{
	if (!radiusDivisor)
	{
		TENLog(std::string("CalculateItemRotationToSurface() attempted division by zero!"), LogLevel::Warning);
		return;
	}

	auto pos = GameVector(
		item->Pose.Position.x,
		item->Pose.Position.y,
		item->Pose.Position.z,
		item->RoomNumber
	);

	auto* bounds = GetBoundsAccurate(item);
	auto radiusX = bounds->X2;
	auto radiusZ = bounds->Z2 / radiusDivisor; // Need divide in any case else it's too much !

	auto ratioXZ = radiusZ / radiusX;
	auto frontX = phd_sin(item->Pose.Orientation.y) * radiusZ;
	auto frontZ = phd_cos(item->Pose.Orientation.y) * radiusZ;
	auto leftX  = -frontZ * ratioXZ;
	auto leftZ  =  frontX * ratioXZ;
	auto rightX =  frontZ * ratioXZ;
	auto rightZ = -frontX * ratioXZ;

	auto frontHeight = GetCollision(pos.x + frontX, pos.y, pos.z + frontZ, pos.roomNumber).Position.Floor;
	auto backHeight  = GetCollision(pos.x - frontX, pos.y, pos.z - frontZ, pos.roomNumber).Position.Floor;
	auto leftHeight  = GetCollision(pos.x + leftX,  pos.y, pos.z + leftZ,  pos.roomNumber).Position.Floor;
	auto rightHeight = GetCollision(pos.x + rightX, pos.y, pos.z + rightZ, pos.roomNumber).Position.Floor;

	auto frontHDif = backHeight  - frontHeight;
	auto sideHDif  = rightHeight - leftHeight;

	// Don't align if height differences are too large
	if ((abs(frontHDif) > STEPUP_HEIGHT) || (abs(sideHDif) > STEPUP_HEIGHT))
		return;

	// NOTE: float(atan2()) is required, else warning about double !
	item->Pose.Orientation.x = ANGLE(float(atan2(frontHDif, 2 * radiusZ)) / RADIAN) + xOffset;
	item->Pose.Orientation.z = ANGLE(float(atan2(sideHDif, 2 * radiusX)) / RADIAN) + zOffset;
}

int GetQuadrant(short angle)
{
	return (unsigned short)(angle + ANGLE(45.0f)) / ANGLE(90.0f);
}

// Determines vertical surfaces and gets nearest ledge angle.
// Allows to eventually use unconstrained vaults and shimmying.

short GetNearestLedgeAngle(ItemInfo* item, CollisionInfo* coll, float& distance)
{
	// Calculation ledge angle for non-Lara objects is unnecessary.
	if (!item->IsLara())
		return 0; 

	// Get item bounds and current rotation
	auto bounds = GetBoundsAccurate(item);
	auto c = phd_cos(coll->Setup.ForwardAngle);
	auto s = phd_sin(coll->Setup.ForwardAngle);

	// Origin test position should be slightly in front of origin, because otherwise
	// misfire may occur near block corners for split angles.
	auto frontalOffset = coll->Setup.Radius * 0.3f;
	auto x = item->Pose.Position.x + frontalOffset * s;
	auto z = item->Pose.Position.z + frontalOffset * c;

	// Determine two Y points to test (lower and higher).
	// 1/10 headroom crop is needed to avoid possible issues with tight diagonal headrooms.
	int headroom = abs(bounds->Y2 - bounds->Y1) / 20.0f;
	int yPoints[2] = { item->Pose.Position.y + bounds->Y1 + headroom,
					   item->Pose.Position.y + bounds->Y2 - headroom };

	// Prepare test data
	float finalDistance[2] = { FLT_MAX, FLT_MAX };
	short finalResult[2] = { 0 };
	bool  hitBridge = false;

	// Do a two-pass surface test for all possible planes in a block.
	// Two-pass test is needed to resolve different scissor cases with diagonal geometry.

	for (int h = 0; h < 2; h++)
	{
		// Use either bottom or top Y point to test
		auto y = yPoints[h];

		// Prepare test data
		Ray   originRay;
		Plane closestPlane[3] = { };
		float closestDistance[3] = { FLT_MAX, FLT_MAX, FLT_MAX };
		short result[3] = { };

		// If bridge was hit on the first pass, stop checking
		if (h == 1 && hitBridge)
			break;

		for (int p = 0; p < 3; p++)
		{
			// Prepare test data
			float distance = 0.0f;

			// Determine horizontal probe coordinates
			auto eX = x;
			auto eZ = z;

			// Determine if probe must be shifted (if left or right probe)
			if (p > 0)
			{
				auto s2 = phd_sin(coll->Setup.ForwardAngle + (p == 1 ? ANGLE(90.0f) : -ANGLE(90.0f)));
				auto c2 = phd_cos(coll->Setup.ForwardAngle + (p == 1 ? ANGLE(90.0f) : -ANGLE(90.0f)));

				// Slightly extend width beyond coll radius to hit adjacent blocks for sure
				eX += s2 * (coll->Setup.Radius * 2);
				eZ += c2 * (coll->Setup.Radius * 2);
			}

			// Debug probe point
			// g_Renderer.AddDebugSphere(Vector3(eX, y, eZ), 16, Vector4(1, 1, 0, 1), RENDERER_DEBUG_PAGE::LOGIC_STATS);

			// Determine front floor probe offset.
			// It is needed to identify if there is bridge or ceiling split in front.
			auto frontFloorProbeOffset = coll->Setup.Radius * 1.5f;
			auto ffpX = eX + frontFloorProbeOffset * s;
			auto ffpZ = eZ + frontFloorProbeOffset * c;

			// Get front floor block
			auto room = GetRoom(item->Location, ffpX, y, ffpZ).roomNumber;
			auto block = GetCollision(ffpX, y, ffpZ, room).Block;

			// Get front floor surface heights
			auto floorHeight   = GetFloorHeight(ROOM_VECTOR{ block->Room, y }, ffpX, ffpZ).value_or(NO_HEIGHT);
			auto ceilingHeight = GetCeilingHeight(ROOM_VECTOR{ block->Room, y }, ffpX, ffpZ).value_or(NO_HEIGHT);

			// If probe landed inside wall (i.e. both floor/ceiling heights are NO_HEIGHT), make a fake
			// ledge for algorithm to further succeed.
			if (floorHeight == NO_HEIGHT && ceilingHeight == NO_HEIGHT)
				floorHeight = y - CLICK(4);

			// If ceiling height tests lower than Y value, it means ceiling
			// ledge is in front and we should use it instead of floor.
			bool useCeilingLedge = ceilingHeight > y;
			int height = useCeilingLedge ? ceilingHeight : floorHeight;

			// Determine if there is a bridge in front
			auto bridge = block->InsideBridge(ffpX, height, ffpZ, true, y == height);

			// Determine floor probe offset.
			// This must be slightly in front of own coll radius so no bridge misfires occur.
			auto floorProbeOffset = coll->Setup.Radius * 0.3f;
			auto fpX = eX + floorProbeOffset * s;
			auto fpZ = eZ + floorProbeOffset * c;

			// Debug probe point
			// g_Renderer.AddDebugSphere(Vector3(fpX, y, fpZ), 16, Vector4(0, 1, 0, 1), RENDERER_DEBUG_PAGE::LOGIC_STATS);

			// Get true room number and block, based on derived height
			room = GetRoom(item->Location, fpX, height, fpZ).roomNumber;
			block = GetCollision(fpX, height, fpZ, room).Block;

			// We don't need actual corner heights to build planes, so just use normalized value here
			auto fY = height - 1;
			auto cY = height + 1;

			// Calculate ray
			auto mxR = Matrix::CreateFromYawPitchRoll(TO_RAD(coll->Setup.ForwardAngle), 0, 0);
			auto direction = (Matrix::CreateTranslation(Vector3::UnitZ) * mxR).Translation();
			auto ray = Ray(Vector3(eX, cY, eZ), direction);

			// Debug ray direction
			// g_Renderer.AddLine3D(Vector3(eX, y, eZ), Vector3(eX, y, eZ) + direction * 256, Vector4(1, 0, 0, 1));

			// Keep origin ray to calculate true centerpoint distance to ledge later
			if (p == 0)
				originRay = Ray(Vector3(eX, cY, eZ), direction);

			if (bridge >= 0) // Surface is inside bridge
			{
				// Get and test DX item coll bounds
				auto bounds = GetBoundsAccurate(&g_Level.Items[bridge]);
				auto dxBounds = TO_DX_BBOX(g_Level.Items[bridge].Pose, bounds);

				// Decompose bounds into planes
				Vector3 corners[8];
				dxBounds.GetCorners(corners);
				Plane plane[4] =
				{
					Plane(corners[2], corners[1], corners[0]),
					Plane(corners[0], corners[4], corners[3]),
					Plane(corners[5], corners[6], corners[7]),
					Plane(corners[6], corners[5], corners[1])
				};

				// Find closest bridge edge plane
				for (int i = 0; i < 4; i++)
				{
					// No plane intersection, quickly discard
					if (!ray.Intersects(plane[i], distance))
						continue;

					// Process plane intersection only if distance is smaller
					// than already found minimum
					if (distance < closestDistance[p])
					{
						closestPlane[p] = plane[i];
						closestDistance[p] = distance;
						auto normal = closestPlane[p].Normal();
						result[p] = FROM_RAD(atan2(normal.x, normal.z));
						hitBridge = true;
					}
				}
			}
			else // Surface is inside block
			{
				// Determine if we should use floor or ceiling split angle based on early tests.
				auto splitAngle = (useCeilingLedge ? block->CeilingCollision.SplitAngle : block->FloorCollision.SplitAngle);

				// Get horizontal block corner coordinates
				auto fX = floor(eX / WALL_SIZE) * WALL_SIZE - 1;
				auto fZ = floor(eZ / WALL_SIZE) * WALL_SIZE - 1;
				auto cX = fX + WALL_SIZE + 1;
				auto cZ = fZ + WALL_SIZE + 1;

				// Debug used block
				// g_Renderer.AddDebugSphere(Vector3(round(eX / WALL_SIZE) * WALL_SIZE + 512, y, round(eZ / WALL_SIZE) * WALL_SIZE + 512), 16, Vector4(1, 1, 1, 1), RENDERER_DEBUG_PAGE::LOGIC_STATS);

				// Get split angle coordinates
				auto sX = fX + 1 + WALL_SIZE / 2;
				auto sZ = fZ + 1 + WALL_SIZE / 2;
				auto sShiftX = coll->Setup.Radius * sin(splitAngle);
				auto sShiftZ = coll->Setup.Radius * cos(splitAngle);

				// Get block edge planes + split angle plane
				Plane plane[5] =
				{
					Plane(Vector3(fX, cY, cZ), Vector3(cX, cY, cZ), Vector3(cX, fY, cZ)), // North 
					Plane(Vector3(fX, cY, fZ), Vector3(fX, cY, cZ), Vector3(fX, fY, cZ)), // West
					Plane(Vector3(cX, fY, fZ), Vector3(cX, cY, fZ), Vector3(fX, cY, fZ)), // South
					Plane(Vector3(cX, fY, cZ), Vector3(cX, cY, cZ), Vector3(cX, cY, fZ)), // East
					Plane(Vector3(sX, cY, sZ), Vector3(sX, fY, sZ), Vector3(sX + sShiftX, cY, sZ + sShiftZ)) // Split
				};

				// If split angle exists, take split plane into account too.
				auto useSplitAngle = (useCeilingLedge ? block->CeilingIsSplit() : block->FloorIsSplit());

				// Find closest block edge plane
				for (int i = 0; i < (useSplitAngle ? 5 : 4); i++)
				{
					// No plane intersection, quickly discard
					if (!ray.Intersects(plane[i], distance))
						continue;

					// Process plane intersection only if distance is smaller
					// than already found minimum
					if (distance < closestDistance[p])
					{
						closestDistance[p] = distance;
						closestPlane[p] = plane[i];

						// Store according rotation.
						// For block edges (cases 0-3), return ordinary normal values.
						// For split angle (case 4), return axis perpendicular to split angle (hence + ANGLE(90.0f)) and dependent on
						// origin sector plane, which determines the direction of edge normal.

						if (i == 4)
						{
							auto usedSectorPlane = useCeilingLedge ? block->SectorPlaneCeiling(eX, eZ) : block->SectorPlane(eX, eZ);
							result[p] = FROM_RAD(splitAngle) + ANGLE(usedSectorPlane * 180.0f) + ANGLE(90.0f);
						}
						else
						{
							auto normal = closestPlane[p].Normal();
							result[p] = FROM_RAD(atan2(normal.x, normal.z)) + ANGLE(180.0f);
						}
					}
				}
			}
		}

		// Compare all 3 probe results and prioritize resulting angle based on
		// angle occurence. This approach is needed to filter out false detections
		// on the near-zero thickness edges of diagonal geometry which probes tend to tunnel through.

		std::set<short> angles;
		for (int p = 0; p < 3; p++)
		{
			// Prioritize ledge angle which was twice recognized
			if (!angles.insert(result[p]).second)
			{
				// Find existing angle in results
				int firstEqualAngle;
				for (firstEqualAngle = 0; firstEqualAngle < 3; firstEqualAngle++)
				{
					if (result[firstEqualAngle] == result[p])
						break;
					else if (firstEqualAngle == 2)
						firstEqualAngle = 0; // No equal angles, use center one
				}
				
				// Remember distance to the closest plane with same angle (it happens sometimes with bridges)
				float dist1 = FLT_MAX;
				float dist2 = FLT_MAX;
				auto r1 = originRay.Intersects(closestPlane[p], dist1);
				auto r2 = originRay.Intersects(closestPlane[firstEqualAngle], dist2);

				finalDistance[h] = (dist1 > dist2 && r2) ? dist2 : (r1 ? dist1 : dist2);
				finalResult[h] = result[p];
				break;
			}
		}

		// Store first result in case all 3 results are different (no priority) or prioritized result if long-distance misfire occured
		if (finalDistance[h] == FLT_MAX || finalDistance[h] > WALL_SIZE / 2)
		{
			finalDistance[h] = closestDistance[0];
			finalResult[h] = result[0];
		}
	}

	// Return upper probe result in case it returned lower distance or has hit a bridge.
	auto usedProbe = ((finalDistance[0] < finalDistance[1]) || hitBridge) ? 0 : 1;
	distance = finalDistance[usedProbe] - (coll->Setup.Radius - frontalOffset);
	return finalResult[usedProbe];
}

FloorInfo* GetFloor(int x, int y, int z, short* roomNumber)
{
	const auto location = GetRoom(ROOM_VECTOR{ *roomNumber, y }, x, y, z);
	*roomNumber = location.roomNumber;
	return &GetFloor(*roomNumber, x, z);
}

int GetFloorHeight(FloorInfo* floor, int x, int y, int z)
{
	return GetFloorHeight(ROOM_VECTOR{ floor->Room, y }, x, z).value_or(NO_HEIGHT);
}

int GetCeiling(FloorInfo* floor, int x, int y, int z)
{
	return GetCeilingHeight(ROOM_VECTOR{ floor->Room, y }, x, z).value_or(NO_HEIGHT);
}

int GetDistanceToFloor(int itemNumber, bool precise)
{
	auto* item = &g_Level.Items[itemNumber];

	auto probe = GetCollision(item);

	// HACK: Remove item from bridge objects temporarily.
	probe.Block->RemoveItem(itemNumber);
	auto height = GetFloorHeight(probe.Block, item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z);
	probe.Block->AddItem(itemNumber);

	auto bounds = GetBoundsAccurate(item);
	int minHeight = precise ? bounds->Y2 : 0;

	return (minHeight + item->Pose.Position.y - height);
}

void AlterFloorHeight(ItemInfo* item, int height)
{
	if (abs(height))
	{
		if (height >= 0)
			height++;
		else
			height--;
	}

	short roomNumber = item->RoomNumber;
	FloorInfo* floor = GetFloor(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, &roomNumber);
	FloorInfo* ceiling = GetFloor(item->Pose.Position.x, height + item->Pose.Position.y - SECTOR(1), item->Pose.Position.z, &roomNumber);

	floor->FloorCollision.Planes[0].z += height;
	floor->FloorCollision.Planes[1].z += height;

	auto* box = &g_Level.Boxes[floor->Box];
	if (box->flags & BLOCKABLE)
	{
		if (height >= 0)
			box->flags &= ~BLOCKED;
		else
			box->flags |= BLOCKED;
	}
}

int GetWaterSurface(int x, int y, int z, short roomNumber)
{
	auto* room = &g_Level.Rooms[roomNumber];
	FloorInfo* floor = GetSector(room, x - room->x, z - room->z);

	if (TestEnvironment(ENV_FLAG_WATER, room))
	{
		while (floor->RoomAbove(x, y, z).value_or(NO_ROOM) != NO_ROOM)
		{
			room = &g_Level.Rooms[floor->RoomAbove(x, y, z).value_or(floor->Room)];
			if (!TestEnvironment(ENV_FLAG_WATER, room))
				return (floor->CeilingHeight(x, z));

			floor = GetSector(room, x - room->x, z - room->z);
		}

		return NO_HEIGHT;
	}
	else
	{
		while (floor->RoomBelow(x, y, z).value_or(NO_ROOM) != NO_ROOM)
		{
			room = &g_Level.Rooms[floor->RoomBelow(x, y, z).value_or(floor->Room)];
			if (TestEnvironment(ENV_FLAG_WATER, room))
				return (floor->FloorHeight(x, z));

			floor = GetSector(room, x - room->x, z - room->z);
		}
	}

	return NO_HEIGHT;
}

int GetWaterSurface(ItemInfo* item)
{
	return GetWaterSurface(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, item->RoomNumber);
}

int GetWaterDepth(int x, int y, int z, short roomNumber)
{
	FloorInfo* floor;
	auto* room = &g_Level.Rooms[roomNumber];

	short roomIndex = NO_ROOM;
	do
	{
		int zFloor = (z - room->z) / SECTOR(1);
		int xFloor = (x - room->x) / SECTOR(1);

		if (zFloor <= 0)
		{
			zFloor = 0;
			if (xFloor < 1)
				xFloor = 1;
			else if (xFloor > room->xSize - 2)
				xFloor = room->xSize - 2;
		}
		else if (zFloor >= room->zSize - 1)
		{
			zFloor = room->zSize - 1;
			if (xFloor < 1)
				xFloor = 1;
			else if (xFloor > room->xSize - 2)
				xFloor = room->xSize - 2;
		}
		else if (xFloor < 0)
			xFloor = 0;
		else if (xFloor >= room->xSize)
			xFloor = room->xSize - 1;

		floor = &room->floor[zFloor + xFloor * room->zSize];
		roomIndex = floor->WallPortal;
		if (roomIndex != NO_ROOM)
		{
			roomNumber = roomIndex;
			room = &g_Level.Rooms[roomIndex];
		}
	}
	while (roomIndex != NO_ROOM);

	if (TestEnvironment(ENV_FLAG_WATER, room) ||
		TestEnvironment(ENV_FLAG_SWAMP, room))
	{
		while (floor->RoomAbove(x, y, z).value_or(NO_ROOM) != NO_ROOM)
		{
			room = &g_Level.Rooms[floor->RoomAbove(x, y, z).value_or(floor->Room)];

			if (!TestEnvironment(ENV_FLAG_WATER, room) &&
				!TestEnvironment(ENV_FLAG_SWAMP, room))
			{
				int waterHeight = floor->CeilingHeight(x, z);
				int floorHeight = GetCollision(floor, x, y, z).BottomBlock->FloorHeight(x, z);
				return (floorHeight - waterHeight);
			}

			floor = GetSector(room, x - room->x, z - room->z);
		}

		return DEEP_WATER;
	}
	else
	{
		while (floor->RoomBelow(x, y, z).value_or(NO_ROOM) != NO_ROOM)
		{
			room = &g_Level.Rooms[floor->RoomBelow(x, y, z).value_or(floor->Room)];

			if (TestEnvironment(ENV_FLAG_WATER, room) ||
				TestEnvironment(ENV_FLAG_SWAMP, room))
			{
				int waterHeight = floor->FloorHeight(x, z);
				floor = GetFloor(x, y, z, &roomNumber);
				return (GetFloorHeight(floor, x, y, z) - waterHeight);
			}

			floor = GetSector(room, x - room->x, z - room->z);
		}

		return NO_HEIGHT;
	}
}

int GetWaterDepth(ItemInfo* item)
{
	return GetWaterDepth(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, item->RoomNumber);
}

int GetWaterHeight(int x, int y, int z, short roomNumber)
{
	auto* room = &g_Level.Rooms[roomNumber];
	FloorInfo* floor;

	short adjoiningRoom = NO_ROOM;
	do
	{
		int xBlock = (x - room->x) / SECTOR(1);
		int zBlock = (z - room->z) / SECTOR(1);

		if (zBlock <= 0)
		{
			zBlock = 0;
			if (xBlock < 1)
				xBlock = 1;
			else if (xBlock > room->xSize - 2)
				xBlock = room->xSize - 2;
		}
		else if (zBlock >= room->zSize - 1)
		{
			zBlock = room->zSize - 1;
			if (xBlock < 1)
				xBlock = 1;
			else if (xBlock > room->xSize - 2)
				xBlock = room->xSize - 2;
		}
		else if (xBlock < 0)
			xBlock = 0;
		else if (xBlock >= room->xSize)
			xBlock = room->xSize - 1;

		floor = &room->floor[zBlock + xBlock * room->zSize];
		adjoiningRoom = floor->WallPortal;

		if (adjoiningRoom != NO_ROOM)
		{
			roomNumber = adjoiningRoom;
			room = &g_Level.Rooms[adjoiningRoom];
		}
	} while (adjoiningRoom != NO_ROOM);

	if (floor->IsWall(x, z))
		return NO_HEIGHT;

	if (TestEnvironment(ENV_FLAG_WATER, room) ||
		TestEnvironment(ENV_FLAG_SWAMP, room))
	{
		while (floor->RoomAbove(x, y, z).value_or(NO_ROOM) != NO_ROOM)
		{
			auto* room = &g_Level.Rooms[floor->RoomAbove(x, y, z).value_or(floor->Room)];

			if (!TestEnvironment(ENV_FLAG_WATER, room) &&
				!TestEnvironment(ENV_FLAG_SWAMP, room))
				break;

			floor = GetSector(room, x - room->x, z - room->z);
		}

		return GetCollision(floor, x, y, z).Block->CeilingHeight(x, y, z);
	}
	else if (floor->RoomBelow(x, y, z).value_or(NO_ROOM) != NO_ROOM)
	{
		while (floor->RoomBelow(x, y, z).value_or(NO_ROOM) != NO_ROOM)
		{
			auto* room2 = &g_Level.Rooms[floor->RoomBelow(x, y, z).value_or(floor->Room)];

			if (TestEnvironment(ENV_FLAG_WATER, room2) ||
				TestEnvironment(ENV_FLAG_SWAMP, room2))
				break;

			floor = GetSector(room2, x - room2->x, z - room2->z);
		}

		return GetCollision(floor, x, y, z).Block->FloorHeight(x, y, z);
	}

	return NO_HEIGHT;
}

int GetWaterHeight(ItemInfo* item)
{
	return GetWaterHeight(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, item->RoomNumber);
}

short GetSurfaceSteepnessAngle(float xTilt, float zTilt)
{
	short stepAngleIncrement = ANGLE(45.0f) / 4;
	return (short)sqrt(pow(xTilt * stepAngleIncrement, 2) + pow(zTilt * stepAngleIncrement, 2));
}

short GetSurfaceAspectAngle(float xTilt, float zTilt)
{
	return (short)phd_atan(-zTilt, -xTilt);
}

bool TestEnvironment(RoomEnvFlags environmentType, int x, int y, int z, int roomNumber)
{
	return TestEnvironment(environmentType, GetCollision(x, y, z, roomNumber).RoomNumber);
}

bool TestEnvironment(RoomEnvFlags environmentType, ItemInfo* item)
{
	return TestEnvironment(environmentType, item->RoomNumber);
}

bool TestEnvironment(RoomEnvFlags environmentType, int roomNumber)
{
	return TestEnvironment(environmentType, &g_Level.Rooms[roomNumber]);
}

bool TestEnvironment(RoomEnvFlags environmentType, ROOM_INFO* room)
{
	return TestEnvironmentFlags(environmentType, room->flags);
}

bool TestEnvironmentFlags(RoomEnvFlags environmentType, int flags)
{
	return ((flags & environmentType) == environmentType);
}
