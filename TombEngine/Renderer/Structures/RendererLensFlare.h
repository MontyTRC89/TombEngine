#pragma once
#include <SimpleMath.h>
#include "Renderer/RendererEnums.h"

namespace TEN::Renderer::Structures
{
	using namespace DirectX::SimpleMath;

	struct RendererLensFlare
	{
		Vector3 Position;
		Vector3 Color;
		Vector3 Direction;
		float Distance;
		bool Global;
		int SpriteIndex;
	};
}