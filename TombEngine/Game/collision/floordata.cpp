#include "framework.h"
#include "Game/collision/floordata.h"

#include "Game/collision/collide_room.h"
#include "Game/items.h"
#include "Game/room.h"
#include "Game/Setup.h"
#include "Math/Math.h"
#include "Renderer/Renderer11.h"
#include "Specific/level.h"
#include "Specific/trutils.h"

using namespace TEN::Collision::Floordata;
using namespace TEN::Math;
using namespace TEN::Utils;

const SectorSurfaceTriangleData& FloorInfo::GetSurfaceTriangle(int x, int z, bool isFloor) const
{
	int triangleID = GetSurfaceTriangleID(x, z, isFloor);
	auto& triangles = isFloor ? FloorSurface.Triangles : CeilingSurface.Triangles;
	return triangles[triangleID];
}

int FloorInfo::GetSurfaceTriangleID(int x, int z, bool isFloor) const
{
	// Calculate bias.
	auto sectorPoint = GetSectorPoint(x, z).ToVector2();
	auto rotMatrix = Matrix::CreateRotationZ(isFloor ? TO_RAD(FloorSurface.SplitAngle) : TO_RAD(CeilingSurface.SplitAngle));
	float bias = Vector2::Transform(sectorPoint, rotMatrix).x;

	// Return triangle ID according to bias.
	return ((bias < 0.0f) ? 0 : 1);
}

Vector3 FloorInfo::GetSurfaceNormal(int x, int z, bool isFloor) const
{
	const auto& triangle = GetSurfaceTriangle(x, z, isFloor);
	return triangle.Plane.Normal();
}

Vector3 FloorInfo::GetSurfaceNormal(int triangleID, bool isFloor) const
{
	// Get triangle.
	const auto& triangles = isFloor ? FloorSurface.Triangles : CeilingSurface.Triangles;
	const auto& triangle = triangles[triangleID];

	// Return plane normal.
	return triangle.Plane.Normal();
}

short FloorInfo::GetSurfaceIllegalSlopeAngle(int x, int z, bool isFloor) const
{
	const auto& triangle = GetSurfaceTriangle(x, z, isFloor);
	return triangle.IllegalSlopeAngle;
}

MaterialType FloorInfo::GetSurfaceMaterial(int x, int z, bool isFloor) const
{
	const auto& triangle = GetSurfaceTriangle(x, z, isFloor);
	return triangle.Material;
}

bool FloorInfo::IsSurfaceSplit(bool isFloor) const
{
	const auto& triangles = isFloor ? FloorSurface.Triangles : CeilingSurface.Triangles;

	// Check if surface planes aren't equal or portal is split.
	bool arePlanesEqual = (triangles[0].Plane == triangles[1].Plane);
	return (!arePlanesEqual || IsSurfaceSplitPortal(isFloor));
}

bool FloorInfo::IsSurfaceDiagonalStep(bool isFloor) const
{
	// 1) Test if surface is split.
	if (!IsSurfaceSplit(isFloor))
		return false;

	const auto& surface = isFloor ? FloorSurface : CeilingSurface;
	
	// 2) Test if plane distances are equal.
	float dist0 = surface.Triangles[0].Plane.D();
	float dist1 = surface.Triangles[1].Plane.D();
	if (dist0 == dist1)
		return false;

	// 3) Test if split angle is aligned diagonal.
	if (surface.SplitAngle != SectorSurfaceData::SPLIT_ANGLE_0 &&
		surface.SplitAngle != SectorSurfaceData::SPLIT_ANGLE_1)
	{
		return false;
	}

	return true;
}

bool FloorInfo::IsSurfaceSplitPortal(bool isFloor) const
{
	const auto& triangles = isFloor ? FloorSurface.Triangles : CeilingSurface.Triangles;
	return (triangles[0].PortalRoomNumber != triangles[1].PortalRoomNumber);
}

std::optional<int> FloorInfo::GetRoomNumberBelow(int triangleID) const
{
	int roomNumber = FloorSurface.Triangles[triangleID].PortalRoomNumber;
	if (roomNumber != NO_ROOM)
		return roomNumber;

	return std::nullopt;
}

std::optional<int> FloorInfo::GetRoomNumberBelow(int x, int z) const
{
	int triangleID = GetSurfaceTriangleID(x, z, true);
	return GetRoomNumberBelow(triangleID);
}

std::optional<int> FloorInfo::GetRoomNumberBelow(const Vector3i& pos) const
{
	// 1) Get sector floor and ceiling heights.
	int floorHeight = GetSurfaceHeight(pos.x, pos.z, true);
	int ceilingHeight = GetSurfaceHeight(pos.x, pos.z, false);

	// 2) Test access to room below.
	for (int itemNumber : BridgeItemNumbers)
	{
		const auto& bridgeItem = g_Level.Items[itemNumber];
		const auto& bridgeObject = Objects[bridgeItem.ObjectNumber];

		// 2.1) Get bridge floor height.
		auto bridgeFloorHeight = bridgeObject.GetFloorHeight(bridgeItem, pos);
		if (!bridgeFloorHeight.has_value())
			continue;

		// 2.2) Test if bridge blocks access to room below.
		if (*bridgeFloorHeight >= pos.y &&		 // Bridge floor height is below position.
			*bridgeFloorHeight <= floorHeight && // Bridge floor height is above sector floor height.
			*bridgeFloorHeight >= ceilingHeight) // Bridge floor height is below sector ceiling height.
		{
			return std::nullopt;
		}
	}

	// 3) Get and return room number below.
	return GetRoomNumberBelow(pos.x, pos.z);
}

std::optional<int> FloorInfo::GetRoomNumberAbove(int triangleID) const
{
	int roomNumber = CeilingSurface.Triangles[triangleID].PortalRoomNumber;
	if (roomNumber != NO_ROOM)
		return roomNumber;

	return std::nullopt;
}

