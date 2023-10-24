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

int FloorInfo::GetSurfacePlaneIndex(int x, int z, bool isFloor) const
{
	// Calculate bias.
	auto point = GetSectorPoint(x, z).ToVector2();
	auto rotMatrix = Matrix::CreateRotationZ(isFloor ? FloorCollision.SplitAngle : CeilingCollision.SplitAngle);
	Vector2::Transform(point, rotMatrix, point);

	// Determine and return plane index.
	return ((point.x < 0) ? 0 : 1);
}

Vector2 FloorInfo::GetSurfaceTilt(int x, int z, bool isFloor) const
{
	// Get surface plane.
	const auto& planes = isFloor ? FloorCollision.Planes : CeilingCollision.Planes;
	int planeIndex = GetSurfacePlaneIndex(x, z, true); // TODO: Check why it only looks at floor planes.
	auto plane = planes[planeIndex];

	// Calculate and return plane tilt.
	return Vector2(
		-int((plane.x * BLOCK(1)) / CLICK(1)),
		-int((plane.y * BLOCK(1)) / CLICK(1)));
}

bool FloorInfo::IsSurfaceSplit(bool isFloor) const
{
	const auto& planes = isFloor ? FloorCollision.Planes : CeilingCollision.Planes;
	bool arePlanesDifferent = (planes[0] != planes[1]);

	// Check if surface planes are different and portal is split.
	return (arePlanesDifferent || IsSurfaceSplitPortal(isFloor));
}

bool FloorInfo::IsSurfaceDiagonalStep(bool isFloor) const
{
	// Check if surface is split.
	if (!IsSurfaceSplit(isFloor))
		return false;

	const auto& surfaceColl = isFloor ? FloorCollision : CeilingCollision;
	
	// Check if ??
	if (round(surfaceColl.Planes[0].z) == round(surfaceColl.Planes[1].z))
		return false;

	// Check if ??
	if (surfaceColl.SplitAngle != SurfaceCollisionData::SPLIT_ANGLE_0 &&
		surfaceColl.SplitAngle != SurfaceCollisionData::SPLIT_ANGLE_1)
	{
		return false;
	}

	return true;
}

bool FloorInfo::IsSurfaceSplitPortal(bool isFloor) const
{
	// Check if surface portals are different.
	const auto& portals = isFloor ? FloorCollision.Portals : CeilingCollision.Portals;
	return (portals[0] != portals[1]);
}

std::optional<int> FloorInfo::GetRoomNumberBelow(int planeIndex) const
{
	int roomNumber = FloorCollision.Portals[planeIndex];
	return ((roomNumber != NO_ROOM) ? std::optional(roomNumber) : std::nullopt);
}

std::optional<int> FloorInfo::GetRoomNumberBelow(int x, int z) const
{
	int planeIndex = GetSurfacePlaneIndex(x, z, true);
	return GetRoomNumberBelow(planeIndex);
}

std::optional<int> FloorInfo::GetRoomNumberBelow(const Vector3i& pos) const
{
	// Get surface heights.
	int floorHeight = GetSurfaceHeight(pos.x, pos.z, true);
	int ceilingHeight = GetSurfaceHeight(pos.x, pos.z, false);

	// Loop through bridges.
	for (int i : BridgeItemNumbers)
	{
		const auto& bridgeItem = g_Level.Items[i];
		const auto& bridgeObject = Objects[bridgeItem.ObjectNumber];

		// Get bridge floor height.
		auto bridgeFloorHeight = bridgeObject.floor(i, pos.x, pos.y, pos.z);
		if (!bridgeFloorHeight.has_value())
			continue;

		// Assess relation of bridge to collision block.
		if (*bridgeFloorHeight >= pos.y &&		 // Below input height bound.
			*bridgeFloorHeight <= floorHeight && // Within floor bound.
			*bridgeFloorHeight >= ceilingHeight) // Within ceiling bound.
		{
			return std::nullopt;
		}
	}

	return GetRoomNumberBelow(pos.x, pos.z);
}

std::optional<int> FloorInfo::GetRoomNumberAbove(int planeIndex) const
{
	int roomNumber = CeilingCollision.Portals[planeIndex];
	return ((roomNumber != NO_ROOM) ? std::optional(roomNumber) : std::nullopt);
}

