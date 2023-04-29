#pragma once
#include <SimpleMath.h>

struct alignas(16) ShaderFogBulb
{
	Vector3 Position;
	float Density;
	Vector3 Color;
	float Radius;
	Vector4 Padding1;
	Vector4 Padding2;
};