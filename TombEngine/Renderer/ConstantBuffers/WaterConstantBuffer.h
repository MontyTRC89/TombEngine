#pragma once
#include <SimpleMath.h>
#include "Renderer/ConstantBuffers/ShaderLight.h"
#include "Renderer/RendererEnums.h"

namespace TEN::Renderer::ConstantBuffers
{
	using namespace DirectX::SimpleMath;

	struct alignas(16) CWaterConstantBuffer
	{
		Matrix WaterReflectionView;
		//--
		Vector3 LightPosition;
		float KSpecular;
		//--
		Vector3 LightColor;
		float Shininess;
		//--
		float MoveFactor;
		float WaveStrength;
		int WaterLevel;
		float Padding;
	};
}