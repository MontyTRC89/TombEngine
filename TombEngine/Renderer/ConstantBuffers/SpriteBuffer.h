#pragma once
#include <d3d11.h>
#include <SimpleMath.h>

using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector4;

struct alignas(16) CSpriteBuffer
{
	float IsSoftParticle;
};
