#pragma once

namespace TEN::Renderer::ConstantBuffers
{
	struct AnimatedFrame
	{
		Vector2 TopLeft;
		Vector2 TopRight;
		//--
		Vector2 BottomRight;
		Vector2 BottomLeft;
	};

	struct alignas(16) CAnimatedBuffer
	{
		std::array<AnimatedFrame, 256> Textures;
		//--
		uint32_t NumFrames;
		uint32_t Fps;
		uint32_t Type;
		uint32_t Padding;
	};
}
