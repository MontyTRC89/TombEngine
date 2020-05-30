#pragma once

struct alignas(16) ShaderLight
{
	Vector4 Position;
	Vector4 Color;
	Vector4 Direction;
	float Intensity;
	float In;
	float Out;
	float Range;
};
