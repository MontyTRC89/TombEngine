#pragma once

#include "RendererAnimatedTexture.h"

namespace TEN::Renderer::Structures
{
	struct RendererAnimatedTextureSet
	{
		int NumTextures;
		int Fps;
		std::vector<RendererAnimatedTexture> Textures;
	};
}