std::optional<int> FloorInfo::GetRoomNumberAbove(int x, int z) const
{
	int triangleID = GetSurfaceTriangleID(x, z, false);
	return GetRoomNumberAbove(triangleID);
}

std::optional<int> FloorInfo::GetRoomNumberAbove(const Vector3i& pos) const
{
	// 1) Get sector floor and ceiling heights.
	int floorHeight = GetSurfaceHeight(pos.x, pos.z, true);
	int ceilingHeight = GetSurfaceHeight(pos.x, pos.z, false);

	// 2) Test access to room above.
	for (int itemNumber : BridgeItemNumbers)
	{
		const auto& bridgeItem = g_Level.Items[itemNumber];
		const auto& bridgeObject = Objects[bridgeItem.ObjectNumber];

		// 2.1) Get bridge ceiling height.
		auto bridgeCeilingHeight = bridgeObject.GetCeilingHeight(bridgeItem, pos);
		if (!bridgeCeilingHeight.has_value())
			continue;

		// 2.2) Test if bridge blocks access to room above.
		if (*bridgeCeilingHeight <= pos.y &&	   // Bridge ceiling height is above position.
			*bridgeCeilingHeight <= floorHeight && // Bridge ceiling height is above sector floor height.
			*bridgeCeilingHeight >= ceilingHeight) // Bridge ceiling height is below sector ceiling height.
		{
			return std::nullopt;
		}
	}

	// 3) Get and return room number above.
	return GetRoomNumberAbove(pos.x, pos.z);
}

std::optional<int> FloorInfo::GetRoomNumberAtSide() const
{
	if (WallPortalRoomNumber != NO_ROOM)
		return WallPortalRoomNumber;

	return std::nullopt;
}

int FloorInfo::GetSurfaceHeight(int x, int z, bool isFloor) const
{
	// Get triangle.
	const auto& triangle = GetSurfaceTriangle(x, z, isFloor);

	// MICRO-OPTIMIZATION: Triangle is flat; return plane height.
	auto normal = triangle.Plane.Normal();
	if (normal == Vector3::UnitY || normal == -Vector3::UnitY)
		return triangle.Plane.D();

	// Calculate relative plane height at intersection using plane equation.
	auto sectorPoint = GetSectorPoint(x, z);
	float relPlaneHeight = -((normal.x * sectorPoint.x) + (normal.z * sectorPoint.y)) / normal.y;

	// Return surface height.
	return (triangle.Plane.D() + relPlaneHeight);
}

int FloorInfo::GetSurfaceHeight(const Vector3i& pos, bool isFloor) const
{
	// 1) Get sector floor and ceiling heights.
	int floorHeight = GetSurfaceHeight(pos.x, pos.z, true);
	int ceilingHeight = GetSurfaceHeight(pos.x, pos.z, false);

	// 2) Find closest floor or ceiling bridge height (if applicable).
	for (int itemNumber : BridgeItemNumbers)
	{
		const auto& bridgeItem = g_Level.Items[itemNumber];
		const auto& bridgeObject = Objects[bridgeItem.ObjectNumber];

		// 2.1) Get bridge surface height.
		auto bridgeSurfaceHeight = isFloor ? bridgeObject.GetFloorHeight(bridgeItem, pos) : bridgeObject.GetCeilingHeight(bridgeItem, pos);
		if (!bridgeSurfaceHeight.has_value())
			continue;

		// 2.2) Track closest floor or ceiling height.
		if (isFloor)
		{
			// Test if bridge floor height is closer.
			if (*bridgeSurfaceHeight >= pos.y &&	   // Bridge floor height is below position.
				*bridgeSurfaceHeight < floorHeight &&  // Bridge floor height is above current closest floor height.
				*bridgeSurfaceHeight >= ceilingHeight) // Bridge ceiling height is below sector ceiling height.
			{
				floorHeight = *bridgeSurfaceHeight;
			}
		}
		else
		{
			// Test if bridge ceiling height is closer.
			if (*bridgeSurfaceHeight <= pos.y &&		// Bridge ceiling height is above position.
				*bridgeSurfaceHeight > ceilingHeight && // Bridge ceiling height is below current closest ceiling height.
				*bridgeSurfaceHeight <= floorHeight)	// Bridge floor height is above sector floor height.
			{
				ceilingHeight = *bridgeSurfaceHeight;
			}
		}
	}

	// 3) Return closest floor or ceiling height.
	return (isFloor ? floorHeight : ceilingHeight);
}

int FloorInfo::GetBridgeSurfaceHeight(const Vector3i& pos, bool isFloor) const
{
	// 1) Find and return intersected bridge floor or ceiling height (if applicable).
	for (int itemNumber : BridgeItemNumbers)
	{
		const auto& bridgeItem = g_Level.Items[itemNumber];
		const auto& bridgeObject = Objects[bridgeItem.ObjectNumber];

		// 1.1) Get bridge floor and ceiling heights.
		auto floorHeight = bridgeObject.GetFloorHeight(bridgeItem, pos);
		auto ceilingHeight = bridgeObject.GetCeilingHeight(bridgeItem, pos);
		if (!floorHeight.has_value() || !ceilingHeight.has_value())
			continue;

		// 1.2) If position is inside bridge, return bridge floor or ceiling height.
		if (isFloor)
		{
			// Test for bridge intersection.
			if (pos.y > *floorHeight &&	 // Position is below bridge floor height.
				pos.y <= *ceilingHeight) // Position is above bridge ceiling height.
			{
				return *floorHeight;
			}
		}
		else
		{
			// Test for bridge intersection.
			if (pos.y >= *floorHeight && // Position is below bridge floor height.
				pos.y < *ceilingHeight)	 // Position is above bridge ceiling height.
			{
				return *ceilingHeight;
			}
		}
	}
	
	// 2) Get and return closest floor or ceiling height.
	return GetSurfaceHeight(pos, isFloor);
}

