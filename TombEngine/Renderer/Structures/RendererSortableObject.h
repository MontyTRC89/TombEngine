#pragma once
#include "Renderer/Graphics/Vertices/Vertex.h"
#include "Renderer/RendererEnums.h"
#include "Renderer/Structures/RendererBucket.h"
#include "Renderer/Structures/RendererEffect.h"
#include "Renderer/Structures/RendererItem.h"
#include "Renderer/Structures/RendererMesh.h"
#include "Renderer/Structures/RendererRoom.h"
#include "Renderer/Structures/RendererSpriteToDraw.h"
#include "Renderer/Structures/RendererStatic.h"
#include "Specific/fast_vector.h"
#include <SimpleMath.h>

namespace TEN::Renderer::Structures
{
	using namespace DirectX::SimpleMath;
	using namespace TEN::Renderer::Graphics::Vertices;

	struct RendererSortableObject
	{
		RendererObjectType ObjectType;

		Matrix World;
		Vector3 Centre;
		int Distance;

		BlendMode BlendMode;
		LightMode LightMode;

		RendererBucket* Bucket;
		RendererPolygon* Polygon;

		union
		{
			RendererRoom* Room;
			RendererStatic* Static;
			RendererItem* Item;
			RendererEffect* Effect;
			RendererSpriteToDraw* Sprite;
		};
	};
}