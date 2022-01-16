#include "framework.h"
#include "Game/collision/collide_room.h"

#include "Game/control/los.h"
#include "Game/collision/collide_item.h"
#include "Game/animation.h"
#include "Game/Lara/lara.h"
#include "Game/items.h"
#include "Game/room.h"
#include "Sound/sound.h"
#include "Specific/trmath.h"
#include "Renderer/Renderer11.h"

using std::vector;
using namespace TEN::Floordata;
using namespace TEN::Renderer;

void ShiftItem(ITEM_INFO* item, COLL_INFO* coll)
{
	item->pos.xPos += coll->Shift.x;
	item->pos.yPos += coll->Shift.y;
	item->pos.zPos += coll->Shift.z;
	coll->Shift.x = 0;
	coll->Shift.y = 0;
	coll->Shift.z = 0;
}

void MoveItem(ITEM_INFO* item, short angle, int x, int y)
{
	if (!x && !y)
		return;

	if (x != 0)
	{
		auto s = phd_sin(angle);
		auto c = phd_cos(angle);
		item->pos.xPos += round(x * s);
		item->pos.zPos += round(x * c);
	}

	if (y != 0)
	{

		auto s = phd_sin(angle + ANGLE(90));
		auto c = phd_cos(angle + ANGLE(90));
		item->pos.xPos += round(y * s);
		item->pos.zPos += round(y * c);
	}
}

void SnapItemToLedge(ITEM_INFO* item, COLL_INFO* coll, float offsetMultiplier)
{
	item->pos.xRot = 0;
	item->pos.yRot = coll->NearestLedgeAngle;
	item->pos.zRot = 0;
	item->pos.xPos += round(phd_sin(coll->NearestLedgeAngle) * (coll->NearestLedgeDistance + (coll->Setup.Radius * offsetMultiplier)));
	item->pos.zPos += round(phd_cos(coll->NearestLedgeAngle) * (coll->NearestLedgeDistance + (coll->Setup.Radius * offsetMultiplier)));
}

void SnapItemToLedge(ITEM_INFO* item, COLL_INFO* coll, short angle, float offsetMultiplier)
{
	auto backup = coll->Setup.ForwardAngle;
	coll->Setup.ForwardAngle = angle;

	float dist;
	auto ang = GetNearestLedgeAngle(item, coll, dist);

	coll->Setup.ForwardAngle = backup;

	item->pos.xRot = 0;
	item->pos.yRot = ang;
	item->pos.zRot = 0;
	item->pos.xPos += round(phd_sin(ang) * (dist + (coll->Setup.Radius * offsetMultiplier)));
	item->pos.zPos += round(phd_cos(ang) * (dist + (coll->Setup.Radius * offsetMultiplier)));
}

void SnapItemToGrid(ITEM_INFO* item, COLL_INFO* coll)
{
	SnapItemToLedge(item, coll);

	int dir = (unsigned short)(item->pos.yRot + ANGLE(45)) / ANGLE(90);

	switch (dir)
	{
	case NORTH:
		item->pos.zPos = (item->pos.zPos | (WALL_SIZE - 1)) - coll->Setup.Radius;
		break;
	case EAST:
		item->pos.xPos = (item->pos.xPos | (WALL_SIZE - 1)) - coll->Setup.Radius;
		break;
	case SOUTH:
		item->pos.zPos = (item->pos.zPos & ~(WALL_SIZE - 1)) + coll->Setup.Radius;
		break;
	case WEST:
		item->pos.xPos = (item->pos.xPos & ~(WALL_SIZE - 1)) + coll->Setup.Radius;
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
		return ((WALL_SIZE + 1) - (x & (WALL_SIZE - 1)));
}

// Overload of GetCollisionResult which can be used to probe collision parameters
// from a given item.

COLL_RESULT GetCollisionResult(ITEM_INFO* item, short angle, int dist, int height, int side)
{
	auto x = item->pos.xPos + (phd_sin(angle) * dist) + (phd_cos(angle) * side);
	auto y = item->pos.yPos + height;
	auto z = item->pos.zPos + (phd_cos(angle) * dist) + (phd_sin(angle) * -side);

	return GetCollisionResult(x, y, z, GetRoom(item->location, item->pos.xPos, y, item->pos.zPos).roomNumber);
}

// A handy overload of GetCollisionResult which can be used to quickly get collision parameters
// such as floor height under specific item.

COLL_RESULT GetCollisionResult(ITEM_INFO* item)
{
	auto room = item->roomNumber;
	auto floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &room);
	auto result = GetCollisionResult(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);

	result.RoomNumber = room;
	return result;
}

