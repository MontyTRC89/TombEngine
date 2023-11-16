#pragma once
#include <SimpleMath.h>
#include "Renderer/ConstantBuffers/ShaderLight.h"
#include "Renderer/ConstantBuffers/ShaderFogBulb.h"
#include "Renderer/RendererEnums.h"

namespace TEN::Renderer::ConstantBuffers
{
	struct alignas(16) CCameraMatrixBuffer
	{
		alignas(16) Matrix ViewProjection;
		//--
		alignas(16) Matrix View;
		//--
		alignas(16) Matrix Projection;
		//--
		alignas(16) Matrix InverseProjection;
		//--
		alignas(16) Matrix DualParaboloidView;
		//--
		alignas(16) Vector4 CamPositionWS;
		//--
		alignas(16) Vector4 CamDirectionWS;
		//--
		alignas(16) Vector2 ViewSize;
		alignas(4) Vector2 InvViewSize;
		//--
		alignas(16) unsigned int Frame;
		alignas(4) unsigned int RoomNumber;
		alignas(4) unsigned int CameraUnderwater;
		alignas(4) int Emisphere;
		//--
		alignas(16) Vector4 FogColor;
		alignas(4) int FogMinDistance;
		alignas(4) int FogMaxDistance;

		alignas(4) float NearPlane;
		alignas(4) float FarPlane;

		alignas(4) int NumFogBulbs;
		ShaderFogBulb FogBulbs[MAX_FOG_BULBS_DRAW];
	};
}

