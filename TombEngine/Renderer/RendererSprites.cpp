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
		spr.Rotation = rotation;
		spr.Scale = scale;
		spr.Width = size.x;
		spr.Height = size.y;
		spr.BlendMode = blendMode;
		spr.SoftParticle = softParticles;
		spr.c1 = color;
		spr.c2 = color;
		spr.c3 = color;
		spr.c4 = color;
		spr.color = color;

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
		spr.Rotation = rotation;
		spr.Scale = scale;
		spr.Width = size.x;
		spr.Height = size.y;
		spr.BlendMode = blendMode;
		spr.ConstrainAxis = constrainAxis;
		spr.SoftParticle = softParticles;
		spr.c1 = color;
		spr.c2 = color;
		spr.c3 = color;
		spr.c4 = color;
		spr.color = color;

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
		spr.Rotation = rotation;
		spr.Scale = scale;
		spr.Width = size.x;
		spr.Height = size.y;
		spr.BlendMode = blendMode;
		spr.LookAtAxis = lookAtAxis;
		spr.SoftParticle = softParticles;
		spr.c1 = color;
		spr.c2 = color;
		spr.c3 = color;
		spr.c4 = color;
		spr.color = color;

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

	void Renderer11::AddColoredQuad(const Vector3& vertex0, const Vector3& vertex1, const Vector3& vertex2, const Vector3& vertex3,
									const Vector4& color, BLEND_MODES blendMode, RenderView& view)
	{
		AddColoredQuad(vertex0, vertex1, vertex2, vertex3, color, color, color, color, blendMode, view);
	}

	void Renderer11::AddColoredQuad(const Vector3& vertex0, const Vector3& vertex1, const Vector3& vertex2, const Vector3& vertex3,
									const Vector4& color0, const Vector4& color1, const Vector4& color2, const Vector4& color3,
									BLEND_MODES blendMode, RenderView& view)
	{
		if (m_Locked)
			return;

		auto sprite = RendererSpriteToDraw{};

		sprite.Type = RENDERER_SPRITE_TYPE::SPRITE_TYPE_3D;
		sprite.Sprite = &m_whiteSprite;
		sprite.vtx1 = vertex0;
		sprite.vtx2 = vertex1;
		sprite.vtx3 = vertex2;
		sprite.vtx4 = vertex3;
		sprite.c1 = color0;
		sprite.c2 = color1;
		sprite.c3 = color2;
		sprite.c4 = color3;
		sprite.BlendMode = blendMode;
		sprite.pos = (vertex0 + vertex1 + vertex2 + vertex3) / 4.0f;
		sprite.SoftParticle = false;

		view.spritesToDraw.push_back(sprite);
	}
}

