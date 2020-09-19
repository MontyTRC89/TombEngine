#pragma once
#include "trmath.h"

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

	static VectorInt2 GetRoomPosition(int roomNumber, int x, int z);
	static FLOOR_INFO& GetFloor(int roomNumber, VectorInt2 pos);
	static FLOOR_INFO& GetFloor(int roomNumber, int x, int z);
	static FLOOR_INFO& GetBottomFloor(int startRoomNumber, int x, int z, bool first = false);
	static FLOOR_INFO& GetTopFloor(int startRoomNumber, int x, int z, bool first = false);
	static FLOOR_INFO* GetNearBottomFloor(int startRoomNumber, int x, int z);
	static FLOOR_INFO* GetNearTopFloor(int startRoomNumber, int x, int z);
	static int GetRoom(int startRoomNumber, int x, int y, int z);
	static VectorInt2 GetSectorPoint(int x, int z);
	static std::optional<int> GetFloorHeight(int startRoomNumber, int x, int z);
	static std::optional<int> GetCeilingHeight(int startRoomNumber, int x, int z);

	int SectorPlane(int x, int z);
	std::optional<int> RoomBelow(int plane);
	std::optional<int> RoomBelow(int x, int z);
	std::optional<int> RoomAbove(int plane);
	std::optional<int> RoomAbove(int x, int z);
	std::optional<int> RoomSide();
	int FloorHeight(int x, int z);
	int CeilingHeight(int x, int z);
	Vector2 FloorSlope(int plane);
	Vector2 FloorSlope(int x, int z);
	Vector2 CeilingSlope(int plane);
	Vector2 CeilingSlope(int x, int z);
	bool IsWall(int plane);
	bool IsWall(int x, int z);
};
