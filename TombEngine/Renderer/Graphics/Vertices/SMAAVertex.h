#pragma once
#include <SimpleMath.h>

namespace TEN::Renderer::Graphics::Vertices
{
	using namespace DirectX::SimpleMath;

	struct SMAAVertex
	{
		Vector3 Position = Vector3::Zero;
		Vector2 UV = Vector3::Zero;
	};
}