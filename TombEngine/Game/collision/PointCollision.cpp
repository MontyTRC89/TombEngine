#include "framework.h"
#include "Game/collision/PointCollision.h"

#include "Game/collision/collide_room.h"
#include "Game/collision/floordata.h"
#include "Game/items.h"
#include "Game/room.h"
#include "Math/Math.h"
#include "Specific/level.h"

using namespace TEN::Collision::Floordata;
using namespace TEN::Math;
using namespace TEN::Room;

namespace TEN::Collision
{
	// TEMP: Wrappers to avoid name clashes.
	static std::optional<int> WrapGetFloorHeight(const RoomVector& roomVector, int x, int z)
	{
		return GetFloorHeight(roomVector, x, z);
	}
	
	static std::optional<int> WrapGetCeilingHeight(const RoomVector& roomVector, int x, int z)
	{
		return GetCeilingHeight(roomVector, x, z);
	}
	
	PointCollision::PointCollision(const Vector3i& pos, int roomNumber) :
		Position(pos),
		RoomNumber(roomNumber)
	{
	}

	FloorInfo& PointCollision::GetSector()
	{
		if (SectorPtr != nullptr)
			return *SectorPtr;

		// Set current sector pointer.
		short probedRoomNumber = RoomNumber;
		auto* sectorPtr = GetFloor(Position.x, Position.y, Position.z, &probedRoomNumber);
		SectorPtr = sectorPtr;

		return *SectorPtr;
	}

	FloorInfo& PointCollision::GetTopSector()
	{
		if (TopSectorPtr != nullptr)
			return *TopSectorPtr;

		// Set top sector pointer.
		auto* topSectorPtr = &GetSector();
		while (topSectorPtr->GetRoomNumberAbove(Position.x, Position.y, Position.z).has_value())
		{
			auto roomNumberAbove = topSectorPtr->GetRoomNumberAbove(Position.x, Position.y, Position.z);
			auto& room = g_Level.Rooms[roomNumberAbove.value_or(topSectorPtr->Room)];

			topSectorPtr = TEN::Room::GetSector(&room, Position.x - room.x, Position.z - room.z);
		}
		TopSectorPtr = topSectorPtr;

		return *TopSectorPtr;
	}

	FloorInfo& PointCollision::GetBottomSector()
	{
		if (BottomSectorPtr != nullptr)
			return *BottomSectorPtr;

		// Set bottom sector pointer.
		auto* bottomSectorPtr = &GetSector();
		while (bottomSectorPtr->GetRoomNumberBelow(Position.x, Position.y, Position.z).has_value())
		{
			auto roomNumberBelow = bottomSectorPtr->GetRoomNumberBelow(Position.x, Position.y, Position.z);
			auto& room = g_Level.Rooms[roomNumberBelow.value_or(bottomSectorPtr->Room)];

			bottomSectorPtr = TEN::Room::GetSector(&room, Position.x - room.x, Position.z - room.z);
		}
		BottomSectorPtr = bottomSectorPtr;

		return *BottomSectorPtr;
	}

	int PointCollision::GetFloorHeight()
	{
		if (FloorHeight.has_value())
			return *FloorHeight;

		// Set floor height.
		auto roomVector = RoomVector(GetSector().Room, Position.y);
		FloorHeight = WrapGetFloorHeight(roomVector, Position.x, Position.z).value_or(NO_HEIGHT);
		
		return *FloorHeight;
	}
	
	int PointCollision::GetCeilingHeight()
	{
		if (CeilingHeight.has_value())
			return *CeilingHeight;

		// Set ceiling height.
		auto roomVector = RoomVector(GetSector().Room, Position.y);
		CeilingHeight = WrapGetCeilingHeight(roomVector, Position.x, Position.z).value_or(NO_HEIGHT);
		
		return *CeilingHeight;
	}

	Vector3 PointCollision::GetFloorNormal()
	{
		if (FloorNormal.has_value())
			return *FloorNormal;

		// Set floor normal.
		if (GetBridgeItemNumber() != NO_ITEM)
		{
			// TODO: Get bridge normal.
			FloorNormal = -Vector3::UnitY;
		}
		else
		{
			auto floorTilt = GetBottomSector().GetSurfaceTilt(Position.x, Position.z, true);
			auto floorNormal = GetSurfaceNormal(floorTilt, true);
			FloorNormal = floorNormal;
		}

		return *FloorNormal;
	}

	Vector3 PointCollision::GetCeilingNormal()
	{
		if (CeilingNormal.has_value())
			return *CeilingNormal;

		// Set ceiling normal.
		if (GetBridgeItemNumber() != NO_ITEM)
		{
			// TODO: Get bridge normal.
			CeilingNormal = Vector3::UnitY;
		}
		else
		{
			// TODO: Check GetTopSector().
			auto ceilingTilt = GetTopSector().GetSurfaceTilt(Position.x, Position.z, false);
			auto ceilingNormal = GetSurfaceNormal(ceilingTilt, false);
			CeilingNormal = ceilingNormal;
		}

		return *CeilingNormal;
	}

	int PointCollision::GetBridgeItemNumber()
	{
		if (BridgeItemNumber.has_value())
			return *BridgeItemNumber;

		// Set bridge item number.
		int floorHeight = GetFloorHeight();
		int bridgItemNumber = GetBottomSector().GetInsideBridgeItemNumber(Position.x, floorHeight, Position.z, true, false);
		BridgeItemNumber = bridgItemNumber;

		return *BridgeItemNumber;
	}

