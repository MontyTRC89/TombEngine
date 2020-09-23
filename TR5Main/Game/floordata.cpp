#include "framework.h"
#include "trmath.h"
#include "floordata.h"
#include "room.h"
#include "level.h"
#include "setup.h"

VectorInt2 FLOOR_INFO::GetRoomPosition(int roomNumber, int x, int z)
{
	auto room = &g_Level.Rooms[roomNumber];
	auto xRoom = (z - room->z) / WALL_SIZE;
	auto yRoom = (x - room->x) / WALL_SIZE;
	auto pos = VectorInt2{xRoom, yRoom};

	if (pos.x < 0)
	{
		pos.x = 0;
	}
	else if (pos.x > room->xSize - 1)
	{
		pos.x = room->xSize - 1;
	}

	if (pos.y < 0)
	{
		pos.y = 0;
	}
	else if (pos.y > room->ySize - 1)
	{
		pos.y = room->ySize - 1;
	}

	return pos;
}

FLOOR_INFO* FLOOR_INFO::GetFloor(int roomNumber, VectorInt2 pos)
{
	auto room = &g_Level.Rooms[roomNumber];
	return &room->floor[room->xSize * pos.y + pos.x];
}

FLOOR_INFO* FLOOR_INFO::GetFloor(int roomNumber, int x, int z)
{
	return GetFloor(roomNumber, GetRoomPosition(roomNumber, x, z));
}

FLOOR_INFO* FLOOR_INFO::GetBottomFloor(int startRoomNumber, int x, int z, bool first)
{
	auto roomNumber = startRoomNumber;
	auto floor = GetFloor(roomNumber, x, z);
	auto plane = floor->SectorPlane(x, z);
	auto roomBelow = floor->RoomBelow(plane);

	while ((!first || floor->IsWall(plane)) && roomBelow)
	{
		roomNumber = *roomBelow;
		floor = GetFloor(roomNumber, x, z);
		auto roomSide = floor->RoomSide();
		if (roomSide)
		{
			roomNumber = *roomSide;
			floor = GetFloor(roomNumber, x, z);
		}
		plane = floor->SectorPlane(x, z);
		roomBelow = floor->RoomBelow(plane);
	}

	return floor;
}

FLOOR_INFO* FLOOR_INFO::GetTopFloor(int startRoomNumber, int x, int z, bool first)
{
	auto roomNumber = startRoomNumber;
	auto floor = GetFloor(roomNumber, x, z);
	auto plane = floor->SectorPlane(x, z);
	auto roomAbove = floor->RoomAbove(plane);

	while ((!first || floor->IsWall(plane)) && roomAbove)
	{
		roomNumber = *roomAbove;
		floor = GetFloor(roomNumber, x, z);
		auto roomSide = floor->RoomSide();
		if (roomSide)
		{
			roomNumber = *roomSide;
			floor = GetFloor(roomNumber, x, z);
		}
		plane = floor->SectorPlane(x, z);
		roomAbove = floor->RoomAbove(plane);
	}

	return floor;
}

FLOOR_INFO* FLOOR_INFO::GetNearBottomFloor(int startRoomNumber, int x, int z)
{
	auto roomNumber = startRoomNumber;
	auto floor = GetFloor(roomNumber, x, z);

	auto roomSide = floor->RoomSide();
	if (roomSide)
	{
		roomNumber = *roomSide;
		floor = GetFloor(roomNumber, x, z);
	}

	if (floor->IsWall(x, z))
	{
		floor = GetTopFloor(roomNumber, x, z, true);
	}
	else
	{
		floor = GetBottomFloor(roomNumber, x, z);
	}

	return floor->IsWall(x, z) ? nullptr : floor;
}

FLOOR_INFO* FLOOR_INFO::GetNearTopFloor(int startRoomNumber, int x, int z)
{
	auto roomNumber = startRoomNumber;
	auto floor = GetFloor(roomNumber, x, z);

	auto roomSide = floor->RoomSide();
	if (roomSide)
	{
		roomNumber = *roomSide;
		floor = GetFloor(roomNumber, x, z);
	}

	if (floor->IsWall(x, z))
	{
		floor = GetBottomFloor(roomNumber, x, z, true);
	}
	else
	{
		floor = GetTopFloor(roomNumber, x, z);
	}

	return floor->IsWall(x, z) ? nullptr : floor;
}

std::optional<int> FLOOR_INFO::GetBottomRoom(int startRoomNumber, int x, int y, int z)
{
	auto roomNumber = startRoomNumber;
	auto floor = GetFloor(roomNumber, x, z);
	auto plane = floor->SectorPlane(x, z);

	if (!floor->IsWall(plane) && y <= floor->FloorHeight(x, z))
		return std::nullopt;

	auto roomBelow = floor->RoomBelow(plane);

	while (roomBelow)
	{
		roomNumber = *roomBelow;
		floor = GetFloor(roomNumber, x, z);
		auto roomSide = floor->RoomSide();
		if (roomSide)
		{
			roomNumber = *roomSide;
			floor = GetFloor(roomNumber, x, z);
		}
		plane = floor->SectorPlane(x, z);

		if (!floor->IsWall(plane) && y <= floor->FloorHeight(x, z))
			return std::optional{roomNumber};

		roomBelow = floor->RoomBelow(plane);
	}

	return std::nullopt;
}

