#pragma once
#include "ShaderLight.h"
#include "Renderer/Renderer11Enums.h"

namespace TEN::Renderer::ConstantBuffers
{
	struct alignas(16) CLightBuffer
	{
		ShaderLight Lights[MAX_LIGHTS_PER_ITEM];
		int NumLights;
	};
}