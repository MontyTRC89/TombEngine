#pragma once

#include "Objects/game_object_ids.h"

struct ItemInfo;

namespace TEN::Effects::Footprint
{
	struct Footprint
	{
		static constexpr auto COUNT_MAX	   = 64;
		static constexpr auto VERTEX_COUNT = 4;

		GAME_OBJECT_ID SpriteSeqAssetID = GAME_OBJECT_ID::ID_DEFAULT_SPRITES;
		bool		   SpriteAssetID	= 0;

		float Life			  = 0.0f;
		float LifeStartFading = 0.0f;
		float Opacity		  = 0.0f;
		float OpacityStart	  = 0.0f;

		std::array<Vector3, VERTEX_COUNT> VertexPoints = {};
	};

	extern std::vector<Footprint> Footprints;

	void SpawnFootprint(bool isRight, const std::array<Vector3, Footprint::VERTEX_COUNT>& vertexPoints);
	void SpawnFootprint(const ItemInfo& item, bool isRight);

	void UpdateFootprints();
	void ClearFootprints();
}
