#pragma once
#include <SimpleMath.h>
#include "Renderer/ConstantBuffers/ShaderLight.h"

namespace TEN::Renderer::ConstantBuffers
{
	using namespace DirectX::SimpleMath;

	struct alignas(16) Sphere
	{
		Vector3 position;
		float radius;
	};

	struct alignas(16) CShadowLightBuffer
	{
		ShaderLight Light;
		//--
		Matrix LightViewProjections[6];
		//--
		int CastShadows;
		int NumSpheres;
		int ShadowMapSize;
		int Padding;
		//--
		Sphere Spheres[16];
	};
}