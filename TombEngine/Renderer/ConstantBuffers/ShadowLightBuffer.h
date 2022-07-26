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
	int ShadowMapSize;
	int padding;
	Sphere Spheres[16];
};
