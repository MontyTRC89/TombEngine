#pragma once

namespace TEN::Renderer::ConstantBuffers
{
	using namespace DirectX::SimpleMath;

	struct alignas(16) CSpriteBuffer
	{
		float IsSoftParticle;
		int RenderType;
	};
}
