#include "framework.h"
#include "trmath.h"
#include "floordata.h"
#include "room.h"
#include "level.h"
#include "setup.h"

using namespace T5M::Floordata;

int FLOOR_INFO::SectorPlane(int x, int z) const
{
	const auto point = GetSectorPoint(x, z);
	auto vector = Vector2(point.x, point.y);
	const auto matrix = Matrix::CreateRotationZ(FloorCollision.SplitAngle);
	Vector2::Transform(vector, matrix, vector);

	return vector.x < 0 ? 0 : 1;
}

std::optional<int> FLOOR_INFO::RoomBelow(int plane) const
{
	const auto room = FloorCollision.Portals[plane];
	return room != -1 ? std::optional{room} : std::nullopt;
}

std::optional<int> FLOOR_INFO::RoomBelow(int x, int z) const
{
	return RoomBelow(SectorPlane(x, z));
}

std::optional<int> FLOOR_INFO::RoomBelow(int x, int z, int y) const
{
	for (const auto itemNumber : BridgeItem)
	{
		const auto& item = g_Level.Items[itemNumber];
		const auto itemHeight = Objects[item.objectNumber].floor(itemNumber, x, y, z);
		if (itemHeight && *itemHeight >= y)
			return std::nullopt;
	}

	return RoomBelow(x, z);
}

std::optional<int> FLOOR_INFO::RoomAbove(int plane) const
{
	const auto room = CeilingCollision.Portals[plane];
	return room != -1 ? std::optional{room} : std::nullopt;
}

std::optional<int> FLOOR_INFO::RoomAbove(int x, int z) const
{
	return RoomAbove(SectorPlane(x, z));
}

std::optional<int> FLOOR_INFO::RoomAbove(int x, int z, int y) const
{
	for (const auto itemNumber : BridgeItem)
	{
		const auto& item = g_Level.Items[itemNumber];
		const auto itemHeight = Objects[item.objectNumber].ceiling(itemNumber, x, y, z);
		if (itemHeight && *itemHeight <= y)
			return std::nullopt;
	}

	return RoomAbove(x, z);
}

std::optional<int> FLOOR_INFO::RoomSide() const
{
	return WallPortal != -1 ? std::optional{WallPortal} : std::nullopt;
}

int FLOOR_INFO::FloorHeight(int x, int z) const
{
	const auto plane = SectorPlane(x, z);
	const auto vector = GetSectorPoint(x, z);

	return FloorCollision.Planes[plane].x * vector.x + FloorCollision.Planes[plane].y * vector.y + FloorCollision.Planes[plane].z;
}

int FLOOR_INFO::FloorHeight(int x, int z, int y) const
{
	auto height = FloorHeight(x, z);

	for (const auto itemNumber : BridgeItem)
	{
		const auto& item = g_Level.Items[itemNumber];
		const auto itemHeight = Objects[item.objectNumber].floor(itemNumber, x, y, z);
		if (itemHeight && *itemHeight >= y && *itemHeight < height)
			height = *itemHeight;
	}

	return height;
}

int FLOOR_INFO::CeilingHeight(int x, int z) const
{
	const auto plane = SectorPlane(x, z);
	const auto vector = GetSectorPoint(x, z);

	return CeilingCollision.Planes[plane].x * vector.x + CeilingCollision.Planes[plane].y * vector.y + CeilingCollision.Planes[plane].z;
}

int FLOOR_INFO::CeilingHeight(int x, int z, int y) const
{
	auto height = CeilingHeight(x, z);

	for (const auto itemNumber : BridgeItem)
	{
		const auto& item = g_Level.Items[itemNumber];
		const auto itemHeight = Objects[item.objectNumber].ceiling(itemNumber, x, y, z);
		if (itemHeight && *itemHeight <= y && *itemHeight > height)
			height = *itemHeight;
	}

	return height;
}

Vector2 FLOOR_INFO::FloorSlope(int plane) const
{
	return Vector2{FloorCollision.Planes[plane].x, FloorCollision.Planes[plane].y};
}

Vector2 FLOOR_INFO::FloorSlope(int x, int z) const
{
	return FloorSlope(SectorPlane(x, z));
}

Vector2 FLOOR_INFO::CeilingSlope(int plane) const
{
	return Vector2{CeilingCollision.Planes[plane].x, CeilingCollision.Planes[plane].y};
}

Vector2 FLOOR_INFO::CeilingSlope(int x, int z) const
{
	return CeilingSlope(SectorPlane(x, z));
}

bool FLOOR_INFO::IsWall(int plane) const
{
	return FloorCollision.SplitAngle == CeilingCollision.SplitAngle && FloorCollision.Planes[plane] == CeilingCollision.Planes[plane];
}

bool FLOOR_INFO::IsWall(int x, int z) const
{
	return IsWall(SectorPlane(x, z));
}

