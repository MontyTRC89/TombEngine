#pragma once
#include <SimpleMath.h>

#define MAX_ROOM_LIGHTS 48

struct alignas(16) CRoomBuffer 
{
	DirectX::SimpleMath::Vector2 CausticsStartUV;
	DirectX::SimpleMath::Vector2 CausticsScale;
	DirectX::SimpleMath::Vector4 AmbientColor;
	ShaderLight RoomLights[MAX_ROOM_LIGHTS];
	int NumRoomLights;
	int Water;
	int Caustics;
	int Padding;
};
