#pragma once

namespace TEN::Renderer::ConstantBuffers
{
	struct alignas(16) CMiscBuffer
	{
		Vector2 CausticsStartUV;
		Vector2 CausticsScale;
		int Caustics;
	};
}