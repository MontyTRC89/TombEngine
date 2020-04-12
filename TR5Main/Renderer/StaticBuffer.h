#pragma once
#include <SimpleMath.h>

struct alignas(16) CStaticBuffer
{
	DirectX::SimpleMath::Matrix World;
	DirectX::SimpleMath::Vector4 Position;
	DirectX::SimpleMath::Vector4 Color;
};

