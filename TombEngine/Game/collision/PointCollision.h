#pragma once
#include "Math/Math.h"

enum RoomEnvFlags;
class FloorInfo;

using namespace TEN::Math;

namespace TEN::Collision
{
	class PointCollision
	{
	public:
		// Members
		const Vector3i Position	  = Vector3i::Zero;
		const int	   RoomNumber = 0;

	private:
		FloorInfo* SectorPtr	   = nullptr;
		FloorInfo* TopSectorPtr	   = nullptr;
		FloorInfo* BottomSectorPtr = nullptr;

		std::optional<int>	   FloorHeight		= std::nullopt;
		std::optional<int>	   CeilingHeight	= std::nullopt;
		std::optional<Vector3> FloorNormal		= std::nullopt;
		std::optional<Vector3> CeilingNormal	= std::nullopt;
		std::optional<int>	   BridgeItemNumber = std::nullopt;

		std::optional<int> WaterSurfaceHeight = std::nullopt;
		std::optional<int> WaterTopHeight	  = std::nullopt;
		std::optional<int> WaterBottomHeight  = std::nullopt;

	public:
		// Constructors
		PointCollision(const Vector3i& pos, int roomNumber);

		// Getters
		FloorInfo& GetSector();
		FloorInfo& GetTopSector();
		FloorInfo& GetBottomSector();

		int		GetFloorHeight();
		int		GetCeilingHeight();
		Vector3 GetFloorNormal();
		Vector3 GetCeilingNormal();
		int		GetBridgeItemNumber();

		int GetWaterSurfaceHeight();
		int GetWaterTopHeight();
		int GetWaterBottomHeight();

		// Inquirers
		bool IsWall();
		bool IsFloorSlope();
		bool IsCeilingSlope();
		bool IsDiagonalStep();
		bool HasDiagonalSplit();
		bool HasFlippedDiagonalSplit();
		bool HasEnvironmentFlag(RoomEnvFlags envFlag);
	};

	PointCollision GetPointCollision(const Vector3i& pos, int roomNumber);
	PointCollision GetPointCollision(const Vector3i& pos, int roomNumber, const Vector3& dir, float dist);
	PointCollision GetPointCollision(const Vector3i& pos, int roomNumber, short headingAngle, float forward, float down = 0.0f, float right = 0.0f);
	PointCollision GetPointCollision(const ItemInfo& item);
	PointCollision GetPointCollision(const ItemInfo& item, const Vector3& dir, float dist);
	PointCollision GetPointCollision(const ItemInfo& item, short headingAngle, float forward, float down = 0.0f, float right = 0.0f);
}
