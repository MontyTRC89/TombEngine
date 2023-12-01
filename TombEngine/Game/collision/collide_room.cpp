#include "framework.h"
#include "Game/collision/collide_room.h"

#include "Game/control/box.h"
#include "Game/control/los.h"
#include "Game/collision/collide_item.h"
#include "Game/animation.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/items.h"
#include "Game/room.h"
#include "Math/Math.h"
#include "Sound/sound.h"
#include "Renderer/Renderer11.h"

using namespace TEN::Collision::Floordata;
using namespace TEN::Math;
using namespace TEN::Renderer;

void ShiftItem(ItemInfo* item, CollisionInfo* coll)
{
	item->Pose.Position += coll->Shift.Position;
	item->Pose.Orientation += coll->Shift.Orientation;
	coll->Shift = Vector3i::Zero;
}

void AlignEntityToEdge(ItemInfo* item, CollisionInfo* coll, float radiusCoeff, bool doSnap)
{
	auto& player = GetLaraInfo(*item);

	TranslateItem(item, coll->NearestLedgeAngle, coll->NearestLedgeDistance + (coll->Setup.Radius * radiusCoeff));
	player.Context.TargetOrientation = EulerAngles(0, coll->NearestLedgeAngle, 0);

	if (doSnap)
		item->Pose.Orientation = player.Context.TargetOrientation;
}

// TODO
void SnapEntityToGrid(ItemInfo* item, CollisionInfo* coll)
{
	int direction = GetQuadrant(item->Pose.Orientation.y);
	switch (direction)
	{
	case NORTH:
		item->Pose.Position.z = (item->Pose.Position.z | WALL_MASK) - coll->Setup.Radius;
		break;

	case EAST:
		item->Pose.Position.x = (item->Pose.Position.x | WALL_MASK) - coll->Setup.Radius;
		break;

	case SOUTH:
		item->Pose.Position.z = (item->Pose.Position.z & ~WALL_MASK) + coll->Setup.Radius;
		break;

	case WEST:
		item->Pose.Position.x = (item->Pose.Position.x & ~WALL_MASK) + coll->Setup.Radius;
		break;
	}
}

int FindGridShift(int x, int z)
{
	if ((x / BLOCK(1)) == (z / BLOCK(1)))
		return 0;

	if ((z / BLOCK(1)) <= (x / BLOCK(1)))
	{
		return (-1 - (x & WALL_MASK));
	}
	else
	{
		return ((BLOCK(1) + 1) - (x & WALL_MASK));
	}
}

// Test if the axis-aligned bounding box collides with geometry at all.
bool TestItemRoomCollisionAABB(ItemInfo* item)
{
	const auto& bounds = GetBestFrame(*item).BoundingBox;
	auto box = bounds + item->Pose;
	short maxY = std::min(box.Y1, box.Y2);
	short minY = std::max(box.Y1, box.Y2);

	auto test = [item](short x, short y, short z, bool floor)
	{
		auto pointColl = GetCollision(x, y, z, item->RoomNumber).Position;
		
		if (floor)
		{
			return (y > pointColl.Floor);
		}
		else
		{
			return (y < pointColl.Ceiling);
		}
	};

	bool hasCollided =
		test(box.X1, minY, box.Z1, true) ||
		test(box.X2, minY, box.Z1, true) ||
		test(box.X1, minY, box.Z2, true) ||
		test(box.X2, minY, box.Z2, true) ||
		test(box.X1, maxY, box.Z1, false) ||
		test(box.X2, maxY, box.Z1, false) ||
		test(box.X1, maxY, box.Z2, false) ||
		test(box.X2, maxY, box.Z2, false);

	return hasCollided;
}

// Overload used to quickly get point collision parameters at a given item's position.
CollisionResult GetCollision(const ItemInfo& item)
{
	auto newRoomNumber = item.RoomNumber;
	auto floor = GetFloor(item.Pose.Position.x, item.Pose.Position.y, item.Pose.Position.z, &newRoomNumber);
	auto pointColl = GetCollision(floor, item.Pose.Position.x, item.Pose.Position.y, item.Pose.Position.z);

	pointColl.RoomNumber = newRoomNumber;
	return pointColl;
}

// Deprecated.
CollisionResult GetCollision(const ItemInfo* item)
{
	return GetCollision(*item);
}

// Overload used to probe point collision parameters from a given item's position.
CollisionResult GetCollision(const ItemInfo* item, short headingAngle, float forward, float down, float right)
{
	short tempRoomNumber = item->RoomNumber;

	// TODO: Find cleaner solution. Constructing a Location for Lara on the spot can result in a stumble when climbing onto thin platforms. -- Sezz 2022.06.14
	auto location = item->IsLara() ?
		item->Location :
		RoomVector(GetFloor(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, &tempRoomNumber)->RoomNumber, item->Pose.Position.y);

	auto point = Geometry::TranslatePoint(item->Pose.Position, headingAngle, forward, down, right);
	int adjacentRoomNumber = GetRoom(location, Vector3i(item->Pose.Position.x, point.y, item->Pose.Position.z)).RoomNumber;
	return GetCollision(point.x, point.y, point.z, adjacentRoomNumber);
}

