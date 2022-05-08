#pragma once
#include <d3d11.h>
#include <SimpleMath.h>

struct alignas(16) CSpriteBuffer
{
	alignas(16) DirectX::SimpleMath::Matrix billboardMatrix;
	alignas(16) DirectX::SimpleMath::Vector4 color;
	alignas(16) bool isBillboard;
};