std::optional<int> FloorInfo::GetRoomNumberAbove(int x, int z) const
{
	int planeIndex = GetSurfacePlaneIndex(x, z, false);
	return GetRoomNumberAbove(planeIndex);
}

std::optional<int> FloorInfo::GetRoomNumberAbove(const Vector3i& pos) const
{
	// Get surface heights.
	int floorHeight = GetSurfaceHeight(pos.x, pos.z, true);
	int ceilingHeight = GetSurfaceHeight(pos.x, pos.z, false);

	// Loop through bridges.
	for (int i : BridgeItemNumbers)
	{
		const auto& bridgeItem = g_Level.Items[i];
		const auto& bridgeObject = Objects[bridgeItem.ObjectNumber];

		// Get bridge ceiling height.
		auto bridgeCeilingHeight = bridgeObject.ceiling(i, pos.x, pos.y, pos.z);
		if (!bridgeCeilingHeight.has_value())
			continue;

		// Assess relation of bridge to collision block.
		if (*bridgeCeilingHeight <= pos.y &&	   // Above input height bound.
			*bridgeCeilingHeight <= floorHeight && // Within floor bound.
			*bridgeCeilingHeight >= ceilingHeight) // Within ceiling bound.
		{
			return std::nullopt;
		}
	}

	return GetRoomNumberAbove(pos.x, pos.z);
}

std::optional<int> FloorInfo::GetRoomNumberAtSide() const
{
	return ((WallPortal != NO_ROOM) ? std::optional(WallPortal) : std::nullopt);
}

int FloorInfo::GetSurfaceHeight(int x, int z, bool isFloor) const
{
	const auto& planes = isFloor ? FloorCollision.Planes : CeilingCollision.Planes;

	// Get surface plane.
	int planeIndex = GetSurfacePlaneIndex(x, z, isFloor);
	const auto& plane = planes[planeIndex];

	auto point = GetSectorPoint(x, z);

	// Return surface height.
	return ((plane.x * point.x) +
			(plane.y * point.y) +
			plane.z);
}

int FloorInfo::GetSurfaceHeight(const Vector3i& pos, bool isFloor) const
{
	// Get surface heights.
	int floorHeight = GetSurfaceHeight(pos.x, pos.z, true);
	int ceilingHeight = GetSurfaceHeight(pos.x, pos.z, false);

	// Loop through bridges.
	for (int i : BridgeItemNumbers)
	{
		const auto& bridgeItem = g_Level.Items[i];
		const auto& bridgeObject = Objects[bridgeItem.ObjectNumber];

		// Get bridge surface height.
		auto bridgeSurfaceHeight = isFloor ? bridgeObject.floor(i, pos.x, pos.y, pos.z) : bridgeObject.ceiling(i, pos.x, pos.y, pos.z);
		if (!bridgeSurfaceHeight.has_value())
			continue;

		// Assess relation of bridge to collision block.
		if (isFloor)
		{
			if (*bridgeSurfaceHeight >= pos.y &&	   // Below input height bound.
				*bridgeSurfaceHeight < floorHeight &&  // Within floor bound.
				*bridgeSurfaceHeight >= ceilingHeight) // Within ceiling bound.
			{
				floorHeight = *bridgeSurfaceHeight;
			}
		}
		else
		{
			if (*bridgeSurfaceHeight <= pos.y &&	   // Above input height bound.
				*bridgeSurfaceHeight <= floorHeight && // Within floor bound.
				*bridgeSurfaceHeight > ceilingHeight)  // Within ceiling bound.
			{
				ceilingHeight = *bridgeSurfaceHeight;
			}
		}
	}

	// Return surface height.
	return (isFloor ? floorHeight : ceilingHeight);
}

