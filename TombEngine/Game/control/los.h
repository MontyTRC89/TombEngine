#pragma once
#include "Game/room.h"
#include "Objects/objectslist.h"
#include "Specific/phd_global.h"

constexpr auto NO_LOS_ITEM = INT_MAX;

bool LOSAndReturnTarget(GameVector* start, GameVector* target, int push);
bool LOS(GameVector* start, GameVector* end);
int xLOS(GameVector* start, GameVector* end);
int zLOS(GameVector* start, GameVector* end);
bool ClipTarget(GameVector* start, GameVector* target);
bool GetTargetOnLOS(GameVector* src, GameVector* dest, bool drawTarget, bool firing);
int ObjectOnLOS2(GameVector* start, GameVector* end, Vector3i* vec, MESH_INFO** mesh, GAME_OBJECT_ID priorityObject = GAME_OBJECT_ID::ID_NO_OBJECT);
bool DoRayBox(GameVector* start, GameVector* end, BOUNDING_BOX* box, PoseData* itemOrStaticPos, Vector3i* hitPos, short closesItemNumber);
