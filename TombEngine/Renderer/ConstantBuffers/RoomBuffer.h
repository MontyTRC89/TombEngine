#pragma once
#include <SimpleMath.h>

namespace TEN::Renderer::ConstantBuffers
{
	struct alignas(16) CRoomBuffer
	{
		int Water;
		int Caustics;
		int NumRoomLights;
		int Padding;
		DirectX::SimpleMath::Vector2 CausticsStartUV;
		DirectX::SimpleMath::Vector2 CausticsScale;
		DirectX::SimpleMath::Vector4 AmbientColor;
		ShaderLight RoomLights[MAX_LIGHTS_PER_ROOM];
	};
}