	int PointCollision::GetWaterSurfaceHeight()
	{
		if (WaterSurfaceHeight.has_value())
			return *WaterSurfaceHeight;

		// Set water surface height.
		WaterSurfaceHeight = GetWaterSurface(Position.x, Position.y, Position.z, RoomNumber);

		return *WaterSurfaceHeight;
	}

	int PointCollision::GetWaterTopHeight()
	{
		if (WaterTopHeight.has_value())
			return *WaterTopHeight;

		// Set water top height.
		WaterTopHeight = GetWaterHeight(Position.x, Position.y, Position.z, RoomNumber);

		return *WaterTopHeight;
	}

	int PointCollision::GetWaterBottomHeight()
	{
		if (WaterBottomHeight.has_value())
			return *WaterBottomHeight;

		// Set water bottom height.
		WaterBottomHeight = GetWaterDepth(Position.x, Position.y, Position.z, RoomNumber);

		return *WaterBottomHeight;
	}

	bool PointCollision::IsWall()
	{
		return ((GetFloorHeight() == NO_HEIGHT) || (GetCeilingHeight() == NO_HEIGHT));
	}

	bool PointCollision::IsFloorSlope()
	{
		// Get floor slope angle.
		auto floorNormal = GetFloorNormal();
		auto slopeAngle = Geometry::GetSurfaceSlopeAngle(floorNormal);

		return ((GetBridgeItemNumber() == NO_ITEM) && (abs(slopeAngle) >= SLIPPERY_FLOOR_SLOPE_ANGLE));
	}

	bool PointCollision::IsCeilingSlope()
	{
		// Get ceiling slope angle.
		auto ceilingNormal = GetCeilingNormal();
		auto slopeAngle = Geometry::GetSurfaceSlopeAngle(ceilingNormal, -Vector3::UnitY);
		
		return (abs(slopeAngle) >= SLIPPERY_CEILING_SLOPE_ANGLE);
	}

	bool PointCollision::IsDiagonalStep()
	{
		return GetBottomSector().IsSurfaceDiagonalStep(true);
	}

	bool PointCollision::HasDiagonalSplit()
	{
		constexpr auto DIAGONAL_SPLIT_0 = 45.0f * RADIAN;
		constexpr auto DIAGONAL_SPLIT_1 = 135.0f * RADIAN;

		float splitAngle = GetBottomSector().FloorCollision.SplitAngle;
		return ((splitAngle == DIAGONAL_SPLIT_0) || (splitAngle == DIAGONAL_SPLIT_1));
	}

	bool PointCollision::HasFlippedDiagonalSplit()
	{
		constexpr auto DIAGONAL_SPLIT_0 = 45.0f * RADIAN;

		float splitAngle = GetBottomSector().FloorCollision.SplitAngle;
		return (HasDiagonalSplit() && (splitAngle != DIAGONAL_SPLIT_0));
	}

	bool PointCollision::HasEnvironmentFlag(RoomEnvFlags envFlag)
	{
		const auto& room = g_Level.Rooms[RoomNumber];
		return ((room.flags & envFlag) == envFlag);
	}

	PointCollision GetPointCollision(const Vector3i& pos, int roomNumber)
	{
		return PointCollision(pos, roomNumber);
	}

	PointCollision GetPointCollision(const Vector3i& pos, int roomNumber, short headingAngle, float forward, float down, float right)
	{
		short tempRoomNumber = roomNumber;
		const auto& sector = *GetFloor(pos.x, pos.y, pos.z, &tempRoomNumber);

		auto roomVector = RoomVector(sector.Room, pos.y);

		auto probePos = Geometry::TranslatePoint(pos, headingAngle, forward, down, right);
		int adjacentRoomNumber = GetRoom(roomVector, pos.x, probePos.y, pos.z).RoomNumber;
		return PointCollision(probePos, adjacentRoomNumber);
	}

	PointCollision GetPointCollision(const ItemInfo& item)
	{
		return PointCollision(item.Pose.Position, item.RoomNumber);
	}

	// TODO: Find cleaner solution. Constructing a room vector (sometimes dubbed "Location")
	// on the spot for the player can result in a stumble when climbing onto thin platforms. -- Sezz 2022.06.14
	static RoomVector GetEntityRoomVector(const ItemInfo& item)
	{
		if (item.IsLara())
			return item.Location;

		short tempRoomNumber = item.RoomNumber;
		const auto& sector = *GetFloor(item.Pose.Position.x, item.Pose.Position.y, item.Pose.Position.z, &tempRoomNumber);

		return RoomVector(sector.Room, item.Pose.Position.y);
	}

	PointCollision GetPointCollision(const ItemInfo& item, short headingAngle, float forward, float down, float right)
	{
		auto roomVector = GetEntityRoomVector(item);

		auto probePos = Geometry::TranslatePoint(item.Pose.Position, headingAngle, forward, down, right);
		int adjacentRoomNumber = GetRoom(roomVector, item.Pose.Position.x, probePos.y, item.Pose.Position.z).RoomNumber;
		return PointCollision(probePos, adjacentRoomNumber);
	}
}