bool FloorInfo::IsWall(int triangleID) const
{
	bool areSplitAnglesEqual = (FloorSurface.SplitAngle == CeilingSurface.SplitAngle);
	bool arePlanesEqual = (FloorSurface.Triangles[triangleID].Plane == CeilingSurface.Triangles[triangleID].Plane);

	return (areSplitAnglesEqual && arePlanesEqual);
}

bool FloorInfo::IsWall(int x, int z) const
{
	int triangleID = GetSurfaceTriangleID(x, z, true);
	return IsWall(triangleID);
}

int FloorInfo::GetInsideBridgeItemNumber(const Vector3i& pos, bool testFloorBorder, bool testCeilingBorder) const
{
	// 1) Find and return intersected bridge item number (if applicable).
	for (int itemNumber : BridgeItemNumbers)
	{
		const auto& bridgeItem = g_Level.Items[itemNumber];
		const auto& bridgeObject = Objects[bridgeItem.ObjectNumber];

		// 1.1) Get bridge floor and ceiling heights.
		auto floorHeight = bridgeObject.GetFloorHeight(bridgeItem, pos);
		auto ceilingHeight = bridgeObject.GetCeilingHeight(bridgeItem, pos);
		if (!floorHeight.has_value() || !ceilingHeight.has_value())
			continue;

		// 1.2) Test for bridge intersection.
		if (pos.y > *floorHeight && // Position is below bridge floor height.
			pos.y < *ceilingHeight) // Position is above bridge ceiling height.
		{
			return itemNumber;
		}

		// TODO: Check what this does.
		// 1.3) Test bridge floor and ceiling borders (if applicable).
		if ((testFloorBorder && pos.y == *floorHeight) ||	// Position matches floor height.
			(testCeilingBorder && pos.y == *ceilingHeight)) // Position matches ceiling height.
		{
			return itemNumber;
		}
	}

	// 2) No bridge intersection; return invalid item number.
	return NO_ITEM;
}

void FloorInfo::AddBridge(int itemNumber)
{
	BridgeItemNumbers.insert(itemNumber);
}

void FloorInfo::RemoveBridge(int itemNumber)
{
	BridgeItemNumbers.erase(itemNumber);
}

namespace TEN::Collision::Floordata
{
	// NOTE: Tilts are deprecated, but until all conversions are complete this function will remain useful.
	Vector2i GetSurfaceTilt(const Vector3& normal, bool isFloor)
	{
		// Calculate tilt values based on normal.
		float xTilt = normal.x * 4;
		float zTilt = normal.z * 4;

		// Scale tilt values to appropriate range.
		int xTiltGrade = (int)round(xTilt * (CLICK(1) / BLOCK(1)));
		int zTiltGrade = (int)round(zTilt * (CLICK(1) / BLOCK(1)));

		// Return tilt.
		return Vector2i(xTiltGrade, zTiltGrade);
	}

	Vector2i GetSectorPoint(int x, int z)
	{
		return Vector2i(
			x % BLOCK(1) - BLOCK(1) / 2,
			z % BLOCK(1) - BLOCK(1) / 2);
	}

	Vector2i GetRoomGridCoord(int roomNumber, int x, int z, bool clampToBounds)
	{
		const auto& room = g_Level.Rooms[roomNumber];

		// Calculate room grid coord.
		auto roomGridCoord = Vector2i((x - room.x) / BLOCK(1), (z - room.z) / BLOCK(1));
		if (x < room.x)
			roomGridCoord.x -= 1;
		if (z < room.z)
			roomGridCoord.y -= 1;

		// Clamp room grid coord to room bounds (if applicable).
		if (clampToBounds)
		{
			roomGridCoord.x = std::clamp(roomGridCoord.x, 0, room.xSize - 1);
			roomGridCoord.y = std::clamp(roomGridCoord.y, 0, room.zSize - 1);
		}

		return roomGridCoord;
	}

	std::vector<Vector2i> GetNeighborRoomGridCoords(const Vector3i& pos, int roomNumber, unsigned int searchDepth)
	{
		auto originRoomGridCoord = GetRoomGridCoord(roomNumber, pos.x, pos.z, false);

		// Determine room grid coord bounds.
		int xMax = originRoomGridCoord.x + searchDepth;
		int xMin = originRoomGridCoord.x - searchDepth;
		int zMax = originRoomGridCoord.y + searchDepth;
		int zMin = originRoomGridCoord.y - searchDepth;

		const auto& room = g_Level.Rooms[roomNumber];

		// Collect room grid coords.
		auto roomGridCoords = std::vector<Vector2i>{};
		for (int x = xMin; x <= xMax; x++)
		{
			// Test if out of room X range.
			if (x <= 0 || x >= (room.xSize - 1))
				continue;

			for (int z = zMin; z <= zMax; z++)
			{
				// Test if out of room Z range.
				if (z <= 0 || z >= (room.zSize - 1))
					continue;

				roomGridCoords.push_back(Vector2i(x, z));
			}
		}

		return roomGridCoords;
	}

	std::vector<FloorInfo*> GetNeighborSectorPtrs(const Vector3i& pos, int roomNumber, unsigned int searchDepth)
	{
		auto sectorPtrs = std::vector<FloorInfo*>{};

		// Run through neighbor rooms.
		auto& room = g_Level.Rooms[roomNumber];
		for (int neighborRoomNumber : room.neighbors)
		{
			// Collect neighbor sector pointers.
			auto roomGridCoords = GetNeighborRoomGridCoords(pos, neighborRoomNumber, searchDepth);
			for (const auto& roomGridCoord : roomGridCoords)
				sectorPtrs.push_back(&GetFloor(neighborRoomNumber, roomGridCoord));
		}

		// Return neighbor sector pointers.
		return sectorPtrs;
	}

