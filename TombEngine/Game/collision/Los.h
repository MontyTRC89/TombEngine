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
	struct RoomLosCollision
	{
		std::optional<const CollisionTriangle*> Triangle	= std::nullopt;
		std::pair<Vector3, int>					Position	= {};
		std::vector<int>						RoomNumbers = {};

		bool  IsIntersected = false;
		float Distance		= 0.0f;
	};

	struct MoveableLosCollision
	{
		ItemInfo*				Moveable = nullptr;
		std::pair<Vector3, int> Position = {};

		bool  IsOriginContained = false;
		float Distance			= 0.0f;
	};
	
	struct SphereLosCollision
	{
		ItemInfo*				Moveable = nullptr;
		int						SphereID = 0;
		std::pair<Vector3, int> Position = {};

		bool  IsOriginContained = false;
		float Distance			= 0.0f;
	};
	
	struct StaticLosCollision
	{
		MESH_INFO*				Static	 = nullptr;
		std::pair<Vector3, int> Position = {};

		bool  IsOriginContained = false;
		float Distance			= 0.0f;
	};

	struct LosCollision
	{
		RoomLosCollision				  Room		= {};
		std::vector<MoveableLosCollision> Moveables = {};
		std::vector<SphereLosCollision>	  Spheres	= {};
		std::vector<StaticLosCollision>	  Statics	= {};
	};

	// Low-level LOS getter
	LosCollision GetLosCollision(const Vector3& origin, int roomNumber, const Vector3& dir, float dist,
								 bool collideMoveables, bool collideSpheres, bool collideStatics);

	// Basic high-level LOS getters
	RoomLosCollision					GetRoomLosCollision(const Vector3& origin, int roomNumber, const Vector3& dir, float dist, bool collideBridges = true);
	std::optional<MoveableLosCollision> GetMoveableLosCollision(const Vector3& origin, int roomNumber, const Vector3& dir, float dist, bool collidePlayer = false);
	std::optional<SphereLosCollision>   GetSphereLosCollision(const Vector3& origin, int roomNumber, const Vector3& dir, float dist, bool collidePlayer = false);
	std::optional<StaticLosCollision>   GetStaticLosCollision(const Vector3& origin, int roomNumber, const Vector3& dir, float dist, bool collideOnlySolid = true);

	std::pair<GameVector, GameVector> GetRayFrom2DPosition(const Vector2& screenPos);
}
