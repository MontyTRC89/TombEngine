#pragma once
#include "ShaderLight.h"
struct alignas(16) Sphere
{
	Vector3 position;
	float radius;
};
struct alignas(16) CShadowLightBuffer
{
	ShaderLight Light;
	Matrix LightViewProjections[6];
	int CastShadows;
	int NumSpheres;
	int padding[2];
	Sphere Spheres[16];
};
