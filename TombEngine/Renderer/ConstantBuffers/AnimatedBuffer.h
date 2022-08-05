#pragma once
#include <array>
#include <cstdint>
#include <SimpleMath.h>

struct AnimatedFrame
{
	DirectX::SimpleMath::Vector2 topLeft;
	DirectX::SimpleMath::Vector2 topRight;
	DirectX::SimpleMath::Vector2 bottomRight;
	DirectX::SimpleMath::Vector2 bottomLeft;
};

struct alignas(16) CAnimatedBuffer
{
	std::array<AnimatedFrame, 128> Textures;
	uint32_t NumFrames;
	uint32_t Fps;
	uint32_t Type;
	uint32_t padding;
};
