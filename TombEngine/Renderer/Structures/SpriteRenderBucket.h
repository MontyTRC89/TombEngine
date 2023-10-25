#pragma once
#include "Renderer/Structures/RendererSprite.h"
#include "Renderer/Structures/RendererSpriteToDraw.h"
#include "Renderer/Renderer11Enums.h"

namespace TEN::Renderer::Structures
{
	struct RendererSpriteBucket
	{
		RendererSprite* Sprite;
		BLEND_MODES BlendMode;
		std::vector<RendererSpriteToDraw> SpritesToDraw;

		bool IsBillboard = false;
		bool IsSoftParticle = false;

		SpriteRenderType RenderType;
	};
}