// Overload used to probe point collision parameters from a given position.
CollisionResult GetCollision(const Vector3i& pos, int roomNumber, short headingAngle, float forward, float down, float right)
{
	short tempRoomNumber = roomNumber;
	auto location = RoomVector(GetFloor(pos.x, pos.y, pos.z, &tempRoomNumber)->RoomNumber, pos.y);

	auto point = Geometry::TranslatePoint(pos, headingAngle, forward, down, right);
	int adjacentRoomNumber = GetRoom(location, Vector3i(pos.x, point.y, pos.z)).RoomNumber;
	return GetCollision(point.x, point.y, point.z, adjacentRoomNumber);
}

CollisionResult GetCollision(const Vector3i& pos, int roomNumber, const EulerAngles& orient, float dist)
{
	auto point = Geometry::TranslatePoint(pos, orient, dist);

	short tempRoomNumber = roomNumber;
	auto location = RoomVector(GetFloor(pos.x, pos.y, pos.z, &tempRoomNumber)->RoomNumber, pos.y);
	int adjacentRoomNumber = GetRoom(location, Vector3i(pos.x, point.y, pos.z)).RoomNumber;
	return GetCollision(point.x, point.y, point.z, adjacentRoomNumber);
}

CollisionResult GetCollision(const Vector3i& pos, int roomNumber, const Vector3& dir, float dist)
{
	auto point = Geometry::TranslatePoint(pos, dir, dist);

	short tempRoomNumber = roomNumber;
	auto location = RoomVector(GetFloor(pos.x, pos.y, pos.z, &tempRoomNumber)->RoomNumber, pos.y);
	int adjacentRoomNumber = GetRoom(location, Vector3i(pos.x, point.y, pos.z)).RoomNumber;
	return GetCollision(point.x, point.y, point.z, adjacentRoomNumber);
}

// Overload used as universal wrapper across collisional code replacing
// triads of roomNumber-GetFloor()-GetFloorHeight() calls.
// Advantage is that it does NOT modify incoming roomNumber argument,
// instead storing one modified by GetFloor() within a returned CollisionResult struct.
// This way, no external variables are modified as output arguments.
CollisionResult GetCollision(const Vector3i& pos, int roomNumber)
{
	return GetCollision(pos.x, pos.y, pos.z, roomNumber);
}

// Deprecated.
CollisionResult GetCollision(int x, int y, int z, short roomNumber)
{
	short room = roomNumber;
	auto* floor = GetFloor(x, y, z, &room);
	auto pointColl = GetCollision(floor, x, y, z);

	pointColl.RoomNumber = room;
	return pointColl;
}

// NOTE: To be used only when absolutely necessary.
CollisionResult GetCollision(const GameVector& pos)
{
	return GetCollision(pos.x, pos.y, pos.z, pos.RoomNumber);
}

// A reworked legacy GetFloorHeight() function which writes data
// into a special CollisionResult struct instead of global variables.
// It writes for both floor and ceiling heights at the same coordinates, meaning it should be used
// in place of successive GetFloorHeight() and GetCeilingHeight() calls to increase readability.
CollisionResult GetCollision(FloorInfo* floor, int x, int y, int z)
{
	auto result = CollisionResult{};

	// Record coordinates.
	result.Coordinates = Vector3i(x, y, z);

	// Return provided collision block into result as itself.
	result.Block = floor;

	// Floor and ceiling heights are borrowed directly from floordata.
	result.Position.Floor = GetFloorHeight(RoomVector(floor->RoomNumber, y), x, z).value_or(NO_HEIGHT);
	result.Position.Ceiling = GetCeilingHeight(RoomVector(floor->RoomNumber, y), x, z).value_or(NO_HEIGHT);

	// Probe bottom collision block through portals.
	while (floor->GetRoomNumberBelow(Vector3i(x, y, z)).value_or(NO_ROOM) != NO_ROOM)
	{
		auto* room = &g_Level.Rooms[floor->GetRoomNumberBelow(Vector3i(x, y, z)).value_or(floor->RoomNumber)];
		floor = GetSector(room, x - room->x, z - room->z);
	}

	// Return probed bottom collision block into result.
	result.BottomBlock = floor;

	// Get surface noramls.
	result.FloorNormal = floor->GetSurfaceNormal(x, z, true);
	result.CeilingNormal = floor->GetSurfaceNormal(x, z, false);

	// Backport surface normals to tilts.
	result.FloorTilt = GetSurfaceTilt(result.FloorNormal, true).ToVector2();
	result.CeilingTilt = GetSurfaceTilt(result.CeilingNormal, false).ToVector2();

	// Split, bridge and slope data.
	result.Position.DiagonalStep = floor->IsSurfaceDiagonalStep(true);
	result.Position.SplitAngle = TO_RAD(floor->FloorSurface.SplitAngle);
	result.Position.Bridge = result.BottomBlock->GetInsideBridgeItemNumber(Vector3i(x, result.Position.Floor, z), true, false);
	result.Position.FloorSlope = result.Position.Bridge < 0 && Geometry::GetSurfaceSlopeAngle(result.FloorNormal) >=
		result.BottomBlock->GetSurfaceIllegalSlopeAngle(x, z, true);
	result.Position.CeilingSlope = Geometry::GetSurfaceSlopeAngle(result.CeilingNormal, -Vector3::UnitY) >=
		result.BottomBlock->GetSurfaceIllegalSlopeAngle(x, z, false); // TODO: Fix on bridges placed beneath ceiling slopes. @Sezz 2022.01.29

	return result;
}

