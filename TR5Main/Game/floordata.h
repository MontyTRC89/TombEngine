#pragma once

struct ROOM_INFO;

struct SECTOR_COLLISION_INFO
{
	float SplitAngle;
	int Portals[2];
	Vector3 Planes[2];
};

struct SECTOR_POSITION
{
	int x;
	int y;
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

	static SECTOR_POSITION GetSectorPosition(ROOM_INFO* room, int x, int z);
	static FLOOR_INFO& GetFloor(ROOM_INFO* room, int x, int z);
	static FLOOR_INFO& GetFloor(ROOM_INFO* room, SECTOR_POSITION pos);
};
