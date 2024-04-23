#pragma once

class GameVector;
struct ItemInfo;
struct MESH_INFO;

namespace TEN::Collision::Los
{
	using LosObjectPtr = std::variant<ItemInfo*, MESH_INFO*>;

	struct LosInstanceData
	{
		std::optional<LosObjectPtr> ObjectPtr = std::nullopt;
		int							SphereID  = NO_VALUE;

		Vector3 Position   = Vector3::Zero;
		int		RoomNumber = 0;

		float Distance = 0.0f;
	};

	struct RoomLosData
	{
		std::optional<std::pair<Vector3, int>> Intersect   = {};
		std::set<int>						   RoomNumbers = {};
	};

	struct MoveableLosData
	{
		ItemInfo&				Moveable;
		std::pair<Vector3, int> Intersect = {};
		int						SphereID  = NO_VALUE;
	};
	
	struct StaticLosData
	{
		MESH_INFO&				Static;
		std::pair<Vector3, int> Intersect = {};
	};

	std::vector<LosInstanceData> GetLosInstances(const Vector3& origin, int originRoomNumber, const Vector3& dir, float dist,
												 bool collideMoveables, bool collideSpheres, bool collideStatics);

	RoomLosData					   GetRoomLos(const Vector3& origin, int originRoomNumber, const Vector3& target);
	std::optional<MoveableLosData> GetMoveableLos(const Vector3& origin, int roomNumber, const Vector3& dir, float dist, bool ignorePlayer = true);
	std::optional<MoveableLosData> GetMoveableSphereLos(const Vector3& origin, int roomNumber, const Vector3& dir, float dist, bool ignorePlayer = true);
	std::optional<StaticLosData>   GetStaticLos(const Vector3& origin, int roomNumber, const Vector3& dir, float dist, bool onlySolid = true);

	std::pair<GameVector, GameVector> GetRayFrom2DPosition(const Vector2& screenPos);
}