	FloorInfo& GetFloor(int roomNumber, const Vector2i& roomGridCoord)
	{
		auto& room = g_Level.Rooms[roomNumber];

		// Get and return sector.
		int sectorID = (room.zSize * roomGridCoord.x) + roomGridCoord.y;
		return room.floor[sectorID];
	}

	FloorInfo& GetFloor(int roomNumber, int x, int z)
	{
		return GetFloor(roomNumber, GetRoomGridCoord(roomNumber, x, z));
	}

	FloorInfo& GetFloorSide(int roomNumber, int x, int z, int* sideRoomNumberPtr)
	{
		auto* sectorPtr = &GetFloor(roomNumber, x, z);

		auto sideRoomNumber = sectorPtr->GetRoomNumberAtSide();
		while (sideRoomNumber.has_value())
		{
			roomNumber = *sideRoomNumber;
			sectorPtr = &GetFloor(roomNumber, x, z);
			sideRoomNumber = sectorPtr->GetRoomNumberAtSide();
		}

		if (sideRoomNumberPtr != nullptr)
			*sideRoomNumberPtr = roomNumber;

		return *sectorPtr;
	}

	FloorInfo& GetBottomFloor(int roomNumber, int x, int z, int* bottomRoomNumberPtr)
	{
		auto* sectorPtr = &GetFloorSide(roomNumber, x, z, bottomRoomNumberPtr);
		
		bool isWall = sectorPtr->IsWall(x, z);
		while (isWall)
		{
			auto roomNumberBelow = sectorPtr->GetRoomNumberBelow(x, z);
			if (!roomNumberBelow.has_value())
				break;

			sectorPtr = &GetFloorSide(*roomNumberBelow, x, z, bottomRoomNumberPtr);
			isWall = sectorPtr->IsWall(x, z);
		}

		return *sectorPtr;
	}

	FloorInfo& GetTopFloor(int roomNumber, int x, int z, int* topRoomNumberPtr)
	{
		auto* sectorPtr = &GetFloorSide(roomNumber, x, z, topRoomNumberPtr);
		
		bool isWall = sectorPtr->IsWall(x, z);
		while (isWall)
		{
			auto roomNumberAbove = sectorPtr->GetRoomNumberAbove(x, z);
			if (!roomNumberAbove)
				break;

			sectorPtr = &GetFloorSide(*roomNumberAbove, x, z, topRoomNumberPtr);
			isWall = sectorPtr->IsWall(x, z);
		}

		return *sectorPtr;
	}

	std::optional<int> GetTopHeight(FloorInfo& startSector, Vector3i pos, int* topRoomNumberPtr, FloorInfo** topSectorPtr)
	{
		int roomNumber = 0;
		if (topRoomNumberPtr != nullptr)
			roomNumber = *topRoomNumberPtr;

		auto* sectorPtr = &startSector;
		do
		{
			pos.y = sectorPtr->GetBridgeSurfaceHeight(pos, true);
			while (pos.y <= sectorPtr->GetSurfaceHeight(pos.x, pos.z, false))
			{
				auto roomNumberAbove = sectorPtr->GetRoomNumberAbove(pos.x, pos.z);
				if (!roomNumberAbove.has_value())
					return std::nullopt;

				sectorPtr = &GetFloorSide(*roomNumberAbove, pos.x, pos.z, &roomNumber);
			}
		}
		while (sectorPtr->GetInsideBridgeItemNumber(pos, false, true) >= 0);

		if (topRoomNumberPtr != nullptr)
			*topRoomNumberPtr = roomNumber;

		if (topSectorPtr != nullptr)
			*topSectorPtr = sectorPtr;

		return pos.y;
	}

	std::optional<int> GetBottomHeight(FloorInfo& startSector, Vector3i pos, int* bottomRoomNumberPtr, FloorInfo** bottomSectorPtr)
	{
		int roomNumber = 0;
		if (bottomRoomNumberPtr)
			roomNumber = *bottomRoomNumberPtr;

		auto* sectorPtr = &startSector;
		do
		{
			pos.y = sectorPtr->GetBridgeSurfaceHeight(pos, false);
			while (pos.y >= sectorPtr->GetSurfaceHeight(pos.x, pos.z, true))
			{
				auto roomNumberBelow = sectorPtr->GetRoomNumberBelow(pos.x, pos.z);
				if (!roomNumberBelow.has_value())
					return std::nullopt;

				sectorPtr = &GetFloorSide(*roomNumberBelow, pos.x, pos.z, &roomNumber);
			}
		}
		while (sectorPtr->GetInsideBridgeItemNumber(pos, true, false) >= 0);

		if (bottomRoomNumberPtr != nullptr)
			*bottomRoomNumberPtr = roomNumber;

		if (bottomSectorPtr != nullptr)
			*bottomSectorPtr = sectorPtr;

		return pos.y;
	}

