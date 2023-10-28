#include "framework.h"
#include "Game/collision/PointCollision.h"

#include "Game/collision/collide_room.h"
#include "Game/collision/floordata.h"
#include "Game/items.h"
#include "Game/room.h"
#include "Game/Setup.h"
#include "Math/Math.h"
#include "Objects/game_object_ids.h"
#include "Specific/level.h"

using namespace TEN::Collision::Floordata;
using namespace TEN::Collision::Room;
using namespace TEN::Math;

namespace TEN::Collision
{
	PointCollisionData::PointCollisionData(const Vector3i& pos, int roomNumber) :
		Position(pos),
		RoomNumber(roomNumber)
	{
	}

	FloorInfo& PointCollisionData::GetSector()
	{
		if (_sectorPtr != nullptr)
			return *_sectorPtr;

		// Set current sector pointer.
		short probeRoomNumber = RoomNumber;
		_sectorPtr = GetFloor(Position.x, Position.y, Position.z, &probeRoomNumber);

		return *_sectorPtr;
	}

	FloorInfo& PointCollisionData::GetBottomSector()
	{
		if (_bottomSectorPtr != nullptr)
			return *_bottomSectorPtr;

		// Set bottom sector pointer.
		auto* bottomSectorPtr = &GetSector();
		auto roomNumberBelow = bottomSectorPtr->GetRoomNumberBelow(Position.x, Position.y, Position.z);
		while (roomNumberBelow.has_value())
		{
			auto& room = g_Level.Rooms[roomNumberBelow.value_or(bottomSectorPtr->Room)];
			bottomSectorPtr = Room::GetSector(&room, Position.x - room.x, Position.z - room.z);

			roomNumberBelow = bottomSectorPtr->GetRoomNumberBelow(Position.x, Position.y, Position.z);
		}
		_bottomSectorPtr = bottomSectorPtr;

		return *_bottomSectorPtr;
	}

	FloorInfo& PointCollisionData::GetTopSector()
	{
		if (_topSectorPtr != nullptr)
			return *_topSectorPtr;

		// Set top sector pointer.
		auto* topSectorPtr = &GetSector();
		auto roomNumberAbove = topSectorPtr->GetRoomNumberAbove(Position.x, Position.y, Position.z);
		while (roomNumberAbove.has_value())
		{
			auto& room = g_Level.Rooms[roomNumberAbove.value_or(topSectorPtr->Room)];
			topSectorPtr = Room::GetSector(&room, Position.x - room.x, Position.z - room.z);

			roomNumberAbove = topSectorPtr->GetRoomNumberAbove(Position.x, Position.y, Position.z);
		}
		_topSectorPtr = topSectorPtr;

		return *_topSectorPtr;
	}

	int PointCollisionData::GetFloorHeight()
	{
		if (_floorHeight.has_value())
			return *_floorHeight;

		// Set floor height.
		auto location = RoomVector(GetSector().Room, Position.y);
		_floorHeight = Floordata::GetFloorHeight(location, Position.x, Position.z).value_or(NO_HEIGHT);
		
		return *_floorHeight;
	}
	
	int PointCollisionData::GetCeilingHeight()
	{
		if (_ceilingHeight.has_value())
			return *_ceilingHeight;

		// Set ceiling height.
		auto location = RoomVector(GetSector().Room, Position.y);
		_ceilingHeight = Floordata::GetCeilingHeight(location, Position.x, Position.z).value_or(NO_HEIGHT);
		
		return *_ceilingHeight;
	}

	Vector3 PointCollisionData::GetFloorNormal()
	{
		if (_floorNormal.has_value())
			return *_floorNormal;

		// Set floor normal.
		if (GetFloorBridgeItemNumber() != NO_ITEM)
		{
			_floorNormal = GetBridgeNormal(true);
		}
		else
		{
			auto floorTilt = GetBottomSector().GetSurfaceTilt(Position.x, Position.z, true);
			_floorNormal = GetSurfaceNormal(floorTilt, true);
		}

		return *_floorNormal;
	}

	Vector3 PointCollisionData::GetCeilingNormal()
	{
		if (_ceilingNormal.has_value())
			return *_ceilingNormal;

		// Set ceiling normal.
		if (GetCeilingBridgeItemNumber() != NO_ITEM)
		{
			_ceilingNormal = GetBridgeNormal(false);
		}
		else
		{
			auto ceilingTilt = GetTopSector().GetSurfaceTilt(Position.x, Position.z, false);
			_ceilingNormal = GetSurfaceNormal(ceilingTilt, false);
		}

		return *_ceilingNormal;
	}