int FloorInfo::GetBridgeSurfaceHeight(const Vector3i& pos, bool isFloor) const
{
	// Loop through bridges.
	for (int i : BridgeItemNumbers)
	{
		const auto& bridgeItem = g_Level.Items[i];
		const auto& bridgeObject = Objects[bridgeItem.ObjectNumber];

		// Get surface heights.
		auto floorHeight = bridgeObject.floor(i, pos.x, pos.y, pos.z);
		auto ceilingHeight = bridgeObject.ceiling(i, pos.x, pos.y, pos.z);
		if (!floorHeight.has_value() || !ceilingHeight.has_value())
			continue;

		// Assess relation of bridge to collision block.
		if (isFloor)
		{
			if (pos.y > *floorHeight &&
				pos.y <= *ceilingHeight)
			{
				return *floorHeight;
			}
		}
		else
		{
			if (pos.y >= *floorHeight &&
				pos.y < *ceilingHeight)
			{
				return *ceilingHeight;
			}
		}
	}
	
	// Return bridge surface height.
	return GetSurfaceHeight(pos, isFloor);
}

Vector2 FloorInfo::GetSurfaceSlope(int planeIndex, bool isFloor) const
{
	const auto& plane = isFloor ? FloorCollision.Planes[planeIndex] : CeilingCollision.Planes[planeIndex];
	return Vector2(plane.x, plane.y);
}

Vector2 FloorInfo::GetSurfaceSlope(int x, int z, bool isFloor) const
{
	int planeIndex = GetSurfacePlaneIndex(x, z, isFloor);
	return GetSurfaceSlope(planeIndex, isFloor);
}

bool FloorInfo::IsWall(int planeIndex) const
{
	bool areSplitAnglesEqual = (FloorCollision.SplitAngle == CeilingCollision.SplitAngle);
	bool arePlanesEqual = (FloorCollision.Planes[planeIndex] == CeilingCollision.Planes[planeIndex]);
	return (areSplitAnglesEqual && arePlanesEqual);
}

bool FloorInfo::IsWall(int x, int z) const
{
	int planeIndex = GetSurfacePlaneIndex(x, z, true);
	return IsWall(planeIndex);
}

