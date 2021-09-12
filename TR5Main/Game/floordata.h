#pragma once
#include "Specific\trmath.h"
#include <optional>
#include "roomvector.h"

struct SECTOR_COLLISION_INFO
{
	float SplitAngle;
	int Portals[2];
	Vector3 Planes[2];
};

struct SECTOR_FLAGS
{
	bool Death;
	bool Monkeyswing;
	bool ClimbNorth;
	bool ClimbSouth;
	bool ClimbWest;
	bool ClimbEast;
	bool MarkBeetle;

	bool MarkTriggerer;
	bool MarkTriggererActive; // TODO: IT NEEDS TO BE WRITTEN/READ FROM SAVEGAMES!
};

class FLOOR_INFO
{
	public:
	int index;
	int box;
	int fx;
	int stopper;
	int floor;
	int ceiling;
	SECTOR_COLLISION_INFO FloorCollision;
	SECTOR_COLLISION_INFO CeilingCollision;
	SECTOR_FLAGS Flags;
	int WallPortal;
	std::set<short> BridgeItem;
	int Room;

	// These two functions return single floor/ceiling portal for legacy code.
	// Later, when there will be possible to make 2 separate portals in 2 
	// planes, these functions should be somehow resolved along with legacy code.

	int RoomAbove() const;
	int RoomBelow() const;

	int SectorPlane(int x, int z) const;
	int SectorPlaneCeiling(int x, int z) const;
	std::pair<int, int> FLOOR_INFO::TiltXZ(int x, int z) const;
	bool FloorIsSplit() const;
	bool CeilingIsSplit() const;
	bool FLOOR_INFO::FloorHasSplitPortal() const;
	bool FLOOR_INFO::CeilingHasSplitPortal() const;
	std::optional<int> RoomBelow(int plane) const;
	std::optional<int> RoomBelow(int x, int z) const;
	std::optional<int> RoomBelow(int x, int z, int y) const;
	std::optional<int> RoomAbove(int plane) const;
	std::optional<int> RoomAbove(int x, int z) const;
	std::optional<int> RoomAbove(int x, int z, int y) const;
	std::optional<int> RoomSide() const;
	int FloorHeight(int x, int z) const;
	int FloorHeight(int x, int z, int y) const;
	int BridgeFloorHeight(int x, int z, int y) const;
	int CeilingHeight(int x, int z) const;
	int CeilingHeight(int x, int z, int y) const;
	int BridgeCeilingHeight(int x, int z, int y) const;
	Vector2 FloorSlope(int plane) const;
	Vector2 FloorSlope(int x, int z) const;
	Vector2 CeilingSlope(int plane) const;
	Vector2 CeilingSlope(int x, int z) const;
	bool IsWall(int plane) const;
	bool IsWall(int x, int z) const;
	bool InsideBridge(int x, int z, int y, bool floorBorder, bool ceilingBorder) const;
	void AddItem(short itemNumber);
	void RemoveItem(short itemNumber);
};

namespace TEN::Floordata
{
	VectorInt2 GetSectorPoint(int x, int z);
	VectorInt2 GetRoomPosition(int roomNumber, int x, int z);
	FLOOR_INFO& GetFloor(int roomNumber, const VectorInt2& pos);
	FLOOR_INFO& GetFloor(int roomNumber, int x, int z);
	FLOOR_INFO& GetFloorSide(int roomNumber, int x, int z, int* sideRoomNumber = nullptr);
	FLOOR_INFO& GetBottomFloor(int roomNumber, int x, int z, int* bottomRoomNumber = nullptr);
	FLOOR_INFO& GetTopFloor(int roomNumber, int x, int z, int* topRoomNumber = nullptr);
	std::optional<int> GetTopHeight(FLOOR_INFO& startFloor, int x, int z, int y, int* topRoomNumber = nullptr, FLOOR_INFO** topFloor = nullptr);
	std::optional<int> GetBottomHeight(FLOOR_INFO& startFloor, int x, int z, int y, int* bottomRoomNumber = nullptr, FLOOR_INFO** bottomFloor = nullptr);
	std::optional<int> GetFloorHeight(const ROOM_VECTOR& location, int x, int z);
	std::optional<int> GetCeilingHeight(const ROOM_VECTOR& location, int x, int z);
	std::optional<ROOM_VECTOR> GetBottomRoom(ROOM_VECTOR location, int x, int y, int z);
	std::optional<ROOM_VECTOR> GetTopRoom(ROOM_VECTOR location, int x, int y, int z);
	ROOM_VECTOR GetRoom(ROOM_VECTOR location, int x, int y, int z);
	void AddBridge(short itemNumber, int x = 0, int z = 0);
	void RemoveBridge(short itemNumber, int x = 0, int z = 0);
}
