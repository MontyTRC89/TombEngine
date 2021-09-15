#pragma once
#include "room.h"
#include "Specific\phd_global.h"

constexpr auto NO_LOS_ITEM = INT_MAX;

int LOS(GAME_VECTOR* start, GAME_VECTOR* end);
int xLOS(GAME_VECTOR* start, GAME_VECTOR* end);
int zLOS(GAME_VECTOR* start, GAME_VECTOR* end);
int ClipTarget(GAME_VECTOR* start, GAME_VECTOR* target);
int GetTargetOnLOS(GAME_VECTOR* src, GAME_VECTOR* dest, int DrawTarget, int firing);
int ObjectOnLOS2(GAME_VECTOR* start, GAME_VECTOR* end, PHD_VECTOR* vec, MESH_INFO** mesh);
int DoRayBox(GAME_VECTOR* start, GAME_VECTOR* end, BOUNDING_BOX* box, PHD_3DPOS* itemOrStaticPos, PHD_VECTOR* hitPos, short closesItemNumber);