std::optional<int> FLOOR_INFO::InsideBridge(int x, int z, int y, bool floor) const
{
	for (const auto itemNumber : BridgeItem)
	{
		const auto& item = g_Level.Items[itemNumber];
		const auto floorHeight = Objects[item.objectNumber].floor(itemNumber, x, y, z);
		const auto ceilingHeight = Objects[item.objectNumber].ceiling(itemNumber, x, y, z);
		if (floorHeight && ceilingHeight && (y > *floorHeight && y < *ceilingHeight || y == (floor ? *ceilingHeight : *floorHeight)))
			return floor ? floorHeight : ceilingHeight;
	}

	return std::nullopt;
}

int FLOOR_INFO::LogicalHeight(int x, int z, int y, bool floor) const
{
	auto height = InsideBridge(x, z, y, floor);
	while (height)
	{
		y = *height;
		height = InsideBridge(x, z, y, floor);
	}

	return y;
}

void FLOOR_INFO::AddItem(short itemNumber)
{
	BridgeItem.insert(itemNumber);
}

void FLOOR_INFO::RemoveItem(short itemNumber)
{
	BridgeItem.erase(itemNumber);
}

namespace T5M::Floordata
{
	VectorInt2 GetSectorPoint(int x, int z)
	{
		const auto xPoint = x % SECTOR(1) - SECTOR(1) / 2;
		const auto yPoint = z % SECTOR(1) - SECTOR(1) / 2;

		return VectorInt2{xPoint, yPoint};
	}

	VectorInt2 GetRoomPosition(int roomNumber, int x, int z)
	{
		const auto& room = g_Level.Rooms[roomNumber];
		const auto xRoom = (z - room.z) / SECTOR(1);
		const auto yRoom = (x - room.x) / SECTOR(1);
		auto pos = VectorInt2{xRoom, yRoom};

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
		else if (pos.y > room.ySize - 1)
		{
			pos.y = room.ySize - 1;
		}

		return pos;
	}

	FLOOR_INFO& GetFloor(int roomNumber, const VectorInt2& pos)
	{
		auto& room = g_Level.Rooms[roomNumber];
		return room.floor[room.xSize * pos.y + pos.x];
	}

	FLOOR_INFO& GetFloor(int roomNumber, int x, int z)
	{
		return GetFloor(roomNumber, GetRoomPosition(roomNumber, x, z));
	}

	FLOOR_INFO& GetFloorSide(int roomNumber, int x, int z, int* sideRoomNumber)
	{
		auto floor = &GetFloor(roomNumber, x, z);

		const auto roomSide = floor->RoomSide();
		if (roomSide)
		{
			roomNumber = *roomSide;
			floor = &GetFloor(roomNumber, x, z);
		}

		if (sideRoomNumber)
			*sideRoomNumber = roomNumber;

		return *floor;
	}

	FLOOR_INFO& GetBottomFloor(const ROOM_VECTOR& location, int x, int z, int& y, bool firstOutside)
	{
		auto floor = &GetFloorSide(location.roomNumber, x, z);
		auto wall = floor->IsWall(x, z);
		y = location.yNumber;
		if (!wall)
			y = std::clamp(y, floor->CeilingHeight(x, z), floor->FloorHeight(x, z));
		auto roomBelow = floor->RoomBelow(x, z, y);

		while ((firstOutside && wall || !firstOutside && !floor->InsideBridge(x, z, y, true)) && roomBelow)
		{
			floor = &GetFloorSide(*roomBelow, x, z);
			wall = floor->IsWall(x, z);
			if (!wall)
				y = floor->CeilingHeight(x, z);
			roomBelow = floor->RoomBelow(x, z, y);
		}

		return *floor;
	}

	FLOOR_INFO& GetTopFloor(const ROOM_VECTOR& location, int x, int z, int& y, bool firstOutside)
	{
		auto floor = &GetFloorSide(location.roomNumber, x, z);
		auto wall = floor->IsWall(x, z);
		y = location.yNumber;
		if (!wall)
			y = std::clamp(y, floor->CeilingHeight(x, z), floor->FloorHeight(x, z));
		auto roomAbove = floor->RoomAbove(x, z, y);

		while ((firstOutside && wall || !firstOutside && !floor->InsideBridge(x, z, y, false)) && roomAbove)
		{
			floor = &GetFloorSide(*roomAbove, x, z);
			wall = floor->IsWall(x, z);
			if (!wall)
				y = floor->FloorHeight(x, z);
			roomAbove = floor->RoomAbove(x, z, y);
		}

		return *floor;
	}

	std::optional<int> GetFloorHeight(const ROOM_VECTOR& location, int x, int z)
	{
		auto floor = &GetFloorSide(location.roomNumber, x, z);

		int y;
		floor = floor->IsWall(x, z) ? &GetTopFloor(location, x, z, y, true) : &GetBottomFloor(location, x, z, y);

		if (!floor->IsWall(x, z))
		{
			y = floor->LogicalHeight(x, z, y, true);
			return std::optional{floor->FloorHeight(x, z, y)};
		}

		return std::nullopt;
	}

