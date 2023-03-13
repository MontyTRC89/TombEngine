#include "framework.h"
#include "Game/collision/floordata.h"

#include "Game/items.h"
#include "Game/room.h"
#include "Math/Math.h"
#include "Specific/level.h"
#include "Specific/setup.h"

using namespace TEN::Floordata;
using namespace TEN::Math;

int FloorInfo::GetSurfacePlaneIndex(int x, int z, bool checkFloor) const
{
	// Calculate bias.
	auto point = GetSectorPoint(x, z).ToVector2();
	auto rotMatrix = Matrix::CreateRotationZ(checkFloor ? FloorCollision.SplitAngle : CeilingCollision.SplitAngle);
	Vector2::Transform(point, rotMatrix, point);

	// Determine and return plane index.
	return ((point.x < 0) ? 0 : 1);
}

Vector2 FloorInfo::GetSurfaceTilt(int x, int z, bool checkFloor) const
{
	// Get surface plane.
	const auto& planes = checkFloor ? this->FloorCollision.Planes : this->CeilingCollision.Planes;
	int planeIndex = GetSurfacePlaneIndex(x, z, true); // TODO: Check why it only looks at floor planes.
	auto plane = planes[planeIndex];

	// Calculate and return plane tilt.
	return Vector2(
		-int((plane.x * BLOCK(1)) / CLICK(1)),
		-int((plane.y * BLOCK(1)) / CLICK(1)));
}

bool FloorInfo::IsSurfaceSplit(bool checkFloor) const
{
	// Check if surface planes are different.
	const auto& planes = checkFloor ? this->FloorCollision.Planes : this->CeilingCollision.Planes;
	bool arePlanesDifferent = (planes[0] != planes[1]);

	return (arePlanesDifferent || IsSurfaceSplitPortal(checkFloor));
}

bool FloorInfo::IsSurfaceDiagonalStep(bool checkFloor) const
{
	// Check if surface is split.
	if (!IsSurfaceSplit(checkFloor))
		return false;

	const auto& collData = checkFloor ? this->FloorCollision : this->CeilingCollision;
	
	// Check if ??
	if (round(collData.Planes[0].z) == round(collData.Planes[1].z))
		return false;

	// Check if ??
	if (collData.SplitAngle != SurfaceCollisionData::SPLIT_ANGLE_1 &&
		collData.SplitAngle != SurfaceCollisionData::SPLIT_ANGLE_2)
		{
			return false;
		}

	return true;
}

bool FloorInfo::IsSurfaceSplitPortal(bool checkFloor) const
{
	// Check if surface planes are different.
	const auto& planes = checkFloor ? this->FloorCollision.Planes : this->CeilingCollision.Planes;
	return (planes[0] != planes[1]);
}

std::optional<int> FloorInfo::RoomBelow(int plane) const
{
	const auto room = FloorCollision.Portals[plane];
	return room != NO_ROOM ? std::optional{room} : std::nullopt;
}

std::optional<int> FloorInfo::RoomBelow(int x, int z) const
{
	return RoomBelow(GetSurfacePlaneIndex(x, z, true));
}

std::optional<int> FloorInfo::RoomBelow(int x, int y, int z) const
{
	const auto floorHeight = FloorHeight(x, z);
	const auto ceilingHeight = CeilingHeight(x, z);

	for (const auto itemNumber : BridgeItem)
	{
		const auto& item = g_Level.Items[itemNumber];
		const auto itemHeight = Objects[item.ObjectNumber].floor(itemNumber, x, y, z);
		if (itemHeight && *itemHeight >= y && *itemHeight <= floorHeight && *itemHeight >= ceilingHeight)
			return std::nullopt;
	}

	return RoomBelow(x, z);
}

std::optional<int> FloorInfo::RoomAbove(int plane) const
{
	const auto room = CeilingCollision.Portals[plane];
	return room != NO_ROOM ? std::optional{room} : std::nullopt;
}

std::optional<int> FloorInfo::RoomAbove(int x, int z) const
{
	return RoomAbove(GetSurfacePlaneIndex(x, z, false));
}

