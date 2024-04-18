#pragma once
#include <SimpleMath.h>
#include "Renderer/RendererEnums.h"

namespace TEN::Renderer::Structures
{
	using namespace DirectX::SimpleMath;

	struct RendererLensFlare
	{
		Vector3 Position;
		float Padding;
	};
}