#pragma once
#include <SimpleMath.h>

namespace TEN::Renderer::Structures
{
	using namespace DirectX;
	using namespace DirectX::SimpleMath;

	struct RendererMirror
	{
		short RealRoom;
		short VirtualRoom;
		Plane Plane;
		Matrix ReflectionMatrix;
	};
}