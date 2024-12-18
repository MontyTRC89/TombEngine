#pragma once
#include <SimpleMath.h>

namespace TEN::Renderer::Structures
{
	using namespace DirectX;
	using namespace DirectX::SimpleMath;

	struct RendererMirror
	{
		short RealRoom;
		Plane Plane;
		Matrix ReflectionMatrix;
		bool ReflectLara;
		bool ReflectMoveables;
		bool ReflectStatics;
		bool ReflectLights;
	};
}