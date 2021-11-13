#pragma once

#include <SimpleMath.h>
#include "RenderEnums.h"

namespace TEN::Renderer {
	struct RendererBucket;
	struct RendererPolygon;
	struct RendererSpriteToDraw;
	struct RendererRoom;
	struct RendererItem;
	struct RendererStatic;

	struct RendererTransparentFace
	{
		RendererTransparentFaceType type;
		int distance;
		RendererPolygon* polygon;
		Vector4 color;
		Matrix world;
		int texture;
		bool animated;
		bool doubleSided;
		BLEND_MODES blendMode;
		RendererSpriteToDraw* sprite;
		RendererRoom* room;
		RendererItem* item;
		RendererStatic* staticMesh;
	};
}