void GetCollisionInfo(CollisionInfo* coll, ItemInfo* item, bool resetRoom)
{
	GetCollisionInfo(coll, item, Vector3i::Zero, resetRoom);
}

static void SetSectorAttribs(CollisionPosition& sectorAttribs, const CollisionSetup& collSetup, const CollisionResult& pointColl,
							 const Vector3i& probePos, int realRoomNumber)
{
	constexpr auto ASPECT_ANGLE_DELTA_MAX = ANGLE(90.0f);

	auto floorNormal = pointColl.FloorNormal;
	short aspectAngle = Geometry::GetSurfaceAspectAngle(floorNormal);
	short aspectAngleDelta = Geometry::GetShortestAngle(collSetup.ForwardAngle, aspectAngle);

	if (collSetup.BlockFloorSlopeUp &&
		sectorAttribs.FloorSlope &&
		sectorAttribs.Floor <= STEPUP_HEIGHT &&
		sectorAttribs.Floor >= -STEPUP_HEIGHT &&
		abs(aspectAngleDelta) >= ASPECT_ANGLE_DELTA_MAX)
	{
		sectorAttribs.Floor = MAX_HEIGHT;
	}
	else if (collSetup.BlockFloorSlopeDown &&
		sectorAttribs.FloorSlope &&
		sectorAttribs.Floor <= STEPUP_HEIGHT &&
		sectorAttribs.Floor >= -STEPUP_HEIGHT &&
		abs(aspectAngleDelta) <= ASPECT_ANGLE_DELTA_MAX)
	{
		sectorAttribs.Floor = MAX_HEIGHT;
	}
	else if (collSetup.BlockCeilingSlope &&
		sectorAttribs.CeilingSlope)
	{
		sectorAttribs.Floor = MAX_HEIGHT;
	}
	else if (collSetup.BlockDeathFloorDown &&
		sectorAttribs.Floor >= CLICK(0.5f) &&
		pointColl.BottomBlock->Flags.Death)
	{
		sectorAttribs.Floor = MAX_HEIGHT;
	}
	else if (collSetup.BlockMonkeySwingEdge)
	{
		auto pointColl = GetCollision(probePos.x, probePos.y + collSetup.Height, probePos.z, realRoomNumber);
		if (!pointColl.BottomBlock->Flags.Monkeyswing)
			sectorAttribs.Floor = MAX_HEIGHT;
	}
}

static void HandleDiagonalShift(ItemInfo& item, CollisionInfo& coll, const Vector3i& pos, CardinalDirection quadrant, bool isRightShift)
{
	// Calculate angles.
	int sign = isRightShift ? 1 : -1;
	short perpSplitAngle = (ANGLE(90.0f) * quadrant) + (ANGLE(45.0f) * sign);
	short deltaAngle = abs(Geometry::GetShortestAngle(coll.Setup.ForwardAngle, perpSplitAngle));

	// HACK: Force slight push left to avoid getting stuck.
	float alpha = 1.0f - ((float)deltaAngle / (float)ANGLE(90.0f));
	if (alpha >= 0.5f)
		TranslateItem(&item, perpSplitAngle, item.Animation.Velocity.z * alpha);

	// Set shift.
	coll.Shift.Position.x += coll.Setup.PrevPosition.x - pos.x;
	coll.Shift.Position.z += coll.Setup.PrevPosition.z - pos.z;
};

