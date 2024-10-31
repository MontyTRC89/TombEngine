#pragma once

#include "Renderer/ConstantBuffers/ShaderLight.h"
#include "Renderer/RendererEnums.h"

namespace TEN::Renderer::ConstantBuffers
{
	struct alignas(16) CRoomBuffer
	{
		int Water;
		int Caustics;
		int NumRoomLights;
		int Padding;
		//--
		Vector2 CausticsStartUV;
		Vector2 CausticsScale;
		//--
		Vector4 AmbientColor;
		//--
		ShaderLight RoomLights[MAX_LIGHTS_PER_ROOM];
	};
}
