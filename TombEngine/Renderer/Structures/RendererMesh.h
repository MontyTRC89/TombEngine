#pragma once

#include "Renderer/RendererEnums.h"
#include "Renderer/Structures/RendererBucket.h"

namespace TEN::Renderer::Structures
{
	struct RendererMesh
	{
		LightMode LightMode;
		BoundingSphere Sphere;
		std::vector<RendererBucket> Buckets;
		std::vector<Vector3> Positions;
	};
}
