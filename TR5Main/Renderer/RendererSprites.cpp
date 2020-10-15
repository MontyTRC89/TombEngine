#include "framework.h"
#include "RendererSprites.h"
#include "Renderer11.h"
namespace T5M::Renderer {
	void addSpriteBillboard(RendererSprite* sprite, Vector3 pos, Vector4 color, float rotation, float scale, Vector2 size, BLEND_MODES blendMode, RenderView& view)
	{
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
		view.spritesToDraw.push_back(spr);

	}
	void addSpriteBillboardConstrained(RendererSprite* sprite, Vector3 pos, Vector4 color, float rotation, float scale, Vector2 size, BLEND_MODES blendMode, Vector3 constrainAxis, RenderView& view)
	{
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
		view.spritesToDraw.push_back(spr);
	}
	void addSpriteBillboardConstrainedLookAt(RendererSprite* sprite, Vector3 pos, Vector4 color, float rotation, float scale, Vector2 size, BLEND_MODES blendMode, Vector3 lookAtAxis, RenderView& view)
	{
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
		view.spritesToDraw.push_back(spr);

	}
	void addSprite3D(RendererSprite* sprite, Vector3 vtx1, Vector3 vtx2, Vector3 vtx3, Vector3 vtx4, Vector4 color, float rotation, float scale, Vector2 size, BLEND_MODES blendMode, RenderView& view)
	{
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
		spr.color = color;
		spr.Rotation = rotation;
		spr.Scale = scale;
		spr.Width = size.x;
		spr.Height = size.y;
		spr.BlendMode = blendMode;
		view.spritesToDraw.push_back(spr);

	}
}

