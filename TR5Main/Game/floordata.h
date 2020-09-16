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

	static VectorInt2 GetRoomPosition(int roomNumber, int x, int z);
	static FLOOR_INFO& GetFloor(int roomNumber, VectorInt2 pos);
	static FLOOR_INFO& GetFloor(int roomNumber, int x, int z);
	static int GetBottomRoom(int startRoomNumber, int x, int z);
	static int GetTopRoom(int startRoomNumber, int x, int z);
	static int GetRoom(int startRoomNumber, int x, int y, int z);
	static VectorInt2 GetSectorPoint(int x, int z);

	int SectorPlane(int x, int z);
	std::optional<int> RoomBelow(int plane);
	std::optional<int> RoomBelow(int x, int z);
	std::optional<int> RoomAbove(int plane);
	std::optional<int> RoomAbove(int x, int z);
	std::optional<int> RoomSide();
	int FloorHeight(int x, int z);
	int CeilingHeight(int x, int z);
};
