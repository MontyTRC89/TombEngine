#pragma once
#include "Renderer/ConstantBuffers/ShaderLight.h"
#include <SimpleMath.h>

using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector4;

struct alignas(16) CStaticBuffer
{
	Matrix World;
	Vector4 Color;
	Vector4 AmbientLight;
	ShaderLight Lights[MAX_LIGHTS_PER_ITEM];
	int NumLights;
	int LightMode;
};
