#include "framework.h"
#include "Game/collision/Point.h"

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

namespace TEN::Collision::Point
{
	PointCollisionData::PointCollisionData(const Vector3i& pos, int roomNumber)
	{
		_position = pos;
		_roomNumber = roomNumber;
	}

	Vector3i PointCollisionData::GetPosition() const
	{
		return _position;
	}

	int PointCollisionData::GetRoomNumber() const
	{
		return _roomNumber;
	}

	FloorInfo& PointCollisionData::GetSector()
	{
		if (_sector != nullptr)
			return *_sector;

		// Set current sector.
		short probeRoomNumber = _roomNumber;
		_sector = GetFloor(_position.x, _position.y, _position.z, &probeRoomNumber);

		return *_sector;
	}

	FloorInfo& PointCollisionData::GetBottomSector()
	{
		if (_bottomSector != nullptr)
			return *_bottomSector;

		// Set bottom sector.
		auto* bottomSector = &GetSector();
		auto roomNumberBelow = bottomSector->GetNextRoomNumber(_position, true);
		while (roomNumberBelow.has_value())
		{
			int roomNumber = roomNumberBelow.value_or(bottomSector->RoomNumber);
			auto& room = g_Level.Rooms[roomNumber];

			bottomSector = Room::GetSector(&room, _position.x - room.x, _position.z - room.z);
			roomNumberBelow = bottomSector->GetNextRoomNumber(_position, true);
		}
		_bottomSector = bottomSector;

		return *_bottomSector;
	}

	FloorInfo& PointCollisionData::GetTopSector()
	{
		if (_topSector != nullptr)
			return *_topSector;

		// Set top sector.
		auto* topSector = &GetSector();
		auto roomNumberAbove = topSector->GetNextRoomNumber(_position, false);
		while (roomNumberAbove.has_value())
		{
			int roomNumber = roomNumberAbove.value_or(topSector->RoomNumber);
			auto& room = g_Level.Rooms[roomNumber];

			topSector = Room::GetSector(&room, _position.x - room.x, _position.z - room.z);
			roomNumberAbove = topSector->GetNextRoomNumber(_position, false);
		}
		_topSector = topSector;

		return *_topSector;
	}

	int PointCollisionData::GetFloorHeight()
	{
		if (_floorHeight.has_value())
			return *_floorHeight;

		// Set floor height.
		auto location = RoomVector(GetSector().RoomNumber, _position.y);
		_floorHeight = Floordata::GetSurfaceHeight(location, _position.x, _position.z, true).value_or(NO_HEIGHT);
		
		return *_floorHeight;
	}
	
	int PointCollisionData::GetCeilingHeight()
	{
		if (_ceilingHeight.has_value())
			return *_ceilingHeight;

		// Set ceiling height.
		auto location = RoomVector(GetSector().RoomNumber, _position.y);
		_ceilingHeight = Floordata::GetSurfaceHeight(location, _position.x, _position.z, false).value_or(NO_HEIGHT);
		
		return *_ceilingHeight;
	}

	Vector3 PointCollisionData::GetFloorNormal()
	{
		if (_floorNormal.has_value())
			return *_floorNormal;

		// Set floor normal.
		if (GetFloorBridgeItemNumber() != NO_VALUE)
		{
			_floorNormal = GetBridgeNormal(true);
		}
		else
		{
			_floorNormal = GetBottomSector().GetSurfaceNormal(_position.x, _position.z, true);
		}

		return *_floorNormal;
	}

	Vector3 PointCollisionData::GetCeilingNormal()
	{
		if (_ceilingNormal.has_value())
			return *_ceilingNormal;

		// Set ceiling normal.
		if (GetCeilingBridgeItemNumber() != NO_VALUE)
		{
			_ceilingNormal = GetBridgeNormal(false);
		}
		else
		{
			_ceilingNormal = GetTopSector().GetSurfaceNormal(_position.x, _position.z, false);
		}

		return *_ceilingNormal;
	}

