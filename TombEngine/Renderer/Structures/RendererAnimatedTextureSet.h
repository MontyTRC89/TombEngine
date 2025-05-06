#pragma once

#include "RendererAnimatedTexture.h"

namespace TEN::Renderer::Structures
{
	enum class AnimatedTextureType
	{
		Frames,
		UVRotate,
		Video
	};

	struct RendererAnimatedTextureSet
	{
		AnimatedTextureType Type = AnimatedTextureType::Frames;
		int NumTextures = 0;
		int Fps = 0;
		std::vector<RendererAnimatedTexture> Textures;
	};
}
