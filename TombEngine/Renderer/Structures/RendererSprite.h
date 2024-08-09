#pragma once
#include "Renderer/Graphics/Texture2D.h"

namespace TEN::Renderer::Structures
{
	using namespace TEN::Renderer::Graphics;

	struct RendererSprite
	{
		int Index;
		int Width;
		int Height;
		Vector2 UV[4];
		Texture2D* Texture;
		int X;
		int Y;
	};
}