	int PointCollisionData::GetFloorBridgeItemNumber()
	{
		if (_floorBridgeItemNumber.has_value())
			return *_floorBridgeItemNumber;

		// Set floor bridge item number.
		int floorHeight = GetFloorHeight();
		_floorBridgeItemNumber = GetBottomSector().GetInsideBridgeItemNumber(Position.x, floorHeight, Position.z, true, false);

		return *_floorBridgeItemNumber;
	}
	
	int PointCollisionData::GetCeilingBridgeItemNumber()
	{
		if (_ceilingBridgeItemNumber.has_value())
			return *_ceilingBridgeItemNumber;

		// Set ceiling bridge item number.
		int ceilingHeight = GetCeilingHeight();
		_ceilingBridgeItemNumber = GetTopSector().GetInsideBridgeItemNumber(Position.x, ceilingHeight, Position.z, false, true);

		return *_ceilingBridgeItemNumber;
	}

	int PointCollisionData::GetWaterSurfaceHeight()
	{
		if (_waterSurfaceHeight.has_value())
			return *_waterSurfaceHeight;

		// Set water surface height.
		_waterSurfaceHeight = GetWaterSurface(Position.x, Position.y, Position.z, RoomNumber);

		return *_waterSurfaceHeight;
	}

	int PointCollisionData::GetWaterBottomHeight()
	{
		if (_waterBottomHeight.has_value())
			return *_waterBottomHeight;

		// Set water bottom height.
		_waterBottomHeight = GetWaterDepth(Position.x, Position.y, Position.z, RoomNumber);

		return *_waterBottomHeight;
	}

	int PointCollisionData::GetWaterTopHeight()
	{
		if (_waterTopHeight.has_value())
			return *_waterTopHeight;

		// Set water top height.
		_waterTopHeight = GetWaterHeight(Position.x, Position.y, Position.z, RoomNumber);

		return *_waterTopHeight;
	}

	bool PointCollisionData::IsWall()
	{
		return (GetFloorHeight() == NO_HEIGHT || GetCeilingHeight() == NO_HEIGHT ||
				GetFloorHeight() <= GetCeilingHeight());
	}

	bool PointCollisionData::IsSlipperyFloor(short slopeAngleMin)
	{
		auto slopeAngle = Geometry::GetSurfaceSlopeAngle(GetFloorNormal());
		return (abs(slopeAngle) >= slopeAngleMin);
	}

	bool PointCollisionData::IsSlipperyCeiling(short slopeAngleMin)
	{
		auto slopeAngle = Geometry::GetSurfaceSlopeAngle(GetCeilingNormal(), -Vector3::UnitY);
		return (abs(slopeAngle) >= slopeAngleMin);
	}

	bool PointCollisionData::IsDiagonalStep()
	{
		return GetBottomSector().IsSurfaceDiagonalStep(true);
	}

	bool PointCollisionData::HasDiagonalSplit()
	{
		float splitAngle = GetBottomSector().FloorCollision.SplitAngle;
		return (splitAngle == SurfaceCollisionData::SPLIT_ANGLE_0 || splitAngle == SurfaceCollisionData::SPLIT_ANGLE_1);
	}

	bool PointCollisionData::HasFlippedDiagonalSplit()
	{
		float splitAngle = GetBottomSector().FloorCollision.SplitAngle;
		return (HasDiagonalSplit() && splitAngle == SurfaceCollisionData::SPLIT_ANGLE_1);
	}

	bool PointCollisionData::HasEnvironmentFlag(RoomEnvFlags envFlag)
	{
		const auto& room = g_Level.Rooms[RoomNumber];
		return ((room.flags & envFlag) == envFlag);
	}

