#pragma once
#include <SimpleMath.h>

namespace TEN::Renderer::Structures
{
	using namespace DirectX::SimpleMath;

	struct RendererLine3D
	{
		Vector3 Start;
		Vector3 End;
		Vector4 Color;
	};
}