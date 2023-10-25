#pragma once
#include <SimpleMath.h>
#include <DirectXCollision.h>
#include "Renderer/Structures/RendererBucket.h"
#include "Renderer/Renderer11Enums.h"

namespace TEN::Renderer::Structures
{
	using namespace DirectX::SimpleMath;

	struct RendererMesh
	{
		LIGHT_MODES LightMode;
		BoundingSphere Sphere;
		std::vector<RendererBucket> Buckets;
		std::vector<Vector3> Positions;
	};
}