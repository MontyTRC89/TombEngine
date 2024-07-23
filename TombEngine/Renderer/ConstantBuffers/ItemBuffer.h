#pragma once
#include <SimpleMath.h>
#include "Renderer/RendererEnums.h"
#include "Renderer/ConstantBuffers/ShaderLight.h"

namespace TEN::Renderer::ConstantBuffers
{
	using namespace DirectX::SimpleMath;

	struct alignas(16) CItemBuffer
	{
		Matrix World;
		//--
		Matrix BonesMatrices[BONE_COUNT];
		//--
		Vector4 Color;
		//--
		Vector4 AmbientLight;
		//--
		int BoneLightModes[BONE_COUNT];
		//--
		ShaderLight Lights[MAX_LIGHTS_PER_ITEM];
		//--
		int NumLights;
	};
}