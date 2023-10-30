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
		short BucketNumber;
		short RoomNumber;
		short ItemNumber;
		short FxNumber;
		short StaticNumber;
	};
}