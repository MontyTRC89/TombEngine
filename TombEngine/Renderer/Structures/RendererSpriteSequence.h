#pragma once

namespace TEN::Renderer::Structures
{
	struct RendererSpriteSequence
	{
		int ID = 0;
		std::vector<RendererSprite> Sprites = {};

		RendererSpriteSequence() = default;

		RendererSpriteSequence(const RendererSpriteSequence& spriteSeq)
		{
			ID = spriteSeq.ID;
			Sprites = spriteSeq.Sprites;
		}
	};
}
