#pragma once
#include <deque>

#include "Math/Math.h"

enum SOUND_EFFECTS;
enum class FLOOR_MATERIAL : unsigned char;
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

	SOUND_EFFECTS		   GetFootprintSoundEffect(FLOOR_MATERIAL material);
	std::array<Vector3, 4> GetFootprintVertexPoints(const ItemInfo& item, const Vector3& pos, const Vector3& normal);

	bool TestMaterial(FLOOR_MATERIAL refMaterial, const std::vector<FLOOR_MATERIAL>& materialList);
	bool TestFootOnFloor(ItemInfo& item, int mesh, Vector3& outFootprintPosition);

	void SpawnFootprint(const std::array<Vector3, 4>& vertexPoints, bool isRightFoot);
	void UpdateFootprints();
}
