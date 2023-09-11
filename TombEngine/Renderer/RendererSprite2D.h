#pragma once
#include "Renderer/Renderer11Enums.h"

namespace TEN::Renderer
{
	struct RendererSprite;

	struct RendererSprite2DToDraw
	{
		RendererSprite* SpritePtr = nullptr;

		Vector2 Position	= Vector2::Zero;
		short	Orientation = 0;
		Vector2 Scale		= Vector2::Zero;
		Vector4 Color		= Vector4::Zero;

		int			Priority  = 0;
		BLEND_MODES BlendMode = BLENDMODE_ALPHABLEND;
	};
}
