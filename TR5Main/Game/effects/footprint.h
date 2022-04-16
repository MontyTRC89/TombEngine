#pragma once
#include <deque>
#include "Specific/phd_global.h"

struct ITEM_INFO;

namespace TEN
{
namespace Effects
{
namespace Footprints
{

	constexpr size_t MAX_FOOTPRINTS = 20;
	constexpr auto FOOTPRINT_SIZE = 64.0f;

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
	constexpr int FOOT_HEIGHT_OFFSET = 64;

	bool CheckFootOnFloor(ITEM_INFO const & item, int mesh, Vector3& outFootprintPosition);
	void AddFootprint(ITEM_INFO* item, bool rightFoot);
	void UpdateFootprints();

}}}