std::optional<int> FloorInfo::RoomAbove(int x, int y, int z) const
{
	const auto floorHeight = FloorHeight(x, z);
	const auto ceilingHeight = CeilingHeight(x, z);

	for (const auto itemNumber : BridgeItem)
	{
		const auto& item = g_Level.Items[itemNumber];
		const auto itemHeight = Objects[item.ObjectNumber].ceiling(itemNumber, x, y, z);
		if (itemHeight && *itemHeight <= y && *itemHeight <= floorHeight && *itemHeight >= ceilingHeight)
			return std::nullopt;
	}

	return RoomAbove(x, z);
}

std::optional<int> FloorInfo::RoomSide() const
{
	return WallPortal != NO_ROOM ? std::optional{WallPortal} : std::nullopt;
}

int FloorInfo::FloorHeight(int x, int z) const
{
	const auto plane = GetSurfacePlaneIndex(x, z, true);
	const auto vector = GetSectorPoint(x, z);

	return FloorCollision.Planes[plane].x * vector.x + FloorCollision.Planes[plane].y * vector.y + FloorCollision.Planes[plane].z;
}

int FloorInfo::FloorHeight(int x, int y, int z) const
{
	auto height = FloorHeight(x, z);
	const auto ceilingHeight = CeilingHeight(x, z);

	for (const auto itemNumber : BridgeItem)
	{
		const auto& item = g_Level.Items[itemNumber];
		const auto itemHeight = Objects[item.ObjectNumber].floor(itemNumber, x, y, z);
		if (itemHeight && *itemHeight >= y && *itemHeight < height && *itemHeight >= ceilingHeight)
			height = *itemHeight;
	}

	return height;
}

int FloorInfo::BridgeFloorHeight(int x, int y, int z) const
{
	for (const auto itemNumber : BridgeItem)
	{
		const auto& item = g_Level.Items[itemNumber];
		const auto floorHeight = Objects[item.ObjectNumber].floor(itemNumber, x, y, z);
		const auto ceilingHeight = Objects[item.ObjectNumber].ceiling(itemNumber, x, y, z);
		if (floorHeight && ceilingHeight && y > *floorHeight && y <= *ceilingHeight)
			return *floorHeight;
	}

	return FloorHeight(x, y, z);
}

int FloorInfo::CeilingHeight(int x, int z) const
{
	const auto plane = GetSurfacePlaneIndex(x, z, false);
	const auto vector = GetSectorPoint(x, z);

	return CeilingCollision.Planes[plane].x * vector.x + CeilingCollision.Planes[plane].y * vector.y + CeilingCollision.Planes[plane].z;
}

int FloorInfo::CeilingHeight(int x, int y, int z) const
{
	auto height = CeilingHeight(x, z);
	const auto floorHeight = FloorHeight(x, z);

	for (const auto itemNumber : BridgeItem)
	{
		const auto& item = g_Level.Items[itemNumber];
		const auto itemHeight = Objects[item.ObjectNumber].ceiling(itemNumber, x, y, z);
		if (itemHeight && *itemHeight <= y && *itemHeight > height && *itemHeight <= floorHeight)
			height = *itemHeight;
	}

	return height;
}

int FloorInfo::BridgeCeilingHeight(int x, int y, int z) const
{
	for (const auto itemNumber : BridgeItem)
	{
		const auto& item = g_Level.Items[itemNumber];
		const auto floorHeight = Objects[item.ObjectNumber].floor(itemNumber, x, y, z);
		const auto ceilingHeight = Objects[item.ObjectNumber].ceiling(itemNumber, x, y, z);
		if (floorHeight && ceilingHeight && y >= *floorHeight && y < *ceilingHeight)
			return *ceilingHeight;
	}

	return CeilingHeight(x, y, z);
}

Vector2 FloorInfo::FloorSlope(int plane) const
{
	return Vector2{FloorCollision.Planes[plane].x, FloorCollision.Planes[plane].y};
}

Vector2 FloorInfo::FloorSlope(int x, int z) const
{
	return FloorSlope(GetSurfacePlaneIndex(x, z, true));
}

