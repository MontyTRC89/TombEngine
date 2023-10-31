#pragma once
#include <SimpleMath.h>
#include "Renderer/RendererEnums.h"

namespace TEN::Renderer::Structures
{
	using namespace DirectX::SimpleMath;

	struct RendererSortableObject
	{
		RendererObjectType ObjectType;
		int Distance;
		Vector3 Centre;
		RendererBucket* Bucket;
		RendererRoom* Room;
		RendererStatic* Static;
		RendererItem* Item;
		RendererEffect* Effect;
	};
}