#pragma once
#include "ShaderLight.h"

struct alignas(16) CShadowLightBuffer {
	ShaderLight Light;
	DirectX::SimpleMath::Matrix LightViewProjection;
	int CastShadows;
	float Padding[15];
};

