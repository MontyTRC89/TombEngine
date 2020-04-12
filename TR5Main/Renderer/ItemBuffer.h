#pragma once
#include <SimpleMath.h>
struct alignas(16) CItemBuffer
{
	DirectX::SimpleMath::Matrix World;
	DirectX::SimpleMath::Matrix BonesMatrices[32];
	DirectX::SimpleMath::Vector4 Position;
	DirectX::SimpleMath::Vector4 AmbientLight;
};

