#pragma once
#include "ShaderLight.h"
#include "Renderer/RenderEnums.h"

struct alignas(16) CLightBuffer
{
	ShaderLight Lights[NUM_LIGHTS_PER_BUFFER];
	int NumLights;
	Vector3 CameraPosition;
};
