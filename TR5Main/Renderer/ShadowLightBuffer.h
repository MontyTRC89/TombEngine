#pragma once
#include "ShaderLight.h"

struct alignas(16) CShadowLightBuffer
{
	ShaderLight Light;
	Matrix LightViewProjection;
	int CastShadows;
	float Padding[15];
};