std::optional<int> FLOOR_INFO::GetTopRoom(int startRoomNumber, int x, int y, int z)
{
	auto roomNumber = startRoomNumber;
	auto floor = GetFloor(roomNumber, x, z);
	auto plane = floor->SectorPlane(x, z);

	if (!floor->IsWall(plane) && y >= floor->CeilingHeight(x, z))
		return std::nullopt;

	auto roomAbove = floor->RoomAbove(plane);

	while (roomAbove)
	{
		roomNumber = *roomAbove;
		floor = GetFloor(roomNumber, x, z);
		auto roomSide = floor->RoomSide();
		if (roomSide)
		{
			roomNumber = *roomSide;
			floor = GetFloor(roomNumber, x, z);
		}
		plane = floor->SectorPlane(x, z);

		if (!floor->IsWall(plane) && y >= floor->CeilingHeight(x, z))
			return std::optional{roomNumber};

		roomAbove = floor->RoomAbove(plane);
	}

	return std::nullopt;
}

int FLOOR_INFO::GetRoom(int startRoomNumber, int x, int y, int z)
{
	auto roomNumber = startRoomNumber;
	auto floor = GetFloor(roomNumber, x, z);

	auto roomSide = floor->RoomSide();
	if (roomSide)
	{
		roomNumber = *roomSide;
		floor = GetFloor(roomNumber, x, z);
	}

	auto roomBelow = GetBottomRoom(roomNumber, x, y, z);
	auto roomAbove = GetTopRoom(roomNumber, x, y, z);

	if (roomBelow)
	{
		roomNumber = *roomBelow;
	}
	else if (roomAbove)
	{
		roomNumber = *roomAbove;
	}

	return roomNumber;
}

VectorInt2 FLOOR_INFO::GetSectorPoint(int x, int z)
{
	auto xPoint = x % WALL_SIZE - WALL_SIZE / 2;
	auto yPoint = z % WALL_SIZE - WALL_SIZE / 2;

	return VectorInt2{xPoint, yPoint};
}

std::optional<int> FLOOR_INFO::GetFloorHeight(int startRoomNumber, int x, int y, int z, bool raw)
{
	auto floor = GetNearBottomFloor(startRoomNumber, x, z);

	if (floor)
	{
		auto height = floor->FloorHeight(x, z);
		if (!raw)
		{
			for (auto itemNumber : floor->FloorItem)
			{
				auto item = &g_Level.Items[itemNumber];
				auto itemHeight = Objects[item->objectNumber].floor(itemNumber, x, y, z);
				if (itemHeight >= y && itemHeight < height)
					height = itemHeight;
			}
		}

		return std::optional{height};
	}

	return std::nullopt;
}

std::optional<int> FLOOR_INFO::GetCeilingHeight(int startRoomNumber, int x, int y, int z, bool raw)
{
	auto floor = GetNearTopFloor(startRoomNumber, x, z);

	if (floor)
	{
		auto height = floor->CeilingHeight(x, z);
		if (!raw)
		{
			for (auto itemNumber : floor->CeilingItem)
			{
				auto item = &g_Level.Items[itemNumber];
				auto itemHeight = Objects[item->objectNumber].ceiling(itemNumber, x, y, z);
				if (itemHeight <= y && itemHeight > height)
					height = itemHeight;
			}
		}

		return std::optional{height};
	}

	return std::nullopt;
}

void FLOOR_INFO::AddFloor(short itemNumber)
{
	auto item = &g_Level.Items[itemNumber];
	auto floor = GetNearBottomFloor(item->roomNumber, item->pos.xPos, item->pos.zPos);

	auto begin = floor->FloorItem.cbegin();
	auto end = floor->FloorItem.cend();
	auto position = std::find(begin, end, itemNumber);
	if (position == end)
		floor->FloorItem.push_back(itemNumber);
}

void FLOOR_INFO::RemoveFloor(short itemNumber)
{
	auto item = &g_Level.Items[itemNumber];
	auto floor = GetNearBottomFloor(item->roomNumber, item->pos.xPos, item->pos.zPos);

	auto begin = floor->FloorItem.cbegin();
	auto end = floor->FloorItem.cend();
	auto position = std::find(begin, end, itemNumber);
	if (position != end)
		floor->FloorItem.erase(position);
}

void FLOOR_INFO::AddCeiling(short itemNumber)
{
	auto item = &g_Level.Items[itemNumber];
	auto floor = GetNearTopFloor(item->roomNumber, item->pos.xPos, item->pos.zPos);

	auto begin = floor->CeilingItem.cbegin();
	auto end = floor->CeilingItem.cend();
	auto position = std::find(begin, end, itemNumber);
	if (position == end)
		floor->CeilingItem.push_back(itemNumber);
}

void FLOOR_INFO::RemoveCeiling(short itemNumber)
{
	auto item = &g_Level.Items[itemNumber];
	auto floor = GetNearTopFloor(item->roomNumber, item->pos.xPos, item->pos.zPos);

	auto begin = floor->CeilingItem.cbegin();
	auto end = floor->CeilingItem.cend();
	auto position = std::find(begin, end, itemNumber);
	if (position != end)
		floor->CeilingItem.erase(position);
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
	return room != -1 ? std::optional{room} : std::nullopt;
}

std::optional<int> FLOOR_INFO::RoomBelow(int x, int z)
{
	return RoomBelow(SectorPlane(x, z));
}

std::optional<int> FLOOR_INFO::RoomAbove(int plane)
{
	auto room = CeilingCollision.Portals[plane];
	return room != -1 ? std::optional{room} : std::nullopt;
}

std::optional<int> FLOOR_INFO::RoomAbove(int x, int z)
{
	return RoomAbove(SectorPlane(x, z));
}

std::optional<int> FLOOR_INFO::RoomSide()
{
	return WallPortal != -1 ? std::optional{WallPortal} : std::nullopt;
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
