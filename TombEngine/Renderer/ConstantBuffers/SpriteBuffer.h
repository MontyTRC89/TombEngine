#pragma once

namespace TEN::Renderer::ConstantBuffers
{
	struct alignas(16) CSpriteBuffer
	{
		float IsSoftParticle;
		int RenderType;
	};
}
