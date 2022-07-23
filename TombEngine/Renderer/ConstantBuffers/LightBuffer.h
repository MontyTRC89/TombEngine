#pragma once
#include "ShaderLight.h"
#include "Renderer/Renderer11Enums.h"

struct alignas(16) CLightBuffer
{
	ShaderLight Lights[NUM_LIGHTS_PER_BUFFER];
	int NumLights;
};
