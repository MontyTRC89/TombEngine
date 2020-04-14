#pragma once
#include <SimpleMath.h>
#include "ShaderLight.h"
#include "Enums.h"

struct alignas(16) CLightBuffer {
	ShaderLight Lights[NUM_LIGHTS_PER_BUFFER];
	int NumLights;
	DirectX::SimpleMath::Vector3 CameraPosition;
};