// This variation of GetCollisionResult is an universal wrapper to be used across whole
// collisional code to replace "holy trinity" of roomNumber-GetFloor-GetFloorHeight operations.
// The advantage of this wrapper is that it does NOT modify incoming roomNumber parameter,
// instead putting modified one returned by GetFloor into return COLL_RESULT structure.
// This way, function never modifies any external variables.

COLL_RESULT GetCollisionResult(int x, int y, int z, short roomNumber)
{
	auto room = roomNumber;
	auto floor = GetFloor(x, y, z, &room);
	auto result = GetCollisionResult(floor, x, y, z);
	
	result.RoomNumber = room;
	return result;
}

// GetCollisionResult is a reworked legacy GetFloorHeight function, which does not
// write any data into globals, but instead into special COLL_RESULT struct.
// Additionally, it writes ceiling height for same coordinates, so this function
// may be reused instead both GetFloorHeight and GetCeilingHeight calls to increase
// readability.

COLL_RESULT GetCollisionResult(FLOOR_INFO* floor, int x, int y, int z)
{
	COLL_RESULT result = {};

	// Return provided block into result as itself.
	result.Block = floor;

	// Floor and ceiling heights are directly borrowed from new floordata.
	result.Position.Floor = GetFloorHeight(ROOM_VECTOR{ floor->Room, y }, x, z).value_or(NO_HEIGHT);
	result.Position.Ceiling = GetCeilingHeight(ROOM_VECTOR{ floor->Room, y }, x, z).value_or(NO_HEIGHT);

	// Probe bottom block through portals.
	while (floor->RoomBelow(x, y, z).value_or(NO_ROOM) != NO_ROOM)
	{
		auto r = &g_Level.Rooms[floor->RoomBelow(x, y, z).value_or(floor->Room)];
		floor = GetSector(r, x - r->x, z - r->z);
	}

	// Return probed bottom block into result.
	result.BottomBlock = floor;

	// Get tilts from new floordata.
	auto tilts = floor->TiltXZ(x, z);
	result.TiltX = tilts.first;
	result.TiltZ = tilts.second;

	// Split, bridge and slope data
	result.Position.DiagonalStep = floor->FloorIsDiagonalStep();
	result.Position.SplitAngle = floor->FloorCollision.SplitAngle;
	result.Position.Bridge = result.BottomBlock->InsideBridge(x, result.Position.Floor, z, true, false);
	result.Position.Slope = (result.Position.Bridge < 0) && ((abs(tilts.first)) > 2 || (abs(tilts.second)) > 2);

	// TODO: check if we need to keep here this slope vs. bridge check from legacy GetTiltType
	if ((y + CLICK(2)) < (floor->FloorHeight(x, z)))
		result.TiltZ = result.TiltX = 0;

	return result;
}

void GetCollisionInfo(COLL_INFO* coll, ITEM_INFO* item, bool resetRoom)
{
	GetCollisionInfo(coll, item, PHD_VECTOR(), resetRoom);
}

