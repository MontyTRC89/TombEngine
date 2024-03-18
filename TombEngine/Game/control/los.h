#pragma once
#include "Game/room.h"
#include "Objects/objectslist.h"
#include "Math/Math.h"

struct ItemInfo;
struct MESH_INFO;

using ItemSpherePair = std::pair<ItemInfo*, int>;
using LosObjectPtr	 = std::variant<ItemInfo*, MESH_INFO*, ItemSpherePair>;

constexpr auto NO_LOS_ITEM = INT_MAX;

struct LosInstanceData
{
	std::optional<LosObjectPtr> ObjectPtr = std::nullopt;

	Vector3 Position   = Vector3::Zero;
	int		RoomNumber = 0;

	float Distance = 0.0f;
};

struct ItemSphereLosData
{
	std::pair<Vector3, int> Position = {};
	int						SphereID = -1;
};

// Low-level LOS getters
std::vector<LosInstanceData> GetLosInstances(const Vector3& origin, int originRoomNumber, const Vector3& dir, float dist,
											 bool collideItems = true, bool collideStatics = true, bool collideSpheres = false);

// High-level LOS getters
std::optional<std::pair<Vector3, int>> GetRoomLosIntersect(const Vector3& origin, int originRoomNumber, const Vector3& target, int targetRoomNumber,
														   std::optional<std::set<int>*> roomNumbers = std::nullopt);
std::optional<std::pair<Vector3, int>> GetItemLosIntersect(const Vector3& origin, int roomNumber, const Vector3& dir, float dist,
														   bool ignorePlayer = true);
std::optional<std::pair<Vector3, int>> GetStaticLosIntersect(const Vector3& origin, int roomNumber, const Vector3& dir, float dist,
															 bool onlySolid = true);
std::optional<std::pair<Vector3, int>> GetItemSphereLosIntersect(const Vector3& origin, int roomNumber, const Vector3& dir, float dist,
																 bool ignorePlayer = true);

// Legacy LOS functions.
bool LOS(const GameVector* origin, GameVector* target, std::optional<std::set<int>*> roomNumbers = std::nullopt);
bool LOSAndReturnTarget(GameVector* origin, GameVector* target, int push);
bool GetTargetOnLOS(GameVector* origin, GameVector* target, bool drawTarget, bool isFiring);
int	 ObjectOnLOS2(GameVector* origin, GameVector* target, Vector3i* vec, MESH_INFO** mesh, GAME_OBJECT_ID priorityObjectID = GAME_OBJECT_ID::ID_NO_OBJECT);

std::pair<GameVector, GameVector> GetRayFrom2DPosition(const Vector2& screenPos);
