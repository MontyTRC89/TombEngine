#include "framework.h"
#include "Game/collision/floordata.h"

#include "Game/collision/collide_room.h"
#include "Game/items.h"
#include "Game/room.h"
#include "Game/Setup.h"
#include "Math/Math.h"
#include "Objects/Generic/Object/BridgeObject.h"
#include "Renderer/Renderer11.h"
#include "Specific/level.h"
#include "Specific/trutils.h"

using namespace TEN::Collision::Floordata;
using namespace TEN::Entities::Generic;
using namespace TEN::Math;
using namespace TEN::Utils;

const SectorSurfaceTriangleData& FloorInfo::GetSurfaceTriangle(int x, int z, bool isFloor) const
{
	int triID = GetSurfaceTriangleID(x, z, isFloor);
	auto& tris = isFloor ? FloorSurface.Triangles : CeilingSurface.Triangles;
	return tris[triID];
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
	const auto& tri = GetSurfaceTriangle(x, z, isFloor);
	return tri.Plane.Normal();
}

Vector3 FloorInfo::GetSurfaceNormal(int triID, bool isFloor) const
{
	// Get triangle.
	const auto& tris = isFloor ? FloorSurface.Triangles : CeilingSurface.Triangles;
	const auto& tri = tris[triID];

	// Return plane normal.
	return tri.Plane.Normal();
}

short FloorInfo::GetSurfaceIllegalSlopeAngle(int x, int z, bool isFloor) const
{
	const auto& tri = GetSurfaceTriangle(x, z, isFloor);
	return tri.IllegalSlopeAngle;
}

MaterialType FloorInfo::GetSurfaceMaterial(int x, int z, bool isFloor) const
{
	const auto& tri = GetSurfaceTriangle(x, z, isFloor);
	return tri.Material;
}

bool FloorInfo::IsSurfaceSplit(bool isFloor) const
{
	const auto& tris = isFloor ? FloorSurface.Triangles : CeilingSurface.Triangles;

	// Check if surface planes aren't equal or portal is split.
	bool arePlanesEqual = (tris[0].Plane == tris[1].Plane);
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
	const auto& tris = isFloor ? FloorSurface.Triangles : CeilingSurface.Triangles;
	return (tris[0].PortalRoomNumber != tris[1].PortalRoomNumber);
}

std::optional<int> FloorInfo::GetRoomNumberBelow(int triID) const
{
	int roomNumber = FloorSurface.Triangles[triID].PortalRoomNumber;
	if (roomNumber != NO_ROOM)
		return roomNumber;

	return std::nullopt;
}

