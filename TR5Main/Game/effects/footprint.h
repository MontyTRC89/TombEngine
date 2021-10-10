#pragma once
#include <deque>
#include "Specific/phd_global.h"

namespace TEN{
namespace Effects {
namespace Footprints {

	constexpr size_t MAX_FOOTPRINTS = 20;

	struct FOOTPRINT_STRUCT 
	{
		PHD_3DPOS pos;
		int foot;
		int life;
		int lifeStartFading;
		byte startOpacity;
		byte opacity;
		bool active;
	};

	extern std::deque<FOOTPRINT_STRUCT> footprints;
	constexpr int FOOT_HEIGHT_OFFSET = 64;

	bool CheckFootOnFloor(ITEM_INFO const & item, int mesh, PHD_3DPOS& outFootprintPosition);
	void UpdateFootprints();

}}}
