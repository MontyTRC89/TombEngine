#pragma once
#include "trmath.h"
#include "items.h"

struct SECTOR_COLLISION_INFO
{
	float SplitAngle;
	int Portals[2];
	Vector3 Planes[2];
};

class FLOOR_INFO
{
public:
	int index;
	int box;
	int fx;
	int stopper;
	int pitRoom;
	int floor;
	int skyRoom;
	int ceiling;
	SECTOR_COLLISION_INFO FloorCollision;
	SECTOR_COLLISION_INFO CeilingCollision;
	int WallPortal;
	int Room;

	int SectorPlane(int x, int z) const;
	std::optional<int> RoomBelow(int plane) const;
	std::optional<int> RoomBelow(int x, int z) const;
	std::optional<int> RoomAbove(int plane) const;
	std::optional<int> RoomAbove(int x, int z) const;
	std::optional<int> RoomSide() const;
	int FloorHeight(int x, int z) const;
	int CeilingHeight(int x, int z) const;
	Vector2 FloorSlope(int plane) const;
	Vector2 FloorSlope(int x, int z) const;
	Vector2 CeilingSlope(int plane) const;
	Vector2 CeilingSlope(int x, int z) const;
	bool IsWall(int plane) const;
	bool IsWall(int x, int z) const;
};

namespace T5M::Floordata
{
	VectorInt2 GetRoomPosition(int roomNumber, int x, int z);
	FLOOR_INFO& GetFloor(int roomNumber, const VectorInt2 pos);
	FLOOR_INFO& GetFloor(int roomNumber, int x, int z);
	std::tuple<FLOOR_INFO&, int> GetBottomFloor(int startRoomNumber, int x, int z, bool first = false);
	std::tuple<FLOOR_INFO&, int> GetTopFloor(int startRoomNumber, int x, int z, bool first = false);
	std::tuple<FLOOR_INFO&, int> GetNearestBottomFloor(int startRoomNumber, int x, int z);
	std::tuple<FLOOR_INFO&, int> GetNearestTopFloor(int startRoomNumber, int x, int z);
	std::optional<int> GetBottomRoom(int startRoomNumber, int x, int y, int z);
	std::optional<int> GetTopRoom(int startRoomNumber, int x, int y, int z);
	int GetRoom(int startRoomNumber, int x, int y, int z);
	VectorInt2 GetSectorPoint(int x, int z);
	std::optional<int> GetFloorHeight(int startRoomNumber, int x, int y, int z, bool raw = false);
	std::optional<int> GetCeilingHeight(int startRoomNumber, int x, int y, int z, bool raw = false);
	void GetBottomRoomList(int startRoomNumber, int x, int z, std::vector<int>& list);
	void GetTopRoomList(int startRoomNumber, int x, int z, std::vector<int>& list);
	void GetRoomList(int startRoomNumber, int x, int z, std::vector<int>& list);
}
