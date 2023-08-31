#pragma once
#include "Renderer/Renderer11Enums.h"

namespace TEN::Renderer
{
	struct RendererSprite;

	struct RendererSprite2DToDraw
	{
		RendererSprite* SpritePtr	= nullptr;
		Vector2			Position	= Vector2::Zero;
		Vector2			Size		= Vector2::Zero;
		Vector4			Color		= Vector4::Zero;
		short			Orientation = 0;
		int				Priority	= 0;
	};
}