	// HACK.
	Vector3 PointCollisionData::GetBridgeNormal(bool isFloor)
	{
		constexpr auto ANGLE_STEP = ANGLE(45.0f / 4);

		int bridgeItemNumber = isFloor ? GetFloorBridgeItemNumber() : GetCeilingBridgeItemNumber();
		if (bridgeItemNumber == NO_ITEM)
		{
			TENLog("PointCollisionData error: invalid bridge item number in GetBridgeNormal().", LogLevel::Warning);
			return -Vector3::UnitY;
		}

		const auto& bridgeItem = g_Level.Items[bridgeItemNumber];

		auto orient = bridgeItem.Pose.Orientation;
		switch (bridgeItem.ObjectNumber)
		{
		default:
		case ID_BRIDGE_FLAT:
			break;

		case ID_BRIDGE_TILT1:
			orient.z -= ANGLE_STEP;
			break;

		case ID_BRIDGE_TILT2:
			orient.z -= ANGLE_STEP * 2;
			break;

		case ID_BRIDGE_TILT3:
			orient.z -= ANGLE_STEP * 3;
			break;

		case ID_BRIDGE_TILT4:
			orient.z -= ANGLE_STEP * 4;
			break;
		}

		int sign = isFloor ? -1 : 1;
		return Vector3::Transform(Vector3::UnitY * sign, orient.ToRotationMatrix());
	}

	static int GetProbeRoomNumber(const Vector3i& pos, const RoomVector& location, const Vector3i& probePos)
	{
		// Conduct L-shaped room traversal.
		short probeRoomNumber = GetRoom(location, pos.x, probePos.y, pos.z).RoomNumber;
		GetFloor(probePos.x, probePos.y, probePos.z, &probeRoomNumber);

		return probeRoomNumber;
	}

	static RoomVector GetLocation(const Vector3i& pos, int roomNumber)
	{
		short tempRoomNumber = roomNumber;
		const auto& sector = *GetFloor(pos.x, pos.y, pos.z, &tempRoomNumber);

		return RoomVector(sector.Room, pos.y);
	}

	static RoomVector GetLocation(const ItemInfo& item)
	{
		// TODO: Find cleaner solution. Constructing a "location" for the player on the spot
		// can result in stumbles when climbing onto thin platforms. 
		// May have to do with player's room number being updated at half-height? -- Sezz 2022.06.14
		if (item.IsLara())
			return item.Location;

		short tempRoomNumber = item.RoomNumber;
		const auto& sector = *GetFloor(item.Pose.Position.x, item.Pose.Position.y, item.Pose.Position.z, &tempRoomNumber);

		return RoomVector(sector.Room, item.Pose.Position.y);
	}

	PointCollisionData GetPointCollision(const Vector3i& pos, int roomNumber)
	{
		return PointCollisionData(pos, roomNumber);
	}

	PointCollisionData GetPointCollision(const Vector3i& pos, int roomNumber, const Vector3& dir, float dist)
	{
		// Get "location".
		auto location = GetLocation(pos, roomNumber);

		// Calculate probe position.
		auto probePos = Geometry::TranslatePoint(pos, dir, dist);
		short probeRoomNumber = GetProbeRoomNumber(pos, location, probePos);

		return PointCollisionData(probePos, probeRoomNumber);
	}

	PointCollisionData GetPointCollision(const Vector3i& pos, int roomNumber, short headingAngle, float forward, float down, float right)
	{
		// Get "location".
		auto location = GetLocation(pos, roomNumber);

		// Calculate probe position.
		auto probePos = Geometry::TranslatePoint(pos, headingAngle, forward, down, right);
		short probeRoomNumber = GetProbeRoomNumber(pos, location, probePos);

		return PointCollisionData(probePos, probeRoomNumber);
	}

	PointCollisionData GetPointCollision(const ItemInfo& item)
	{
		return PointCollisionData(item.Pose.Position, item.RoomNumber);
	}

	PointCollisionData GetPointCollision(const ItemInfo& item, const Vector3& dir, float dist)
	{
		// Get "location".
		auto location = GetLocation(item);

		// Calculate probe position.
		auto probePos = Geometry::TranslatePoint(item.Pose.Position, dir, dist);
		short probeRoomNumber = GetProbeRoomNumber(item.Pose.Position, location, probePos);

		return PointCollisionData(probePos, probeRoomNumber);
	}

	PointCollisionData GetPointCollision(const ItemInfo& item, short headingAngle, float forward, float down, float right)
	{
		// Get "location".
		auto location = GetLocation(item);

		// Calculate probe position.
		auto probePos = Geometry::TranslatePoint(item.Pose.Position, headingAngle, forward, down, right);
		short probeRoomNumber = GetProbeRoomNumber(item.Pose.Position, location, probePos);

		return PointCollisionData(probePos, probeRoomNumber);
	}
}
