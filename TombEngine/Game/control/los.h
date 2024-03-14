#pragma once
#include "Game/room.h"
#include "Objects/objectslist.h"
#include "Math/Math.h"

constexpr auto NO_LOS_ITEM = INT_MAX;

// WIP new LOS functions.
std::optional<std::pair<Vector3, int>> GetRoomLos(const Vector3& origin, int originRoomNumber, const Vector3& target, int targetRoomNumber);
std::optional<Vector3>				   GetStaticObjectLos(const Vector3& origin, int roomNumber, const Vector3& dir, float dist, bool onlySolid);

// Legacy LOS functions.
bool LOS(const GameVector* origin, GameVector* target);
bool LOSAndReturnTarget(GameVector* origin, GameVector* target, int push);
bool GetTargetOnLOS(GameVector* origin, GameVector* target, bool drawTarget, bool isFiring);
int	 ObjectOnLOS2(GameVector* origin, GameVector* target, Vector3i* vec, MESH_INFO** mesh, GAME_OBJECT_ID priorityObjectID = GAME_OBJECT_ID::ID_NO_OBJECT);

std::pair<GameVector, GameVector> GetRayFrom2DPosition(const Vector2& screenPos);