	int PointCollisionData::GetFloorBridgeItemNumber()
	{
		if (_floorBridgeItemNumber.has_value())
			return *_floorBridgeItemNumber;

		// Set floor bridge item number.
		int floorHeight = GetFloorHeight();
		auto pos = Vector3i(_position.x, floorHeight, _position.z);
		_floorBridgeItemNumber = GetBottomSector().GetInsideBridgeItemNumber(pos, true, false);

		return *_floorBridgeItemNumber;
	}
	
	int PointCollisionData::GetCeilingBridgeItemNumber()
	{
		if (_ceilingBridgeItemNumber.has_value())
			return *_ceilingBridgeItemNumber;

		// Set ceiling bridge item number.
		int ceilingHeight = GetCeilingHeight();
		auto pos = Vector3i(_position.x, ceilingHeight, _position.z);
		_ceilingBridgeItemNumber = GetTopSector().GetInsideBridgeItemNumber(pos, false, true);

		return *_ceilingBridgeItemNumber;
	}

	int PointCollisionData::GetWaterSurfaceHeight()
	{
		if (_waterSurfaceHeight.has_value())
			return *_waterSurfaceHeight;

		// Set water surface height. TODO: Calculate here.
		_waterSurfaceHeight = GetWaterSurface(_position.x, _position.y, _position.z, _roomNumber);

		return *_waterSurfaceHeight;
	}

	int PointCollisionData::GetWaterBottomHeight()
	{
		if (_waterBottomHeight.has_value())
			return *_waterBottomHeight;

		// Set water bottom height. TODO: Calculate here.
		_waterBottomHeight = GetWaterDepth(_position.x, _position.y, _position.z, _roomNumber);

		return *_waterBottomHeight;
	}

	int PointCollisionData::GetWaterTopHeight()
	{
		if (_waterTopHeight.has_value())
			return *_waterTopHeight;

		// Set water top height. TODO: Calculate here.
		_waterTopHeight = GetWaterHeight(_position.x, _position.y, _position.z, _roomNumber);

		return *_waterTopHeight;
	}

	bool PointCollisionData::IsWall()
	{
		return (GetFloorHeight() == NO_HEIGHT || GetCeilingHeight() == NO_HEIGHT ||
				GetFloorHeight() <= GetCeilingHeight());
	}

	bool PointCollisionData::IsIllegalFloor()
	{
		short slopeAngle = Geometry::GetSurfaceSlopeAngle(GetFloorNormal());
		short illegalSlopeAngle = (GetFloorBridgeItemNumber() != NO_VALUE) ?
			DEFAULT_ILLEGAL_FLOOR_SLOPE_ANGLE :
			GetBottomSector().GetSurfaceIllegalSlopeAngle(_position.x, _position.z, true);
		
		return (abs(slopeAngle) >= illegalSlopeAngle);
	}

	bool PointCollisionData::IsIllegalCeiling()
	{
		short slopeAngle = Geometry::GetSurfaceSlopeAngle(GetCeilingNormal(), -Vector3::UnitY);
		short illegalSlopeAngle = (GetCeilingBridgeItemNumber() != NO_VALUE) ?
			DEFAULT_ILLEGAL_CEILING_SLOPE_ANGLE :
			GetTopSector().GetSurfaceIllegalSlopeAngle(_position.x, _position.z, false);
		
		return (abs(slopeAngle) >= illegalSlopeAngle);
	}

	bool PointCollisionData::IsDiagonalFloorStep()
	{
		return GetBottomSector().IsSurfaceDiagonalStep(true);
	}
	
	bool PointCollisionData::IsDiagonalCeilingStep()
	{
		return GetTopSector().IsSurfaceDiagonalStep(false);
	}

	bool PointCollisionData::IsDiagonalFloorSplit()
	{
		float splitAngle = GetBottomSector().FloorSurface.SplitAngle;
		return (splitAngle == SectorSurfaceData::SPLIT_ANGLE_0 || splitAngle == SectorSurfaceData::SPLIT_ANGLE_1);
	}

