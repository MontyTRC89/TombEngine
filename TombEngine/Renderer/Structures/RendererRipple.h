#pragma once
#include <SimpleMath.h>

namespace TEN::Renderer::Structures
{
	using namespace DirectX::SimpleMath;

	struct RendererRipple
	{
		Vector2 Position;
		float Size;
		float Opacity;
		float Time;
	};
}