	std::optional<int> GetCeilingHeight(const ROOM_VECTOR& location, int x, int z)
	{
		auto floor = &GetFloorSide(location.roomNumber, x, z);

		int y;
		floor = floor->IsWall(x, z) ? &GetBottomFloor(location, x, z, y, true) : &GetTopFloor(location, x, z, y);

		if (!floor->IsWall(x, z))
		{
			y = floor->LogicalHeight(x, z, y, false);
			return std::optional{floor->CeilingHeight(x, z, y)};
		}

		return std::nullopt;
	}

	std::optional<ROOM_VECTOR> GetBottomRoom(ROOM_VECTOR location, int x, int y, int z)
	{
		auto floor = &GetFloorSide(location.roomNumber, x, z, &location.roomNumber);

		if (!floor->IsWall(x, z))
		{
			location.yNumber = floor->LogicalHeight(x, z, std::clamp(location.yNumber, floor->CeilingHeight(x, z), floor->FloorHeight(x, z)), false);
			const auto floorHeight = floor->FloorHeight(x, z, location.yNumber);
			const auto ceilingHeight = floor->CeilingHeight(x, z, location.yNumber);

			if (y < ceilingHeight)
				return std::nullopt;
			if (y <= floorHeight && y >= ceilingHeight)
			{
				location.yNumber = y;
				return std::optional{location};
			}
		}

		auto roomBelow = floor->RoomBelow(x, z, location.yNumber);

		while (roomBelow)
		{
			floor = &GetFloorSide(*roomBelow, x, z, &location.roomNumber);

			if (!floor->IsWall(x, z))
			{
				location.yNumber = floor->LogicalHeight(x, z, floor->CeilingHeight(x, z), false);
				const auto floorHeight = floor->FloorHeight(x, z, location.yNumber);
				const auto ceilingHeight = floor->CeilingHeight(x, z, location.yNumber);

				if (y < ceilingHeight)
					return std::nullopt;
				if (y <= floorHeight && y >= ceilingHeight)
				{
					location.yNumber = y;
					return std::optional{location};
				}
			}

			roomBelow = floor->RoomBelow(x, z, location.yNumber);
		}

		return std::nullopt;
	}

	std::optional<ROOM_VECTOR> GetTopRoom(ROOM_VECTOR location, int x, int y, int z)
	{
		auto floor = &GetFloorSide(location.roomNumber, x, z, &location.roomNumber);

		if (!floor->IsWall(x, z))
		{
			location.yNumber = floor->LogicalHeight(x, z, std::clamp(location.yNumber, floor->CeilingHeight(x, z), floor->FloorHeight(x, z)), true);
			const auto floorHeight = floor->FloorHeight(x, z, location.yNumber);
			const auto ceilingHeight = floor->CeilingHeight(x, z, location.yNumber);

			if (y > floorHeight)
				return std::nullopt;
			if (y <= floorHeight && y >= ceilingHeight)
			{
				location.yNumber = y;
				return std::optional{location};
			}
		}

		auto roomAbove = floor->RoomAbove(x, z, location.yNumber);

		while (roomAbove)
		{
			floor = &GetFloorSide(*roomAbove, x, z, &location.roomNumber);

			if (!floor->IsWall(x, z))
			{
				location.yNumber = floor->LogicalHeight(x, z, floor->FloorHeight(x, z), true);
				const auto floorHeight = floor->FloorHeight(x, z, location.yNumber);
				const auto ceilingHeight = floor->CeilingHeight(x, z, location.yNumber);

				if (y > floorHeight)
					return std::nullopt;
				if (y <= floorHeight && y >= ceilingHeight)
				{
					location.yNumber = y;
					return std::optional{location};
				}
			}

			roomAbove = floor->RoomAbove(x, z, location.yNumber);
		}

		return std::nullopt;
	}

	ROOM_VECTOR GetRoom(ROOM_VECTOR location, int x, int y, int z)
	{
		const auto locationBelow = GetBottomRoom(location, x, y, z);
		const auto locationAbove = GetTopRoom(location, x, y, z);

		if (locationBelow)
		{
			location = *locationBelow;
		}
		else if (locationAbove)
		{
			location = *locationAbove;
		}

		return location;
	}

	void AddBridge(short itemNumber)
	{
		const auto& item = g_Level.Items[itemNumber];
		auto& floor = GetFloor(item.roomNumber, item.pos.xPos, item.pos.zPos);
		floor.AddItem(itemNumber);
	}

	void RemoveBridge(short itemNumber)
	{
		const auto& item = g_Level.Items[itemNumber];
		auto& floor = GetFloor(item.roomNumber, item.pos.xPos, item.pos.zPos);
		floor.RemoveItem(itemNumber);
	}
}
