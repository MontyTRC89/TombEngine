#pragma once
#include "trmath.h"
#include "items.h"

struct ROOM_VECTOR
{
	int roomNumber;
	int yNumber;
};

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
	std::set<short> BridgeItem;
	int Room;

	int SectorPlane(int x, int z) const;
	std::optional<int> RoomBelow(int plane) const;
	std::optional<int> RoomBelow(int x, int z) const;
	std::optional<int> RoomBelow(int x, int z, int y) const;
	std::optional<int> RoomAbove(int plane) const;
	std::optional<int> RoomAbove(int x, int z) const;
	std::optional<int> RoomAbove(int x, int z, int y) const;
	std::optional<int> RoomSide() const;
	int FloorHeight(int x, int z) const;
	int FloorHeight(int x, int z, int y) const;
	int CeilingHeight(int x, int z) const;
	int CeilingHeight(int x, int z, int y) const;
	Vector2 FloorSlope(int plane) const;
	Vector2 FloorSlope(int x, int z) const;
	Vector2 CeilingSlope(int plane) const;
	Vector2 CeilingSlope(int x, int z) const;
	bool IsWall(int plane) const;
	bool IsWall(int x, int z) const;
	std::optional<int> InsideBridge(int x, int z, int y, bool floor) const;
	int LogicalHeight(int x, int z, int y, bool floor) const;
	void AddItem(short itemNumber);
	void RemoveItem(short itemNumber);
};

namespace T5M::Floordata
{
	VectorInt2 GetSectorPoint(int x, int z);
	VectorInt2 GetRoomPosition(int roomNumber, int x, int z);
	FLOOR_INFO& GetFloor(int roomNumber, const VectorInt2& pos);
	FLOOR_INFO& GetFloor(int roomNumber, int x, int z);
	FLOOR_INFO& GetFloorSide(int roomNumber, int x, int z, int* sideRoomNumber = nullptr);
	FLOOR_INFO& GetBottomFloor(const ROOM_VECTOR& location, int x, int z, int& y, bool firstOutside = false);
	FLOOR_INFO& GetTopFloor(const ROOM_VECTOR& location, int x, int z, int& y, bool firstOutside = false);
	std::optional<int> GetFloorHeight(const ROOM_VECTOR& location, int x, int z);
	std::optional<int> GetCeilingHeight(const ROOM_VECTOR& location, int x, int z);
	std::optional<ROOM_VECTOR> GetBottomRoom(ROOM_VECTOR location, int x, int y, int z);
	std::optional<ROOM_VECTOR> GetTopRoom(ROOM_VECTOR location, int x, int y, int z);
	ROOM_VECTOR GetRoom(ROOM_VECTOR location, int x, int y, int z);
	void AddBridge(short itemNumber);
	void RemoveBridge(short itemNumber);
}
