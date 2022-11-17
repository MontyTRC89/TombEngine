#pragma once
#include <deque>

#include "Math/Math.h"

struct ItemInfo;

namespace TEN::Effects::Footprints
{
	constexpr auto FOOTPRINTS_NUM_MAX = 32;

	struct Footprint 
	{
		bool IsActive	 = false;
		bool IsRightFoot = false;

		std::array<Vector3, 4> VertexPoints = {};

		float Life			  = 0.0f;
		float LifeStartFading = 0.0f;
		float Opacity		  = 0.0f;
		float OpacityStart	  = 0.0f;
	};

	extern std::deque<Footprint> Footprints;

	void AddFootprint(ItemInfo* item, bool isRightFoot);
	bool TestFootOnFloor(ItemInfo& item, int mesh, Vector3& outFootprintPosition);
	//bool TestFootprintMaterial(FLOOR_MATERIAL refMaterial, const std::vector<FLOOR_MATERIAL>& materialList);

	//SOUND_EFFECTS		   GetFootprintSoundEffect(FLOOR_MATERIAL material);
	//std::array<Vector3, 4> GetFootprintVertexPoints(const ItemInfo& item, const Vector3& pos, const Vector3& normal);

	void SpawnFootprint(const std::array<Vector3, 4>& vertexPoints, bool isRightFoot);
	void UpdateFootprints();
}
