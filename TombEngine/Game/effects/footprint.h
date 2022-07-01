#pragma once
#include <deque>
#include "Specific/phd_global.h"

struct ItemInfo;

namespace TEN::Effects::Footprints
{
	constexpr size_t MAX_FOOTPRINTS = 20;
	constexpr auto FOOTPRINT_SIZE = 64.0f;
	constexpr int FOOT_HEIGHT_OFFSET = 64;

	struct FOOTPRINT_STRUCT 
	{
		Vector3 Position[4];
		bool RightFoot;
		int Life;
		int LifeStartFading;
		float StartOpacity;
		float Opacity;
		bool Active;
	};
	extern std::deque<FOOTPRINT_STRUCT> footprints;

	bool CheckFootOnFloor(ItemInfo const & item, int mesh, Vector3& outFootprintPosition);
	void AddFootprint(ItemInfo* item, bool rightFoot);
	void UpdateFootprints();
}
