#include "framework.h"
#include "RendererSprites.h"
#include "Renderer/Renderer11.h"

namespace TEN::Renderer 
{
	void Renderer11::AddSpriteBillboard(RendererSprite* sprite, Vector3 pos, Vector4 color, float rotation, float scale, Vector2 size, BLEND_MODES blendMode, bool softParticles, RenderView& view)
	{
		if (m_Locked)
			return;

		if (scale <= 0.0f)
			scale = 1.0f;

		size.x *= scale;
		size.y *= scale;

		RendererSpriteToDraw spr = {};

		spr.Type = RENDERER_SPRITE_TYPE::SPRITE_TYPE_BILLBOARD;
		spr.Sprite = sprite;
		spr.pos = pos;
		spr.color = color;
		spr.Rotation = rotation;
		spr.Scale = scale;
		spr.Width = size.x;
		spr.Height = size.y;
		spr.BlendMode = blendMode;
		spr.SoftParticle = softParticles;

		view.spritesToDraw.push_back(spr);
	}

	void Renderer11::AddSpriteBillboardConstrained(RendererSprite* sprite, Vector3 pos, Vector4 color, float rotation, float scale, Vector2 size, BLEND_MODES blendMode, Vector3 constrainAxis, bool softParticles, RenderView& view)
	{
		if (m_Locked)
			return;

		if (scale <= 0.0f)
			scale = 1.0f;

		size.x *= scale;
		size.y *= scale;

		RendererSpriteToDraw spr = {};

		spr.Type = RENDERER_SPRITE_TYPE::SPRITE_TYPE_BILLBOARD_CUSTOM;
		spr.Sprite = sprite;
		spr.pos = pos;
		spr.color = color;
		spr.Rotation = rotation;
		spr.Scale = scale;
		spr.Width = size.x;
		spr.Height = size.y;
		spr.BlendMode = blendMode;
		spr.ConstrainAxis = constrainAxis;
		spr.SoftParticle = softParticles;

		view.spritesToDraw.push_back(spr);
	}

	void Renderer11::AddSpriteBillboardConstrainedLookAt(RendererSprite* sprite, Vector3 pos, Vector4 color, float rotation, float scale, Vector2 size, BLEND_MODES blendMode, Vector3 lookAtAxis, bool softParticles, RenderView& view)
	{
		if (m_Locked)
			return;

		if (scale <= 0.0f)
			scale = 1.0f;

		size.x *= scale;
		size.y *= scale;
		RendererSpriteToDraw spr = {};
		spr.Type = RENDERER_SPRITE_TYPE::SPRITE_TYPE_BILLBOARD_LOOKAT;
		spr.Sprite = sprite;
		spr.pos = pos;
		spr.color = color;
		spr.Rotation = rotation;
		spr.Scale = scale;
		spr.Width = size.x;
		spr.Height = size.y;
		spr.BlendMode = blendMode;
		spr.LookAtAxis = lookAtAxis;
		spr.SoftParticle = softParticles;

		view.spritesToDraw.push_back(spr);

	}

	void Renderer11::AddQuad(RendererSprite* sprite, Vector3 vtx1, Vector3 vtx2, Vector3 vtx3, Vector3 vtx4, Vector4 color, float rotation, float scale, Vector2 size, BLEND_MODES blendMode, bool softParticles, RenderView& view)
	{
		AddQuad(sprite, vtx1, vtx2, vtx3, vtx4, color, color, color, color, rotation, scale, size, blendMode, softParticles, view);
	}

	void Renderer11::AddQuad(RendererSprite* sprite, Vector3 vtx1, Vector3 vtx2, Vector3 vtx3, Vector3 vtx4, Vector4 c1, Vector4 c2, Vector4 c3, Vector4 c4, float rotation, float scale, Vector2 size, BLEND_MODES blendMode, bool softParticles, RenderView& view)
	{
		if (m_Locked)
			return;

		if (scale <= 0.0f)
			scale = 1.0f;

		size.x *= scale;
		size.y *= scale;

		RendererSpriteToDraw spr = {};

		spr.Type = RENDERER_SPRITE_TYPE::SPRITE_TYPE_3D;
		spr.Sprite = sprite;
		spr.vtx1 = vtx1;
		spr.vtx2 = vtx2;
		spr.vtx3 = vtx3;
		spr.vtx4 = vtx4;
		spr.c1 = c1;
		spr.c2 = c2;
		spr.c3 = c3;
		spr.c4 = c4;
		spr.Rotation = rotation;
		spr.Scale = scale;
		spr.Width = size.x;
		spr.Height = size.y;
		spr.BlendMode = blendMode;
		spr.pos = (vtx1 + vtx2 + vtx3 + vtx4) / 4.0f;
		spr.SoftParticle = softParticles;

		view.spritesToDraw.push_back(spr);
	}
}

