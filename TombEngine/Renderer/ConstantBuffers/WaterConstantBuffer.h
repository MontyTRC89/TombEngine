#pragma once
#include <SimpleMath.h>
#include "Renderer/ConstantBuffers/ShaderLight.h"
#include "Renderer/RendererEnums.h"

namespace TEN::Renderer::ConstantBuffers
{
	using namespace DirectX::SimpleMath;

	struct alignas(16) CWaterConstantBuffer
	{
		Matrix WaterReflectionViews[8];
		//--
		Vector4 WaterLevels[8];
		//--
		Matrix SkyWorldMatrices[8];
		//--
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
		int WaterPlaneIndex;
		//--
		Vector4 SkyColor;
		//--
		Vector3 WaterFogColor;
		float WaterDepthScale;
		//--
		Vector3 AbsorptionCoefficient;
		float WaterFogDensity;
#ifdef NEW_RIPPLES
		//--
		Vector4 RipplesPositionSize[MAX_WAVES];
		//--
		Vector4 RipplesParameters[MAX_WAVES];
		//--
		int RipplesCount;
#endif
		Vector4 WaterDistortionMapUvCoordinates;
		//--
		Vector4 WaterNormalMapUvCoordinates;
		//--
		Vector4 WaterFoamMapUvCoordinates;
		//--
		Vector2 WaterDirection;
		float WaterSpeed;
		float WaterRefractionStrength;
	};
}