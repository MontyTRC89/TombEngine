#pragma once
#include <SimpleMath.h>

using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector4;

struct alignas(16) CSkyBuffer
{
	Matrix World;
	Vector4 Color;
	int ApplyFogBulbs;
};
