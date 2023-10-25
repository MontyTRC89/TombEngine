#pragma once
#include <SimpleMath.h>
#include "Renderer/Structures/RendererMesh.h"
#include "Renderer/Structures/RendererLight.h"

namespace TEN::Renderer::Structures
{
	using namespace DirectX::SimpleMath;

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