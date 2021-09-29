#include "framework.h"
#include "floordata.h"
#include "Specific\trmath.h"
#include "room.h"
#include "level.h"
#include "setup.h"
#include "items.h"

using namespace TEN::Floordata;

int FLOOR_INFO::SectorPlane(int x, int z) const
{
	const auto point = GetSectorPoint(x, z);
	auto vector = Vector2(point.x, point.y);
	const auto matrix = Matrix::CreateRotationZ(FloorCollision.SplitAngle);
	Vector2::Transform(vector, matrix, vector);

	return vector.x < 0 ? 0 : 1;
}

int FLOOR_INFO::SectorPlaneCeiling(int x, int z) const
{
	const auto point = GetSectorPoint(x, z);
	auto vector = Vector2(point.x, point.y);
	const auto matrix = Matrix::CreateRotationZ(CeilingCollision.SplitAngle);
	Vector2::Transform(vector, matrix, vector);

	return vector.x < 0 ? 0 : 1;
}

std::pair<int, int> FLOOR_INFO::TiltXZ(int x, int z) const
{
	auto plane = FloorCollision.Planes[SectorPlane(x, z)];
	auto tiltX = (int)-(plane.x * WALL_SIZE / STEP_SIZE);
	auto tiltZ = (int)-(plane.y * WALL_SIZE / STEP_SIZE);

	return std::make_pair(tiltX, tiltZ);
}

bool FLOOR_INFO::FloorIsSplit() const
{
	bool differentPlanes  = FloorCollision.Planes[0] != FloorCollision.Planes[1];
	return differentPlanes || FloorHasSplitPortal();
}

bool FLOOR_INFO::FloorIsDiagonalStep() const
{
	return FloorIsSplit() && 
		   round(FloorCollision.Planes[0].z) != round(FloorCollision.Planes[1].z) &&
		   (FloorCollision.SplitAngle == 45.0f * RADIAN || FloorCollision.SplitAngle == 135.0f * RADIAN);
}

bool FLOOR_INFO::CeilingIsDiagonalStep() const
{
	return CeilingIsSplit() &&
		round(CeilingCollision.Planes[0].z) != round(CeilingCollision.Planes[1].z) &&
		(CeilingCollision.SplitAngle == 45.0f * RADIAN || CeilingCollision.SplitAngle == 135.0f * RADIAN);
}

bool FLOOR_INFO::CeilingIsSplit() const
{
	bool differentPlanes = CeilingCollision.Planes[0] != CeilingCollision.Planes[1];
	return differentPlanes || CeilingHasSplitPortal();
}

bool FLOOR_INFO::FloorHasSplitPortal() const
{
	return FloorCollision.Portals[0] != FloorCollision.Portals[1];
}

