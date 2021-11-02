#pragma once
#include <deque>
#include "Specific/phd_global.h"

namespace TEN{
namespace Effects {
namespace Footprints {

	constexpr size_t MAX_FOOTPRINTS = 20;

	struct FOOTPRINT_STRUCT 
	{
		Vector3 Position;
		Vector3 Rotation;
		bool RightFoot;
		int Life;
		int LifeStartFading;
		float StartOpacity;
		float Opacity;
		bool Active;
	};

	extern std::deque<FOOTPRINT_STRUCT> footprints;
	constexpr int FOOT_HEIGHT_OFFSET = 64;

	bool CheckFootOnFloor(ITEM_INFO const & item, int mesh, Vector3& outFootprintPosition);
	void AddFootprint(ITEM_INFO* item, bool rightFoot);
	void UpdateFootprints();

}}}
