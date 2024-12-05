#pragma once
#include <SimpleMath.h>

namespace TEN::Renderer::Structures
{
	using namespace DirectX;
	using namespace DirectX::SimpleMath;

	struct RendererMirror
	{
		short RoomNumber;
		short MirroredRoomNumber;
		Plane Plane;
		Matrix ReflectionMatrix;
	};
}