std::optional<int> FloorInfo::GetRoomNumberBelow(int x, int z) const
{
	int triID = GetSurfaceTriangleID(x, z, true);
	return GetRoomNumberBelow(triID);
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
		const auto& bridge = GetBridgeObject(bridgeItem);

		// 2.1) Get bridge floor height.
		auto bridgeFloorHeight = bridge.GetFloorHeight(bridgeItem, pos);
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

std::optional<int> FloorInfo::GetRoomNumberAbove(int triID) const
{
	int roomNumber = CeilingSurface.Triangles[triID].PortalRoomNumber;
	if (roomNumber != NO_ROOM)
		return roomNumber;

	return std::nullopt;
}

std::optional<int> FloorInfo::GetRoomNumberAbove(int x, int z) const
{
	int triID = GetSurfaceTriangleID(x, z, false);
	return GetRoomNumberAbove(triID);
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
		const auto& bridge = GetBridgeObject(bridgeItem);

		// 2.1) Get bridge ceiling height.
		auto bridgeCeilingHeight = bridge.GetCeilingHeight(bridgeItem, pos);
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
	const auto& tri = GetSurfaceTriangle(x, z, isFloor);

	// Calculate relative plane height at intersection using plane equation.
	auto sectorPoint = GetSectorPoint(x, z);
	auto normal = tri.Plane.Normal();
	float relPlaneHeight = -((normal.x * sectorPoint.x) + (normal.z * sectorPoint.y)) / normal.y;

	// Return surface height.
	return (tri.Plane.D() + relPlaneHeight);
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
		const auto& bridge = GetBridgeObject(bridgeItem);

		// 2.1) Get bridge surface height.
		auto bridgeSurfaceHeight = isFloor ? bridge.GetFloorHeight(bridgeItem, pos) : bridge.GetCeilingHeight(bridgeItem, pos);
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
		const auto& bridge = GetBridgeObject(bridgeItem);

		// 1.1) Get bridge floor and ceiling heights.
		auto floorHeight = bridge.GetFloorHeight(bridgeItem, pos);
		auto ceilingHeight = bridge.GetCeilingHeight(bridgeItem, pos);
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

bool FloorInfo::IsWall(int triID) const
{
	bool areSplitAnglesEqual = (FloorSurface.SplitAngle == CeilingSurface.SplitAngle);
	bool areNormalsParallel = (FloorSurface.Triangles[triID].Plane.Normal() == -CeilingSurface.Triangles[triID].Plane.Normal());
	bool areDistsEqual = (FloorSurface.Triangles[triID].Plane.D() == CeilingSurface.Triangles[triID].Plane.D());

	return (areSplitAnglesEqual && areNormalsParallel && areDistsEqual);
}

bool FloorInfo::IsWall(int x, int z) const
{
	int triID = GetSurfaceTriangleID(x, z, true);
	return IsWall(triID);
}

int FloorInfo::GetInsideBridgeItemNumber(const Vector3i& pos, bool testFloorBorder, bool testCeilingBorder) const
{
	// 1) Find and return intersected bridge item number (if applicable).
	for (int itemNumber : BridgeItemNumbers)
	{
		const auto& bridgeItem = g_Level.Items[itemNumber];
		const auto& bridge = GetBridgeObject(bridgeItem);

		// 1.1) Get bridge floor and ceiling heights.
		auto floorHeight = bridge.GetFloorHeight(bridgeItem, pos);
		auto ceilingHeight = bridge.GetCeilingHeight(bridgeItem, pos);
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
		// Scale normal to original fake plane length.
		float scaleFactor = 1.0f / normal.y;
		auto scaledNormal = normal * scaleFactor;

		// Calculate and return tilt.
		auto sign = isFloor ? 1 : -1;
		return Vector2i(
			round(scaledNormal.x * 4),
			round(scaledNormal.z * 4)) * sign;
	}

	Vector2i GetSectorCenter(int x, int z)
	{
		return Vector2i(
			((x / BLOCK(1)) * BLOCK(1)) + BLOCK(0.5f),
			((z / BLOCK(1)) * BLOCK(1)) + BLOCK(0.5f));
	}

	Vector2i GetSectorPoint(int x, int z)
	{
		// Return relative 2D point in range [0, BLOCK(1)).
		return Vector2i(
			(x % BLOCK(1)) - (int)BLOCK(0.5f),
			(z % BLOCK(1)) - (int)BLOCK(0.5f));
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

		// Determine search area bounds.
		int xMax = originRoomGridCoord.x + searchDepth;
		int xMin = originRoomGridCoord.x - searchDepth;
		int zMax = originRoomGridCoord.y + searchDepth;
		int zMin = originRoomGridCoord.y - searchDepth;

		const auto& room = g_Level.Rooms[roomNumber];

		// Search area out of range; return empty vector.
		if (xMax <= 0 || xMin >= (room.xSize - 1) ||
			xMax <= 0 || xMin >= (room.xSize - 1))
		{
			return {};
		}

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

		int sectorID = (room.zSize * roomGridCoord.x) + roomGridCoord.y;
		return room.floor[sectorID];
	}

	FloorInfo& GetFloor(int roomNumber, int x, int z)
	{
		auto roomGridCoord = GetRoomGridCoord(roomNumber, x, z);
		return GetFloor(roomNumber, roomGridCoord);
	}

	FloorInfo& GetFloorSide(int roomNumber, int x, int z, int* sideRoomNumberPtr)
	{
		auto* sectorPtr = &GetFloor(roomNumber, x, z);

		// Find side sector.
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
		
		// Find bottom sector.
		bool isWall = sectorPtr->IsWall(x, z);
		while (isWall)
		{
			auto roomNumberBelow = sectorPtr->GetRoomNumberBelow(x, z);
			if (!roomNumberBelow.has_value())
				break;

			// TODO: Check.
			sectorPtr = &GetFloorSide(*roomNumberBelow, x, z, bottomRoomNumberPtr);
			isWall = sectorPtr->IsWall(x, z);
		}

		return *sectorPtr;
	}

	FloorInfo& GetTopFloor(int roomNumber, int x, int z, int* topRoomNumberPtr)
	{
		auto* sectorPtr = &GetFloorSide(roomNumber, x, z, topRoomNumberPtr);
		
		// Find top sector.
		bool isWall = sectorPtr->IsWall(x, z);
		while (isWall)
		{
			auto roomNumberAbove = sectorPtr->GetRoomNumberAbove(x, z);
			if (!roomNumberAbove)
				break;

			// TODO: Check.
			sectorPtr = &GetFloorSide(*roomNumberAbove, x, z, topRoomNumberPtr);
			isWall = sectorPtr->IsWall(x, z);
		}

		return *sectorPtr;
	}

	std::optional<int> GetBottomHeight(FloorInfo& startSector, Vector3i pos, int* bottomRoomNumberPtr, FloorInfo** bottomSectorPtr)
	{
		int roomNumber = (bottomRoomNumberPtr != nullptr) ? *bottomRoomNumberPtr : 0;

		// Find bottom height.
		auto* sectorPtr = &startSector;
		do
		{
			// Set vertical position to lowest bridge ceiling height.
			pos.y = sectorPtr->GetBridgeSurfaceHeight(pos, false);

			// Find sector at lowest bridge floor height.
			while (pos.y >= sectorPtr->GetSurfaceHeight(pos.x, pos.z, true))
			{
				auto roomNumberBelow = sectorPtr->GetRoomNumberBelow(pos.x, pos.z);
				if (!roomNumberBelow.has_value())
					return std::nullopt;

				sectorPtr = &GetFloorSide(*roomNumberBelow, pos.x, pos.z, &roomNumber);
			}
		}
		// Continue running while bridge exists(?).
		while (sectorPtr->GetInsideBridgeItemNumber(pos, true, false) != NO_ITEM);

		// Set output bottom room number.
		if (bottomRoomNumberPtr != nullptr)
			*bottomRoomNumberPtr = roomNumber;

		// Set output bottom sector pointer.
		if (bottomSectorPtr != nullptr)
			*bottomSectorPtr = sectorPtr;

		return pos.y;
	}

	std::optional<int> GetTopHeight(FloorInfo& startSector, Vector3i pos, int* topRoomNumberPtr, FloorInfo** topSectorPtr)
	{
		int roomNumber = (topRoomNumberPtr != nullptr) ? *topRoomNumberPtr : 0;

		// Find top height.
		auto* sectorPtr = &startSector;
		do
		{
			// Set vertical position to highest bridge floor height.
			pos.y = sectorPtr->GetBridgeSurfaceHeight(pos, true);

			// Find sector at highest bridge ceiling height.
			while (pos.y <= sectorPtr->GetSurfaceHeight(pos.x, pos.z, false))
			{
				auto roomNumberAbove = sectorPtr->GetRoomNumberAbove(pos.x, pos.z);
				if (!roomNumberAbove.has_value())
					return std::nullopt;

				sectorPtr = &GetFloorSide(*roomNumberAbove, pos.x, pos.z, &roomNumber);
			}
		}
		// Continue running while bridge exists(?).
		while (sectorPtr->GetInsideBridgeItemNumber(pos, false, true) >= 0);

		// Set output top room number.
		if (topRoomNumberPtr != nullptr)
			*topRoomNumberPtr = roomNumber;

		// Set output top sector pointer.
		if (topSectorPtr != nullptr)
			*topSectorPtr = sectorPtr;

		return pos.y;
	}

	std::optional<int> GetSurfaceHeight(const RoomVector& location, int x, int z, bool isFloor)
	{
		auto* sectorPtr = &GetFloorSide(location.RoomNumber, x, z);

		auto pos = Vector3i(x, location.Height, z);
		int polarity = 0;

		if (sectorPtr->IsWall(x, z))
		{
			sectorPtr = isFloor ? &GetTopFloor(location.RoomNumber, x, z) : &GetBottomFloor(location.RoomNumber, x, z);

			if (!sectorPtr->IsWall(x, z))
			{
				pos.y = sectorPtr->GetSurfaceHeight(x, z, isFloor);
				polarity = isFloor ? -1 : 1;
			}
			else
			{
				sectorPtr = isFloor ? &GetBottomFloor(location.RoomNumber, x, z) : &GetTopFloor(location.RoomNumber, x, z);

				if (!sectorPtr->IsWall(x, z))
				{
					pos.y = sectorPtr->GetSurfaceHeight(x, z, !isFloor);
					polarity = isFloor ? 1 : -1;
				}
				else
				{
					return std::nullopt;
				}
			}
		}

		int floorHeight = sectorPtr->GetSurfaceHeight(pos, true);
		int ceilingHeight = sectorPtr->GetSurfaceHeight(pos, false);

		pos.y = std::clamp(pos.y, std::min(floorHeight, ceilingHeight), std::max(floorHeight, ceilingHeight));

		bool testFloorBorder = (pos.y == ceilingHeight);
		bool testCeilBorder = (pos.y == floorHeight);
		int insideBridgeItemNumber = sectorPtr->GetInsideBridgeItemNumber(pos, testFloorBorder, testCeilBorder);

		if (insideBridgeItemNumber != NO_ITEM)
		{
			if (isFloor ? (polarity <= 0) : (polarity >= 0))
			{
				auto heightBound = isFloor ? GetTopHeight(*sectorPtr, pos) : GetBottomHeight(*sectorPtr, pos);
				if (heightBound.has_value())
					return heightBound;
			}

			if (isFloor ? (polarity >= 0) : (polarity <= 0))
			{
				auto heightBound = isFloor ?
					GetBottomHeight(*sectorPtr, pos, nullptr, &sectorPtr) :
					GetTopHeight(*sectorPtr, pos, nullptr, &sectorPtr);

				if (!heightBound.has_value())
					return std::nullopt;

				pos.y = *heightBound;
			}
		}

		if (isFloor ? (polarity >= 0) : (polarity <= 0))
		{
			auto nextRoomNumber = isFloor ? sectorPtr->GetRoomNumberBelow(pos) : sectorPtr->GetRoomNumberAbove(pos);
			while (nextRoomNumber.has_value())
			{
				sectorPtr = &GetFloorSide(*nextRoomNumber, x, z);
				nextRoomNumber = isFloor ? sectorPtr->GetRoomNumberBelow(pos) : sectorPtr->GetRoomNumberAbove(pos);
			}
		}

		return sectorPtr->GetSurfaceHeight(pos, isFloor);
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
		const auto& bridge = GetBridgeObject(bridgeItem);

		if (!Objects.CheckID(bridgeItem.ObjectNumber))
			return;

		x += bridgeItem.Pose.Position.x;
		z += bridgeItem.Pose.Position.z;

		auto* sectorPtr = &GetFloorSide(bridgeItem.RoomNumber, x, z);
		sectorPtr->AddBridge(itemNumber);

		if (bridge.GetFloorBorder != nullptr)
		{
			int floorBorder = bridge.GetFloorBorder(bridgeItem);
			while (floorBorder <= sectorPtr->GetSurfaceHeight(x, z, false))
			{
				auto roomNumberAbove = sectorPtr->GetRoomNumberAbove(x, z);
				if (!roomNumberAbove.has_value())
					break;

				sectorPtr = &GetFloorSide(*roomNumberAbove, x, z);
				sectorPtr->AddBridge(itemNumber);
			}
		}
		
		if (bridge.GetCeilingBorder != nullptr)
		{
			int ceilingBorder = bridge.GetCeilingBorder(bridgeItem);
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
		const auto& bridge = GetBridgeObject(bridgeItem);

		if (!Objects.CheckID(bridgeItem.ObjectNumber))
			return;

		x += bridgeItem.Pose.Position.x;
		z += bridgeItem.Pose.Position.z;

		auto* sectorPtr = &GetFloorSide(bridgeItem.RoomNumber, x, z);
		sectorPtr->RemoveBridge(itemNumber);

		if (bridge.GetFloorBorder != nullptr)
		{
			int floorBorder = bridge.GetFloorBorder(bridgeItem);
			while (floorBorder <= sectorPtr->GetSurfaceHeight(x, z, false))
			{
				auto roomNumberAbove = sectorPtr->GetRoomNumberAbove(x, z);
				if (!roomNumberAbove.has_value())
					break;

				sectorPtr = &GetFloorSide(*roomNumberAbove, x, z);
				sectorPtr->RemoveBridge(itemNumber);
			}
		}

		if (bridge.GetCeilingBorder != nullptr)
		{
			int ceilingBorder = bridge.GetCeilingBorder(bridgeItem);
			while (ceilingBorder >= sectorPtr->GetSurfaceHeight(x, z, true))
			{
				auto roomNumberBelow = sectorPtr->GetRoomNumberBelow(x, z);
				if (!roomNumberBelow.has_value())
					break;

				sectorPtr = &GetFloorSide(*roomNumberBelow, x, z);
				sectorPtr->RemoveBridge(itemNumber);
			}
		}
	}

	// Get precise floor/ceiling height from object's bounding box.
	// Animated objects are also supported, although horizontal collision shifting is unstable.
	// Method: get accurate bounds in world transform by converting to OBB, then do a ray test
	// on top or bottom (depending on test side) to determine if box is present at a particular point.
	std::optional<int> GetBridgeItemIntersect(const ItemInfo& item, const Vector3i& pos, bool useBottomHeight)
	{
		constexpr auto VERTICAL_MARGIN = 4;

		auto bounds = GameBoundingBox(&item);
		auto box = bounds.ToBoundingOrientedBox(item.Pose);
		
		auto origin = Vector3(pos.x, pos.y + (useBottomHeight ? VERTICAL_MARGIN : -VERTICAL_MARGIN), pos.z);
		auto dir = useBottomHeight ? -Vector3::UnitY : Vector3::UnitY;

		// Ray intersects box; return bridge box height.
		float dist = 0.0f;
		if (box.Intersects(origin, dir, dist))
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
		constexpr auto SECTOR_EXTENTS = Vector3(BLOCK(0.5f));

		if (!Objects.CheckID(item.ObjectNumber))
			return;

		if (!Objects[item.ObjectNumber].loaded)
			return;

		// Force removal if item was killed.
		if (item.Flags & IFLAG_KILLED)
			forceRemoval = true;

		// Get bridge OBB.
		auto bridgeBox = GameBoundingBox(&item).ToBoundingOrientedBox(item.Pose);

		// Get bridge OBB corners. NOTE: only 0, 1, 4, 5 are relevant.
		auto corners = std::array<Vector3, 8>{};
		bridgeBox.GetCorners(corners.data());

		const auto& room = g_Level.Rooms[item.RoomNumber];

		// Get projected AABB min and max of bridge OBB.
		float xMin = floor((std::min(std::min(std::min(corners[0].x, corners[1].x), corners[4].x), corners[5].x) - room.x) / BLOCK(1));
		float zMin = floor((std::min(std::min(std::min(corners[0].z, corners[1].z), corners[4].z), corners[5].z) - room.z) / BLOCK(1));
		float xMax =  ceil((std::max(std::max(std::max(corners[0].x, corners[1].x), corners[4].x), corners[5].x) - room.x) / BLOCK(1));
		float zMax =  ceil((std::max(std::max(std::max(corners[0].z, corners[1].z), corners[4].z), corners[5].z) - room.z) / BLOCK(1));

		// Run through sectors enclosed in projected bridge AABB.
		for (int x = 0; x < room.xSize; x++)
		{
			for (int z = 0; z < room.zSize; z++)
			{
				float pX = (room.x + BLOCK(x)) + BLOCK(0.5f);
				float pZ = (room.z + BLOCK(z)) + BLOCK(0.5f);
				float offX = pX - item.Pose.Position.x;
				float offZ = pZ - item.Pose.Position.z;

				// Clean previous bridge state.
				RemoveBridge(item.Index, offX, offZ);

				// In sweep mode; don't try readding to sector.
				if (forceRemoval)
					continue;

				// Sector is outside enclosed AABB space; ignore precise check.
				if (x < xMin || z < zMin ||
					x > xMax || z > zMax)
				{
					continue;
				}

				// Sector is in enclosed bridge AABB space; do more precise test.
				// Construct OBB within same plane as bridge OBB and test intersection.
				auto sectorBox = BoundingOrientedBox(Vector3(pX, bridgeBox.Center.y, pZ), SECTOR_EXTENTS, Vector4::UnitY);

				// Add bridge to current sector if intersection is valid.
				if (bridgeBox.Intersects(sectorBox))
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