	std::optional<int> GetFloorHeight(const RoomVector& location, int x, int z)
	{
		auto* sectorPtr = &GetFloorSide(location.RoomNumber, x, z);

		int y = location.Height;
		int direction = 0;

		if (sectorPtr->IsWall(x, z))
		{
			sectorPtr = &GetTopFloor(location.RoomNumber, x, z);

			if (!sectorPtr->IsWall(x, z))
			{
				y = sectorPtr->GetSurfaceHeight(x, z, true);
				direction = -1;
			}
			else
			{
				sectorPtr = &GetBottomFloor(location.RoomNumber, x, z);

				if (!sectorPtr->IsWall(x, z))
				{
					y = sectorPtr->GetSurfaceHeight(x, z, false);
					direction = 1;
				}
				else
				{
					return std::nullopt;
				}
			}
		}

		int floorHeight = sectorPtr->GetSurfaceHeight(Vector3i(x, y, z), true);
		int ceilingHeight = sectorPtr->GetSurfaceHeight(Vector3i(x, y, z), false);

		y = std::clamp(y, std::min(ceilingHeight, floorHeight), std::max(ceilingHeight, floorHeight));

		if (sectorPtr->GetInsideBridgeItemNumber(Vector3i(x, y, z), y == ceilingHeight, y == floorHeight) >= 0)
		{
			if (direction <= 0)
			{
				auto topHeight = GetTopHeight(*sectorPtr, Vector3i(x, y, z));
				if (topHeight.has_value())
					return topHeight;
			}

			if (direction >= 0)
			{
				auto bottomHeight = GetBottomHeight(*sectorPtr, Vector3i(x, y, z), nullptr, &sectorPtr);
				if (!bottomHeight.has_value())
					return std::nullopt;

				y = *bottomHeight;
			}
		}

		if (direction >= 0)
		{
			auto roomNumberBelow = sectorPtr->GetRoomNumberBelow(Vector3i(x, y, z));
			while (roomNumberBelow.has_value())
			{
				sectorPtr = &GetFloorSide(*roomNumberBelow, x, z);
				roomNumberBelow = sectorPtr->GetRoomNumberBelow(Vector3i(x, y, z));
			}
		}

		return sectorPtr->GetSurfaceHeight(Vector3i(x, y, z), true);
	}

	std::optional<int> GetCeilingHeight(const RoomVector& location, int x, int z)
	{
		auto* sectorPtr = &GetFloorSide(location.RoomNumber, x, z);

		int y = location.Height;
		int direction = 0;

		if (sectorPtr->IsWall(x, z))
		{
			sectorPtr = &GetBottomFloor(location.RoomNumber, x, z);

			if (!sectorPtr->IsWall(x, z))
			{
				y = sectorPtr->GetSurfaceHeight(x, z, false);
				direction = 1;
			}
			else
			{
				sectorPtr = &GetTopFloor(location.RoomNumber, x, z);

				if (!sectorPtr->IsWall(x, z))
				{
					y = sectorPtr->GetSurfaceHeight(x, z, true);
					direction = -1;
				}
				else
				{
					return std::nullopt;
				}
			}
		}

		int floorHeight = sectorPtr->GetSurfaceHeight(Vector3i(x, y, z), true);
		int ceilingHeight = sectorPtr->GetSurfaceHeight(Vector3i(x, y, z), false);

		y = std::clamp(y, std::min(ceilingHeight, floorHeight), std::max(ceilingHeight, floorHeight));

		if (sectorPtr->GetInsideBridgeItemNumber(Vector3i(x, y, z), y == ceilingHeight, y == floorHeight) >= 0)
		{
			if (direction >= 0)
			{
				auto bottomHeight = GetBottomHeight(*sectorPtr, Vector3i(x, y, z));
				if (bottomHeight.has_value())
					return bottomHeight;
			}

			if (direction <= 0)
			{
				auto topHeight = GetTopHeight(*sectorPtr, Vector3i(x, y, z), nullptr, &sectorPtr);
				if (!topHeight.has_value())
					return std::nullopt;

				y = *topHeight;
			}
		}

		if (direction <= 0)
		{
			auto roomNumberAbove = sectorPtr->GetRoomNumberAbove(Vector3i(x, y, z));
			while (roomNumberAbove.has_value())
			{
				sectorPtr = &GetFloorSide(*roomNumberAbove, x, z);
				roomNumberAbove = sectorPtr->GetRoomNumberAbove(Vector3i(x, y, z));
			}
		}

		return sectorPtr->GetSurfaceHeight(Vector3i(x, y, z), false);
	}

	std::optional<RoomVector> GetBottomRoom(RoomVector location, const Vector3i& pos)
	{
		auto* sectorPtr = &GetFloorSide(location.RoomNumber, pos.x, pos.z, &location.RoomNumber);

		if (sectorPtr->IsWall(pos.x, pos.z))
		{
			sectorPtr = &GetBottomFloor(location.RoomNumber, pos.x, pos.z, &location.RoomNumber);

			if (sectorPtr->IsWall(pos.x, pos.z))
				return std::nullopt;

			location.Height = sectorPtr->GetSurfaceHeight(pos.x, pos.z, false);
		}

		int floorHeight = sectorPtr->GetSurfaceHeight(Vector3i(pos.x, location.Height, pos.z), true);
		int ceilingHeight = sectorPtr->GetSurfaceHeight(Vector3i(pos.x, location.Height, pos.z), false);

		location.Height = std::clamp(location.Height, std::min(ceilingHeight, floorHeight), std::max(ceilingHeight, floorHeight));

		bool testFloorBorder = (location.Height == ceilingHeight);
		bool testCeilBorder = (location.Height == floorHeight);
		int insideBridgeItemNumber = sectorPtr->GetInsideBridgeItemNumber(Vector3i(pos.x, location.Height, pos.z), testFloorBorder, testCeilBorder);

		if (insideBridgeItemNumber != NO_ITEM)
		{
			auto bottomHeight = GetBottomHeight(*sectorPtr, Vector3i(pos.x, location.Height, pos.z), &location.RoomNumber, &sectorPtr);
			if (!bottomHeight.has_value())
				return std::nullopt;

			location.Height = *bottomHeight;
		}

		ceilingHeight = sectorPtr->GetSurfaceHeight(Vector3i(pos.x, location.Height, pos.z), false);
		if (pos.y < ceilingHeight && sectorPtr->GetRoomNumberAbove(Vector3i(pos.x, location.Height, pos.z)))
			return std::nullopt;

		floorHeight = sectorPtr->GetSurfaceHeight(Vector3i(pos.x, location.Height, pos.z), true);
		if (pos.y <= floorHeight)
		{
			location.Height = std::max(pos.y, ceilingHeight);
			return location;
		}

		auto roomNumberBelow = sectorPtr->GetRoomNumberBelow(Vector3i(pos.x, location.Height, pos.z));
		while (roomNumberBelow.has_value())
		{
			sectorPtr = &GetFloorSide(*roomNumberBelow, pos.x, pos.z, &location.RoomNumber);
			location.Height = sectorPtr->GetSurfaceHeight(pos.x, pos.z, false);

			ceilingHeight = sectorPtr->GetSurfaceHeight(Vector3i(pos.x, location.Height, pos.z), false);
			if (pos.y < ceilingHeight && sectorPtr->GetRoomNumberAbove(Vector3i(pos.x, location.Height, pos.z)))
				return std::nullopt;

			floorHeight = sectorPtr->GetSurfaceHeight(Vector3i(pos.x, location.Height, pos.z), true);
			if (pos.y <= floorHeight)
			{
				location.Height = std::max(pos.y, ceilingHeight);
				return location;
			}

			roomNumberBelow = sectorPtr->GetRoomNumberBelow(Vector3i(pos.x, location.Height, pos.z));
		}

		return std::nullopt;
	}

