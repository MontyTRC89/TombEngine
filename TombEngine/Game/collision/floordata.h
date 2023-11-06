#pragma once
#include "Math/Math.h"
#include "Specific/newtypes.h"

using namespace TEN::Math;

// Ceiling:		  Upper surface of a sector.
// Floor:		  Lower surface of a sector.
// Floordata:	  Name of the engine's level geometry collision system composed of rooms with sectors.
// Plane:		  Mathematical representation of one of two surface triangles.
// Portal:		  Link from one room to another allowing traversal between them.
// Room number:	  Unique ID of a room.
// Room position: Relative grid coordinates of room sectors.
// Sector/block:  Collision data describing a single grid division within a room.
// Surface:		  Floor or ceiling consisting of two triangles.
// Triangle:	  Surface subdivision.
// Wall:		  Inferred from a floor or ceiling with max height. Note that true "walls" don't exist in floordata.
// 
// The way floordata "planes" are stored is non-standard.
// Instead of a Plane object with a normal + distance,
// they use a Vector3 object with data laid out as follows:
// x: X tilt grade (0.25f = 1/4 block).
// y: Z tilt grade (0.25f = 1/4 block).
// z: Plane's absolute height at the sector's center (i.e. distance in regular plane terms).

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
	static constexpr auto SPLIT_ANGLE_0 = 45.0f * RADIAN;
	static constexpr auto SPLIT_ANGLE_1 = 135.0f * RADIAN;

	float SplitAngle = 0.0f;

	std::array<int, SURFACE_TRIANGLE_COUNT>		Portals = {};
	std::array<Vector3, SURFACE_TRIANGLE_COUNT> Planes	= {};
};

struct CollisionBlockFlagData
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

// Collision block
class FloorInfo
{
	public:
		// Components
		int					   Room				 = 0; // RoomNumber
		int					   WallPortal		 = 0; // Number of room through wall portal (only one)?
		SurfaceCollisionData   FloorCollision	 = {};
		SurfaceCollisionData   CeilingCollision  = {};
		CollisionBlockFlagData Flags			 = {};
		std::set<int>		   BridgeItemNumbers = {};

		MaterialType Material = MaterialType::Stone;

		int	 Box		  = 0;
		int	 TriggerIndex = 0;
		bool Stopper	  = true;

		// Getters
		int		GetSurfacePlaneIndex(int x, int z, bool isFloor) const;
		Vector2 GetSurfaceTilt(int x, int z, bool isFloor) const;

		std::optional<int> GetRoomNumberAbove(int planeIndex) const;
		std::optional<int> GetRoomNumberAbove(int x, int z) const;
		std::optional<int> GetRoomNumberAbove(const Vector3i& pos) const;
		std::optional<int> GetRoomNumberBelow(int planeIndex) const;
		std::optional<int> GetRoomNumberBelow(int x, int z) const;
		std::optional<int> GetRoomNumberBelow(const Vector3i& pos) const;
		std::optional<int> GetRoomNumberAtSide() const;

		int GetSurfaceHeight(int x, int z, bool isFloor) const;
		int GetSurfaceHeight(const Vector3i& pos, bool isFloor) const;
		int GetBridgeSurfaceHeight(const Vector3i& pos, bool isFloor) const;

		Vector2 GetSurfaceSlope(int planeIndex, bool isFloor) const;
		Vector2 GetSurfaceSlope(int x, int z, bool isFloor) const;

		// Inquirers
		bool IsSurfaceSplit(bool isFloor) const;
		bool IsSurfaceDiagonalStep(bool isFloor) const;
		bool IsSurfaceSplitPortal(bool isFloor) const;
		bool IsWall(int planeIndex) const;
		bool IsWall(int x, int z) const;

		// Bridge methods
		int	 GetInsideBridgeItemNumber(const Vector3i& pos, bool floorBorder, bool ceilingBorder) const;
		void AddBridge(int itemNumber);
		void RemoveBridge(int itemNumber);
};

namespace TEN::Collision::Floordata
{
	// TODO: Use normals natively.
	Vector3 GetSurfaceNormal(const Vector2& tilt, bool isFloor);

	Vector2i GetSectorPoint(int x, int z);
	Vector2i GetRoomPosition(int roomNumber, int x, int z);
	
	FloorInfo& GetFloor(int roomNumber, const Vector2i& roomPos);
	FloorInfo& GetFloor(int roomNumber, int x, int z);
	FloorInfo& GetFloorSide(int roomNumber, int x, int z, int* sideRoomNumber = nullptr);
	FloorInfo& GetBottomFloor(int roomNumber, int x, int z, int* bottomRoomNumber = nullptr);
	FloorInfo& GetTopFloor(int roomNumber, int x, int z, int* topRoomNumber = nullptr);
	
	std::optional<int> GetTopHeight(FloorInfo& startSector, Vector3i pos, int* topRoomNumberPtr = nullptr, FloorInfo** topSectorPtr = nullptr);
	std::optional<int> GetBottomHeight(FloorInfo& startSector, Vector3i pos, int* bottomRoomNumberPtr = nullptr, FloorInfo** bottomSectorPtr = nullptr);
	std::optional<int> GetFloorHeight(const ROOM_VECTOR& location, int x, int z);
	std::optional<int> GetCeilingHeight(const ROOM_VECTOR& location, int x, int z);
	
	std::optional<ROOM_VECTOR> GetBottomRoom(ROOM_VECTOR location, const Vector3i& pos);
	std::optional<ROOM_VECTOR> GetTopRoom(ROOM_VECTOR location, const Vector3i& pos);
	ROOM_VECTOR				   GetRoom(ROOM_VECTOR location, const Vector3i& pos);

	void AddBridge(int itemNumber, int x = 0, int z = 0);
	void RemoveBridge(int itemNumber, int x = 0, int z = 0);

	std::optional<int> GetBridgeItemIntersect(const Vector3i& pos, int itemNumber, bool useBottomHeight);
	int				   GetBridgeBorder(int itemNumber, bool bottom);
	void			   UpdateBridgeItem(int itemNumber, bool forceRemoval = false);

	bool TestMaterial(MaterialType refMaterial, const std::vector<MaterialType>& materials);
	
	void DrawNearbySectorFlags(const ItemInfo& item);
}