void GetCollisionInfo(CollisionInfo* coll, ItemInfo* item, const Vector3i& offset, bool resetRoom)
{
	// Player collision has several more precise checks for bridge collisions.
	// Therefore, we should differentiate these code paths.
	bool doPlayerCollision = item->IsLara();

	// Reset collision parameters.
	coll->CollisionType = CollisionType::CT_NONE;
	coll->Shift = Pose::Zero;

	// Offset base probe position by provided offset, if any.
	auto entityPos = item->Pose.Position + offset;

	// Specify base probe position, with Y position being bounds top side.
	auto probePos = Vector3i(entityPos.x, entityPos.y - coll->Setup.Height, entityPos.z);

	// Declare side probe offsets.
	int xFront, zFront, xRight, zRight, xLeft, zLeft;

	// Get nearest 90-degree snapped angle (quadrant).
	int quadrant = GetQuadrant(coll->Setup.ForwardAngle);

	// Get side probe offsets depending on quadrant.
	// If unconstrained mode is specified, don't use quadrant.
	switch ((coll->Setup.Mode == CollisionProbeMode::Quadrants) ? quadrant : -1)
	{
	case NORTH:
		xFront =  phd_sin(coll->Setup.ForwardAngle) * coll->Setup.Radius;
		zFront =  coll->Setup.Radius;
		xLeft  = -coll->Setup.Radius;
		zLeft  =  coll->Setup.Radius;
		xRight =  coll->Setup.Radius;
		zRight =  coll->Setup.Radius;
		break;

	case EAST:
		xFront =  coll->Setup.Radius;
		zFront =  phd_cos(coll->Setup.ForwardAngle) * coll->Setup.Radius;
		xLeft  =  coll->Setup.Radius;
		zLeft  =  coll->Setup.Radius;
		xRight =  coll->Setup.Radius;
		zRight = -coll->Setup.Radius;
		break;

	case SOUTH:
		xFront =  phd_sin(coll->Setup.ForwardAngle) * coll->Setup.Radius;
		zFront = -coll->Setup.Radius;
		xLeft  =  coll->Setup.Radius;
		zLeft  = -coll->Setup.Radius;
		xRight = -coll->Setup.Radius;
		zRight = -coll->Setup.Radius;
		break;

	case WEST:
		xFront = -coll->Setup.Radius;
		zFront =  phd_cos(coll->Setup.ForwardAngle) * coll->Setup.Radius;
		xLeft  = -coll->Setup.Radius;
		zLeft  = -coll->Setup.Radius;
		xRight = -coll->Setup.Radius;
		zRight =  coll->Setup.Radius;
		break;

	// No valid quadrant; get true probe offsets from object rotation.
	default: 
		xFront = phd_sin(coll->Setup.ForwardAngle) * coll->Setup.Radius;
		zFront = phd_cos(coll->Setup.ForwardAngle) * coll->Setup.Radius;
		xLeft  = (xFront * (coll->Setup.Mode == CollisionProbeMode::FreeForward ? 0.5f : 1.0f)) + phd_sin(coll->Setup.ForwardAngle - ANGLE(90.0f)) * coll->Setup.Radius;
		zLeft  = (zFront * (coll->Setup.Mode == CollisionProbeMode::FreeForward ? 0.5f : 1.0f)) + phd_cos(coll->Setup.ForwardAngle - ANGLE(90.0f)) * coll->Setup.Radius;
		xRight = (xFront * (coll->Setup.Mode == CollisionProbeMode::FreeForward ? 0.5f : 1.0f)) + phd_sin(coll->Setup.ForwardAngle + ANGLE(90.0f)) * coll->Setup.Radius;
		zRight = (zFront * (coll->Setup.Mode == CollisionProbeMode::FreeForward ? 0.5f : 1.0f)) + phd_cos(coll->Setup.ForwardAngle + ANGLE(90.0f)) * coll->Setup.Radius;
		break;
	}

	// Define generic variables used for later object-specific position test shifts.
	auto tfLocation = RoomVector();
	auto tcLocation = RoomVector();
	auto lrfLocation = RoomVector();
	auto lrcLocation = RoomVector();
	int height = 0;
	int ceiling = 0;

	// Parameter definition ends here, now process to actual collision tests...

	// HACK: when using SetPosition animcommand, item->RoomNumber does not immediately
	// update, but only at the end of control loop. This may cause bugs when Lara is
	// climbing or vaulting ledges under slopes. Using Location->roomNumber solves
	// these bugs, as it is updated immediately. But since Location->roomNumber is ONLY
	// updated for Lara, we can't use it for all objects for now. In future, we should
	// either update Location field for all objects or use this value as it is now.

	int realRoomNumber = doPlayerCollision ? item->Location.RoomNumber : item->RoomNumber;
	
	// TEST 1: TILT AND NEAREST LEDGE CALCULATION

	auto collResult = GetCollision(probePos.x, item->Pose.Position.y, probePos.z, realRoomNumber);

	coll->FloorNormal = collResult.FloorNormal;
	coll->CeilingNormal = collResult.CeilingNormal;
	coll->FloorTilt = collResult.FloorTilt;
	coll->CeilingTilt = collResult.CeilingTilt;

	// Debug angle and distance
	// g_Renderer.PrintDebugMessage("Nearest angle: %d", coll->NearestLedgeAngle);
	// g_Renderer.PrintDebugMessage("Nearest dist:  %f", coll->NearestLedgeDistance);
	
	// TEST 2: CENTERPOINT PROBE

	collResult = GetCollision(probePos.x, probePos.y, probePos.z, realRoomNumber);
	int topRoomNumber = collResult.RoomNumber; // Keep top room number as we need it to re-probe from origin room.
	
	if (doPlayerCollision)
	{
		tfLocation = GetRoom(item->Location, probePos);
		height = GetFloorHeight(tfLocation, probePos.x, probePos.z).value_or(NO_HEIGHT);

		tcLocation = GetRoom(item->Location, Vector3i(probePos.x, probePos.y - item->Animation.Velocity.y, probePos.z));
		ceiling = GetCeilingHeight(tcLocation, probePos.x, probePos.z).value_or(NO_HEIGHT);
	}
	else
	{
		height = collResult.Position.Floor;
		ceiling = GetCeiling(collResult.Block, probePos.x, probePos.y - item->Animation.Velocity.y, probePos.z);
	}

	if (height != NO_HEIGHT)
		height -= (doPlayerCollision ? entityPos.y : probePos.y);

	if (ceiling != NO_HEIGHT)
		ceiling -= probePos.y;

	coll->Middle = collResult.Position;
	coll->Middle.Floor = height;
	coll->Middle.Ceiling = ceiling;

	// Additionally calculate bridge shifts, if present.
	CollideBridgeItems(*item, *coll, collResult);

	// TEST 3: FRONTAL PROBE

	probePos.x = entityPos.x + xFront;
	probePos.z = entityPos.z + zFront;

	g_Renderer.AddDebugSphere(probePos.ToVector3(), 32, Vector4(1, 0, 0, 1), RendererDebugPage::CollisionStats);

	collResult = GetCollision(probePos.x, probePos.y, probePos.z, topRoomNumber);

	if (doPlayerCollision)
	{
		if (resetRoom)
		{
			tfLocation = item->Location;
			tcLocation = item->Location;
			topRoomNumber = realRoomNumber;
		}

		tfLocation = GetRoom(tfLocation, probePos);
		height = GetFloorHeight(tfLocation, probePos.x, probePos.z).value_or(NO_HEIGHT);

		tcLocation = GetRoom(tcLocation, Vector3i(probePos.x, probePos.y - item->Animation.Velocity.y, probePos.z));
		ceiling = GetCeilingHeight(tcLocation, probePos.x, probePos.z).value_or(NO_HEIGHT);
	}
	else
	{
		height = collResult.Position.Floor;
		ceiling = GetCeiling(collResult.Block, probePos.x, probePos.y - item->Animation.Velocity.y, probePos.z);
	}

	if (height != NO_HEIGHT)
		height -= (doPlayerCollision ? entityPos.y : probePos.y);

	if (ceiling != NO_HEIGHT)
		ceiling -= probePos.y;

	coll->Front = collResult.Position;
	coll->Front.Floor = height;
	coll->Front.Ceiling = ceiling;

	if (doPlayerCollision)
	{
		tfLocation = GetRoom(tfLocation, Vector3i(probePos.x + xFront, probePos.y, probePos.z + zFront));
		height = GetFloorHeight(tfLocation, probePos.x + xFront, probePos.z + zFront).value_or(NO_HEIGHT);
	}
	else
	{
		height = GetCollision(probePos.x + xFront, probePos.y, probePos.z + zFront, topRoomNumber).Position.Floor;
	}
	
	if (height != NO_HEIGHT)
		height -= (doPlayerCollision ? entityPos.y : probePos.y);

	SetSectorAttribs(coll->Front, coll->Setup, collResult, probePos, realRoomNumber);

	// TEST 4: MIDDLE-LEFT PROBE

	probePos.x = entityPos.x + xLeft;
	probePos.z = entityPos.z + zLeft;

	g_Renderer.AddDebugSphere(probePos.ToVector3(), 32, Vector4(0, 0, 1, 1), RendererDebugPage::CollisionStats);

	collResult = GetCollision(probePos.x, probePos.y, probePos.z, item->RoomNumber);

	if (doPlayerCollision)
	{
		lrfLocation = GetRoom(item->Location, probePos);
		height = GetFloorHeight(lrfLocation, probePos.x, probePos.z).value_or(NO_HEIGHT);

		lrcLocation = GetRoom(item->Location, Vector3i(probePos.x, probePos.y - item->Animation.Velocity.y, probePos.z));
		ceiling = GetCeilingHeight(lrcLocation, probePos.x, probePos.z).value_or(NO_HEIGHT);
	}
	else
	{
		height = collResult.Position.Floor;
		ceiling = GetCeiling(collResult.Block, probePos.x, probePos.y - item->Animation.Velocity.y, probePos.z);
	}

	if (height != NO_HEIGHT)
		height -= (doPlayerCollision ? entityPos.y : probePos.y);

	if (ceiling != NO_HEIGHT)
		ceiling -= probePos.y;

	coll->MiddleLeft = collResult.Position;
	coll->MiddleLeft.Floor = height;
	coll->MiddleLeft.Ceiling = ceiling;

	SetSectorAttribs(coll->MiddleLeft, coll->Setup, collResult, probePos, realRoomNumber);

	// TEST 5: FRONT-LEFT PROBE

	collResult = GetCollision(probePos.x, probePos.y, probePos.z, topRoomNumber); // Use plain X/Z values here as proposed by Choco.

	if (doPlayerCollision)
	{
		tfLocation = GetRoom(tfLocation, probePos);
		height = GetFloorHeight(tfLocation, probePos.x, probePos.z).value_or(NO_HEIGHT);

		tcLocation = GetRoom(tcLocation, Vector3i(probePos.x, probePos.y - item->Animation.Velocity.y, probePos.z));
		ceiling = GetCeilingHeight(tcLocation, probePos.x, probePos.z).value_or(NO_HEIGHT);
	}
	else
	{
		height = collResult.Position.Floor;
		ceiling = GetCeiling(collResult.Block, probePos.x, probePos.y - item->Animation.Velocity.y, probePos.z);
	}

	if (height != NO_HEIGHT)
		height -= (doPlayerCollision ? entityPos.y : probePos.y);

	if (ceiling != NO_HEIGHT)
		ceiling -= probePos.y;

	coll->FrontLeft = collResult.Position;
	coll->FrontLeft.Floor = height;
	coll->FrontLeft.Ceiling = ceiling;

	SetSectorAttribs(coll->FrontLeft, coll->Setup, collResult, probePos, realRoomNumber);

	// TEST 6: MIDDLE-RIGHT PROBE

	probePos.x = entityPos.x + xRight;
	probePos.z = entityPos.z + zRight;

	g_Renderer.AddDebugSphere(probePos.ToVector3(), 32, Vector4(0, 1, 0, 1), RendererDebugPage::CollisionStats);

	collResult = GetCollision(probePos.x, probePos.y, probePos.z, item->RoomNumber);

	if (doPlayerCollision)
	{
		lrfLocation = GetRoom(item->Location, probePos);
		height = GetFloorHeight(lrfLocation, probePos.x, probePos.z).value_or(NO_HEIGHT);

		lrcLocation = GetRoom(item->Location, Vector3i(probePos.x, probePos.y - item->Animation.Velocity.y, probePos.z));
		ceiling = GetCeilingHeight(lrcLocation, probePos.x, probePos.z).value_or(NO_HEIGHT);
	}
	else
	{
		height = collResult.Position.Floor;
		ceiling = GetCeiling(collResult.Block, probePos.x, probePos.y - item->Animation.Velocity.y, probePos.z);
	}

	if (height != NO_HEIGHT)
		height -= (doPlayerCollision ? entityPos.y : probePos.y);

	if (ceiling != NO_HEIGHT)
		ceiling -= probePos.y;

	coll->MiddleRight = collResult.Position;
	coll->MiddleRight.Floor = height;
	coll->MiddleRight.Ceiling = ceiling;

	SetSectorAttribs(coll->MiddleRight, coll->Setup, collResult, probePos, realRoomNumber);

	// TEST 7: FRONT-RIGHT PROBE

	collResult = GetCollision(probePos.x, probePos.y, probePos.z, topRoomNumber);

	if (doPlayerCollision)
	{
		tfLocation = GetRoom(tfLocation, probePos);
		height = GetFloorHeight(tfLocation, probePos.x, probePos.z).value_or(NO_HEIGHT);

		tcLocation = GetRoom(tcLocation, Vector3i(probePos.x, probePos.y - item->Animation.Velocity.y, probePos.z));
		ceiling = GetCeilingHeight(tcLocation, probePos.x, probePos.z).value_or(NO_HEIGHT);
	}
	else
	{
		height = collResult.Position.Floor;
		ceiling = GetCeiling(collResult.Block, probePos.x, probePos.y - item->Animation.Velocity.y, probePos.z);
	}

	if (height != NO_HEIGHT)
		height -= (doPlayerCollision ? entityPos.y : probePos.y);

	if (ceiling != NO_HEIGHT)
		ceiling -= probePos.y;

	coll->FrontRight = collResult.Position;
	coll->FrontRight.Floor = height;
	coll->FrontRight.Ceiling = ceiling;

	SetSectorAttribs(coll->FrontRight, coll->Setup, collResult, probePos, realRoomNumber);

	// TEST 8: SOLID STATIC MESHES

	CollideSolidStatics(item, coll);

	// Collision tests now end.
	// Get to calculation of collision side and shifts.

	if (coll->Middle.Floor == NO_HEIGHT)
	{
		coll->Shift.Position += coll->Setup.PrevPosition - entityPos;
		coll->CollisionType = CT_FRONT;
		return;
	}

	if (coll->Middle.Floor - coll->Middle.Ceiling <= 0)
	{
		coll->Shift.Position += coll->Setup.PrevPosition - entityPos;
		coll->CollisionType = CT_CLAMP;
		return;
	}

	if (coll->Middle.Ceiling >= 0)
	{
		coll->Shift.Position.y += coll->Middle.Ceiling;
		coll->CollisionType = CT_TOP;
	}

	// Shift away from front wall.
	if (coll->Front.Floor > coll->Setup.LowerFloorBound ||
		coll->Front.Floor < coll->Setup.UpperFloorBound ||
		coll->Front.Ceiling > coll->Setup.LowerCeilingBound ||
		coll->Front.Ceiling < coll->Setup.UpperCeilingBound ||
		coll->Front.Floor - coll->Front.Ceiling <= 0)
	{
		if (coll->Front.HasDiagonalSplit())
		{
			coll->Shift.Position.x += coll->Setup.PrevPosition.x - entityPos.x;
			coll->Shift.Position.z += coll->Setup.PrevPosition.z - entityPos.z;
		}
		else
		{
			switch (quadrant)
			{
			case NORTH:
			case SOUTH:
				coll->Shift.Position.x += coll->Setup.PrevPosition.x - entityPos.x;
				coll->Shift.Position.z += FindGridShift(entityPos.z + zFront, entityPos.z);
				break;

			case EAST:
			case WEST:
				coll->Shift.Position.x += FindGridShift(entityPos.x + xFront, entityPos.x);
				coll->Shift.Position.z += coll->Setup.PrevPosition.z - entityPos.z;
				break;
			}
		}

		coll->CollisionType = (coll->CollisionType == CT_TOP) ? CT_TOP_FRONT : CT_FRONT;
		return;
	}

	if (coll->Front.Ceiling > coll->Setup.LowerCeilingBound ||
		coll->Front.Ceiling < coll->Setup.UpperCeilingBound)
	{
		coll->Shift.Position += coll->Setup.PrevPosition - entityPos;
		coll->CollisionType = CT_TOP_FRONT;
		return;
	}

	// Shift away from left wall.
	if (coll->MiddleLeft.Floor > coll->Setup.LowerFloorBound ||
		coll->MiddleLeft.Floor < coll->Setup.UpperFloorBound ||
		coll->MiddleLeft.Ceiling > coll->Setup.LowerCeilingBound ||
		coll->MiddleLeft.Ceiling < coll->Setup.UpperCeilingBound ||
		coll->MiddleLeft.Floor - coll->MiddleLeft.Ceiling <= 0)
	{
		if (coll->TriangleAtLeft() && !coll->MiddleLeft.FloorSlope)
		{
			HandleDiagonalShift(*item, *coll, entityPos, (CardinalDirection)quadrant, true);
		}
		else
		{
			switch (quadrant)
			{
			case NORTH:
			case SOUTH:
				coll->Shift.Position.x += FindGridShift(entityPos.x + xLeft, entityPos.x + xFront);
				break;

			case EAST:
			case WEST:
				coll->Shift.Position.z += FindGridShift(entityPos.z + zLeft, entityPos.z + zFront);
				break;
			}
		}

		if (coll->DiagonalStepAtLeft())
		{
			// NOTE: Different from quadrant.
			int quarter = (unsigned short)coll->Setup.ForwardAngle / ANGLE(90.0f);
			quarter %= 2;

			if (coll->MiddleLeft.HasFlippedDiagonalSplit())
			{
				if (quarter)
					coll->CollisionType = CT_LEFT;
			}
			else
			{
				if (!quarter)
					coll->CollisionType = CT_LEFT;
			}
		}
		else
		{
			coll->CollisionType = CT_LEFT;
		}

		return;
	}

	// Shift away from right wall.
	if (coll->MiddleRight.Floor > coll->Setup.LowerFloorBound ||
		coll->MiddleRight.Floor < coll->Setup.UpperFloorBound ||
		coll->MiddleRight.Ceiling > coll->Setup.LowerCeilingBound ||
		coll->MiddleRight.Ceiling < coll->Setup.UpperCeilingBound ||
		coll->MiddleRight.Floor - coll->MiddleRight.Ceiling <= 0)
	{
		if (coll->TriangleAtRight() && !coll->MiddleRight.FloorSlope)
		{
			HandleDiagonalShift(*item, *coll, entityPos, (CardinalDirection)quadrant, false);
		}
		else
		{
			switch (quadrant)
			{
			case NORTH:
			case SOUTH:
				coll->Shift.Position.x += FindGridShift(entityPos.x + xRight, entityPos.x + xFront);
				break;

			case EAST:
			case WEST:
				coll->Shift.Position.z += FindGridShift(entityPos.z + zRight, entityPos.z + zFront);
				break;
			}
		}

		if (coll->DiagonalStepAtRight())
		{
			int quarter = unsigned short(coll->Setup.ForwardAngle) / ANGLE(90.0f); // NOTE: Different from quadrant!
			quarter %= 2;

			if (coll->MiddleRight.HasFlippedDiagonalSplit())
			{
				if (quarter)
					coll->CollisionType = CT_RIGHT;
			}
			else
			{
				if (!quarter)
					coll->CollisionType = CT_RIGHT;
			}
		}
		else
		{
			coll->CollisionType = CT_RIGHT;
		}

		return;
	}
}

