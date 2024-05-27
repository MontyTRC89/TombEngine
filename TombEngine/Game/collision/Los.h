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
	struct RoomLosCollisionData
	{
		const CollisionTriangle* Triangle	 = nullptr;
		Vector3					 Position	 = Vector3::Zero;
		int						 RoomNumber	 = 0;
		std::vector<int>		 RoomNumbers = {};

		bool  IsIntersected = false;
		float Distance		= 0.0f;
	};

	struct MoveableLosCollisionData
	{
		ItemInfo* Moveable	 = nullptr;
		Vector3	  Position	 = Vector3::Zero;
		int		  RoomNumber = 0;

		bool  IsOriginContained = false;
		float Distance			= 0.0f;
	};
	
	struct SphereLosCollisionData
	{
		ItemInfo* Moveable	 = nullptr;
		int		  SphereID	 = 0;
		Vector3	  Position	 = Vector3::Zero;
		int		  RoomNumber = 0;

		bool  IsOriginContained = false;
		float Distance			= 0.0f;
	};
	
	struct StaticLosCollisionData
	{
		MESH_INFO* Static	  = nullptr;
		Vector3	   Position	  = Vector3::Zero;
		int		   RoomNumber = 0;

		bool  IsOriginContained = false;
		float Distance			= 0.0f;
	};

	struct LosCollisionData
	{
		RoomLosCollisionData				  Room		= {};
		std::vector<MoveableLosCollisionData> Moveables = {};
		std::vector<SphereLosCollisionData>	  Spheres	= {};
		std::vector<StaticLosCollisionData>	  Statics	= {};
	};

	// Low-level LOS collision
	LosCollisionData GetLosCollision(const Vector3& origin, int roomNumber, const Vector3& dir, float dist,
									 bool collideMoveables, bool collideSpheres, bool collideStatics);

	// High-level LOS collision
	RoomLosCollisionData					GetRoomLosCollision(const Vector3& origin, int roomNumber, const Vector3& dir, float dist, bool collideBridges = true);
	std::optional<MoveableLosCollisionData> GetMoveableLosCollision(const Vector3& origin, int roomNumber, const Vector3& dir, float dist, bool collidePlayer = false);
	std::optional<SphereLosCollisionData>	GetSphereLosCollision(const Vector3& origin, int roomNumber, const Vector3& dir, float dist, bool collidePlayer = false);
	std::optional<StaticLosCollisionData>	GetStaticLosCollision(const Vector3& origin, int roomNumber, const Vector3& dir, float dist, bool collideOnlySolid = true);

	std::pair<GameVector, GameVector> GetRayFrom2DPosition(const Vector2& screenPos);
}
