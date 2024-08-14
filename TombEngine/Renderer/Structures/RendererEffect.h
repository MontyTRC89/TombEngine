#pragma once

#include "Renderer/Structures/RendererLight.h"
#include "Renderer/Structures/RendererMesh.h"

namespace TEN::Renderer::Structures
{
	struct RendererEffect
	{
		int ObjectNumber;
		int RoomNumber;
		Vector3 Position;
		Matrix World;
		Vector4 Color;
		Vector4 AmbientLight;
		RendererMesh* Mesh;
		std::vector<RendererLight*> LightsToDraw;
	};
}
