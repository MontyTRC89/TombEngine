#pragma once
#include <deque>

#include "Math/Math.h"

enum SOUND_EFFECTS;
enum class FLOOR_MATERIAL : unsigned char;
struct ItemInfo;

namespace TEN::Effects::Footprint
{
	constexpr auto FOOTPRINT_NUM_MAX = 32;

	struct Footprint 
	{
		unsigned int SpriteIndex = 0;
		bool		 IsRightFoot = false;

		std::array<Vector3, 4> VertexPoints = {};

		float Life			  = 0.0f;
		float LifeStartFading = 0.0f;
		float Opacity		  = 0.0f;
		float OpacityStart	  = 0.0f;
	};

	extern std::deque<Footprint> Footprints;

	SOUND_EFFECTS		   GetFootprintSoundEffectID(FLOOR_MATERIAL material);
	std::array<Vector3, 4> GetFootprintVertexPoints(const ItemInfo& item, const Vector3& pos, const Vector3& normal);

	bool TestMaterial(FLOOR_MATERIAL refMaterial, const std::vector<FLOOR_MATERIAL>& materialList);
	bool TestFootHeight(const ItemInfo& item, int meshIndex, Vector3& outFootprintPos);
	bool TestFootprintFloor(const ItemInfo& item, const Vector3& pos, const std::array<Vector3, 4>& vertexPoints);

	void SpawnFootprint(const ItemInfo& item, bool isRightFoot);
	void SpawnFootprint(bool isRightFoot, const std::array<Vector3, 4>& vertexPoints);

	void UpdateFootprints();
	void ClearFootprints();
}
