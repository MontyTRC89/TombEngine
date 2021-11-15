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
	struct RendererObject;

	struct RendererTransparentFaceInfo
	{
		RendererPolygon* polygon;
		RendererSpriteToDraw* sprite;
		RendererRoom* room;
		RendererItem* item;
		RendererStatic* staticMesh;
		Vector4 color;
		Matrix world;
		Vector3 position;
		int texture;
		bool animated;
		bool doubleSided;
		BLEND_MODES blendMode;
	};

	struct RendererTransparentFace
	{
		RendererTransparentFaceType type;
		int distance;
		RendererTransparentFaceInfo info;
	};
}