void AlignEntityToSurface(ItemInfo* item, const Vector2& ellipse, float alpha, short constraintAngle)
{
	// Reduce ellipse axis lengths for stability.
	auto reducedEllipse = ellipse * 0.75f;

	// Probe heights at points around entity.
	int frontHeight = GetCollision(item, item->Pose.Orientation.y, reducedEllipse.y).Position.Floor;
	int backHeight	= GetCollision(item, item->Pose.Orientation.y + ANGLE(180.0f), reducedEllipse.y).Position.Floor;
	int leftHeight	= GetCollision(item, item->Pose.Orientation.y - ANGLE(90.0f), reducedEllipse.x).Position.Floor;
	int rightHeight = GetCollision(item, item->Pose.Orientation.y + ANGLE(90.0f), reducedEllipse.x).Position.Floor;

	// Calculate height deltas.
	int forwardHeightDelta = backHeight - frontHeight;
	int lateralHeightDelta = rightHeight - leftHeight;

	// Calculate extra rotation required.
	auto extraRot = EulerAngles(
		FROM_RAD(atan2(forwardHeightDelta, ellipse.y * 2)),
		0,
		FROM_RAD(atan2(lateralHeightDelta, ellipse.x * 2))) -
		EulerAngles(item->Pose.Orientation.x, 0, item->Pose.Orientation.z);

	// Rotate X axis.
	if (abs(forwardHeightDelta) <= STEPUP_HEIGHT)
	{
		if (abs(extraRot.x) <= constraintAngle)
			item->Pose.Orientation.x += extraRot.x * alpha;
	}

	// Rotate Z axis.
	if (abs(lateralHeightDelta) <= STEPUP_HEIGHT)
	{
		if (abs(extraRot.z) <= constraintAngle)
			item->Pose.Orientation.z += extraRot.z * alpha;
	}
}

