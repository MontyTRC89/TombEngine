#pragma once
#include <SimpleMath.h>
namespace T5M {
	namespace Renderer {
		using DirectX::SimpleMath::Matrix;
		struct alignas(16) CCameraMatrixBuffer
		{
			Matrix ViewProjection;
			Matrix View;
			Matrix Projection;
			Vector3 WorldPosition;
			Vector3 WorldDirection;
			Vector2 ViewSize;
			Vector2 InvViewSize;
			alignas(16) int Frame;
			int CameraUnderwater;
		};
	}
}

