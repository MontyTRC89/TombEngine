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

std::optional<int> FLOOR_INFO::RoomAbove(int plane) const
{
	const auto room = CeilingCollision.Portals[plane];
	return room != -1 ? std::optional{room} : std::nullopt;
}

std::optional<int> FLOOR_INFO::RoomAbove(int x, int z) const
{
	return RoomAbove(SectorPlane(x, z));
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

int FLOOR_INFO::CeilingHeight(int x, int z) const
{
	const auto plane = SectorPlane(x, z);
	const auto vector = GetSectorPoint(x, z);

	return CeilingCollision.Planes[plane].x * vector.x + CeilingCollision.Planes[plane].y * vector.y + CeilingCollision.Planes[plane].z;
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

namespace T5M::Floordata
{
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

	FLOOR_INFO& GetFloor(int roomNumber, const VectorInt2 pos)
	{
		auto& room = g_Level.Rooms[roomNumber];
		return room.floor[room.xSize * pos.y + pos.x];
	}

	FLOOR_INFO& GetFloor(int roomNumber, int x, int z)
	{
		return GetFloor(roomNumber, GetRoomPosition(roomNumber, x, z));
	}

	std::tuple<FLOOR_INFO&, int> GetBottomFloor(int startRoomNumber, int x, int z, bool first)
	{
		auto roomNumber = startRoomNumber;
		auto floor = &GetFloor(roomNumber, x, z);
		auto plane = floor->SectorPlane(x, z);
		auto roomBelow = floor->RoomBelow(plane);

		while ((!first || floor->IsWall(plane)) && roomBelow)
		{
			roomNumber = *roomBelow;
			floor = &GetFloor(roomNumber, x, z);
			const auto roomSide = floor->RoomSide();
			if (roomSide)
			{
				roomNumber = *roomSide;
				floor = &GetFloor(roomNumber, x, z);
			}
			plane = floor->SectorPlane(x, z);
			roomBelow = floor->RoomBelow(plane);
		}

		return std::tie(*floor, roomNumber);
	}

	std::tuple<FLOOR_INFO&, int> GetTopFloor(int startRoomNumber, int x, int z, bool first)
	{
		auto roomNumber = startRoomNumber;
		auto floor = &GetFloor(roomNumber, x, z);
		auto plane = floor->SectorPlane(x, z);
		auto roomAbove = floor->RoomAbove(plane);

		while ((!first || floor->IsWall(plane)) && roomAbove)
		{
			roomNumber = *roomAbove;
			floor = &GetFloor(roomNumber, x, z);
			const auto roomSide = floor->RoomSide();
			if (roomSide)
			{
				roomNumber = *roomSide;
				floor = &GetFloor(roomNumber, x, z);
			}
			plane = floor->SectorPlane(x, z);
			roomAbove = floor->RoomAbove(plane);
		}

		return std::tie(*floor, roomNumber);
	}

	std::tuple<FLOOR_INFO&, int> GetNearestBottomFloor(int startRoomNumber, int x, int z)
	{
		auto roomNumber = startRoomNumber;
		auto floor = &GetFloor(roomNumber, x, z);

		const auto roomSide = floor->RoomSide();
		if (roomSide)
		{
			roomNumber = *roomSide;
			floor = &GetFloor(roomNumber, x, z);
		}

		return floor->IsWall(x, z) ? GetTopFloor(roomNumber, x, z, true) : GetBottomFloor(roomNumber, x, z);
	}

	std::tuple<FLOOR_INFO&, int> GetNearestTopFloor(int startRoomNumber, int x, int z)
	{
		auto roomNumber = startRoomNumber;
		auto floor = &GetFloor(roomNumber, x, z);

		const auto roomSide = floor->RoomSide();
		if (roomSide)
		{
			roomNumber = *roomSide;
			floor = &GetFloor(roomNumber, x, z);
		}

		return floor->IsWall(x, z) ? GetBottomFloor(roomNumber, x, z, true) : GetTopFloor(roomNumber, x, z);
	}

	std::optional<int> GetBottomRoom(int startRoomNumber, int x, int y, int z)
	{
		auto roomNumber = startRoomNumber;
		auto floor = &GetFloor(roomNumber, x, z);
		auto plane = floor->SectorPlane(x, z);

		if (!floor->IsWall(plane) && y <= floor->FloorHeight(x, z) && y >= floor->CeilingHeight(x, z))
			return std::nullopt;

		auto roomBelow = floor->RoomBelow(plane);

		while (roomBelow)
		{
			roomNumber = *roomBelow;
			floor = &GetFloor(roomNumber, x, z);
			const auto roomSide = floor->RoomSide();
			if (roomSide)
			{
				roomNumber = *roomSide;
				floor = &GetFloor(roomNumber, x, z);
			}
			plane = floor->SectorPlane(x, z);

			if (!floor->IsWall(plane) && y <= floor->FloorHeight(x, z) && y >= floor->CeilingHeight(x, z))
				return std::optional{roomNumber};

			roomBelow = floor->RoomBelow(plane);
		}

		return std::nullopt;
	}

	std::optional<int> GetTopRoom(int startRoomNumber, int x, int y, int z)
	{
		auto roomNumber = startRoomNumber;
		auto floor = &GetFloor(roomNumber, x, z);
		auto plane = floor->SectorPlane(x, z);

		if (!floor->IsWall(plane) && y <= floor->FloorHeight(x, z) && y >= floor->CeilingHeight(x, z))
			return std::nullopt;

		auto roomAbove = floor->RoomAbove(plane);

		while (roomAbove)
		{
			roomNumber = *roomAbove;
			floor = &GetFloor(roomNumber, x, z);
			const auto roomSide = floor->RoomSide();
			if (roomSide)
			{
				roomNumber = *roomSide;
				floor = &GetFloor(roomNumber, x, z);
			}
			plane = floor->SectorPlane(x, z);

			if (!floor->IsWall(plane) && y <= floor->FloorHeight(x, z) && y >= floor->CeilingHeight(x, z))
				return std::optional{roomNumber};

			roomAbove = floor->RoomAbove(plane);
		}

		return std::nullopt;
	}

	int GetRoom(int startRoomNumber, int x, int y, int z)
	{
		auto roomNumber = startRoomNumber;
		auto floor = &GetFloor(roomNumber, x, z);

		const auto roomSide = floor->RoomSide();
		if (roomSide)
		{
			roomNumber = *roomSide;
			floor = &GetFloor(roomNumber, x, z);
		}

		const auto roomBelow = GetBottomRoom(roomNumber, x, y, z);
		const auto roomAbove = GetTopRoom(roomNumber, x, y, z);

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

	VectorInt2 GetSectorPoint(int x, int z)
	{
		const auto xPoint = x % SECTOR(1) - SECTOR(1) / 2;
		const auto yPoint = z % SECTOR(1) - SECTOR(1) / 2;

		return VectorInt2{xPoint, yPoint};
	}

	std::optional<int> GetFloorHeight(int startRoomNumber, int x, int y, int z, bool raw)
	{
		const auto [floor, roomNumber] = GetNearestBottomFloor(startRoomNumber, x, z);

		if (!floor.IsWall(x, z))
		{
			auto height = floor.FloorHeight(x, z);
			if (!raw)
			{
				auto list = std::vector<int>{};
				GetRoomList(roomNumber, x, z, list);

				for (const auto stackNumber : list)
				{
					for (auto itemNumber = g_Level.Rooms[stackNumber].itemNumber; itemNumber != NO_ITEM; itemNumber = g_Level.Items[itemNumber].nextItem)
					{
						const auto& item = g_Level.Items[itemNumber];
						if (Objects[item.objectNumber].floor)
						{
							const auto itemHeight = Objects[item.objectNumber].floor(itemNumber, x, y, z);
							if (itemHeight && *itemHeight >= y && *itemHeight < height)
								height = *itemHeight;
						}
					}
				}
			}

			return std::optional{height};
		}

		return std::nullopt;
	}

	std::optional<int> GetCeilingHeight(int startRoomNumber, int x, int y, int z, bool raw)
	{
		const auto [floor, roomNumber] = GetNearestTopFloor(startRoomNumber, x, z);

		if (!floor.IsWall(x, z))
		{
			auto height = floor.CeilingHeight(x, z);
			if (!raw)
			{
				auto list = std::vector<int>{};
				GetRoomList(roomNumber, x, z, list);

				for (const auto stackNumber : list)
				{
					for (auto itemNumber = g_Level.Rooms[stackNumber].itemNumber; itemNumber != NO_ITEM; itemNumber = g_Level.Items[itemNumber].nextItem)
					{
						const auto& item = g_Level.Items[itemNumber];
						if (Objects[item.objectNumber].ceiling)
						{
							const auto itemHeight = Objects[item.objectNumber].ceiling(itemNumber, x, y, z);
							if (itemHeight && *itemHeight <= y && *itemHeight > height)
								height = *itemHeight;
						}
					}
				}
			}

			return std::optional{height};
		}

		return std::nullopt;
	}

	void GetBottomRoomList(int startRoomNumber, int x, int z, std::vector<int>& list)
	{
		auto roomNumber = startRoomNumber;
		auto floor = &GetFloor(roomNumber, x, z);
		auto plane = floor->SectorPlane(x, z);
		auto roomBelow = floor->RoomBelow(plane);

		while (!floor->IsWall(plane) && roomBelow)
		{
			roomNumber = *roomBelow;
			floor = &GetFloor(roomNumber, x, z);
			const auto roomSide = floor->RoomSide();
			if (roomSide)
			{
				roomNumber = *roomSide;
				floor = &GetFloor(roomNumber, x, z);
			}

			plane = floor->SectorPlane(x, z);
			roomBelow = floor->RoomBelow(plane);

			list.push_back(roomNumber);
		}
	}

	void GetTopRoomList(int startRoomNumber, int x, int z, std::vector<int>& list)
	{
		auto roomNumber = startRoomNumber;
		auto floor = &GetFloor(roomNumber, x, z);
		auto plane = floor->SectorPlane(x, z);
		auto roomAbove = floor->RoomAbove(plane);

		while (!floor->IsWall(plane) && roomAbove)
		{
			roomNumber = *roomAbove;
			floor = &GetFloor(roomNumber, x, z);
			const auto roomSide = floor->RoomSide();
			if (roomSide)
			{
				roomNumber = *roomSide;
				floor = &GetFloor(roomNumber, x, z);
			}

			plane = floor->SectorPlane(x, z);
			roomAbove = floor->RoomAbove(plane);

			list.push_back(roomNumber);
		}
	}

	void GetRoomList(int startRoomNumber, int x, int z, std::vector<int>& list)
	{
		auto roomNumber = startRoomNumber;
		auto floor = &GetFloor(roomNumber, x, z);

		const auto roomSide = floor->RoomSide();
		if (roomSide)
		{
			roomNumber = *roomSide;
			floor = &GetFloor(roomNumber, x, z);
		}

		list.push_back(roomNumber);

		GetBottomRoomList(roomNumber, x, z, list);
		GetTopRoomList(roomNumber, x, z, list);
	}
}
