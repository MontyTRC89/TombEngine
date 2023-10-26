#pragma once
#include <SimpleMath.h>
#include "Renderer/Structures/RendererSprite.h"
#include "Renderer/RendererEnums.h"

namespace TEN::Renderer::Structures
{
	using namespace DirectX::SimpleMath;

	struct RendererDisplaySpriteToDraw
	{
		const RendererSprite* SpritePtr = nullptr;

		Vector2 Position	= Vector2::Zero;
		short	Orientation = 0;
		Vector2 Size		= Vector2::Zero;
		Vector4 Color		= Vector4::Zero;

		int			Priority  = 0;
		BLEND_MODES BlendMode = BLENDMODE_ALPHABLEND;

		Vector2 AspectCorrection = Vector2::One;
	};
}
