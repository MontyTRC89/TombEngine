#include "framework.h"
#include "trmath.h"
#include "floordata.h"
#include "room.h"
#include "level.h"

VectorInt2 FLOOR_INFO::GetRoomPosition(int roomNumber, int x, int z)
{
	auto& room = g_Level.Rooms[roomNumber];
	auto xRoom = (x - room.x) / WALL_SIZE;
	auto zRoom = (z - room.z) / WALL_SIZE;
	auto pos = VectorInt2{xRoom, zRoom};

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

FLOOR_INFO& FLOOR_INFO::GetFloor(int roomNumber, VectorInt2 pos)
{
	auto& room = g_Level.Rooms[roomNumber];
	return room.floor[pos.x + room.xSize * pos.y];
}

FLOOR_INFO& FLOOR_INFO::GetFloor(int roomNumber, int x, int z)
{
	return GetFloor(roomNumber, GetRoomPosition(roomNumber, x, z));
}

int FLOOR_INFO::GetBottomRoom(int startRoomNumber, int x, int z)
{
	auto roomNumber = startRoomNumber;
	auto roomBelow = GetFloor(roomNumber, x, z).RoomBelow(x, z);

	while (roomBelow)
	{
		roomNumber = *roomBelow;
		roomBelow = GetFloor(roomNumber, x, z).RoomBelow(x, z);
	}

	return roomNumber;
}

int FLOOR_INFO::GetTopRoom(int startRoomNumber, int x, int z)
{
	auto roomNumber = startRoomNumber;
	auto roomAbove = GetFloor(roomNumber, x, z).RoomAbove(x, z);

	while (roomAbove)
	{
		roomNumber = *roomAbove;
		roomAbove = GetFloor(roomNumber, x, z).RoomAbove(x, z);
	}

	return roomNumber;
}

int FLOOR_INFO::GetRoom(int startRoomNumber, int x, int y, int z)
{
	auto roomNumber = startRoomNumber;
	auto pos = GetRoomPosition(roomNumber, x, z);
	auto& room = g_Level.Rooms[roomNumber];
	auto floor = GetFloor(roomNumber, pos);

	if (pos.x > 0 && pos.y > 0 && pos.x < room.xSize - 1 && pos.y < room.ySize - 1)
	{
		if (y > floor.FloorHeight(x, z))
		{
			auto roomBelow = floor.RoomBelow(x, z);
			if (roomBelow)
				roomNumber = *roomBelow;
		}
		else if (y < floor.CeilingHeight(x, z))
		{
			auto roomAbove = floor.RoomAbove(x, z);
			if (roomAbove)
				roomNumber = *roomAbove;
		}
	}
	else
	{
		auto roomSide = floor.RoomSide();
		if (roomSide)
			roomNumber = *roomSide;
	}

	return roomNumber;
}

VectorInt2 FLOOR_INFO::GetSectorPoint(int x, int z)
{
	auto xPoint = x % WALL_SIZE - WALL_SIZE / 2;
	auto yPoint = z % WALL_SIZE - WALL_SIZE / 2;

	return VectorInt2{xPoint, yPoint};
}

int FLOOR_INFO::SectorPlane(int x, int z)
{
	auto vector = GetSectorPoint(x, z);
	auto matrix = Matrix::CreateRotationZ(FloorCollision.SplitAngle);
	auto result = Vector2::Transform(Vector2(vector.x, vector.y), matrix);

	return result.x < 0 ? 0 : 1;
}

std::optional<int> FLOOR_INFO::RoomBelow(int plane)
{
	auto room = FloorCollision.Portals[plane];
	return room != -1 ? std::optional<int>{room} : std::nullopt;
}

std::optional<int> FLOOR_INFO::RoomBelow(int x, int z)
{
	return RoomBelow(SectorPlane(x, z));
}

std::optional<int> FLOOR_INFO::RoomAbove(int plane)
{
	auto room = CeilingCollision.Portals[plane];
	return room != -1 ? std::optional<int>{room} : std::nullopt;
}

std::optional<int> FLOOR_INFO::RoomAbove(int x, int z)
{
	return RoomAbove(SectorPlane(x, z));
}

std::optional<int> FLOOR_INFO::RoomSide()
{
	return WallPortal != -1 ? std::optional<int>{WallPortal} : std::nullopt;
}

int FLOOR_INFO::FloorHeight(int x, int z)
{
	auto plane = SectorPlane(x, z);
	auto vector = GetSectorPoint(x, z);

	return FloorCollision.Planes[plane].x * vector.x + FloorCollision.Planes[plane].y * vector.y + FloorCollision.Planes[plane].z;
}

int FLOOR_INFO::CeilingHeight(int x, int z)
{
	auto plane = SectorPlane(x, z);
	auto vector = GetSectorPoint(x, z);

	return CeilingCollision.Planes[plane].x * vector.x + CeilingCollision.Planes[plane].y * vector.y + CeilingCollision.Planes[plane].z;
}
