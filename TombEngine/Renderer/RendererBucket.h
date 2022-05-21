#pragma once
#include "Renderer/Renderer11Enums.h"
#include "RendererVertex.h"
#include "RendererPolygon.h"
#include <vector>

namespace TEN::Renderer 
{
	struct RendererBucket 
{
		int Texture;
		bool Animated;
		BLEND_MODES BlendMode;
		int StartVertex;
		int StartIndex;
		int NumVertices;
		int NumIndices;
		std::vector<RendererPolygon> Polygons;
	};
}