	std::optional<RoomVector> GetTopRoom(RoomVector location, const Vector3i& pos)
	{
		auto* sectorPtr = &GetFloorSide(location.RoomNumber, pos.x, pos.z, &location.RoomNumber);

		if (sectorPtr->IsWall(pos.x, pos.z))
		{
			sectorPtr = &GetTopFloor(location.RoomNumber, pos.x, pos.z, &location.RoomNumber);

			if (sectorPtr->IsWall(pos.x, pos.z))
				return std::nullopt;

			location.Height = sectorPtr->GetSurfaceHeight(pos.x, pos.z, true);
		}

		int floorHeight = sectorPtr->GetSurfaceHeight(Vector3i(pos.x, location.Height, pos.z), true);
		int ceilingHeight = sectorPtr->GetSurfaceHeight(Vector3i(pos.x, location.Height, pos.z), false);

		location.Height = std::clamp(location.Height, std::min(ceilingHeight, floorHeight), std::max(ceilingHeight, floorHeight));

		bool testFloorBorder = (location.Height == ceilingHeight);
		bool testCeilBorder = (location.Height == floorHeight);
		int insideBridgeItemNumber = sectorPtr->GetInsideBridgeItemNumber(Vector3i(pos.x, location.Height, pos.z), testFloorBorder, testCeilBorder);

		if (insideBridgeItemNumber != NO_ITEM)
		{
			auto topHeight = GetTopHeight(*sectorPtr, Vector3i(pos.x, location.Height, pos.z), &location.RoomNumber, &sectorPtr);
			if (!topHeight.has_value())
				return std::nullopt;

			location.Height = *topHeight;
		}

		floorHeight = sectorPtr->GetSurfaceHeight(Vector3i(pos.x, location.Height, pos.z), true);
		if (pos.y > floorHeight && sectorPtr->GetRoomNumberBelow(Vector3i(pos.x, location.Height, pos.z)))
			return std::nullopt;

		ceilingHeight = sectorPtr->GetSurfaceHeight(Vector3i(pos.x, location.Height, pos.z), false);
		if (pos.y >= ceilingHeight)
		{
			location.Height = std::min(pos.y, floorHeight);
			return location;
		}

		auto roomNumberAbove = sectorPtr->GetRoomNumberAbove(Vector3i(pos.x, location.Height, pos.z));
		while (roomNumberAbove.has_value())
		{
			sectorPtr = &GetFloorSide(*roomNumberAbove, pos.x, pos.z, &location.RoomNumber);
			location.Height = sectorPtr->GetSurfaceHeight(pos.x, pos.z, true);

			floorHeight = sectorPtr->GetSurfaceHeight(Vector3i(pos.x, location.Height, pos.z), true);
			if (pos.y > floorHeight && sectorPtr->GetRoomNumberBelow(Vector3i(pos.x, location.Height, pos.z)))
				return std::nullopt;

			ceilingHeight = sectorPtr->GetSurfaceHeight(Vector3i(pos.x, location.Height, pos.z), false);
			if (pos.y >= ceilingHeight)
			{
				location.Height = std::min(pos.y, floorHeight);
				return location;
			}

			roomNumberAbove = sectorPtr->GetRoomNumberAbove(Vector3i(pos.x, location.Height, pos.z));
		}

		return std::nullopt;
	}

	RoomVector GetRoom(RoomVector location, const Vector3i& pos)
	{
		auto locationBelow = GetBottomRoom(location, pos);
		if (locationBelow.has_value())
			return *locationBelow;

		auto locationAbove = GetTopRoom(location, pos);
		if (locationAbove.has_value())
			return *locationAbove;

		return location;
	}

