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
		Vector3			Color		= Vector3::Zero;
		short			Angle		= 0;
		int				Priority	= 0;
		float			Opacity		= 0.0f;
		BLEND_MODES		BlendMode	= BLENDMODE_ALPHABLEND;
	};
}