int GetQuadrant(short angle)
{
	return (unsigned short(angle + ANGLE(45.0f)) / ANGLE(90.0f));
}

FloorInfo* GetFloor(int x, int y, int z, short* roomNumber)
{
	const auto location = GetRoom(RoomVector(*roomNumber, y), Vector3i(x, y, z));
	*roomNumber = location.RoomNumber;
	return &GetFloor(*roomNumber, x, z);
}

int GetFloorHeight(FloorInfo* floor, int x, int y, int z)
{
	return GetFloorHeight(RoomVector(floor->RoomNumber, y), x, z).value_or(NO_HEIGHT);
}

int GetCeiling(FloorInfo* floor, int x, int y, int z)
{
	return GetCeilingHeight(RoomVector(floor->RoomNumber, y), x, z).value_or(NO_HEIGHT);
}

int GetDistanceToFloor(int itemNumber, bool precise)
{
	auto* item = &g_Level.Items[itemNumber];

	auto pointColl = GetCollision(item);

	// HACK: Remove item from bridge objects temporarily.
	pointColl.Block->RemoveBridge(itemNumber);
	int height = GetFloorHeight(pointColl.Block, item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z);
	pointColl.Block->AddBridge(itemNumber);

	auto bounds = GameBoundingBox(item);
	int minHeight = precise ? bounds.Y2 : 0;

	return (minHeight + item->Pose.Position.y - height);
}

