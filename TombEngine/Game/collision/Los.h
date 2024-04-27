#pragma once

class GameVector;
struct ItemInfo;
struct MESH_INFO;

namespace TEN::Collision::Los
{
	struct RoomLosData
	{
		bool					IsIntersected = false;
		std::pair<Vector3, int> Position	  = {};
		std::set<int>			RoomNumbers	  = {};

		float Distance = 0.0f;
	};

	struct MoveableLosData
	{
		ItemInfo*				Moveable;
		std::pair<Vector3, int> Intersect = {};

		float Distance = 0.0f;
	};
	
	struct MoveableSphereLosData
	{
		ItemInfo*				Moveable;
		std::pair<Vector3, int> Intersect = {};
		int						SphereID  = 0;

		float Distance = 0.0f;
	};
	
	struct StaticLosData
	{
		MESH_INFO*				Static;
		std::pair<Vector3, int> Intersect = {};

		float Distance = 0.0f;
	};

	struct LosData
	{
		RoomLosData						   Room			   = {};
		std::vector<MoveableLosData>	   Moveables	   = {};
		std::vector<MoveableSphereLosData> MoveableSpheres = {};
		std::vector<StaticLosData>		   Statics		   = {};
	};

	// Low-level LOS getter
	LosData GetLos(const Vector3& origin, int roomNumber, const Vector3& dir, float dist,
				   bool collideMoveables, bool collideSpheres, bool collideStatics);

	// Basic high-level LOS getters
	RoomLosData							 GetRoomLos(const Vector3& origin, int roomNumber, const Vector3& dir, float dist, bool collideBridges = true);
	std::optional<MoveableLosData>		 GetMoveableLos(const Vector3& origin, int roomNumber, const Vector3& dir, float dist, bool collidePlayer = false);
	std::optional<MoveableSphereLosData> GetMoveableSphereLos(const Vector3& origin, int roomNumber, const Vector3& dir, float dist, bool collidePlayer = false);
	std::optional<StaticLosData>		 GetStaticLos(const Vector3& origin, int roomNumber, const Vector3& dir, float dist, bool collideOnlySolid = true);

	std::pair<GameVector, GameVector> GetRayFrom2DPosition(const Vector2& screenPos);
}
