#pragma once
#include "Math/Math.h"

enum SOUND_EFFECTS;
enum class FLOOR_MATERIAL : unsigned char;
struct ItemInfo;

namespace TEN::Effects::Footprint
{
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

	extern std::vector<Footprint> Footprints;

	void SpawnFootprint(const ItemInfo& item, bool isRightFoot);
	void SpawnFootprint(bool isRightFoot, const std::array<Vector3, 4>& vertexPoints);

	void UpdateFootprints();
	void ClearFootprints();
}
