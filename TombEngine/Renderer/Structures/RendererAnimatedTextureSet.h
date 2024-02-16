#pragma once
#include <vector>
#include "RendererAnimatedTexture.h"

namespace TEN::Renderer::Structures
{
	using namespace DirectX::SimpleMath;

	struct RendererAnimatedTextureSet
	{
		int NumTextures;
		int Fps;
		std::vector<RendererAnimatedTexture> Textures;
	};
}
