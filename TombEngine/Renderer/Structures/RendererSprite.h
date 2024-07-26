#pragma once

#include "Renderer/Graphics/Texture2D.h"

using namespace TEN::Renderer::Graphics;

namespace TEN::Renderer::Structures
{
	struct RendererSprite
	{
		static const auto UV_COUNT = 4;

		std::array<Vector2, UV_COUNT> Uvs	  = {};
		Texture2D*					  Texture = nullptr;

		int Width  = 0;
		int Height = 0;
		int X	   = 0;
		int Y	   = 0;
	};
}