bool FLOOR_INFO::CeilingHasSplitPortal() const
{
	return CeilingCollision.Portals[0] != CeilingCollision.Portals[1];
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
	const auto floorHeight = FloorHeight(x, z);
	const auto ceilingHeight = CeilingHeight(x, z);

	for (const auto itemNumber : BridgeItem)
	{
		const auto& item = g_Level.Items[itemNumber];
		const auto itemHeight = Objects[item.objectNumber].floor(itemNumber, x, y, z);
		if (itemHeight && *itemHeight >= y && *itemHeight <= floorHeight && *itemHeight >= ceilingHeight)
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
	return RoomAbove(SectorPlaneCeiling(x, z));
}

std::optional<int> FLOOR_INFO::RoomAbove(int x, int z, int y) const
{
	const auto floorHeight = FloorHeight(x, z);
	const auto ceilingHeight = CeilingHeight(x, z);

	for (const auto itemNumber : BridgeItem)
	{
		const auto& item = g_Level.Items[itemNumber];
		const auto itemHeight = Objects[item.objectNumber].ceiling(itemNumber, x, y, z);
		if (itemHeight && *itemHeight <= y && *itemHeight <= floorHeight && *itemHeight >= ceilingHeight)
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
	const auto ceilingHeight = CeilingHeight(x, z);

	for (const auto itemNumber : BridgeItem)
	{
		const auto& item = g_Level.Items[itemNumber];
		const auto itemHeight = Objects[item.objectNumber].floor(itemNumber, x, y, z);
		if (itemHeight && *itemHeight >= y && *itemHeight < height && *itemHeight >= ceilingHeight)
			height = *itemHeight;
	}

	return height;
}

int FLOOR_INFO::BridgeFloorHeight(int x, int z, int y) const
{
	for (const auto itemNumber : BridgeItem)
	{
		const auto& item = g_Level.Items[itemNumber];
		const auto floorHeight = Objects[item.objectNumber].floor(itemNumber, x, y, z);
		const auto ceilingHeight = Objects[item.objectNumber].ceiling(itemNumber, x, y, z);
		if (floorHeight && ceilingHeight && y > *floorHeight && y <= *ceilingHeight)
			return *floorHeight;
	}

	return FloorHeight(x, z, y);
}

int FLOOR_INFO::CeilingHeight(int x, int z) const
{
	const auto plane = SectorPlaneCeiling(x, z);
	const auto vector = GetSectorPoint(x, z);

	return CeilingCollision.Planes[plane].x * vector.x + CeilingCollision.Planes[plane].y * vector.y + CeilingCollision.Planes[plane].z;
}

int FLOOR_INFO::CeilingHeight(int x, int z, int y) const
{
	auto height = CeilingHeight(x, z);
	const auto floorHeight = FloorHeight(x, z);

	for (const auto itemNumber : BridgeItem)
	{
		const auto& item = g_Level.Items[itemNumber];
		const auto itemHeight = Objects[item.objectNumber].ceiling(itemNumber, x, y, z);
		if (itemHeight && *itemHeight <= y && *itemHeight > height && *itemHeight <= floorHeight)
			height = *itemHeight;
	}

	return height;
}

int FLOOR_INFO::BridgeCeilingHeight(int x, int z, int y) const
{
	for (const auto itemNumber : BridgeItem)
	{
		const auto& item = g_Level.Items[itemNumber];
		const auto floorHeight = Objects[item.objectNumber].floor(itemNumber, x, y, z);
		const auto ceilingHeight = Objects[item.objectNumber].ceiling(itemNumber, x, y, z);
		if (floorHeight && ceilingHeight && y >= *floorHeight && y < *ceilingHeight)
			return *ceilingHeight;
	}

	return CeilingHeight(x, z, y);
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
	return CeilingSlope(SectorPlaneCeiling(x, z));
}

bool FLOOR_INFO::IsWall(int plane) const
{
	return FloorCollision.SplitAngle == CeilingCollision.SplitAngle && FloorCollision.Planes[plane] == CeilingCollision.Planes[plane];
}

bool FLOOR_INFO::IsWall(int x, int z) const
{
	return IsWall(SectorPlane(x, z));
}

bool FLOOR_INFO::InsideBridge(int x, int z, int y, bool floorBorder, bool ceilingBorder) const
{
	for (const auto itemNumber : BridgeItem)
	{
		const auto& item = g_Level.Items[itemNumber];
		const auto floorHeight = Objects[item.objectNumber].floor(itemNumber, x, y, z);
		const auto ceilingHeight = Objects[item.objectNumber].ceiling(itemNumber, x, y, z);
		if (floorHeight && ceilingHeight && (y > *floorHeight && y < *ceilingHeight || floorBorder && y == *floorHeight || ceilingBorder && y == *ceilingHeight))
			return true;
	}

	return false;
}

void FLOOR_INFO::AddItem(short itemNumber)
{
	BridgeItem.insert(itemNumber);
}

void FLOOR_INFO::RemoveItem(short itemNumber)
{
	BridgeItem.erase(itemNumber);
}

namespace TEN::Floordata
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

	FLOOR_INFO& GetBottomFloor(int roomNumber, int x, int z, int* bottomRoomNumber)
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

	FLOOR_INFO& GetTopFloor(int roomNumber, int x, int z, int* topRoomNumber)
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

	std::optional<int> GetTopHeight(FLOOR_INFO& startFloor, int x, int z, int y, int* topRoomNumber, FLOOR_INFO** topFloor)
	{
		auto floor = &startFloor;
		int roomNumber;
		if (topRoomNumber)
			roomNumber = *topRoomNumber;

		do
		{
			y = floor->BridgeFloorHeight(x, z, y);
			while (y <= floor->CeilingHeight(x, z))
			{
				const auto roomAbove = floor->RoomAbove(x, z);
				if (!roomAbove)
					return std::nullopt;

				floor = &GetFloorSide(*roomAbove, x, z, &roomNumber);
			}
		}
		while (floor->InsideBridge(x, z, y, false, true));

		if (topRoomNumber)
			*topRoomNumber = roomNumber;
		if (topFloor)
			*topFloor = floor;
		return std::optional{y};
	}

	std::optional<int> GetBottomHeight(FLOOR_INFO& startFloor, int x, int z, int y, int* bottomRoomNumber, FLOOR_INFO** bottomFloor)
	{
		auto floor = &startFloor;
		int roomNumber;
		if (bottomRoomNumber)
			roomNumber = *bottomRoomNumber;

		do
		{
			y = floor->BridgeCeilingHeight(x, z, y);
			while (y >= floor->FloorHeight(x, z))
			{
				const auto roomBelow = floor->RoomBelow(x, z);
				if (!roomBelow)
					return std::nullopt;

				floor = &GetFloorSide(*roomBelow, x, z, &roomNumber);
			}
		}
		while (floor->InsideBridge(x, z, y, true, false));

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

		if (floor->IsWall(x, z))
		{
			floor = &GetTopFloor(location.roomNumber, x, z);

			if (!floor->IsWall(x, z))
			{
				y = floor->FloorHeight(x, z);
			}
			else
			{
				floor = &GetBottomFloor(location.roomNumber, x, z);

				if (!floor->IsWall(x, z))
				{
					y = floor->CeilingHeight(x, z);
				}
				else
				{
					return std::nullopt;
				}
			}
		}

		const auto floorHeight = floor->FloorHeight(x, z, y);
		const auto ceilingHeight = floor->CeilingHeight(x, z, y);

		y = std::clamp(y, std::min(ceilingHeight, floorHeight), std::max(ceilingHeight, floorHeight));

		if (floor->InsideBridge(x, z, y, y == ceilingHeight, y == floorHeight))
		{
			auto height = GetTopHeight(*floor, x, z, y);
			if (height)
				return height;

			height = GetBottomHeight(*floor, x, z, y, nullptr, &floor);
			if (!height)
				return std::nullopt;

			y = *height;
		}

		auto roomBelow = floor->RoomBelow(x, z, y);
		while (roomBelow)
		{
			floor = &GetFloorSide(*roomBelow, x, z);
			roomBelow = floor->RoomBelow(x, z, y);
		}

		return std::optional{floor->FloorHeight(x, z, y)};
	}

	std::optional<int> GetCeilingHeight(const ROOM_VECTOR& location, int x, int z)
	{
		auto floor = &GetFloorSide(location.roomNumber, x, z);
		auto y = location.yNumber;

		if (floor->IsWall(x, z))
		{
			floor = &GetBottomFloor(location.roomNumber, x, z);

			if (!floor->IsWall(x, z))
			{
				y = floor->CeilingHeight(x, z);
			}
			else
			{
				floor = &GetTopFloor(location.roomNumber, x, z);

				if (!floor->IsWall(x, z))
				{
					y = floor->FloorHeight(x, z);
				}
				else
				{
					return std::nullopt;
				}
			}
		}

		const auto floorHeight = floor->FloorHeight(x, z, y);
		const auto ceilingHeight = floor->CeilingHeight(x, z, y);

		y = std::clamp(y, std::min(ceilingHeight, floorHeight), std::max(ceilingHeight, floorHeight));

		if (floor->InsideBridge(x, z, y, y == ceilingHeight, y == floorHeight))
		{
			auto height = GetBottomHeight(*floor, x, z, y);
			if (height)
				return height;

			height = GetTopHeight(*floor, x, z, y, nullptr, &floor);
			if (!height)
				return std::nullopt;

			y = *height;
		}

		auto roomAbove = floor->RoomAbove(x, z, y);
		while (roomAbove)
		{
			floor = &GetFloorSide(*roomAbove, x, z);
			roomAbove = floor->RoomAbove(x, z, y);
		}

		return std::optional{floor->CeilingHeight(x, z, y)};
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

		auto floorHeight = floor->FloorHeight(x, z, location.yNumber);
		auto ceilingHeight = floor->CeilingHeight(x, z, location.yNumber);

		location.yNumber = std::clamp(location.yNumber, std::min(ceilingHeight, floorHeight), std::max(ceilingHeight, floorHeight));

		if (floor->InsideBridge(x, z, location.yNumber, location.yNumber == ceilingHeight, location.yNumber == floorHeight))
		{
			const auto height = GetBottomHeight(*floor, x, z, location.yNumber, &location.roomNumber, &floor);
			if (!height)
				return std::nullopt;

			location.yNumber = *height;
		}

		floorHeight = floor->FloorHeight(x, z, location.yNumber);
		ceilingHeight = floor->CeilingHeight(x, z, location.yNumber);

		if (y < ceilingHeight)
			return std::nullopt;
		if (y <= floorHeight && y >= ceilingHeight)
		{
			location.yNumber = y;
			return std::optional{location};
		}

		auto roomBelow = floor->RoomBelow(x, z, location.yNumber);
		while (roomBelow)
		{
			floor = &GetFloorSide(*roomBelow, x, z, &location.roomNumber);
			location.yNumber = floor->CeilingHeight(x, z);

			floorHeight = floor->FloorHeight(x, z, location.yNumber);
			ceilingHeight = floor->CeilingHeight(x, z, location.yNumber);

			if (y < ceilingHeight)
				return std::nullopt;
			if (y <= floorHeight && y >= ceilingHeight)
			{
				location.yNumber = y;
				return std::optional{location};
			}

			roomBelow = floor->RoomBelow(x, z, location.yNumber);
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

		auto floorHeight = floor->FloorHeight(x, z, location.yNumber);
		auto ceilingHeight = floor->CeilingHeight(x, z, location.yNumber);

		location.yNumber = std::clamp(location.yNumber, std::min(ceilingHeight, floorHeight), std::max(ceilingHeight, floorHeight));

		if (floor->InsideBridge(x, z, location.yNumber, location.yNumber == ceilingHeight, location.yNumber == floorHeight))
		{
			const auto height = GetTopHeight(*floor, x, z, location.yNumber, &location.roomNumber, &floor);
			if (!height)
				return std::nullopt;

			location.yNumber = *height;
		}

		floorHeight = floor->FloorHeight(x, z, location.yNumber);
		ceilingHeight = floor->CeilingHeight(x, z, location.yNumber);

		if (y > floorHeight)
			return std::nullopt;
		if (y <= floorHeight && y >= ceilingHeight)
		{
			location.yNumber = y;
			return std::optional{location};
		}

		auto roomAbove = floor->RoomAbove(x, z, location.yNumber);
		while (roomAbove)
		{
			floor = &GetFloorSide(*roomAbove, x, z, &location.roomNumber);
			location.yNumber = floor->FloorHeight(x, z);

			floorHeight = floor->FloorHeight(x, z, location.yNumber);
			ceilingHeight = floor->CeilingHeight(x, z, location.yNumber);

			if (y > floorHeight)
				return std::nullopt;
			if (y <= floorHeight && y >= ceilingHeight)
			{
				location.yNumber = y;
				return std::optional{location};
			}

			roomAbove = floor->RoomAbove(x, z, location.yNumber);
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

	void AddBridge(short itemNumber, int x, int z)
	{
		const auto& item = g_Level.Items[itemNumber];
		x += item.pos.xPos;
		z += item.pos.zPos;

		auto floor = &GetFloorSide(item.roomNumber, x, z);
		floor->AddItem(itemNumber);

		const auto floorBorder = Objects[item.objectNumber].floorBorder(itemNumber);
		while (floorBorder <= floor->CeilingHeight(x, z))
		{
			const auto roomAbove = floor->RoomAbove(x, z);
			if (!roomAbove)
				break;

			floor = &GetFloorSide(*roomAbove, x, z);
			floor->AddItem(itemNumber);
		}

		const auto ceilingBorder = Objects[item.objectNumber].ceilingBorder(itemNumber);
		while (ceilingBorder >= floor->FloorHeight(x, z))
		{
			const auto roomBelow = floor->RoomBelow(x, z);
			if (!roomBelow)
				break;

			floor = &GetFloorSide(*roomBelow, x, z);
			floor->AddItem(itemNumber);
		}
	}

	void RemoveBridge(short itemNumber, int x, int z)
	{
		const auto& item = g_Level.Items[itemNumber];
		x += item.pos.xPos;
		z += item.pos.zPos;

		auto floor = &GetFloorSide(item.roomNumber, x, z);
		floor->RemoveItem(itemNumber);

		const auto floorBorder = Objects[item.objectNumber].floorBorder(itemNumber);
		while (floorBorder <= floor->CeilingHeight(x, z))
		{
			const auto roomAbove = floor->RoomAbove(x, z);
			if (!roomAbove)
				break;

			floor = &GetFloorSide(*roomAbove, x, z);
			floor->RemoveItem(itemNumber);
		}

		const auto ceilingBorder = Objects[item.objectNumber].ceilingBorder(itemNumber);
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
		auto item = &g_Level.Items[itemNumber];

		auto bounds = GetBoundsAccurate(item);
		auto dxBounds = TO_DX_BBOX(item->pos, bounds);

		Vector3 pos = Vector3(x, y + (bottom ? 4 : -4), z); // Introduce slight vertical margin just in case

		static float distance;
		if (dxBounds.Intersects(pos, (bottom ? -Vector3::UnitY : Vector3::UnitY), distance))
			return std::optional{ item->pos.yPos + (bottom ? bounds->Y2 : bounds->Y1) };
		else
			return std::nullopt;
	}

	// Gets bridge min or max height regardless of actual X/Z world position

	int GetBridgeBorder(int itemNumber, bool bottom)
	{
		auto item = &g_Level.Items[itemNumber];

		auto bounds = GetBoundsAccurate(item);
		return item->pos.yPos + (bottom ? bounds->Y2 : bounds->Y1);
	}

	// Updates BridgeItem for all blocks which are enclosed by bridge bounds.

	void UpdateBridgeItem(int itemNumber, bool forceRemoval)
	{
		auto item = &g_Level.Items[itemNumber];

		// Get real OBB bounds of a bridge in world space
		auto bounds = GetBoundsAccurate(item);
		auto dxBounds = TO_DX_BBOX(item->pos, bounds);

		// Get corners of a projected OBB
		Vector3 corners[8];
		dxBounds.GetCorners(corners); //corners[0], corners[1], corners[4] corners[5]

		auto room = &g_Level.Rooms[item->roomNumber];

		// Get min/max of a projected AABB
		auto minX = floor((std::min(std::min(std::min(corners[0].x, corners[1].x), corners[4].x), corners[5].x) - room->x) / SECTOR(1));
		auto minZ = floor((std::min(std::min(std::min(corners[0].z, corners[1].z), corners[4].z), corners[5].z) - room->z) / SECTOR(1));
		auto maxX =  ceil((std::max(std::max(std::max(corners[0].x, corners[1].x), corners[4].x), corners[5].x) - room->x) / SECTOR(1));
		auto maxZ =  ceil((std::max(std::max(std::max(corners[0].z, corners[1].z), corners[4].z), corners[5].z) - room->z) / SECTOR(1));

		// Run through all blocks enclosed in AABB
		for (int x = 0; x < room->ySize; x++)
			for (int z = 0; z < room->xSize; z++)
			{
				auto pX = room->x + (x * WALL_SIZE) + (WALL_SIZE / 2);
				auto pZ = room->z + (z * WALL_SIZE) + (WALL_SIZE / 2);
				auto offX = pX - item->pos.xPos;
				auto offZ = pZ - item->pos.zPos;

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
				auto blockBox = BoundingOrientedBox(Vector3(pX, dxBounds.Center.y, pZ), Vector3(WALL_SIZE / 2), Vector4::UnitY);
				if (dxBounds.Intersects(blockBox))
					AddBridge(itemNumber, offX, offZ); // Intersects, try to add bridge to this block.
			}
	}
}
