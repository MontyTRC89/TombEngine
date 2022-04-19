#pragma once
#include <SimpleMath.h>

namespace TEN
{
	namespace Renderer
	{
		using DirectX::SimpleMath::Matrix;
		using DirectX::SimpleMath::Vector3;
		using DirectX::SimpleMath::Vector2;

		struct alignas(16) CCameraMatrixBuffer
		{
			alignas(16) Matrix ViewProjection;
			//--
			alignas(16) Matrix View;
			//--
			alignas(16) Matrix Projection;
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
			//--
			alignas(16) Vector4 FogColor;
			alignas(4) int FogMinDistance;
			alignas(4) int FogMaxDistance;
		};
	}
}
