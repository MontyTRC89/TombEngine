#pragma once
#include <vector>

namespace TEN::Renderer::Structures
{
	struct RendererSpriteSequence
	{
		int ID	  = 0;
		int Count = 0;
		std::vector<RendererSprite*> Sprites = {};

		RendererSpriteSequence()
		{
			ID = 0;
			Count = 0;
		}

		RendererSpriteSequence(int id, int count)
		{
			ID = id;
			Count = count;
			Sprites = std::vector<RendererSprite*>(Count);
		}

		RendererSpriteSequence(const RendererSpriteSequence& spriteSeq)
		{
			ID = spriteSeq.ID;
			Count = spriteSeq.Count;
			Sprites = spriteSeq.Sprites;
		}

		RendererSpriteSequence& operator=(const RendererSpriteSequence& spriteSeq)
		{
			if (this != &spriteSeq)
			{
				ID = spriteSeq.ID;
				Count = spriteSeq.Count;
				Sprites = std::vector<RendererSprite*>(Count);
				std::copy(spriteSeq.Sprites.begin(), spriteSeq.Sprites.end(), back_inserter(Sprites));
			}

			return *this;
		}
	};
}
