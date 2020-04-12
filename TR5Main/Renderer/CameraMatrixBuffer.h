#pragma once
#include <SimpleMath.h>
struct alignas(16) CCameraMatrixBuffer
{
	DirectX::SimpleMath::Matrix ViewProjection;
};

