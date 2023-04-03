#pragma once
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
		Vector3 pos;
		Vector3 vtx1;
		Vector3 vtx2;
		Vector3 vtx3;
		Vector3 vtx4;
		Vector4 c1;
		Vector4 c2;
		Vector4 c3;
		Vector4 c4;
		Vector4 color;
		float Rotation;
		float Width;
		float Height;
		BLEND_MODES BlendMode;
		Vector3 ConstrainAxis;
		Vector3 LookAtAxis;
		bool SoftParticle;
	};
}
