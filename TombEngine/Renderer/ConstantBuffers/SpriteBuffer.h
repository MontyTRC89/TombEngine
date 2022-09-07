#pragma once
#include <d3d11.h>
#include <SimpleMath.h>

using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector4;

struct alignas(4) CSpriteBuffer
{
	Matrix billboardMatrix;
	Vector4 color;
	float IsBillboard;
	float IsSoftParticle;
	Vector2 SpritesPadding;
};