int GetWaterSurface(int x, int y, int z, short roomNumber)
{
	auto* room = &g_Level.Rooms[roomNumber];
	FloorInfo* floor = GetSector(room, x - room->x, z - room->z);

	if (TestEnvironment(ENV_FLAG_WATER, room))
	{
		while (floor->GetRoomNumberAbove(Vector3i(x, y, z)).value_or(NO_ROOM) != NO_ROOM)
		{
			room = &g_Level.Rooms[floor->GetRoomNumberAbove(Vector3i(x, y, z)).value_or(floor->RoomNumber)];
			if (!TestEnvironment(ENV_FLAG_WATER, room))
				return (floor->GetSurfaceHeight(x, z, false));

			floor = GetSector(room, x - room->x, z - room->z);
		}

		return NO_HEIGHT;
	}
	else
	{
		while (floor->GetRoomNumberBelow(Vector3i(x, y, z)).value_or(NO_ROOM) != NO_ROOM)
		{
			room = &g_Level.Rooms[floor->GetRoomNumberBelow(Vector3i(x, y, z)).value_or(floor->RoomNumber)];
			if (TestEnvironment(ENV_FLAG_WATER, room))
				return (floor->GetSurfaceHeight(x, z, true));

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
		int zFloor = (z - room->z) / BLOCK(1);
		int xFloor = (x - room->x) / BLOCK(1);

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
		roomIndex = floor->WallPortalRoomNumber;
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
		while (floor->GetRoomNumberAbove(Vector3i(x, y, z)).value_or(NO_ROOM) != NO_ROOM)
		{
			room = &g_Level.Rooms[floor->GetRoomNumberAbove(Vector3i(x, y, z)).value_or(floor->RoomNumber)];

			if (!TestEnvironment(ENV_FLAG_WATER, room) &&
				!TestEnvironment(ENV_FLAG_SWAMP, room))
			{
				int waterHeight = floor->GetSurfaceHeight(x, z, false);
				int floorHeight = GetCollision(floor, x, y, z).BottomBlock->GetSurfaceHeight(x, z, true);
				return (floorHeight - waterHeight);
			}

			floor = GetSector(room, x - room->x, z - room->z);
		}

		return DEEP_WATER;
	}
	else
	{
		while (floor->GetRoomNumberBelow(Vector3i(x, y, z)).value_or(NO_ROOM) != NO_ROOM)
		{
			room = &g_Level.Rooms[floor->GetRoomNumberBelow(Vector3i(x, y, z)).value_or(floor->RoomNumber)];

			if (TestEnvironment(ENV_FLAG_WATER, room) ||
				TestEnvironment(ENV_FLAG_SWAMP, room))
			{
				int waterHeight = floor->GetSurfaceHeight(x, z, true);
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
		int xBlock = (x - room->x) / BLOCK(1);
		int zBlock = (z - room->z) / BLOCK(1);

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
		adjoiningRoom = floor->WallPortalRoomNumber;

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
		while (floor->GetRoomNumberAbove(Vector3i(x, y, z)).value_or(NO_ROOM) != NO_ROOM)
		{
			auto* room = &g_Level.Rooms[floor->GetRoomNumberAbove(Vector3i(x, y, z)).value_or(floor->RoomNumber)];

			if (!TestEnvironment(ENV_FLAG_WATER, room) &&
				!TestEnvironment(ENV_FLAG_SWAMP, room))
				break;

			floor = GetSector(room, x - room->x, z - room->z);
		}

		return GetCollision(floor, x, y, z).Block->GetSurfaceHeight(Vector3i(x, y, z), false);
	}
	else if (floor->GetRoomNumberBelow(Vector3i(x, y, z)).value_or(NO_ROOM) != NO_ROOM)
	{
		while (floor->GetRoomNumberBelow(Vector3i(x, y, z)).value_or(NO_ROOM) != NO_ROOM)
		{
			auto* room2 = &g_Level.Rooms[floor->GetRoomNumberBelow(Vector3i(x, y, z)).value_or(floor->RoomNumber)];

			if (TestEnvironment(ENV_FLAG_WATER, room2) ||
				TestEnvironment(ENV_FLAG_SWAMP, room2))
				break;

			floor = GetSector(room2, x - room2->x, z - room2->z);
		}

		return GetCollision(floor, x, y, z).Block->GetSurfaceHeight(Vector3i(x, y, z), true);
	}

	return NO_HEIGHT;
}

int GetWaterHeight(ItemInfo* item)
{
	return GetWaterHeight(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, item->RoomNumber);
}

bool TestEnvironment(RoomEnvFlags environmentType, int x, int y, int z, int roomNumber)
{
	return TestEnvironment(environmentType, GetCollision(x, y, z, roomNumber).RoomNumber);
}

bool TestEnvironment(RoomEnvFlags environmentType, Vector3i pos, int roomNumber)
{
	return TestEnvironment(environmentType, GetCollision(pos.x, pos.y, pos.z, roomNumber).RoomNumber);
}

bool TestEnvironment(RoomEnvFlags environmentType, const ItemInfo* item)
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
