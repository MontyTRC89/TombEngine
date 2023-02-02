#pragma once

#include <SimpleMath.h>

namespace TEN::Renderer
{
	struct RendererDoor
	{
		bool Visited;
		bool InvisibleFromCamera;
		short RoomNumber;
		Vector3 Normal;
		Vector3 CameraToDoor;
		Vector4 AbsoluteVertices[4];
		Vector4 TransformedVertices[4];
	};
}
