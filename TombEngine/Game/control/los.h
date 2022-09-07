#pragma once
#include "Game/room.h"
#include "Objects/objectslist.h"
#include "Specific/phd_global.h"

constexpr auto NO_LOS_ITEM = INT_MAX;

bool LOSAndReturnTarget(GameVector* origin, GameVector* target, int push);
bool LOS(GameVector* origin, GameVector* target);
int xLOS(GameVector* origin, GameVector* target);
int zLOS(GameVector* origin, GameVector* target);
bool ClipTarget(GameVector* origin, GameVector* target);
bool GetTargetOnLOS(GameVector* origin, GameVector* target, bool drawTarget, bool isFiring);
int ObjectOnLOS2(GameVector* origin, GameVector* target, Vector3i* vec, MESH_INFO** mesh, GAME_OBJECT_ID priorityObject = GAME_OBJECT_ID::ID_NO_OBJECT);
bool DoRayBox(GameVector* origin, GameVector* target, BOUNDING_BOX* box, PoseData* itemOrStaticPos, Vector3i* hitPos, short closesItemNumber);
