#pragma once

struct alignas(16) ShaderLight
{
	Vector3 Position;
	int Type;
	Vector3 Color;
	int LocalIntensity;
	Vector3 Direction;
	float Distance;
	float Intensity;
	float In;
	float Out;
	float Range;
};
