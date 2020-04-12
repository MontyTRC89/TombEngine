#pragma once
#include <SimpleMath.h>

struct alignas(16) ShaderLight {
	DirectX::SimpleMath::Vector4 Position;
	DirectX::SimpleMath::Vector4 Color;
	DirectX::SimpleMath::Vector4 Direction;
	float Intensity;
	float In;
	float Out;
	float Range;
};