	bool PointCollisionData::IsDiagonalCeilingSplit()
	{
		float splitAngle = GetTopSector().CeilingSurface.SplitAngle;
		return (splitAngle == SectorSurfaceData::SPLIT_ANGLE_0 || splitAngle == SectorSurfaceData::SPLIT_ANGLE_1);
	}

	bool PointCollisionData::IsFlippedDiagonalFloorSplit()
	{
		float splitAngle = GetBottomSector().FloorSurface.SplitAngle;
		return (IsDiagonalFloorStep() && splitAngle == SectorSurfaceData::SPLIT_ANGLE_1);
	}
	
	bool PointCollisionData::IsFlippedDiagonalCeilingSplit()
	{
		float splitAngle = GetTopSector().CeilingSurface.SplitAngle;
		return (IsDiagonalCeilingStep() && splitAngle == SectorSurfaceData::SPLIT_ANGLE_1);
	}

	bool PointCollisionData::TestEnvironmentFlag(RoomEnvFlags envFlag)
	{
		const auto& room = g_Level.Rooms[_roomNumber];
		return ((room.flags & envFlag) == envFlag);
	}

	// HACK.
	Vector3 PointCollisionData::GetBridgeNormal(bool isFloor)
	{
		constexpr auto ANGLE_STEP = ANGLE(45.0f / 4);

		int bridgeItemNumber = isFloor ? GetFloorBridgeItemNumber() : GetCeilingBridgeItemNumber();
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
		short probeRoomNumber = GetRoomVector(location, Vector3i(pos.x, probePos.y, pos.z)).RoomNumber;
		GetFloor(probePos.x, probePos.y, probePos.z, &probeRoomNumber);

		return probeRoomNumber;
	}

	static RoomVector GetLocation(const Vector3i& pos, int roomNumber)
	{
		short tempRoomNumber = roomNumber;
		const auto& sector = *GetFloor(pos.x, pos.y, pos.z, &tempRoomNumber);

		return RoomVector(sector.RoomNumber, pos.y);
	}

	static RoomVector GetLocation(const ItemInfo& item)
	{
		// TODO: Find cleaner solution. Manually constructing a player "location" can result in stumbles when climbing onto thin platforms. 
		// May have to do with player's room number being updated at half-height? -- Sezz 2022.06.14
		if (item.IsLara())
			return item.Location;

		short tempRoomNumber = item.RoomNumber;
		const auto& sector = *GetFloor(item.Pose.Position.x, item.Pose.Position.y, item.Pose.Position.z, &tempRoomNumber);

		return RoomVector(sector.RoomNumber, item.Pose.Position.y);
	}

	PointCollisionData GetPointCollision(const Vector3i& pos, int roomNumber)
	{
		// HACK: Ensure room number is correct if position extends to another room.
		// Accounts for some calls to this function which directly pass offset position instead of using dedicated probe overloads.
		GetFloor(pos.x, pos.y, pos.z, (short*)&roomNumber);

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

	PointCollisionData GetPointCollision(const Vector3i& pos, int roomNumber, short headingAngle, float forward, float down, float right, const Vector3& axis)
	{
		// Get "location".
		auto location = GetLocation(pos, roomNumber);

		// Calculate probe position.
		auto probePos = Geometry::TranslatePoint(pos, headingAngle, forward, down, right, axis);
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

	PointCollisionData GetPointCollision(const ItemInfo& item, short headingAngle, float forward, float down, float right, const Vector3& axis)
	{
		// Get "location".
		auto location = GetLocation(item);

		// Calculate probe position.
		auto probePos = Geometry::TranslatePoint(item.Pose.Position, headingAngle, forward, down, right, axis);
		short probeRoomNumber = GetProbeRoomNumber(item.Pose.Position, location, probePos);

		return PointCollisionData(probePos, probeRoomNumber);
	}
}
