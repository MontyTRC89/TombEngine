#pragma once

#include "Renderer/ConstantBuffers/ShaderLight.h"
#include "Renderer/RendererEnums.h"

namespace TEN::Renderer::ConstantBuffers
{
	struct alignas(16) CStaticBuffer
	{
		Matrix World;
		//--
		Vector4 Color;
		//--
		Vector4 AmbientLight;
		//--
		ShaderLight Lights[MAX_LIGHTS_PER_ITEM];
		//--
		int NumLights;
		int LightMode;
		int ApplyFogBulbs; // Used only by sky
	};
}
