#pragma once

struct alignas(16) ShaderLight
{
	Vector3 Position;
	int Type;
	Vector3 Color;
	int Dynamic;
	Vector3 Direction;
	float Distance;
	float Intensity;
	float In;
	float Out;
	float Range;
};
