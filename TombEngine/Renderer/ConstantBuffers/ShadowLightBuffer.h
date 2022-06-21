#pragma once
#include "ShaderLight.h"

struct alignas(16) CShadowLightBuffer
{
	ShaderLight Light;
	Matrix LightViewProjections[6];
	int CastShadows;
};
