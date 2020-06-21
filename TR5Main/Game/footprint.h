#pragma once
#include "phd_global.h"
#include "items.h"
namespace T5M{
	namespace Effects {
		namespace Footprints {
			constexpr size_t MAX_FOOTPRINTS = 20;
			typedef struct footprint_t {
				PHD_3DPOS pos;
				int life;
				int lifeStartFading;
				byte startOpacity;
				byte opacity;
				bool active;
			} FOOTPRINT_STRUCT;
			extern std::deque<FOOTPRINT_STRUCT> footprints;
			constexpr int FOOT_HEIGHT_OFFSET = 64;
			bool CheckFootOnFloor(ITEM_INFO& const item, int mesh, PHD_3DPOS& outFootprintPosition);
			void updateFootprints();
		}
	}
}
