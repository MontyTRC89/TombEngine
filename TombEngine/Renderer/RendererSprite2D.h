#pragma once
#include "Renderer/Renderer11Enums.h"

namespace TEN::Renderer
{
	struct RendererSprite;

	struct RendererSprite2DToDraw
	{
		RendererSprite* Sprite;
		Vector2 Position;
		Vector2 Size;
		Vector3 Color;
		short Angle;
		float Opacity;
		int Priority;
	};
}
