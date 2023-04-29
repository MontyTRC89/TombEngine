#pragma once
#include <SimpleMath.h>

struct alignas(16) ShaderLight
{
	Vector3 Position;
	unsigned int Type;
	Vector3 Color;
	float Intensity;
	Vector3 Direction;
	float In;
	float Out;
	float InRange;
	float OutRange;

	float padding;
};
