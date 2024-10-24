#pragma once

#include "Renderer/RendererEnums.h"
#include "Renderer/Structures/RendererSprite.h"
#include "Renderer/Structures/RendererSpriteToDraw.h"

namespace TEN::Renderer::Structures
{
	struct RendererSpriteBucket
	{
		RendererSprite* Sprite;
		BlendMode BlendMode;
		std::vector<RendererSpriteToDraw> SpritesToDraw;

		bool IsBillboard = false;
		bool IsSoftParticle = false;

		SpriteRenderType RenderType;
	};
}
