#include "../Global/types.h"
#include "math.h"
#include <queue>
#pragma once

constexpr size_t MAX_FOOTPRINTS = 20;
typedef struct footprint_t {
	PHD_3DPOS pos;
	int life;
	int lifeStartFading;
	byte startOpacity;
	byte opacity;
	bool active;
}FOOTPRINT_STRUCT;

constexpr int FOOT_HEIGHT_OFFSET = 64;
bool CheckFootOnFloor(ITEM_INFO& const item, int mesh, PHD_3DPOS& outFootprintPosition);
void updateFootprints();