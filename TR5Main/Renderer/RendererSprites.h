#pragma once
#include <SimpleMath.h>
#include "Renderer/Renderer11Enums.h"

namespace TEN::Renderer
{
	struct RenderView;
	struct RendererSprite;

	struct RendererSpriteToDraw
	{
		RENDERER_SPRITE_TYPE Type;
		RendererSprite* Sprite;
		float Scale;
		DirectX::SimpleMath::Vector3 pos;
		DirectX::SimpleMath::Vector3 vtx1;
		DirectX::SimpleMath::Vector3 vtx2;
		DirectX::SimpleMath::Vector3 vtx3;
		DirectX::SimpleMath::Vector3 vtx4;
		DirectX::SimpleMath::Vector4 color;
		float Rotation;
		float Width;
		float Height;
		BLEND_MODES BlendMode;
		DirectX::SimpleMath::Vector3 ConstrainAxis;
		DirectX::SimpleMath::Vector3 LookAtAxis;
	};
	
	using namespace DirectX::SimpleMath;
}
