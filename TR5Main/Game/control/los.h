#pragma once
#include "room.h"
#include "objectslist.h"
#include "Specific/phd_global.h"

constexpr auto NO_LOS_ITEM = INT_MAX;

bool LOSAndReturnTarget(GAME_VECTOR* start, GAME_VECTOR* target, int push);
bool LOS(GAME_VECTOR* start, GAME_VECTOR* end);
int xLOS(GAME_VECTOR* start, GAME_VECTOR* end);
int zLOS(GAME_VECTOR* start, GAME_VECTOR* end);
int ClipTarget(GAME_VECTOR* start, GAME_VECTOR* target);
int GetTargetOnLOS(GAME_VECTOR* src, GAME_VECTOR* dest, int DrawTarget, int firing);
int ObjectOnLOS2(GAME_VECTOR* start, GAME_VECTOR* end, PHD_VECTOR* vec, MESH_INFO** mesh, GAME_OBJECT_ID priorityObject = GAME_OBJECT_ID::ID_NO_OBJECT);
int DoRayBox(GAME_VECTOR* start, GAME_VECTOR* end, BOUNDING_BOX* box, PHD_3DPOS* itemOrStaticPos, PHD_VECTOR* hitPos, short closesItemNumber);