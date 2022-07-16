#pragma once
#include <SimpleMath.h>

struct alignas(16) CRoomBuffer 
{
	DirectX::SimpleMath::Vector4 AmbientColor;
	unsigned int Water;
};