Vector2 FloorInfo::CeilingSlope(int plane) const
{
	return Vector2{CeilingCollision.Planes[plane].x, CeilingCollision.Planes[plane].y};
}

Vector2 FloorInfo::CeilingSlope(int x, int z) const
{
	return CeilingSlope(GetSurfacePlaneIndex(x, z, false));
}

bool FloorInfo::IsWall(int plane) const
{
	return FloorCollision.SplitAngle == CeilingCollision.SplitAngle && FloorCollision.Planes[plane] == CeilingCollision.Planes[plane];
}

bool FloorInfo::IsWall(int x, int z) const
{
	return IsWall(GetSurfacePlaneIndex(x, z, true));
}

int FloorInfo::InsideBridge(int x, int y, int z, bool floorBorder, bool ceilingBorder) const
{
	for (const auto itemNumber : BridgeItem)
	{
		const auto& item = g_Level.Items[itemNumber];
		const auto floorHeight = Objects[item.ObjectNumber].floor(itemNumber, x, y, z);
		const auto ceilingHeight = Objects[item.ObjectNumber].ceiling(itemNumber, x, y, z);
		if (floorHeight && ceilingHeight && (y > *floorHeight && y < *ceilingHeight || floorBorder && y == *floorHeight || ceilingBorder && y == *ceilingHeight))
			return itemNumber;
	}

	return NO_ITEM;
}

void FloorInfo::AddItem(int itemNumber)
{
	BridgeItem.insert(itemNumber);
}

void FloorInfo::RemoveItem(int itemNumber)
{
	BridgeItem.erase(itemNumber);
}

namespace TEN::Floordata
{
	Vector2i GetSectorPoint(int x, int z)
	{
		const auto xPoint = x % SECTOR(1) - SECTOR(1) / 2;
		const auto yPoint = z % SECTOR(1) - SECTOR(1) / 2;

		return Vector2i{xPoint, yPoint};
	}

