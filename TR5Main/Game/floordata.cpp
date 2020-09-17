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

FLOOR_INFO& FLOOR_INFO::GetBottomFloor(int startRoomNumber, int x, int z, bool first)
{
	auto roomNumber = startRoomNumber;
	auto floor = GetFloor(roomNumber, x, z);
	auto plane = floor.SectorPlane(x, z);
	auto roomBelow = floor.RoomBelow(plane);

	while ((!first || floor.IsWall(plane)) && roomBelow)
	{
		roomNumber = *roomBelow;
		floor = GetFloor(roomNumber, x, z);
		plane = floor.SectorPlane(x, z);
		roomBelow = floor.RoomBelow(plane);
	}

	return floor;
}

FLOOR_INFO& FLOOR_INFO::GetTopFloor(int startRoomNumber, int x, int z, bool first)
{
	auto roomNumber = startRoomNumber;
	auto floor = GetFloor(roomNumber, x, z);
	auto plane = floor.SectorPlane(x, z);
	auto roomAbove = floor.RoomAbove(plane);

	while ((!first || floor.IsWall(plane)) && roomAbove)
	{
		roomNumber = *roomAbove;
		floor = GetFloor(roomNumber, x, z);
		plane = floor.SectorPlane(x, z);
		roomAbove = floor.RoomAbove(plane);
	}

	return floor;
}

FLOOR_INFO* FLOOR_INFO::GetNearBottomFloor(int startRoomNumber, int x, int y, int z)
{
	auto roomNumber = GetRoom(startRoomNumber, x, y, z);

	auto floor = GetFloor(roomNumber, x, z);
	if (floor.IsWall(x, z))
	{
		floor = GetTopFloor(roomNumber, x, z, true);
	}
	else
	{
		floor = GetBottomFloor(roomNumber, x, z);
	}

	return floor.IsWall(x, z) ? nullptr : &floor;
}

FLOOR_INFO* FLOOR_INFO::GetNearTopFloor(int startRoomNumber, int x, int y, int z)
{
	auto roomNumber = GetRoom(startRoomNumber, x, y, z);

	auto floor = GetFloor(roomNumber, x, z);
	if (floor.IsWall(x, z))
	{
		floor = GetBottomFloor(roomNumber, x, z, true);
	}
	else
	{
		floor = GetTopFloor(roomNumber, x, z);
	}

	return floor.IsWall(x, z) ? nullptr : &floor;
}

int FLOOR_INFO::GetRoom(int startRoomNumber, int x, int y, int z)
{
	auto roomNumber = startRoomNumber;
	auto pos = GetRoomPosition(roomNumber, x, z);
	auto& room = g_Level.Rooms[roomNumber];
	auto floor = GetFloor(roomNumber, pos);

	if (pos.x > 0 && pos.y > 0 && pos.x < room.xSize - 1 && pos.y < room.ySize - 1)
	{
		auto plane = floor.SectorPlane(x, z);

		if (!floor.IsWall(plane))
		{
			if (y > floor.FloorHeight(x, z))
			{
				auto roomBelow = floor.RoomBelow(plane);
				if (roomBelow)
					roomNumber = *roomBelow;
			}
			else if (y < floor.CeilingHeight(x, z))
			{
				auto roomAbove = floor.RoomAbove(plane);
				if (roomAbove)
					roomNumber = *roomAbove;
			}
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

std::optional<int> FLOOR_INFO::GetFloorHeight(int startRoomNumber, int x, int y, int z)
{
	auto floor = GetNearBottomFloor(startRoomNumber, x, y, z);
	return floor ? std::optional<int>{floor->FloorHeight(x, z)} : std::nullopt;
}

std::optional<int> FLOOR_INFO::GetCeilingHeight(int startRoomNumber, int x, int y, int z)
{
	auto floor = GetNearTopFloor(startRoomNumber, x, y, z);
	return floor ? std::optional<int>{floor->CeilingHeight(x, z)} : std::nullopt;
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

Vector2 FLOOR_INFO::FloorSlope(int plane)
{
	return Vector2{FloorCollision.Planes[plane].x, FloorCollision.Planes[plane].y};
}

Vector2 FLOOR_INFO::FloorSlope(int x, int z)
{
	return FloorSlope(SectorPlane(x, z));
}

Vector2 FLOOR_INFO::CeilingSlope(int plane)
{
	return Vector2{CeilingCollision.Planes[plane].x, CeilingCollision.Planes[plane].y};
}

Vector2 FLOOR_INFO::CeilingSlope(int x, int z)
{
	return CeilingSlope(SectorPlane(x, z));
}

bool FLOOR_INFO::IsWall(int plane)
{
	return FloorCollision.SplitAngle == CeilingCollision.SplitAngle && FloorCollision.Planes[plane] == CeilingCollision.Planes[plane];
}

bool FLOOR_INFO::IsWall(int x, int z)
{
	return IsWall(SectorPlane(x, z));
}
