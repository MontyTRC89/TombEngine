#pragma once
#include <SimpleMath.h>
struct alignas(16) CHUDBuffer {
	DirectX::SimpleMath::Matrix View;
	DirectX::SimpleMath::Matrix Projection;
};