	void AddBridge(int itemNumber, int x, int z)
	{
		const auto& bridgeItem = g_Level.Items[itemNumber];
		const auto& bridgeObject = Objects[bridgeItem.ObjectNumber];

		if (!Objects.CheckID(bridgeItem.ObjectNumber))
			return;

		x += bridgeItem.Pose.Position.x;
		z += bridgeItem.Pose.Position.z;

		auto sectorPtr = &GetFloorSide(bridgeItem.RoomNumber, x, z);
		sectorPtr->AddBridge(itemNumber);

		if (bridgeObject.GetFloorBorder != nullptr)
		{
			int floorBorder = bridgeObject.GetFloorBorder(bridgeItem);
			while (floorBorder <= sectorPtr->GetSurfaceHeight(x, z, false))
			{
				auto roomNumberAbove = sectorPtr->GetRoomNumberAbove(x, z);
				if (!roomNumberAbove.has_value())
					break;

				sectorPtr = &GetFloorSide(*roomNumberAbove, x, z);
				sectorPtr->AddBridge(itemNumber);
			}
		}
		
		if (bridgeObject.GetCeilingBorder != nullptr)
		{
			int ceilingBorder = bridgeObject.GetCeilingBorder(bridgeItem);
			while (ceilingBorder >= sectorPtr->GetSurfaceHeight(x, z, true))
			{
				auto roomNumberBelow = sectorPtr->GetRoomNumberBelow(x, z);
				if (!roomNumberBelow.has_value())
					break;

				sectorPtr = &GetFloorSide(*roomNumberBelow, x, z);
				sectorPtr->AddBridge(itemNumber);
			}
		}
	}

	void RemoveBridge(int itemNumber, int x, int z)
	{
		const auto& bridgeItem = g_Level.Items[itemNumber];
		const auto& bridgeObject = Objects[bridgeItem.ObjectNumber];

		if (!Objects.CheckID(bridgeItem.ObjectNumber))
			return;

		x += bridgeItem.Pose.Position.x;
		z += bridgeItem.Pose.Position.z;

		auto* sectroPtr = &GetFloorSide(bridgeItem.RoomNumber, x, z);
		sectroPtr->RemoveBridge(itemNumber);

		if (bridgeObject.GetFloorBorder != nullptr)
		{
			int floorBorder = bridgeObject.GetFloorBorder(bridgeItem);
			while (floorBorder <= sectroPtr->GetSurfaceHeight(x, z, false))
			{
				auto roomNumberAbove = sectroPtr->GetRoomNumberAbove(x, z);
				if (!roomNumberAbove.has_value())
					break;

				sectroPtr = &GetFloorSide(*roomNumberAbove, x, z);
				sectroPtr->RemoveBridge(itemNumber);
			}
		}

		if (bridgeObject.GetCeilingBorder != nullptr)
		{
			int ceilingBorder = bridgeObject.GetCeilingBorder(bridgeItem);
			while (ceilingBorder >= sectroPtr->GetSurfaceHeight(x, z, true))
			{
				auto roomNumberBelow = sectroPtr->GetRoomNumberBelow(x, z);
				if (!roomNumberBelow.has_value())
					break;

				sectroPtr = &GetFloorSide(*roomNumberBelow, x, z);
				sectroPtr->RemoveBridge(itemNumber);
			}
		}
	}

	// Gets precise floor/ceiling height from object's bounding box.
	// Animated objects are also supported, although horizontal collision shifting is unstable.
	// Method: get accurate bounds in world transform by converting to OBB, then do a ray test
	// on top or bottom (depending on test side) to determine if box is present at a particular point.
	std::optional<int> GetBridgeItemIntersect(const ItemInfo& item, const Vector3i& pos, bool useBottomHeight)
	{
		auto bounds = GameBoundingBox(&item);
		auto dxBounds = bounds.ToBoundingOrientedBox(item.Pose);

		// Introduce slight vertical margin just in case.
		auto origin = Vector3(pos.x, pos.y + (useBottomHeight ? 4 : -4), pos.z);
		auto dir = useBottomHeight ? -Vector3::UnitY : Vector3::UnitY;

		float dist = 0.0f;
		if (dxBounds.Intersects(origin, dir, dist))
			return (item.Pose.Position.y + (useBottomHeight ? bounds.Y2 : bounds.Y1));

		return std::nullopt;
	}

	// Gets bridge min or max height regardless of actual X/Z world position.
	int GetBridgeBorder(const ItemInfo& item, bool isBottom)
	{
		auto bounds = GameBoundingBox(&item);
		return (item.Pose.Position.y + (isBottom ? bounds.Y2 : bounds.Y1));
	}

	// Updates BridgeItem for all blocks which are enclosed by bridge bounds.
	void UpdateBridgeItem(const ItemInfo& item, bool forceRemoval)
	{
		constexpr auto SECTOR_EXTENTS = Vector3(BLOCK(1 / 2.0f));

		if (!Objects.CheckID(item.ObjectNumber))
			return;

		if (!Objects[item.ObjectNumber].loaded)
			return;

		// Force removal if item was killed.
		if (item.Flags & IFLAG_KILLED)
			forceRemoval = true;

		// Get real OBB of bridge in world space.
		auto box = GameBoundingBox(&item).ToBoundingOrientedBox(item.Pose);

		// Get corners of a projected OBB
		Vector3 corners[8];
		box.GetCorners(corners); // corners[0], corners[1], corners[4] corners[5]

		auto room = &g_Level.Rooms[item.RoomNumber];

		// Get min/max of projected AABB.
		auto minX = floor((std::min(std::min(std::min(corners[0].x, corners[1].x), corners[4].x), corners[5].x) - room->x) / BLOCK(1));
		auto minZ = floor((std::min(std::min(std::min(corners[0].z, corners[1].z), corners[4].z), corners[5].z) - room->z) / BLOCK(1));
		auto maxX =  ceil((std::max(std::max(std::max(corners[0].x, corners[1].x), corners[4].x), corners[5].x) - room->x) / BLOCK(1));
		auto maxZ =  ceil((std::max(std::max(std::max(corners[0].z, corners[1].z), corners[4].z), corners[5].z) - room->z) / BLOCK(1));

		// Run through all blocks enclosed in AABB.
		for (int x = 0; x < room->xSize; x++)
		{
			for (int z = 0; z < room->zSize; z++)
			{
				float pX = room->x + (x * BLOCK(1)) + BLOCK(0.5f);
				float pZ = room->z + (z * BLOCK(1)) + BLOCK(0.5f);
				float offX = pX - item.Pose.Position.x;
				float offZ = pZ - item.Pose.Position.z;

				// Clean previous bridge state.
				RemoveBridge(item.Index, offX, offZ);

				// If in sweeping mode, don't try readding to block.
				if (forceRemoval)
					continue;

				// If block isn't in enclosed AABB space, ignore precise check.
				if (x < minX || z < minZ ||
					x > maxX || z > maxZ)
				{
					continue;
				}

				// Block is in enclosed AABB space; do more precise test.
				// Construct block bounding box within same plane as bridge bounding box and test intersection.
				auto blockBox = BoundingOrientedBox(Vector3(pX, box.Center.y, pZ), SECTOR_EXTENTS, Vector4::UnitY);

				// Add bridge to current sector if intersection is valid.
				if (box.Intersects(blockBox))
					AddBridge(item.Index, offX, offZ);
			}
		}
	}