	Vector2i GetRoomPosition(int roomNumber, int x, int z)
	{
		const auto& room = g_Level.Rooms[roomNumber];
		const auto zRoom = (z - room.z) / SECTOR(1);
		const auto xRoom = (x - room.x) / SECTOR(1);
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

		auto roomSide = floor->RoomSide();
		while (roomSide)
		{
			roomNumber = *roomSide;
			floor = &GetFloor(roomNumber, x, z);
			roomSide = floor->RoomSide();
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
			const auto roomBelow = floor->RoomBelow(x, z);
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
			const auto roomAbove = floor->RoomAbove(x, z);
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
			y = floor->BridgeFloorHeight(x, y, z);
			while (y <= floor->CeilingHeight(x, z))
			{
				const auto roomAbove = floor->RoomAbove(x, z);
				if (!roomAbove)
					return std::nullopt;

				floor = &GetFloorSide(*roomAbove, x, z, &roomNumber);
			}
		}
		while (floor->InsideBridge(x, y, z, false, true) >= 0);

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
			y = floor->BridgeCeilingHeight(x, y, z);
			while (y >= floor->FloorHeight(x, z))
			{
				const auto roomBelow = floor->RoomBelow(x, z);
				if (!roomBelow)
					return std::nullopt;

				floor = &GetFloorSide(*roomBelow, x, z, &roomNumber);
			}
		}
		while (floor->InsideBridge(x, y, z, true, false) >= 0);

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
				y = floor->FloorHeight(x, z);
				direction = -1;
			}
			else
			{
				floor = &GetBottomFloor(location.roomNumber, x, z);

				if (!floor->IsWall(x, z))
				{
					y = floor->CeilingHeight(x, z);
					direction = 1;
				}
				else
					return std::nullopt;
			}
		}

		const auto floorHeight = floor->FloorHeight(x, y, z);
		const auto ceilingHeight = floor->CeilingHeight(x, y, z);

		y = std::clamp(y, std::min(ceilingHeight, floorHeight), std::max(ceilingHeight, floorHeight));

		if (floor->InsideBridge(x, y, z, y == ceilingHeight, y == floorHeight) >= 0)
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
			auto roomBelow = floor->RoomBelow(x, y, z);
			while (roomBelow)
			{
				floor = &GetFloorSide(*roomBelow, x, z);
				roomBelow = floor->RoomBelow(x, y, z);
			}
		}

		return std::optional{floor->FloorHeight(x, y, z)};
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
				y = floor->CeilingHeight(x, z);
				direction = 1;
			}
			else
			{
				floor = &GetTopFloor(location.roomNumber, x, z);

				if (!floor->IsWall(x, z))
				{
					y = floor->FloorHeight(x, z);
					direction = -1;
				}
				else
					return std::nullopt;
			}
		}

		const auto floorHeight = floor->FloorHeight(x, y, z);
		const auto ceilingHeight = floor->CeilingHeight(x, y, z);

		y = std::clamp(y, std::min(ceilingHeight, floorHeight), std::max(ceilingHeight, floorHeight));

		if (floor->InsideBridge(x, y, z, y == ceilingHeight, y == floorHeight) >= 0)
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
			auto roomAbove = floor->RoomAbove(x, y, z);
			while (roomAbove)
			{
				floor = &GetFloorSide(*roomAbove, x, z);
				roomAbove = floor->RoomAbove(x, y, z);
			}
		}

		return std::optional{floor->CeilingHeight(x, y, z)};
	}

	std::optional<ROOM_VECTOR> GetBottomRoom(ROOM_VECTOR location, int x, int y, int z)
	{
		auto floor = &GetFloorSide(location.roomNumber, x, z, &location.roomNumber);

		if (floor->IsWall(x, z))
		{
			floor = &GetBottomFloor(location.roomNumber, x, z, &location.roomNumber);

			if (floor->IsWall(x, z))
				return std::nullopt;

			location.yNumber = floor->CeilingHeight(x, z);
		}

		auto floorHeight = floor->FloorHeight(x, location.yNumber, z);
		auto ceilingHeight = floor->CeilingHeight(x, location.yNumber, z);

		location.yNumber = std::clamp(location.yNumber, std::min(ceilingHeight, floorHeight), std::max(ceilingHeight, floorHeight));

		if (floor->InsideBridge(x, location.yNumber, z, location.yNumber == ceilingHeight, location.yNumber == floorHeight) >= 0)
		{
			const auto height = GetBottomHeight(*floor, x, location.yNumber, z, &location.roomNumber, &floor);
			if (!height)
				return std::nullopt;

			location.yNumber = *height;
		}

		floorHeight = floor->FloorHeight(x, location.yNumber, z);
		ceilingHeight = floor->CeilingHeight(x, location.yNumber, z);

		if (y < ceilingHeight && floor->RoomAbove(x, location.yNumber, z))
			return std::nullopt;
		if (y <= floorHeight)
		{
			location.yNumber = std::max(y, ceilingHeight);
			return std::optional{location};
		}

		auto roomBelow = floor->RoomBelow(x, location.yNumber, z);
		while (roomBelow)
		{
			floor = &GetFloorSide(*roomBelow, x, z, &location.roomNumber);
			location.yNumber = floor->CeilingHeight(x, z);

			floorHeight = floor->FloorHeight(x, location.yNumber, z);
			ceilingHeight = floor->CeilingHeight(x, location.yNumber, z);

			if (y < ceilingHeight && floor->RoomAbove(x, location.yNumber, z))
				return std::nullopt;
			if (y <= floorHeight)
			{
				location.yNumber = std::max(y, ceilingHeight);
				return std::optional{location};
			}

			roomBelow = floor->RoomBelow(x, location.yNumber, z);
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

			location.yNumber = floor->FloorHeight(x, z);
		}

		auto floorHeight = floor->FloorHeight(x, location.yNumber, z);
		auto ceilingHeight = floor->CeilingHeight(x, location.yNumber, z);

		location.yNumber = std::clamp(location.yNumber, std::min(ceilingHeight, floorHeight), std::max(ceilingHeight, floorHeight));

		if (floor->InsideBridge(x, location.yNumber, z, location.yNumber == ceilingHeight, location.yNumber == floorHeight) >= 0)
		{
			const auto height = GetTopHeight(*floor, x, location.yNumber, z, &location.roomNumber, &floor);
			if (!height)
				return std::nullopt;

			location.yNumber = *height;
		}

		floorHeight = floor->FloorHeight(x, location.yNumber, z);
		ceilingHeight = floor->CeilingHeight(x, location.yNumber, z);

		if (y > floorHeight && floor->RoomBelow(x, location.yNumber, z))
			return std::nullopt;
		if (y >= ceilingHeight)
		{
			location.yNumber = std::min(y, floorHeight);
			return std::optional{location};
		}

		auto roomAbove = floor->RoomAbove(x, location.yNumber, z);
		while (roomAbove)
		{
			floor = &GetFloorSide(*roomAbove, x, z, &location.roomNumber);
			location.yNumber = floor->FloorHeight(x, z);

			floorHeight = floor->FloorHeight(x, location.yNumber, z);
			ceilingHeight = floor->CeilingHeight(x, location.yNumber, z);

			if (y > floorHeight && floor->RoomBelow(x, location.yNumber, z))
				return std::nullopt;
			if (y >= ceilingHeight)
			{
				location.yNumber = std::min(y, floorHeight);
				return std::optional{location};
			}

			roomAbove = floor->RoomAbove(x, location.yNumber, z);
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
		floor->AddItem(itemNumber);

		const auto floorBorder = Objects[item.ObjectNumber].floorBorder(itemNumber);
		while (floorBorder <= floor->CeilingHeight(x, z))
		{
			const auto roomAbove = floor->RoomAbove(x, z);
			if (!roomAbove)
				break;

			floor = &GetFloorSide(*roomAbove, x, z);
			floor->AddItem(itemNumber);
		}

		const auto ceilingBorder = Objects[item.ObjectNumber].ceilingBorder(itemNumber);
		while (ceilingBorder >= floor->FloorHeight(x, z))
		{
			const auto roomBelow = floor->RoomBelow(x, z);
			if (!roomBelow)
				break;

			floor = &GetFloorSide(*roomBelow, x, z);
			floor->AddItem(itemNumber);
		}
	}

	void RemoveBridge(int itemNumber, int x, int z)
	{
		const auto& item = g_Level.Items[itemNumber];
		x += item.Pose.Position.x;
		z += item.Pose.Position.z;

		auto floor = &GetFloorSide(item.RoomNumber, x, z);
		floor->RemoveItem(itemNumber);

		const auto floorBorder = Objects[item.ObjectNumber].floorBorder(itemNumber);
		while (floorBorder <= floor->CeilingHeight(x, z))
		{
			const auto roomAbove = floor->RoomAbove(x, z);
			if (!roomAbove)
				break;

			floor = &GetFloorSide(*roomAbove, x, z);
			floor->RemoveItem(itemNumber);
		}

		const auto ceilingBorder = Objects[item.ObjectNumber].ceilingBorder(itemNumber);
		while (ceilingBorder >= floor->FloorHeight(x, z))
		{
			const auto roomBelow = floor->RoomBelow(x, z);
			if (!roomBelow)
				break;

			floor = &GetFloorSide(*roomBelow, x, z);
			floor->RemoveItem(itemNumber);
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
		auto minX = floor((std::min(std::min(std::min(corners[0].x, corners[1].x), corners[4].x), corners[5].x) - room->x) / SECTOR(1));
		auto minZ = floor((std::min(std::min(std::min(corners[0].z, corners[1].z), corners[4].z), corners[5].z) - room->z) / SECTOR(1));
		auto maxX =  ceil((std::max(std::max(std::max(corners[0].x, corners[1].x), corners[4].x), corners[5].x) - room->x) / SECTOR(1));
		auto maxZ =  ceil((std::max(std::max(std::max(corners[0].z, corners[1].z), corners[4].z), corners[5].z) - room->z) / SECTOR(1));

		// Run through all blocks enclosed in AABB.
		for (int x = 0; x < room->xSize; x++)
		{
			for (int z = 0; z < room->zSize; z++)
			{
				auto pX = room->x + (x * BLOCK(1)) + BLOCK(1 / 2.0f);
				auto pZ = room->z + (z * BLOCK(1)) + BLOCK(1 / 2.0f);
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
