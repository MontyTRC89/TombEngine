#pragma once
#include "Game/room.h"
#include "Objects/objectslist.h"
#include "Math/Math.h"

constexpr auto NO_LOS_ITEM = INT_MAX;

bool LOSAndReturnTarget(GameVector* origin, GameVector* target, int push);
bool LOS(GameVector* origin, GameVector* target);
bool ClipTarget(GameVector* origin, GameVector* target);
bool GetTargetOnLOS(GameVector* origin, GameVector* target, bool drawTarget, bool isFiring);
int ObjectOnLOS2(GameVector* origin, GameVector* target, Vector3i* vec, MESH_INFO** mesh, GAME_OBJECT_ID priorityObject = GAME_OBJECT_ID::ID_NO_OBJECT);
