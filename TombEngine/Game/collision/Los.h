#pragma once
#include "Math/Math.h"
#include "Physics/Physics.h"

class GameVector;
struct ItemInfo;
struct MESH_INFO;

using namespace TEN::Math;
using namespace TEN::Physics;

namespace TEN::Collision::Los
{
	struct RoomLosData
	{
		std::optional<const CollisionTriangle*> Triangle	= std::nullopt;
		std::pair<Vector3, int>					Position	= {};
		std::vector<int>						RoomNumbers = {};

		bool  IsIntersected = false;
		float Distance		= 0.0f;
	};

	struct MoveableLosData
	{
		ItemInfo*				Moveable = nullptr;
		std::pair<Vector3, int> Position = {};

		bool  IsOriginContained = false;
		float Distance			= 0.0f;
	};
	
	struct SphereLosData
	{
		ItemInfo*				Moveable = nullptr;
		int						SphereID = 0;
		std::pair<Vector3, int> Position = {};

		bool  IsOriginContained = false;
		float Distance			= 0.0f;
	};
	
	struct StaticLosData
	{
		MESH_INFO*				Static	 = nullptr;
		std::pair<Vector3, int> Position = {};

		bool  IsOriginContained = false;
		float Distance			= 0.0f;
	};

	struct LosData
	{
		RoomLosData					 Room	   = {};
		std::vector<MoveableLosData> Moveables = {};
		std::vector<SphereLosData>	 Spheres   = {};
		std::vector<StaticLosData>	 Statics   = {};
	};

	// Low-level LOS getter
	LosData GetLos(const Vector3& origin, int roomNumber, const Vector3& dir, float dist,
				   bool collideMoveables, bool collideSpheres, bool collideStatics);

	// Basic high-level LOS getters
	RoomLosData					   GetRoomLos(const Vector3& origin, int roomNumber, const Vector3& dir, float dist, bool collideBridges = true);
	std::optional<MoveableLosData> GetMoveableLos(const Vector3& origin, int roomNumber, const Vector3& dir, float dist, bool collidePlayer = false);
	std::optional<SphereLosData>   GetSphereLos(const Vector3& origin, int roomNumber, const Vector3& dir, float dist, bool collidePlayer = false);
	std::optional<StaticLosData>   GetStaticLos(const Vector3& origin, int roomNumber, const Vector3& dir, float dist, bool collideOnlySolid = true);

	std::pair<GameVector, GameVector> GetRayFrom2DPosition(const Vector2& screenPos);
}
