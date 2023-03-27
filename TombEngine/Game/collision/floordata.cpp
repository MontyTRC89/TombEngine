#include "framework.h"
#include "Game/collision/floordata.h"

#include "Game/items.h"
#include "Game/room.h"
#include "Math/Math.h"
#include "Specific/level.h"
#include "Specific/setup.h"

using namespace TEN::Floordata;
using namespace TEN::Math;

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

std::optional<int> FloorInfo::GetRoomNumberBelow(int x, int y, int z) const
{
	// Get surface heights.
	int floorHeight = GetSurfaceHeight(x, z, true);
	int ceilingHeight = GetSurfaceHeight(x, z, false);

	// Loop through bridges.
	for (const int& i : BridgeItemNumbers)
	{
		const auto& bridgeItem = g_Level.Items[i];
		const auto& bridgeObject = Objects[bridgeItem.ObjectNumber];

		// Get bridge floor height.
		auto bridgeFloorHeight = bridgeObject.floor(i, x, y, z);
		if (!bridgeFloorHeight.has_value())
			continue;

		// Assess relation of bridge to collision block.
		if (bridgeFloorHeight.value() >= y &&			// Below input height bound.
			bridgeFloorHeight.value() <= floorHeight && // Within floor bound.
			bridgeFloorHeight.value() >= ceilingHeight) // Within ceiling bound.
		{
			return std::nullopt;
		}
	}

	return GetRoomNumberBelow(x, z);
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

std::optional<int> FloorInfo::GetRoomNumberAbove(int x, int y, int z) const
{
	// Get surface heights.
	int floorHeight = GetSurfaceHeight(x, z, true);
	int ceilingHeight = GetSurfaceHeight(x, z, false);

	// Loop through bridges.
	for (const int& i : BridgeItemNumbers)
	{
		const auto& bridgeItem = g_Level.Items[i];
		const auto& bridgeObject = Objects[bridgeItem.ObjectNumber];

		// Get bridge ceiling height.
		auto bridgeCeilingHeight = bridgeObject.ceiling(i, x, y, z);
		if (!bridgeCeilingHeight.has_value())
			continue;

		// Assess relation of bridge to collision block.
		if (bridgeCeilingHeight.value() <= y &&			  // Above input height bound.
			bridgeCeilingHeight.value() <= floorHeight && // Within floor bound.
			bridgeCeilingHeight.value() >= ceilingHeight) // Within ceiling bound.
		{
			return std::nullopt;
		}
	}

	return GetRoomNumberAbove(x, z);
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

int FloorInfo::GetSurfaceHeight(int x, int y, int z, bool isFloor) const
{
	// Get surface heights.
	int floorHeight = GetSurfaceHeight(x, z, true);
	int ceilingHeight = GetSurfaceHeight(x, z, false);

	// Loop through bridges.
	for (const int& i : BridgeItemNumbers)
	{
		const auto& bridgeItem = g_Level.Items[i];
		const auto& bridgeObject = Objects[bridgeItem.ObjectNumber];

		// Get bridge surface height.
		auto bridgeSurfaceHeight = isFloor ? bridgeObject.floor(i, x, y, z) : bridgeObject.ceiling(i, x, y, z);
		if (!bridgeSurfaceHeight.has_value())
			continue;

		// Assess relation of bridge to collision block.
		if (isFloor)
		{
			if (bridgeSurfaceHeight.value() >= y &&			  // Below input height bound.
				bridgeSurfaceHeight.value() < floorHeight &&  // Within floor bound.
				bridgeSurfaceHeight.value() >= ceilingHeight) // Within ceiling bound.
			{
				floorHeight = bridgeSurfaceHeight.value();
			}
		}
		else
		{
			if (bridgeSurfaceHeight.value() <= y &&			  // Above input height bound.
				bridgeSurfaceHeight.value() <= floorHeight && // Within floor bound.
				bridgeSurfaceHeight.value() > ceilingHeight)  // Within ceiling bound.
			{
				ceilingHeight = bridgeSurfaceHeight.value();
			}
		}
	}

	// Return surface height.
	return (isFloor ? floorHeight : ceilingHeight);
}

int FloorInfo::GetBridgeSurfaceHeight(int x, int y, int z, bool isFloor) const
{
	// Loop through bridges.
	for (const int& i : BridgeItemNumbers)
	{
		const auto& bridgeItem = g_Level.Items[i];
		const auto& bridgeObject = Objects[bridgeItem.ObjectNumber];

		// Get surface heights.
		auto floorHeight = bridgeObject.floor(i, x, y, z);
		auto ceilingHeight = bridgeObject.ceiling(i, x, y, z);
		if (!floorHeight.has_value() || !ceilingHeight.has_value())
			continue;

		// Assess relation of bridge to collision block.
		if (isFloor)
		{
			if (y > floorHeight.value() &&
				y <= ceilingHeight.value())
			{
				return floorHeight.value();
			}
		}
		else
		{
			if (y >= floorHeight.value() &&
				y < ceilingHeight.value())
			{
				return ceilingHeight.value();
			}
		}
	}
	
	// Return bridge surface height.
	return GetSurfaceHeight(x, y, z, isFloor);
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

int FloorInfo::GetInsideBridgeItemNumber(int x, int y, int z, bool testFloorBorder, bool testCeilingBorder) const
{
	for (const int& itemNumber : BridgeItemNumbers)
	{
		const auto& bridgeItem = g_Level.Items[itemNumber];
		const auto& bridgeObject = Objects[bridgeItem.ObjectNumber];

		// Get surface heights.
		auto floorHeight = bridgeObject.floor(itemNumber, x, y, z);
		auto ceilingHeight = bridgeObject.ceiling(itemNumber, x, y, z);
		if (!floorHeight.has_value() || !ceilingHeight.has_value())
			continue;

		if ((y > floorHeight.value() && y < ceilingHeight.value()) ||
			(testFloorBorder && y == floorHeight.value()) ||
			(testCeilingBorder && y == ceilingHeight.value()))
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

namespace TEN::Floordata
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

	FloorInfo& GetFloor(int roomNumber, const Vector2i& pos)
	{
		auto& room = g_Level.Rooms[roomNumber];
		return room.floor[room.zSize * pos.x + pos.y];
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

	std::optional<int> GetTopHeight(FloorInfo& startFloor, int x, int y, int z, int* topRoomNumber, FloorInfo** topFloor)
	{
		auto floor = &startFloor;
		int roomNumber;
		if (topRoomNumber)
			roomNumber = *topRoomNumber;

		do
		{
			y = floor->GetBridgeSurfaceHeight(x, y, z, true);
			while (y <= floor->GetSurfaceHeight(x, z, false))
			{
				const auto roomAbove = floor->GetRoomNumberAbove(x, z);
				if (!roomAbove)
					return std::nullopt;

				floor = &GetFloorSide(*roomAbove, x, z, &roomNumber);
			}
		}
		while (floor->GetInsideBridgeItemNumber(x, y, z, false, true) >= 0);

		if (topRoomNumber)
			*topRoomNumber = roomNumber;
		if (topFloor)
			*topFloor = floor;
		return std::optional{y};
	}

	std::optional<int> GetBottomHeight(FloorInfo& startFloor, int x, int y, int z, int* bottomRoomNumber, FloorInfo** bottomFloor)
	{
		auto floor = &startFloor;
		int roomNumber;
		if (bottomRoomNumber)
			roomNumber = *bottomRoomNumber;

		do
		{
			y = floor->GetBridgeSurfaceHeight(x, y, z, false);
			while (y >= floor->GetSurfaceHeight(x, z, true))
			{
				const auto roomBelow = floor->GetRoomNumberBelow(x, z);
				if (!roomBelow)
					return std::nullopt;

				floor = &GetFloorSide(*roomBelow, x, z, &roomNumber);
			}
		}
		while (floor->GetInsideBridgeItemNumber(x, y, z, true, false) >= 0);

		if (bottomRoomNumber)
			*bottomRoomNumber = roomNumber;
		if (bottomFloor)
			*bottomFloor = floor;
		return std::optional{y};
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
					return std::nullopt;
			}
		}

		const auto floorHeight = floor->GetSurfaceHeight(x, y, z, true);
		const auto ceilingHeight = floor->GetSurfaceHeight(x, y, z, false);

		y = std::clamp(y, std::min(ceilingHeight, floorHeight), std::max(ceilingHeight, floorHeight));

		if (floor->GetInsideBridgeItemNumber(x, y, z, y == ceilingHeight, y == floorHeight) >= 0)
		{
			if (direction <= 0)
			{
				auto height = GetTopHeight(*floor, x, y, z);
				if (height)
					return height;
			}

			if (direction >= 0)
			{
				auto height = GetBottomHeight(*floor, x, y, z, nullptr, &floor);
				if (!height)
					return std::nullopt;

				y = *height;
			}
		}

		if (direction >= 0)
		{
			auto roomBelow = floor->GetRoomNumberBelow(x, y, z);
			while (roomBelow)
			{
				floor = &GetFloorSide(*roomBelow, x, z);
				roomBelow = floor->GetRoomNumberBelow(x, y, z);
			}
		}

		return std::optional{floor->GetSurfaceHeight(x, y, z, true)};
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
					return std::nullopt;
			}
		}

		const auto floorHeight = floor->GetSurfaceHeight(x, y, z, true);
		const auto ceilingHeight = floor->GetSurfaceHeight(x, y, z, false);

		y = std::clamp(y, std::min(ceilingHeight, floorHeight), std::max(ceilingHeight, floorHeight));

		if (floor->GetInsideBridgeItemNumber(x, y, z, y == ceilingHeight, y == floorHeight) >= 0)
		{
			if (direction >= 0)
			{
				auto height = GetBottomHeight(*floor, x, y, z);
				if (height)
					return height;
			}

			if (direction <= 0)
			{
				auto height = GetTopHeight(*floor, x, y, z, nullptr, &floor);
				if (!height)
					return std::nullopt;

				y = *height;
			}
		}

		if (direction <= 0)
		{
			auto roomAbove = floor->GetRoomNumberAbove(x, y, z);
			while (roomAbove)
			{
				floor = &GetFloorSide(*roomAbove, x, z);
				roomAbove = floor->GetRoomNumberAbove(x, y, z);
			}
		}

		return std::optional{floor->GetSurfaceHeight(x, y, z, false)};
	}

	std::optional<ROOM_VECTOR> GetBottomRoom(ROOM_VECTOR location, int x, int y, int z)
	{
		auto floor = &GetFloorSide(location.roomNumber, x, z, &location.roomNumber);

		if (floor->IsWall(x, z))
		{
			floor = &GetBottomFloor(location.roomNumber, x, z, &location.roomNumber);

			if (floor->IsWall(x, z))
				return std::nullopt;

			location.yNumber = floor->GetSurfaceHeight(x, z, false);
		}

		auto floorHeight = floor->GetSurfaceHeight(x, location.yNumber, z, true);
		auto ceilingHeight = floor->GetSurfaceHeight(x, location.yNumber, z, false);

		location.yNumber = std::clamp(location.yNumber, std::min(ceilingHeight, floorHeight), std::max(ceilingHeight, floorHeight));

		if (floor->GetInsideBridgeItemNumber(x, location.yNumber, z, location.yNumber == ceilingHeight, location.yNumber == floorHeight) >= 0)
		{
			const auto height = GetBottomHeight(*floor, x, location.yNumber, z, &location.roomNumber, &floor);
			if (!height)
				return std::nullopt;

			location.yNumber = *height;
		}

		floorHeight = floor->GetSurfaceHeight(x, location.yNumber, z, true);
		ceilingHeight = floor->GetSurfaceHeight(x, location.yNumber, z, false);

		if (y < ceilingHeight && floor->GetRoomNumberAbove(x, location.yNumber, z))
			return std::nullopt;
		if (y <= floorHeight)
		{
			location.yNumber = std::max(y, ceilingHeight);
			return std::optional{location};
		}

		auto roomBelow = floor->GetRoomNumberBelow(x, location.yNumber, z);
		while (roomBelow)
		{
			floor = &GetFloorSide(*roomBelow, x, z, &location.roomNumber);
			location.yNumber = floor->GetSurfaceHeight(x, z, false);

			floorHeight = floor->GetSurfaceHeight(x, location.yNumber, z, true);
			ceilingHeight = floor->GetSurfaceHeight(x, location.yNumber, z, false);

			if (y < ceilingHeight && floor->GetRoomNumberAbove(x, location.yNumber, z))
				return std::nullopt;
			if (y <= floorHeight)
			{
				location.yNumber = std::max(y, ceilingHeight);
				return std::optional{location};
			}

			roomBelow = floor->GetRoomNumberBelow(x, location.yNumber, z);
		}

		return std::nullopt;
	}

	std::optional<ROOM_VECTOR> GetTopRoom(ROOM_VECTOR location, int x, int y, int z)
	{
		auto floor = &GetFloorSide(location.roomNumber, x, z, &location.roomNumber);

		if (floor->IsWall(x, z))
		{
			floor = &GetTopFloor(location.roomNumber, x, z, &location.roomNumber);

			if (floor->IsWall(x, z))
				return std::nullopt;

			location.yNumber = floor->GetSurfaceHeight(x, z, true);
		}

		auto floorHeight = floor->GetSurfaceHeight(x, location.yNumber, z, true);
		auto ceilingHeight = floor->GetSurfaceHeight(x, location.yNumber, z, false);

		location.yNumber = std::clamp(location.yNumber, std::min(ceilingHeight, floorHeight), std::max(ceilingHeight, floorHeight));

		if (floor->GetInsideBridgeItemNumber(x, location.yNumber, z, location.yNumber == ceilingHeight, location.yNumber == floorHeight) >= 0)
		{
			const auto height = GetTopHeight(*floor, x, location.yNumber, z, &location.roomNumber, &floor);
			if (!height)
				return std::nullopt;

			location.yNumber = *height;
		}

		floorHeight = floor->GetSurfaceHeight(x, location.yNumber, z, true);
		ceilingHeight = floor->GetSurfaceHeight(x, location.yNumber, z, false);

		if (y > floorHeight && floor->GetRoomNumberBelow(x, location.yNumber, z))
			return std::nullopt;
		if (y >= ceilingHeight)
		{
			location.yNumber = std::min(y, floorHeight);
			return std::optional{location};
		}

		auto roomAbove = floor->GetRoomNumberAbove(x, location.yNumber, z);
		while (roomAbove)
		{
			floor = &GetFloorSide(*roomAbove, x, z, &location.roomNumber);
			location.yNumber = floor->GetSurfaceHeight(x, z, true);

			floorHeight = floor->GetSurfaceHeight(x, location.yNumber, z, true);
			ceilingHeight = floor->GetSurfaceHeight(x, location.yNumber, z, false);

			if (y > floorHeight && floor->GetRoomNumberBelow(x, location.yNumber, z))
				return std::nullopt;
			if (y >= ceilingHeight)
			{
				location.yNumber = std::min(y, floorHeight);
				return std::optional{location};
			}

			roomAbove = floor->GetRoomNumberAbove(x, location.yNumber, z);
		}

		return std::nullopt;
	}

	ROOM_VECTOR GetRoom(ROOM_VECTOR location, int x, int y, int z)
	{
		const auto locationBelow = GetBottomRoom(location, x, y, z);
		if (locationBelow)
			return *locationBelow;

		const auto locationAbove = GetTopRoom(location, x, y, z);
		if (locationAbove)
			return *locationAbove;

		return location;
	}

	void AddBridge(int itemNumber, int x, int z)
	{
		const auto& item = g_Level.Items[itemNumber];
		x += item.Pose.Position.x;
		z += item.Pose.Position.z;

		auto floor = &GetFloorSide(item.RoomNumber, x, z);
		floor->AddBridge(itemNumber);

		const auto floorBorder = Objects[item.ObjectNumber].floorBorder(itemNumber);
		while (floorBorder <= floor->GetSurfaceHeight(x, z, false))
		{
			const auto roomAbove = floor->GetRoomNumberAbove(x, z);
			if (!roomAbove)
				break;

			floor = &GetFloorSide(*roomAbove, x, z);
			floor->AddBridge(itemNumber);
		}

		const auto ceilingBorder = Objects[item.ObjectNumber].ceilingBorder(itemNumber);
		while (ceilingBorder >= floor->GetSurfaceHeight(x, z, true))
		{
			const auto roomBelow = floor->GetRoomNumberBelow(x, z);
			if (!roomBelow)
				break;

			floor = &GetFloorSide(*roomBelow, x, z);
			floor->AddBridge(itemNumber);
		}
	}

	void RemoveBridge(int itemNumber, int x, int z)
	{
		const auto& item = g_Level.Items[itemNumber];
		x += item.Pose.Position.x;
		z += item.Pose.Position.z;

		auto floor = &GetFloorSide(item.RoomNumber, x, z);
		floor->RemoveBridge(itemNumber);

		const auto floorBorder = Objects[item.ObjectNumber].floorBorder(itemNumber);
		while (floorBorder <= floor->GetSurfaceHeight(x, z, false))
		{
			const auto roomAbove = floor->GetRoomNumberAbove(x, z);
			if (!roomAbove)
				break;

			floor = &GetFloorSide(*roomAbove, x, z);
			floor->RemoveBridge(itemNumber);
		}

		const auto ceilingBorder = Objects[item.ObjectNumber].ceilingBorder(itemNumber);
		while (ceilingBorder >= floor->GetSurfaceHeight(x, z, true))
		{
			const auto roomBelow = floor->GetRoomNumberBelow(x, z);
			if (!roomBelow)
				break;

			floor = &GetFloorSide(*roomBelow, x, z);
			floor->RemoveBridge(itemNumber);
		}
	}

	// New function which gets precise floor/ceiling collision from actual object bounding box.
	// Animated objects are also supported, although horizontal collision shift is unstable.
	// Method: get accurate bounds in world transform by converting to DirectX OBB, then do a
	// ray test on top or bottom (depending on test side) to determine if box is present at 
	// this particular point.

	std::optional<int> GetBridgeItemIntersect(int itemNumber, int x, int y, int z, bool bottom)
	{
		auto* item = &g_Level.Items[itemNumber];

		auto bounds = GameBoundingBox(item);
		auto dxBounds = bounds.ToBoundingOrientedBox(item->Pose);

		Vector3 pos = Vector3(x, y + (bottom ? 4 : -4), z); // Introduce slight vertical margin just in case

		static float distance;
		if (dxBounds.Intersects(pos, (bottom ? -Vector3::UnitY : Vector3::UnitY), distance))
			return std::optional{ item->Pose.Position.y + (bottom ? bounds.Y2 : bounds.Y1) };
		else
			return std::nullopt;
	}

	// Gets bridge min or max height regardless of actual X/Z world position

	int GetBridgeBorder(int itemNumber, bool bottom)
	{
		auto* item = &g_Level.Items[itemNumber];

		auto bounds = GameBoundingBox(item);
		return (item->Pose.Position.y + (bottom ? bounds.Y2 : bounds.Y1));
	}

	// Updates BridgeItem for all blocks which are enclosed by bridge bounds.
	void UpdateBridgeItem(int itemNumber, bool forceRemoval)
	{
		auto* item = &g_Level.Items[itemNumber];

		// Force removal if object was killed.
		if (item->Flags & IFLAG_KILLED)
			forceRemoval = true;

		// Get real OBB bounds of a bridge in world space.
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

		// Run through all blocks enclosed in AABB.
		for (int x = 0; x < room->xSize; x++)
		{
			for (int z = 0; z < room->zSize; z++)
			{
				auto pX = room->x + (x * BLOCK(1)) + BLOCK(0.5f);
				auto pZ = room->z + (z * BLOCK(1)) + BLOCK(0.5f);
				auto offX = pX - item->Pose.Position.x;
				auto offZ = pZ - item->Pose.Position.z;

				// Clean previous bridge state.
				RemoveBridge(itemNumber, offX, offZ);

				// If we're in sweeping mode, don't attempt re-adding block.
				if (forceRemoval)
					continue;

				// If block isn't in enclosed AABB space, ignore precise check.
				if (x < minX || z < minZ || x > maxX || z > maxZ)
					continue;

				// Block is in enclosed AABB space; do more precise test.
				// Construct a block bounding box within same plane as bridge bounding box and test intersection.
				auto blockBox = BoundingOrientedBox(Vector3(pX, dxBounds.Center.y, pZ), Vector3(BLOCK(1 / 2.0f)), Vector4::UnitY);
				if (dxBounds.Intersects(blockBox))
					AddBridge(itemNumber, offX, offZ); // Intersects; try adding bridge to this block.
			}
		}
	}
}