void GetCollisionInfo(COLL_INFO* coll, ITEM_INFO* item, PHD_VECTOR offset, bool resetRoom)
{
	// Player collision has several more precise checks for bridge collisions.
	// Therefore, we should differentiate these code paths.
	bool playerCollision = item->data.is<LaraInfo*>();

	// Reset out collision parameters.
	coll->CollisionType = CT_NONE;
	coll->Shift.x = 0;
	coll->Shift.y = 0;
	coll->Shift.z = 0;

	// Offset base probe position by provided offset, if any.
	int xPos = item->pos.xPos + offset.x;
	int yPos = item->pos.yPos + offset.y;
	int zPos = item->pos.zPos + offset.z;

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
	switch (coll->Setup.Mode == COLL_PROBE_MODE::QUADRANTS ? quadrant : -1)
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
		xleft  = (xfront * (coll->Setup.Mode == COLL_PROBE_MODE::FREE_FORWARD ? 0.5f : 1.0f)) + phd_sin(coll->Setup.ForwardAngle - ANGLE(90)) * coll->Setup.Radius;
		zleft  = (zfront * (coll->Setup.Mode == COLL_PROBE_MODE::FREE_FORWARD ? 0.5f : 1.0f)) + phd_cos(coll->Setup.ForwardAngle - ANGLE(90)) * coll->Setup.Radius;
		xright = (xfront * (coll->Setup.Mode == COLL_PROBE_MODE::FREE_FORWARD ? 0.5f : 1.0f)) + phd_sin(coll->Setup.ForwardAngle + ANGLE(90)) * coll->Setup.Radius;
		zright = (zfront * (coll->Setup.Mode == COLL_PROBE_MODE::FREE_FORWARD ? 0.5f : 1.0f)) + phd_cos(coll->Setup.ForwardAngle + ANGLE(90)) * coll->Setup.Radius;
		break;
	}

	// Define generic variables used for later object-specific position test shifts.
	ROOM_VECTOR tfLocation{}, tcLocation{}, lrfLocation{}, lrcLocation{};
	int height, ceiling;

	// Parameter definition ends here, now process to actual collision tests...
	
	// TEST 1: TILT AND NEAREST LEDGE CALCULATION

	auto collResult = GetCollisionResult(x, item->pos.yPos, z, item->roomNumber);
	coll->TiltX = collResult.TiltX;
	coll->TiltZ = collResult.TiltZ;
	coll->NearestLedgeAngle = GetNearestLedgeAngle(item, coll, coll->NearestLedgeDistance);

	// Debug angle and distance
	// g_Renderer.printDebugMessage("Nearest angle: %d", coll->NearestLedgeAngle);
	// g_Renderer.printDebugMessage("Nearest dist:  %f", coll->NearestLedgeDistance);
	
	// TEST 2: CENTERPOINT PROBE

	collResult = GetCollisionResult(x, y, z, item->roomNumber);
	auto topRoomNumber = collResult.RoomNumber; // Keep top room number as we need it to re-probe from origin room

	if (playerCollision)
	{
		tfLocation = GetRoom(item->location, x, y, z);
		height = GetFloorHeight(tfLocation, x, z).value_or(NO_HEIGHT);

		tcLocation = GetRoom(item->location, x, y - item->fallspeed, z);
		ceiling = GetCeilingHeight(tcLocation, x, z).value_or(NO_HEIGHT);
	}
	else
	{
		height = collResult.Position.Floor;
		ceiling = GetCeiling(collResult.Block, x, y - item->fallspeed, z);
	}
	if (height  != NO_HEIGHT) height -= (playerCollision ? yPos : y);
	if (ceiling != NO_HEIGHT) ceiling -= y;

	coll->Middle = collResult.Position;
	coll->Middle.Floor = height;
	coll->Middle.Ceiling = ceiling;

	// TEST 3: FRONTAL PROBE

	x = xfront + xPos;
	z = zfront + zPos;

	g_Renderer.addDebugSphere(Vector3(x, y, z), 32, Vector4(1, 0, 0, 1), RENDERER_DEBUG_PAGE::LOGIC_STATS);

	collResult = GetCollisionResult(x, y, z, topRoomNumber);

	if (playerCollision)
	{
		if (resetRoom)
		{
			tfLocation = item->location;
			tcLocation = item->location;
			topRoomNumber = item->roomNumber;
		}

		tfLocation = GetRoom(tfLocation, x, y, z);
		height = GetFloorHeight(tfLocation, x, z).value_or(NO_HEIGHT);

		tcLocation = GetRoom(tcLocation, x, y - item->fallspeed, z);
		ceiling = GetCeilingHeight(tcLocation, x, z).value_or(NO_HEIGHT);
	}
	else
	{
		height = collResult.Position.Floor;
		ceiling = GetCeiling(collResult.Block, x, y - item->fallspeed, z);
	}
	if (height  != NO_HEIGHT) height -= (playerCollision ? yPos : y);
	if (ceiling != NO_HEIGHT) ceiling -= y;

	coll->Front = collResult.Position;
	coll->Front.Floor = height;
	coll->Front.Ceiling = ceiling;

	collResult = GetCollisionResult(x + xfront, y, z + zfront, topRoomNumber);

	if (playerCollision)
	{
		tfLocation = GetRoom(tfLocation, x + xfront, y, z + zfront);
		height = GetFloorHeight(tfLocation, x + xfront, z + zfront).value_or(NO_HEIGHT);
	}
	else
	{
		height = collResult.Position.Floor;
	}
	if (height != NO_HEIGHT) height -= (playerCollision ? yPos : y);

	if (coll->Setup.SlopesAreWalls && 
		coll->Front.Slope && 
		coll->Front.Floor < coll->Middle.Floor && 
		coll->Front.Floor < 0 &&
		height < coll->Front.Floor)
	{
		coll->Front.Floor = MAX_HEIGHT;
	}
	else if (coll->Setup.SlopesArePits && 
			 coll->Front.Slope && 
			 coll->Front.Floor > coll->Middle.Floor)
	{
		coll->Front.Floor = STOP_SIZE;
	}
	else if (coll->Setup.DeathFlagIsPit && 
			 coll->MiddleLeft.Floor >= CLICK(0.5f) &&
			 collResult.BottomBlock->Flags.Death)
	{
		coll->Front.Floor = STOP_SIZE;
	}
	else if (coll->Setup.NoMonkeyFlagIsWall &&
			!collResult.BottomBlock->Flags.Monkeyswing)
	{
		coll->Front.Floor = MAX_HEIGHT;
	}

	// TEST 4: MIDDLE-LEFT PROBE

	x = xPos + xleft;
	z = zPos + zleft;

	g_Renderer.addDebugSphere(Vector3(x, y, z), 32, Vector4(0, 0, 1, 1), RENDERER_DEBUG_PAGE::LOGIC_STATS);

	collResult = GetCollisionResult(x, y, z, item->roomNumber);

	if (playerCollision)
	{
		lrfLocation = GetRoom(item->location, x, y, z);
		height = GetFloorHeight(lrfLocation, x, z).value_or(NO_HEIGHT);

		lrcLocation = GetRoom(item->location, x, y - item->fallspeed, z);
		ceiling = GetCeilingHeight(lrcLocation, x, z).value_or(NO_HEIGHT);
	}
	else
	{
		height = collResult.Position.Floor;
		ceiling = GetCeiling(collResult.Block, x, y - item->fallspeed, z);
	}
	if (height  != NO_HEIGHT) height -= (playerCollision ? yPos : y);
	if (ceiling != NO_HEIGHT) ceiling -= y;

	coll->MiddleLeft = collResult.Position;
	coll->MiddleLeft.Floor = height;
	coll->MiddleLeft.Ceiling = ceiling;

	if (coll->Setup.SlopesAreWalls && 
		coll->MiddleLeft.Slope && 
		coll->MiddleLeft.Floor < 0)
	{
		coll->MiddleLeft.Floor = MAX_HEIGHT;
	}
	else if (coll->Setup.SlopesArePits && 
			 coll->MiddleLeft.Slope && 
			 coll->MiddleLeft.Floor > 0)
	{
		coll->MiddleLeft.Floor = STOP_SIZE;
	}
	else if (coll->Setup.DeathFlagIsPit && 
			 coll->MiddleLeft.Floor >= CLICK(0.5f) &&
			 collResult.BottomBlock->Flags.Death)
	{
		coll->MiddleLeft.Floor = STOP_SIZE;
	}
	else if (coll->Setup.NoMonkeyFlagIsWall &&
			!collResult.BottomBlock->Flags.Monkeyswing)
	{
		coll->MiddleLeft.Floor = MAX_HEIGHT;
	}

	// TEST 5: FRONT-LEFT PROBE

	collResult = GetCollisionResult(x, y, z, topRoomNumber); // We use plain x/z values here, proposed by Choco

	if (playerCollision)
	{
		tfLocation = GetRoom(tfLocation, x, y, z);
		height = GetFloorHeight(tfLocation, x, z).value_or(NO_HEIGHT);

		tcLocation = GetRoom(tcLocation, x, y - item->fallspeed, z);
		ceiling = GetCeilingHeight(tcLocation, x, z).value_or(NO_HEIGHT);
	}
	else
	{
		height = collResult.Position.Floor;
		ceiling = GetCeiling(collResult.Block, x, y - item->fallspeed, z);
	}
	if (height  != NO_HEIGHT) height -= (playerCollision ? yPos : y);
	if (ceiling != NO_HEIGHT) ceiling -= y;

	coll->FrontLeft = collResult.Position;
	coll->FrontLeft.Floor = height;
	coll->FrontLeft.Ceiling = ceiling;

	if (coll->Setup.SlopesAreWalls && 
		coll->FrontLeft.Slope && 
		coll->FrontLeft.Floor < 0)
	{
		coll->FrontLeft.Floor = MAX_HEIGHT;
	}
	else if (coll->Setup.SlopesArePits && 
			 coll->FrontLeft.Slope && 
			 coll->FrontLeft.Floor > 0)
	{
		coll->FrontLeft.Floor = STOP_SIZE;
	}
	else if (coll->Setup.DeathFlagIsPit && 
			 coll->MiddleLeft.Floor >= CLICK(0.5f) &&
			 collResult.BottomBlock->Flags.Death)
	{
		coll->FrontLeft.Floor = STOP_SIZE;
	}
	else if (coll->Setup.NoMonkeyFlagIsWall &&
			!collResult.BottomBlock->Flags.Monkeyswing)
	{
		coll->FrontLeft.Floor = MAX_HEIGHT;
	}

	// TEST 6: MIDDLE-RIGHT PROBE

	x = xPos + xright;
	z = zPos + zright;

	g_Renderer.addDebugSphere(Vector3(x, y, z), 32, Vector4(0, 1, 0, 1), RENDERER_DEBUG_PAGE::LOGIC_STATS);

	collResult = GetCollisionResult(x, y, z, item->roomNumber);

	if (playerCollision)
	{
		lrfLocation = GetRoom(item->location, x, y, z);
		height = GetFloorHeight(lrfLocation, x, z).value_or(NO_HEIGHT);

		lrcLocation = GetRoom(item->location, x, y - item->fallspeed, z);
		ceiling = GetCeilingHeight(lrcLocation, x, z).value_or(NO_HEIGHT);
	}
	else
	{
		height = collResult.Position.Floor;
		ceiling = GetCeiling(collResult.Block, x, y - item->fallspeed, z);
	}
	if (height  != NO_HEIGHT) height -= (playerCollision ? yPos : y);
	if (ceiling != NO_HEIGHT) ceiling -= y;

	coll->MiddleRight = collResult.Position;
	coll->MiddleRight.Floor = height;
	coll->MiddleRight.Ceiling = ceiling;

	if (coll->Setup.SlopesAreWalls && 
		coll->MiddleRight.Slope && 
		coll->MiddleRight.Floor < 0)
	{
		coll->MiddleRight.Floor = MAX_HEIGHT;
	}
	else if (coll->Setup.SlopesArePits && 
			 coll->MiddleRight.Slope && 
			 coll->MiddleRight.Floor > 0)
	{
		coll->MiddleRight.Floor = STOP_SIZE;
	}
	else if (coll->Setup.DeathFlagIsPit && 
			 coll->MiddleLeft.Floor >= CLICK(0.5f) &&
			 collResult.BottomBlock->Flags.Death)
	{
		coll->MiddleRight.Floor = STOP_SIZE;
	}
	else if (coll->Setup.NoMonkeyFlagIsWall &&
			!collResult.BottomBlock->Flags.Monkeyswing)
	{
		coll->MiddleRight.Floor = MAX_HEIGHT;
	}

	// TEST 7: FRONT-RIGHT PROBE

	collResult = GetCollisionResult(x, y, z, topRoomNumber);

	if (playerCollision)
	{
		tfLocation = GetRoom(tfLocation, x, y, z);
		height = GetFloorHeight(tfLocation, x, z).value_or(NO_HEIGHT);

		tcLocation = GetRoom(tcLocation, x, y - item->fallspeed, z);
		ceiling = GetCeilingHeight(tcLocation, x, z).value_or(NO_HEIGHT);
	}
	else
	{
		height = collResult.Position.Floor;
		ceiling = GetCeiling(collResult.Block, x, y - item->fallspeed, z);
	}
	if (height  != NO_HEIGHT) height -= (playerCollision ? yPos : y);
	if (ceiling != NO_HEIGHT) ceiling -= y;

	coll->FrontRight = collResult.Position;
	coll->FrontRight.Floor = height;
	coll->FrontRight.Ceiling = ceiling;

	if (coll->Setup.SlopesAreWalls && 
		coll->FrontRight.Slope && 
		coll->FrontRight.Floor < 0)
	{
		coll->FrontRight.Floor = MAX_HEIGHT;
	}
	else if (coll->Setup.SlopesArePits && 
			 coll->FrontRight.Slope && 
			 coll->FrontRight.Floor > 0)
	{
		coll->FrontRight.Floor = STOP_SIZE;
	}
	else if (coll->Setup.DeathFlagIsPit && 
			 coll->MiddleLeft.Floor >= CLICK(0.5f) &&
			 collResult.BottomBlock->Flags.Death)
	{
		coll->FrontRight.Floor = STOP_SIZE;
	}
	else if (coll->Setup.NoMonkeyFlagIsWall &&
			!collResult.BottomBlock->Flags.Monkeyswing)
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

	if (coll->Front.Floor > coll->Setup.BadFloorHeightDown ||
		coll->Front.Floor < coll->Setup.BadFloorHeightUp ||
		coll->Front.Ceiling > coll->Setup.BadCeilingHeightDown ||
		coll->Front.Ceiling < coll->Setup.BadCeilingHeightUp ||
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

	// TODO: Check whether HeightUp should be <= or <. @Sezz 2021.01.16
	if (coll->Front.Ceiling >= coll->Setup.BadCeilingHeightDown ||
		coll->Front.Ceiling <= coll->Setup.BadCeilingHeightUp)
	{
		coll->Shift.x = coll->Setup.OldPosition.x - xPos;
		coll->Shift.y = coll->Setup.OldPosition.y - yPos;
		coll->Shift.z = coll->Setup.OldPosition.z - zPos;
		coll->CollisionType = CT_TOP_FRONT;
		return;
	}

	if (coll->MiddleLeft.Floor > coll->Setup.BadFloorHeightDown ||
		coll->MiddleLeft.Floor < coll->Setup.BadFloorHeightUp ||
		coll->MiddleLeft.Ceiling > coll->Setup.BadCeilingHeightDown ||
		coll->MiddleLeft.Ceiling < coll->Setup.BadCeilingHeightUp ||
		coll->MiddleLeft.Floor - coll->MiddleLeft.Ceiling <= 0)
	{
		if (coll->TriangleAtLeft() && !coll->MiddleLeft.Slope)
		{
			// HACK: Force slight push-out to the left side to avoid stucking
			MoveItem(item, coll->Setup.ForwardAngle + ANGLE(8), item->speed);

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

	if (coll->MiddleRight.Floor > coll->Setup.BadFloorHeightDown ||
		coll->MiddleRight.Floor < coll->Setup.BadFloorHeightUp ||
		coll->MiddleRight.Ceiling > coll->Setup.BadCeilingHeightDown ||
		coll->MiddleRight.Ceiling < coll->Setup.BadCeilingHeightUp ||
		coll->MiddleRight.Floor - coll->MiddleRight.Ceiling <= 0)
	{
		if (coll->TriangleAtRight() && !coll->MiddleRight.Slope)
		{
			// HACK: Force slight push-out to the right side to avoid stucking
			MoveItem(item, coll->Setup.ForwardAngle - ANGLE(8), item->speed);

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
// Dont need to set a value in radiusDivide if you dont need it (radiusDivide is set to 1 by default).
// Warning: dont set it to 0 !!!!

void CalcItemToFloorRotation(ITEM_INFO* item, int radiusDivide)
{
	if (!radiusDivide)
		return;

	GAME_VECTOR pos = {};
	pos.x = item->pos.xPos;
	pos.y = item->pos.yPos;
	pos.z = item->pos.zPos;
	pos.roomNumber = item->roomNumber;

	auto bounds = GetBoundsAccurate(item);
	auto radiusX = bounds->X2;
	auto radiusZ = bounds->Z2 / radiusDivide; // Need divide in any case else it's too much !

	auto ratioXZ = radiusZ / radiusX;
	auto frontX = phd_sin(item->pos.yRot) * radiusZ;
	auto frontZ = phd_cos(item->pos.yRot) * radiusZ;
	auto leftX  = -frontZ * ratioXZ;
	auto leftZ  =  frontX * ratioXZ;
	auto rightX =  frontZ * ratioXZ;
	auto rightZ = -frontX * ratioXZ;

	auto frontHeight = GetCollisionResult(pos.x + frontX, pos.y, pos.z + frontZ, pos.roomNumber).Position.Floor;
	auto backHeight  = GetCollisionResult(pos.x - frontX, pos.y, pos.z - frontZ, pos.roomNumber).Position.Floor;
	auto leftHeight  = GetCollisionResult(pos.x + leftX,  pos.y, pos.z + leftZ,  pos.roomNumber).Position.Floor;
	auto rightHeight = GetCollisionResult(pos.x + rightX, pos.y, pos.z + rightZ, pos.roomNumber).Position.Floor;

	auto frontHDif = backHeight  - frontHeight;
	auto sideHDif  = rightHeight - leftHeight;

	// Don't align if height differences are too large
	if ((abs(frontHDif) > STEPUP_HEIGHT) || (abs(sideHDif) > STEPUP_HEIGHT))
		return;

	// NOTE: float(atan2()) is required, else warning about double !
	item->pos.xRot = ANGLE(float(atan2(frontHDif, 2 * radiusZ)) / RADIAN);
	item->pos.zRot = ANGLE(float(atan2(sideHDif, 2 * radiusX)) / RADIAN);
}

int GetQuadrant(short angle)
{
	return (unsigned short) (angle + ANGLE(45)) / ANGLE(90);
}

// Determines vertical surfaces and gets nearest ledge angle.
// Allows to eventually use unconstrained vaults and shimmying.

short GetNearestLedgeAngle(ITEM_INFO* item, COLL_INFO* coll, float& dist)
{
	// Get item bounds and current rotation
	auto bounds = GetBoundsAccurate(item);
	auto c = phd_cos(coll->Setup.ForwardAngle);
	auto s = phd_sin(coll->Setup.ForwardAngle);

	// Origin test position should be slightly in front of origin, because otherwise
	// misfire may occur near block corners for split angles.
	auto frontalOffset = coll->Setup.Radius * 0.3f;
	auto x = item->pos.xPos + frontalOffset * s;
	auto z = item->pos.zPos + frontalOffset * c;

	// Determine two Y points to test (lower and higher).
	// 1/10 headroom crop is needed to avoid possible issues with tight diagonal headrooms.
	int headroom = abs(bounds->Y2 - bounds->Y1) / 20.0f;
	int yPoints[2] = { item->pos.yPos + bounds->Y1 + headroom,
					   item->pos.yPos + bounds->Y2 - headroom };

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
				auto s2 = phd_sin(coll->Setup.ForwardAngle + (p == 1 ? ANGLE(90) : ANGLE(-90)));
				auto c2 = phd_cos(coll->Setup.ForwardAngle + (p == 1 ? ANGLE(90) : ANGLE(-90)));

				// Slightly extend width beyond coll radius to hit adjacent blocks for sure
				eX += s2 * (coll->Setup.Radius * 2);
				eZ += c2 * (coll->Setup.Radius * 2);
			}

			// Debug probe point
			// g_Renderer.addDebugSphere(Vector3(eX, y, eZ), 16, Vector4(1, 1, 0, 1), RENDERER_DEBUG_PAGE::LOGIC_STATS);

			// Determine front floor probe offset.
			// It is needed to identify if there is bridge or ceiling split in front.
			auto frontFloorProbeOffset = coll->Setup.Radius * 1.5f;
			auto ffpX = eX + frontFloorProbeOffset * s;
			auto ffpZ = eZ + frontFloorProbeOffset * c;

			// Get front floor block
			auto room = GetRoom(item->location, ffpX, y, ffpZ).roomNumber;
			auto block = GetCollisionResult(ffpX, y, ffpZ, room).Block;

			// Get front floor surface heights
			auto floorHeight   = GetFloorHeight(ROOM_VECTOR{ block->Room, y }, ffpX, ffpZ).value_or(NO_HEIGHT);
			auto ceilingHeight = GetCeilingHeight(ROOM_VECTOR{ block->Room, y }, ffpX, ffpZ).value_or(NO_HEIGHT);

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
			// g_Renderer.addDebugSphere(Vector3(fpX, y, fpZ), 16, Vector4(0, 1, 0, 1), RENDERER_DEBUG_PAGE::LOGIC_STATS);

			// Get true room number and block, based on derived height
			room = GetRoom(item->location, fpX, height, fpZ).roomNumber;
			block = GetCollisionResult(fpX, height, fpZ, room).Block;

			// We don't need actual corner heights to build planes, so just use normalized value here
			auto fY = height - 1;
			auto cY = height + 1;

			// Calculate ray
			auto mxR = Matrix::CreateFromYawPitchRoll(TO_RAD(coll->Setup.ForwardAngle), 0, 0);
			auto direction = (Matrix::CreateTranslation(Vector3::UnitZ) * mxR).Translation();
			auto ray = Ray(Vector3(eX, cY, eZ), direction);

			// Debug ray direction
			// g_Renderer.addLine3D(Vector3(eX, y, eZ), Vector3(eX, y, eZ) + direction * 256, Vector4(1, 0, 0, 1));

			// Keep origin ray to calculate true centerpoint distance to ledge later
			if (p == 0)
				originRay = Ray(Vector3(eX, cY, eZ), direction);

			if (bridge >= 0) // Surface is inside bridge
			{
				// Get and test DX item coll bounds
				auto bounds = GetBoundsAccurate(&g_Level.Items[bridge]);
				auto dxBounds = TO_DX_BBOX(g_Level.Items[bridge].pos, bounds);

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
				// g_Renderer.addDebugSphere(Vector3(round(eX / WALL_SIZE) * WALL_SIZE + 512, y, round(eZ / WALL_SIZE) * WALL_SIZE + 512), 16, Vector4(1, 1, 1, 1), RENDERER_DEBUG_PAGE::LOGIC_STATS);

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
						// For split angle (case 4), return axis perpendicular to split angle (hence + ANGLE(90)) and dependent on
						// origin sector plane, which determines the direction of edge normal.

						if (i == 4)
						{
							auto usedSectorPlane = useCeilingLedge ? block->SectorPlaneCeiling(eX, eZ) : block->SectorPlane(eX, eZ);
							result[p] = FROM_RAD(splitAngle) + ANGLE(usedSectorPlane * 180.0f) + ANGLE(90);
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
	dist = finalDistance[usedProbe] - (coll->Setup.Radius - frontalOffset);
	return finalResult[usedProbe];
}