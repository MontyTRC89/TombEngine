#pragma once
#include "Math/Math.h"
#include "Specific/newtypes.h"

using namespace TEN::Math;

// GLOSSARY OF TERMS:
// Ceiling:		Upper surface of a grid cell.
// Cell:		Collision unit describing a room's grid cell. Also "block" or "sector".
// Floor:		Lower surface of a grid cell.
// Floordata:	Name of the engine's level geometry collision system consisting of rooms divided into a grid with cells.
// Plane:		One of the two surface triangles.
// Portal:		Link to another room allowing traversal through to it.
// Room number: Unique numeric index of a room.
// Surface:		Floor or ceiling.
// Wall:		Inferred from a floor or ceiling with max height. Note that true "walls" do not exist.

constexpr auto WALL_PLANE = Vector3(0, 0, -CLICK(127));

enum class MaterialType
{
	Mud = 0,
	Snow = 1,
	Sand = 2,
	Gravel = 3,
	Ice = 4,
	Water = 5,
	Stone = 6,
	Wood = 7,
	Metal = 8,
	Marble = 9,
	Grass = 10,
	Concrete = 11,
	OldWood = 12,
	OldMetal = 13,
	Custom1 = 14,
	Custom2 = 15,
	Custom3 = 16,
	Custom4 = 17,
	Custom5 = 18,
	Custom6 = 19,
	Custom7 = 20,
	Custom8 = 21
};

enum class ClimbDirectionFlags
{
	North = (1 << 8),
	East  = (1 << 9),
	South = (1 << 10),
	West  = (1 << 11)
};

struct SurfaceCollisionData
{
private:
	static constexpr auto SURFACE_TRIANGLE_COUNT = 2;

public:
	static constexpr auto SPLIT_ANGLE_1 = 45.0f * RADIAN;
	static constexpr auto SPLIT_ANGLE_2 = 135.0f * RADIAN;

	float SplitAngle = 0.0f;

	std::array<int, SURFACE_TRIANGLE_COUNT>		Portals = {};
	std::array<Vector3, SURFACE_TRIANGLE_COUNT> Planes	= {};
};

struct SectorFlagData
{
	bool Death		 = false;
	bool Monkeyswing = false;
	bool ClimbNorth	 = false;
	bool ClimbSouth	 = false;
	bool ClimbWest	 = false;
	bool ClimbEast	 = false;
	bool MarkBeetle	 = false;

	bool MarkTriggerer		 = false;
	bool MarkTriggererActive = false; // TODO: Must be written to and read from savegames.

	bool MinecartLeft() { return MarkTriggerer; }
	bool MinecartRight() { return MarkBeetle; }
	bool MinecartStop() { return (MarkBeetle && MarkTriggerer); }

	bool IsWallClimbable(ClimbDirectionFlags flag)
	{
		switch (flag)
		{
		case ClimbDirectionFlags::North:
			return ClimbNorth;

		case ClimbDirectionFlags::South:
			return ClimbSouth;

		case ClimbDirectionFlags::East:
			return ClimbEast;

		case ClimbDirectionFlags::West:
			return ClimbWest;
		}

		return false;
	}
};

// Cell
class FloorInfo
{
	public:
		int			  Room		 = 0;
		int			  WallPortal = 0;
		std::set<int> BridgeItem = {};

		int			   Box			= 0;
		bool		   Stopper		= true;
		int			   TriggerIndex = 0;
		MaterialType   Material		= MaterialType::Stone;
		SectorFlagData Flags		= {};

		SurfaceCollisionData FloorCollision	  = {};
		SurfaceCollisionData CeilingCollision = {};

		int		GetSurfacePlaneIndex(int x, int z, bool checkFloor) const;
		Vector2 GetSurfaceTilt(int x, int z, bool checkFloor) const;

		bool IsSurfaceSplit(bool checkFloor) const;
		bool IsSurfaceDiagonalStep(bool checkFloor) const;
		bool IsSurfaceSplitPortal(bool checkFloor) const;

		std::optional<int> RoomBelow(int plane) const;
		std::optional<int> RoomBelow(int x, int z) const;
		std::optional<int> RoomBelow(int x, int y, int z) const;
		std::optional<int> RoomAbove(int plane) const;
		std::optional<int> RoomAbove(int x, int z) const;
		std::optional<int> RoomAbove(int x, int y, int z) const;
		std::optional<int> RoomSide() const;

		int FloorHeight(int x, int z) const;
		int FloorHeight(int x, int y, int z) const;
		int BridgeFloorHeight(int x, int y, int z) const;
		int CeilingHeight(int x, int z) const;
		int CeilingHeight(int x, int y, int z) const;
		int BridgeCeilingHeight(int x, int y, int z) const;

		Vector2 FloorSlope(int plane) const;
		Vector2 FloorSlope(int x, int z) const;
		Vector2 CeilingSlope(int plane) const;
		Vector2 CeilingSlope(int x, int z) const;

		bool IsWall(int plane) const;
		bool IsWall(int x, int z) const;

		int	 InsideBridge(int x, int y, int z, bool floorBorder, bool ceilingBorder) const;
		void AddItem(int itemNumber);
		void RemoveItem(int itemNumber);
};

namespace TEN::Floordata
{
	Vector2i GetSectorPoint(int x, int z);
	Vector2i GetRoomPosition(int roomNumber, int x, int z);
	
	FloorInfo& GetFloor(int roomNumber, const Vector2i& pos);
	FloorInfo& GetFloor(int roomNumber, int x, int z);
	FloorInfo& GetFloorSide(int roomNumber, int x, int z, int* sideRoomNumber = nullptr);
	FloorInfo& GetBottomFloor(int roomNumber, int x, int z, int* bottomRoomNumber = nullptr);
	FloorInfo& GetTopFloor(int roomNumber, int x, int z, int* topRoomNumber = nullptr);
	
	std::optional<int> GetTopHeight(FloorInfo& startFloor, int x, int y, int z, int* topRoomNumber = nullptr, FloorInfo** topFloor = nullptr);
	std::optional<int> GetBottomHeight(FloorInfo& startFloor, int x, int y, int z, int* bottomRoomNumber = nullptr, FloorInfo** bottomFloor = nullptr);
	std::optional<int> GetFloorHeight(const ROOM_VECTOR& location, int x, int z);
	std::optional<int> GetCeilingHeight(const ROOM_VECTOR& location, int x, int z);
	
	std::optional<ROOM_VECTOR> GetBottomRoom(ROOM_VECTOR location, int x, int y, int z);
	std::optional<ROOM_VECTOR> GetTopRoom(ROOM_VECTOR location, int x, int y, int z);
	ROOM_VECTOR				   GetRoom(ROOM_VECTOR location, int x, int y, int z);

	void AddBridge(int itemNumber, int x = 0, int z = 0);
	void RemoveBridge(int itemNumber, int x = 0, int z = 0);

	std::optional<int> GetBridgeItemIntersect(int itemNumber, int x, int y, int z, bool bottom);
	int				   GetBridgeBorder(int itemNumber, bool bottom);
	void			   UpdateBridgeItem(int itemNumber, bool forceRemoval = false);
}