	bool TestMaterial(MaterialType refMaterial, const std::vector<MaterialType>& materials)
	{
		return Contains(materials, refMaterial);
	}

	static void DrawSectorFlagLabel(const Vector3& pos, const std::string& string, const Vector4& color, float verticalOffset)
	{
		constexpr auto LABEL_SCALE = 0.8f;
		constexpr auto HALF_BLOCK  = BLOCK(0.5f);

		// Get 2D label position.
		auto labelPos = pos + Vector3(HALF_BLOCK, 0.0f, HALF_BLOCK);
		auto labelPos2D = g_Renderer.Get2DPosition(labelPos);

		// Draw label.
		if (labelPos2D.has_value())
		{
			*labelPos2D += Vector2(0.0f, verticalOffset);
			g_Renderer.AddDebugString(string, *labelPos2D, color, LABEL_SCALE, 0, RendererDebugPage::CollisionStats);
		}
	}

	void DrawNearbySectorFlags(const ItemInfo& item)
	{
		constexpr auto SECTOR_SEARCH_DEPTH = 2;
		constexpr auto STRING_SPACING	   = -20.0f;

		constexpr auto STOPPER_COLOR				 = Vector4(1.0f, 0.4f, 0.4f, 1.0f);
		constexpr auto DEATH_COLOR					 = Vector4(0.4f, 1.0f, 0.4f, 1.0f);
		constexpr auto MONKEY_SWING_COLOR			 = Vector4(1.0f, 0.4f, 0.4f, 1.0f);
		constexpr auto BEETLE_MINECART_RIGHT_COLOR	 = Vector4(0.4f, 0.4f, 1.0f, 1.0f);
		constexpr auto ACTIVATOR_MINECART_LEFT_COLOR = Vector4(1.0f, 0.4f, 1.0f, 1.0f);
		constexpr auto MINECART_STOP_COLOR			 = Vector4(0.4f, 1.0f, 1.0f, 1.0f);

		// Get point collision.
		auto pointColl = GetCollision(item);
		auto pos = item.Pose.Position.ToVector3();

		// Run through neighboring rooms.
		const auto& room = g_Level.Rooms[item.RoomNumber];
		for (int neighborRoomNumber : room.neighbors)
		{
			const auto& neighborRoom = g_Level.Rooms[neighborRoomNumber];

			// Run through neighbor sectors.
			auto roomGridCoords = GetNeighborRoomGridCoords(item.Pose.Position, neighborRoomNumber, SECTOR_SEARCH_DEPTH);
			for (const auto& roomGridCoord : roomGridCoords)
			{
				pos.x = BLOCK(roomGridCoord.x) + neighborRoom.x;
				pos.z = BLOCK(roomGridCoord.y) + neighborRoom.z;

				pointColl = GetCollision(pos, neighborRoomNumber);
				pos.y = pointColl.Position.Floor;

				float verticalOffset = STRING_SPACING;

				// Stopper
				if (pointColl.Block->Stopper)
				{
					DrawSectorFlagLabel(pos, "Stopper", STOPPER_COLOR, verticalOffset);
					verticalOffset += STRING_SPACING;
				}

				// Death
				if (pointColl.Block->Flags.Death)
				{
					DrawSectorFlagLabel(pos, "Death", DEATH_COLOR, verticalOffset);
					verticalOffset += STRING_SPACING;
				}

				// Monkey Swing
				if (pointColl.Block->Flags.Monkeyswing)
				{
					DrawSectorFlagLabel(pos, "Monkey Swing", MONKEY_SWING_COLOR, verticalOffset);
					verticalOffset += STRING_SPACING;
				}

				// Beetle / Minecart Right
				if (pointColl.Block->Flags.MarkBeetle)
				{
					auto labelString = std::string("Beetle") + (!pointColl.Block->Flags.MinecartStop() ? " / Minecart Right" : "");
					DrawSectorFlagLabel(pos, labelString, BEETLE_MINECART_RIGHT_COLOR, verticalOffset);
					verticalOffset += STRING_SPACING;
				}

				// Activator / Minecart Left
				if (pointColl.Block->Flags.MarkTriggerer)
				{
					auto labelString = std::string("Activator") + (!pointColl.Block->Flags.MinecartStop() ? " / Minecart Left" : "");
					DrawSectorFlagLabel(pos, labelString, ACTIVATOR_MINECART_LEFT_COLOR, verticalOffset);
					verticalOffset += STRING_SPACING;
				}

				// Minecart Stop
				if (pointColl.Block->Flags.MinecartStop())
				{
					DrawSectorFlagLabel(pos, "Minecart Stop", MINECART_STOP_COLOR, verticalOffset);
					verticalOffset += STRING_SPACING;
				}
			}
		}
	}
}