int FloorInfo::GetInsideBridgeItemNumber(const Vector3i& pos, bool testFloorBorder, bool testCeilingBorder) const
{
	for (int itemNumber : BridgeItemNumbers)
	{
		const auto& bridgeItem = g_Level.Items[itemNumber];
		const auto& bridgeObject = Objects[bridgeItem.ObjectNumber];

		// Get surface heights.
		auto floorHeight = bridgeObject.floor(itemNumber, pos.x, pos.y, pos.z);
		auto ceilingHeight = bridgeObject.ceiling(itemNumber, pos.x, pos.y, pos.z);
		if (!floorHeight.has_value() || !ceilingHeight.has_value())
			continue;

		if (pos.y > *floorHeight &&
			pos.y < *ceilingHeight)
		{
			return itemNumber;
		}

		if ((testFloorBorder && pos.y == *floorHeight) ||
			(testCeilingBorder && pos.y == *ceilingHeight))
		{
			return itemNumber;
		}
	}

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
	Vector3 GetSurfaceNormal(const Vector2& tilt, bool isFloor)
	{
		int sign = isFloor ? -1 : 1;
		auto normal = Vector3(tilt.x / 4, 1.0f, tilt.y / 4) * sign;
		normal.Normalize();
		return normal;
	}

	Vector2i GetSectorPoint(int x, int z)
	{
		const auto xPoint = x % BLOCK(1) - BLOCK(1) / 2;
		const auto yPoint = z % BLOCK(1) - BLOCK(1) / 2;

		return Vector2i{xPoint, yPoint};
	}

	Vector2i GetRoomPosition(int roomNumber, int x, int z)
	{
		const auto& room = g_Level.Rooms[roomNumber];
		const auto zRoom = (z - room.z) / BLOCK(1);
		const auto xRoom = (x - room.x) / BLOCK(1);
		auto pos = Vector2i{xRoom, zRoom};

		if (pos.x < 0)
		{
			pos.x = 0;
		}
		else if (pos.x > room.xSize - 1)
		{
			pos.x = room.xSize - 1;
		}

		if (pos.y < 0)
		{
			pos.y = 0;
		}
		else if (pos.y > room.zSize - 1)
		{
			pos.y = room.zSize - 1;
		}

		return pos;
	}

	FloorInfo& GetFloor(int roomNumber, const Vector2i& roomPos)
	{
		auto& room = g_Level.Rooms[roomNumber];
		return room.floor[(room.zSize * roomPos.x) + roomPos.y];
	}

	FloorInfo& GetFloor(int roomNumber, int x, int z)
	{
		return GetFloor(roomNumber, GetRoomPosition(roomNumber, x, z));
	}

	FloorInfo& GetFloorSide(int roomNumber, int x, int z, int* sideRoomNumber)
	{
		auto floor = &GetFloor(roomNumber, x, z);

		auto roomSide = floor->GetRoomNumberAtSide();
		while (roomSide)
		{
			roomNumber = *roomSide;
			floor = &GetFloor(roomNumber, x, z);
			roomSide = floor->GetRoomNumberAtSide();
		}

		if (sideRoomNumber)
			*sideRoomNumber = roomNumber;

		return *floor;
	}

	FloorInfo& GetBottomFloor(int roomNumber, int x, int z, int* bottomRoomNumber)
	{
		auto floor = &GetFloorSide(roomNumber, x, z, bottomRoomNumber);
		auto wall = floor->IsWall(x, z);
		while (wall)
		{
			const auto roomBelow = floor->GetRoomNumberBelow(x, z);
			if (!roomBelow)
				break;

			floor = &GetFloorSide(*roomBelow, x, z, bottomRoomNumber);
			wall = floor->IsWall(x, z);
		}

		return *floor;
	}

	FloorInfo& GetTopFloor(int roomNumber, int x, int z, int* topRoomNumber)
	{
		auto floor = &GetFloorSide(roomNumber, x, z, topRoomNumber);
		auto wall = floor->IsWall(x, z);
		while (wall)
		{
			const auto roomAbove = floor->GetRoomNumberAbove(x, z);
			if (!roomAbove)
				break;

			floor = &GetFloorSide(*roomAbove, x, z, topRoomNumber);
			wall = floor->IsWall(x, z);
		}

		return *floor;
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

		if (bottomRoomNumberPtr)
			*bottomRoomNumberPtr = roomNumber;

		if (bottomSectorPtr)
			*bottomSectorPtr = sectorPtr;

		return pos.y;
	}

	std::optional<int> GetFloorHeight(const ROOM_VECTOR& location, int x, int z)
	{
		auto floor = &GetFloorSide(location.roomNumber, x, z);
		auto y = location.yNumber;
		auto direction = 0;

		if (floor->IsWall(x, z))
		{
			floor = &GetTopFloor(location.roomNumber, x, z);

			if (!floor->IsWall(x, z))
			{
				y = floor->GetSurfaceHeight(x, z, true);
				direction = -1;
			}
			else
			{
				floor = &GetBottomFloor(location.roomNumber, x, z);

				if (!floor->IsWall(x, z))
				{
					y = floor->GetSurfaceHeight(x, z, false);
					direction = 1;
				}
				else
				{
					return std::nullopt;
				}
			}
		}

		const auto floorHeight = floor->GetSurfaceHeight(Vector3i(x, y, z), true);
		const auto ceilingHeight = floor->GetSurfaceHeight(Vector3i(x, y, z), false);

		y = std::clamp(y, std::min(ceilingHeight, floorHeight), std::max(ceilingHeight, floorHeight));

		if (floor->GetInsideBridgeItemNumber(Vector3i(x, y, z), y == ceilingHeight, y == floorHeight) >= 0)
		{
			if (direction <= 0)
			{
				auto height = GetTopHeight(*floor, Vector3i(x, y, z));
				if (height)
					return height;
			}

			if (direction >= 0)
			{
				auto height = GetBottomHeight(*floor, Vector3i(x, y, z), nullptr, &floor);
				if (!height)
					return std::nullopt;

				y = *height;
			}
		}

		if (direction >= 0)
		{
			auto roomBelow = floor->GetRoomNumberBelow(Vector3i(x, y, z));
			while (roomBelow)
			{
				floor = &GetFloorSide(*roomBelow, x, z);
				roomBelow = floor->GetRoomNumberBelow(Vector3i(x, y, z));
			}
		}

		return floor->GetSurfaceHeight(Vector3i(x, y, z), true);
	}

	std::optional<int> GetCeilingHeight(const ROOM_VECTOR& location, int x, int z)
	{
		auto floor = &GetFloorSide(location.roomNumber, x, z);
		auto y = location.yNumber;
		auto direction = 0;

		if (floor->IsWall(x, z))
		{
			floor = &GetBottomFloor(location.roomNumber, x, z);

			if (!floor->IsWall(x, z))
			{
				y = floor->GetSurfaceHeight(x, z, false);
				direction = 1;
			}
			else
			{
				floor = &GetTopFloor(location.roomNumber, x, z);

				if (!floor->IsWall(x, z))
				{
					y = floor->GetSurfaceHeight(x, z, true);
					direction = -1;
				}
				else
				{
					return std::nullopt;
				}
			}
		}

		const auto floorHeight = floor->GetSurfaceHeight(Vector3i(x, y, z), true);
		const auto ceilingHeight = floor->GetSurfaceHeight(Vector3i(x, y, z), false);

		y = std::clamp(y, std::min(ceilingHeight, floorHeight), std::max(ceilingHeight, floorHeight));

		if (floor->GetInsideBridgeItemNumber(Vector3i(x, y, z), y == ceilingHeight, y == floorHeight) >= 0)
		{
			if (direction >= 0)
			{
				auto height = GetBottomHeight(*floor, Vector3i(x, y, z));
				if (height)
					return height;
			}

			if (direction <= 0)
			{
				auto height = GetTopHeight(*floor, Vector3i(x, y, z), nullptr, &floor);
				if (!height)
					return std::nullopt;

				y = *height;
			}
		}

		if (direction <= 0)
		{
			auto roomAbove = floor->GetRoomNumberAbove(Vector3i(x, y, z));
			while (roomAbove)
			{
				floor = &GetFloorSide(*roomAbove, x, z);
				roomAbove = floor->GetRoomNumberAbove(Vector3i(x, y, z));
			}
		}

		return floor->GetSurfaceHeight(Vector3i(x, y, z), false);
	}

	std::optional<ROOM_VECTOR> GetBottomRoom(ROOM_VECTOR location, const Vector3i& pos)
	{
		auto floor = &GetFloorSide(location.roomNumber, pos.x, pos.z, &location.roomNumber);

		if (floor->IsWall(pos.x, pos.z))
		{
			floor = &GetBottomFloor(location.roomNumber, pos.x, pos.z, &location.roomNumber);

			if (floor->IsWall(pos.x, pos.z))
				return std::nullopt;

			location.yNumber = floor->GetSurfaceHeight(pos.x, pos.z, false);
		}

		auto floorHeight = floor->GetSurfaceHeight(Vector3i(pos.x, location.yNumber, pos.z), true);
		auto ceilingHeight = floor->GetSurfaceHeight(Vector3i(pos.x, location.yNumber, pos.z), false);

		location.yNumber = std::clamp(location.yNumber, std::min(ceilingHeight, floorHeight), std::max(ceilingHeight, floorHeight));

		if (floor->GetInsideBridgeItemNumber(Vector3i(pos.x, location.yNumber, pos.z), location.yNumber == ceilingHeight, location.yNumber == floorHeight) >= 0)
		{
			const auto height = GetBottomHeight(*floor, Vector3i(pos.x, location.yNumber, pos.z), &location.roomNumber, &floor);
			if (!height)
				return std::nullopt;

			location.yNumber = *height;
		}

		floorHeight = floor->GetSurfaceHeight(Vector3i(pos.x, location.yNumber, pos.z), true);
		ceilingHeight = floor->GetSurfaceHeight(Vector3i(pos.x, location.yNumber, pos.z), false);

		if (pos.y < ceilingHeight && floor->GetRoomNumberAbove(Vector3i(pos.x, location.yNumber, pos.z)))
			return std::nullopt;

		if (pos.y <= floorHeight)
		{
			location.yNumber = std::max(pos.y, ceilingHeight);
			return location;
		}

		auto roomBelow = floor->GetRoomNumberBelow(Vector3i(pos.x, location.yNumber, pos.z));
		while (roomBelow)
		{
			floor = &GetFloorSide(*roomBelow, pos.x, pos.z, &location.roomNumber);
			location.yNumber = floor->GetSurfaceHeight(pos.x, pos.z, false);

			floorHeight = floor->GetSurfaceHeight(Vector3i(pos.x, location.yNumber, pos.z), true);
			ceilingHeight = floor->GetSurfaceHeight(Vector3i(pos.x, location.yNumber, pos.z), false);

			if (pos.y < ceilingHeight && floor->GetRoomNumberAbove(Vector3i(pos.x, location.yNumber, pos.z)))
				return std::nullopt;

			if (pos.y <= floorHeight)
			{
				location.yNumber = std::max(pos.y, ceilingHeight);
				return location;
			}

			roomBelow = floor->GetRoomNumberBelow(Vector3i(pos.x, location.yNumber, pos.z));
		}

		return std::nullopt;
	}

	std::optional<ROOM_VECTOR> GetTopRoom(ROOM_VECTOR location, const Vector3i& pos)
	{
		auto floor = &GetFloorSide(location.roomNumber, pos.x, pos.z, &location.roomNumber);

		if (floor->IsWall(pos.x, pos.z))
		{
			floor = &GetTopFloor(location.roomNumber, pos.x, pos.z, &location.roomNumber);

			if (floor->IsWall(pos.x, pos.z))
				return std::nullopt;

			location.yNumber = floor->GetSurfaceHeight(pos.x, pos.z, true);
		}

		auto floorHeight = floor->GetSurfaceHeight(Vector3i(pos.x, location.yNumber, pos.z), true);
		auto ceilingHeight = floor->GetSurfaceHeight(Vector3i(pos.x, location.yNumber, pos.z), false);

		location.yNumber = std::clamp(location.yNumber, std::min(ceilingHeight, floorHeight), std::max(ceilingHeight, floorHeight));

		if (floor->GetInsideBridgeItemNumber(Vector3i(pos.x, location.yNumber, pos.z), location.yNumber == ceilingHeight, location.yNumber == floorHeight) >= 0)
		{
			const auto height = GetTopHeight(*floor, Vector3i(pos.x, location.yNumber, pos.z), &location.roomNumber, &floor);
			if (!height)
				return std::nullopt;

			location.yNumber = *height;
		}

		floorHeight = floor->GetSurfaceHeight(Vector3i(pos.x, location.yNumber, pos.z), true);
		ceilingHeight = floor->GetSurfaceHeight(Vector3i(pos.x, location.yNumber, pos.z), false);

		if (pos.y > floorHeight && floor->GetRoomNumberBelow(Vector3i(pos.x, location.yNumber, pos.z)))
			return std::nullopt;

		if (pos.y >= ceilingHeight)
		{
			location.yNumber = std::min(pos.y, floorHeight);
			return location;
		}

		auto roomAbove = floor->GetRoomNumberAbove(Vector3i(pos.x, location.yNumber, pos.z));
		while (roomAbove)
		{
			floor = &GetFloorSide(*roomAbove, pos.x, pos.z, &location.roomNumber);
			location.yNumber = floor->GetSurfaceHeight(pos.x, pos.z, true);

			floorHeight = floor->GetSurfaceHeight(Vector3i(pos.x, location.yNumber, pos.z), true);
			ceilingHeight = floor->GetSurfaceHeight(Vector3i(pos.x, location.yNumber, pos.z), false);

			if (pos.y > floorHeight && floor->GetRoomNumberBelow(Vector3i(pos.x, location.yNumber, pos.z)))
				return std::nullopt;

			if (pos.y >= ceilingHeight)
			{
				location.yNumber = std::min(pos.y, floorHeight);
				return location;
			}

			roomAbove = floor->GetRoomNumberAbove(Vector3i(pos.x, location.yNumber, pos.z));
		}

		return std::nullopt;
	}

	ROOM_VECTOR GetRoom(ROOM_VECTOR location, const Vector3i& pos)
	{
		const auto locationBelow = GetBottomRoom(location, pos);
		if (locationBelow)
			return *locationBelow;

		const auto locationAbove = GetTopRoom(location, pos);
		if (locationAbove)
			return *locationAbove;

		return location;
	}

	void AddBridge(int itemNumber, int x, int z)
	{
		const auto& item = g_Level.Items[itemNumber];

		if (!Objects.CheckID(item.ObjectNumber))
			return;

		x += item.Pose.Position.x;
		z += item.Pose.Position.z;

		auto floor = &GetFloorSide(item.RoomNumber, x, z);
		floor->AddBridge(itemNumber);

		if (Objects[item.ObjectNumber].floorBorder != nullptr)
		{
			int floorBorder = Objects[item.ObjectNumber].floorBorder(itemNumber);
			while (floorBorder <= floor->GetSurfaceHeight(x, z, false))
			{
				const auto roomAbove = floor->GetRoomNumberAbove(x, z);
				if (!roomAbove.has_value())
					break;

				floor = &GetFloorSide(*roomAbove, x, z);
				floor->AddBridge(itemNumber);
			}
		}
		
		if (Objects[item.ObjectNumber].ceilingBorder != nullptr)
		{
			int ceilingBorder = Objects[item.ObjectNumber].ceilingBorder(itemNumber);
			while (ceilingBorder >= floor->GetSurfaceHeight(x, z, true))
			{
				const auto roomBelow = floor->GetRoomNumberBelow(x, z);
				if (!roomBelow.has_value())
					break;

				floor = &GetFloorSide(*roomBelow, x, z);
				floor->AddBridge(itemNumber);
			}
		}
	}

	void RemoveBridge(int itemNumber, int x, int z)
	{
		const auto& item = g_Level.Items[itemNumber];

		if (!Objects.CheckID(item.ObjectNumber))
			return;

		x += item.Pose.Position.x;
		z += item.Pose.Position.z;

		auto* floor = &GetFloorSide(item.RoomNumber, x, z);
		floor->RemoveBridge(itemNumber);

		if (Objects[item.ObjectNumber].floorBorder != nullptr)
		{
			int floorBorder = Objects[item.ObjectNumber].floorBorder(itemNumber);
			while (floorBorder <= floor->GetSurfaceHeight(x, z, false))
			{
				const auto roomAbove = floor->GetRoomNumberAbove(x, z);
				if (!roomAbove.has_value())
					break;

				floor = &GetFloorSide(*roomAbove, x, z);
				floor->RemoveBridge(itemNumber);
			}
		}

		if (Objects[item.ObjectNumber].ceilingBorder != nullptr)
		{
			int ceilingBorder = Objects[item.ObjectNumber].ceilingBorder(itemNumber);
			while (ceilingBorder >= floor->GetSurfaceHeight(x, z, true))
			{
				const auto roomBelow = floor->GetRoomNumberBelow(x, z);
				if (!roomBelow.has_value())
					break;

				floor = &GetFloorSide(*roomBelow, x, z);
				floor->RemoveBridge(itemNumber);
			}
		}
	}

	// Gets precise floor/ceiling height from object's bounding box.
	// Animated objects are also supported, although horizontal collision shifting is unstable.
	// Method: get accurate bounds in world transform by converting to OBB, then do a ray test
	// on top or bottom (depending on test side) to determine if box is present at a particular point.
	std::optional<int> GetBridgeItemIntersect(const Vector3i& pos, int itemNumber, bool useBottomHeight)
	{
		auto& item = g_Level.Items[itemNumber];

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
	int GetBridgeBorder(int itemNumber, bool bottom)
	{
		auto item = &g_Level.Items[itemNumber];

		auto bounds = GameBoundingBox(item);
		return item->Pose.Position.y + (bottom ? bounds.Y2 : bounds.Y1);
	}

	// Updates BridgeItem for all blocks which are enclosed by bridge bounds.
	void UpdateBridgeItem(int itemNumber, bool forceRemoval)
	{
		auto item = &g_Level.Items[itemNumber];

		if (!Objects.CheckID(item->ObjectNumber))
			return;

		if (!Objects[item->ObjectNumber].loaded)
			return;

		// Force removal if object was killed
		if (item->Flags & IFLAG_KILLED)
			forceRemoval = true;

		// Get real OBB bounds of a bridge in world space
		auto bounds = GameBoundingBox(item);
		auto dxBounds = bounds.ToBoundingOrientedBox(item->Pose);

		// Get corners of a projected OBB
		Vector3 corners[8];
		dxBounds.GetCorners(corners); //corners[0], corners[1], corners[4] corners[5]

		auto room = &g_Level.Rooms[item->RoomNumber];

		// Get min/max of a projected AABB
		auto minX = floor((std::min(std::min(std::min(corners[0].x, corners[1].x), corners[4].x), corners[5].x) - room->x) / BLOCK(1));
		auto minZ = floor((std::min(std::min(std::min(corners[0].z, corners[1].z), corners[4].z), corners[5].z) - room->z) / BLOCK(1));
		auto maxX =  ceil((std::max(std::max(std::max(corners[0].x, corners[1].x), corners[4].x), corners[5].x) - room->x) / BLOCK(1));
		auto maxZ =  ceil((std::max(std::max(std::max(corners[0].z, corners[1].z), corners[4].z), corners[5].z) - room->z) / BLOCK(1));

		// Run through all blocks enclosed in AABB
		for (int x = 0; x < room->xSize; x++)
		{
			for (int z = 0; z < room->zSize; z++)
			{
				float pX = room->x + (x * BLOCK(1)) + BLOCK(0.5f);
				float pZ = room->z + (z * BLOCK(1)) + BLOCK(0.5f);
				float offX = pX - item->Pose.Position.x;
				float offZ = pZ - item->Pose.Position.z;

				// Clean previous bridge state
				RemoveBridge(itemNumber, offX, offZ);

				// If we're in sweeping mode, don't try to re-add block
				if (forceRemoval)
					continue;

				// If block isn't in enclosed AABB space, ignore precise check
				if (x < minX || z < minZ || x > maxX || z > maxZ)
					continue;

				// Block is in enclosed AABB space, do more precise test.
				// Construct a block bounding box within same plane as bridge bounding box and test intersection.
				auto blockBox = BoundingOrientedBox(Vector3(pX, dxBounds.Center.y, pZ), Vector3(BLOCK(1 / 2.0f)), Vector4::UnitY);
				if (dxBounds.Intersects(blockBox))
					AddBridge(itemNumber, offX, offZ); // Intersects, try to add bridge to this block.
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
		constexpr auto DRAW_RANGE	  = BLOCK(3);
		constexpr auto STRING_SPACING = -20.0f;

		constexpr auto STOPPER_COLOR				 = Vector4(1.0f, 0.4f, 0.4f, 1.0f);
		constexpr auto DEATH_COLOR					 = Vector4(0.4f, 1.0f, 0.4f, 1.0f);
		constexpr auto MONKEY_SWING_COLOR			 = Vector4(1.0f, 0.4f, 0.4f, 1.0f);
		constexpr auto BEETLE_MINECART_RIGHT_COLOR	 = Vector4(0.4f, 0.4f, 1.0f, 1.0f);
		constexpr auto ACTIVATOR_MINECART_LEFT_COLOR = Vector4(1.0f, 0.4f, 1.0f, 1.0f);
		constexpr auto MINECART_STOP_COLOR			 = Vector4(0.4f, 1.0f, 1.0f, 1.0f);
		
		// Only check sectors in player's vicinity.
		const auto& room = g_Level.Rooms[item.RoomNumber];
		int minX = std::max(item.Pose.Position.x - DRAW_RANGE, room.x) / BLOCK(1);
		int maxX = std::min(item.Pose.Position.x + DRAW_RANGE, room.x + (room.xSize * BLOCK(1))) / BLOCK(1);
		int minZ = std::max(item.Pose.Position.z - DRAW_RANGE, room.z) / BLOCK(1);
		int maxZ = std::min(item.Pose.Position.z + DRAW_RANGE, room.z + (room.zSize * BLOCK(1))) / BLOCK(1);
		
		auto pointColl = GetCollision(item);
		auto pos = item.Pose.Position.ToVector3();

		// Draw sector flag labels.
		for (int x = minX; x < maxX; x++)
		{
			for (int z = minZ; z < maxZ; z++)
			{
				pos.x = BLOCK(x);
				pos.z = BLOCK(z);
				
				pointColl = GetCollision(pos, item.RoomNumber);
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
