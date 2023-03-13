#pragma once
#include <vector>
#include <SimpleMath.h>

#include "Renderer/RendererRectangle.h"

struct MESH_INFO;

namespace TEN::Renderer
{
	struct RendererItem;
	struct RendererBucket;
	struct RendererLight;
	struct RendererEffect;
	struct RendererTransparentFace;
	struct RendererDoor;

	struct RendererRoom
	{
		bool Visited;
		short RoomNumber;
		Vector4 AmbientLight;
		Vector4 ViewPort;
		std::vector<RendererBucket> Buckets;
		std::vector<RendererLight> Lights;
		std::vector<RendererStatic> Statics;
		std::vector<RendererItem*> ItemsToDraw;
		std::vector<RendererEffect*> EffectsToDraw;
		std::vector<RendererStatic*> StaticsToDraw;
		std::vector<RendererTransparentFace> TransparentFacesToDraw;
		std::vector<RendererLight*> LightsToDraw;
		std::vector<RendererDoor> Doors;
		BoundingBox BoundingBox;
		RendererRectangle ClipBounds;
		std::vector<int> Neighbors;
	};
}
