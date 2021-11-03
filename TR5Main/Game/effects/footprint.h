#pragma once
#include <deque>
#include "Specific/phd_global.h"

namespace TEN{
namespace Effects {
namespace Footprints {

	enum class GroundMaterial : unsigned char
	{
		Mud = 0,
		Snow = 1,
		Sand = 2,
		Gravel = 3,
		Ice = 4,
		Water = 5,
		Stone = 6,
		Wood = 7,
		Metal = 8,
		Marble = 9,
		Grass = 10,
		Concrete = 11,
		OldWood = 12,
		OldMetal = 13,
		Unknown14 = 14,
		Unknown15 = 15
	};

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
