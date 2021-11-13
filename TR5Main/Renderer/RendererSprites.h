#pragma once
#include <SimpleMath.h>
#include "RenderEnums.h"

namespace TEN::Renderer {
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
	void addSpriteBillboard(RendererSprite* sprite,Vector3 pos, Vector4 color, float rotation, float scale, Vector2 size, BLEND_MODES blendMode, RenderView& view);
	void addSpriteBillboardConstrained(RendererSprite* sprite,Vector3 pos,Vector4 color, float rotation, float scale, Vector2 size, BLEND_MODES blendMode,Vector3 constrainAxis, RenderView& view);
	void addSpriteBillboardConstrainedLookAt(RendererSprite* sprite,Vector3 pos,Vector4 color, float rotation, float scale, Vector2 size, BLEND_MODES blendMode,Vector3 lookAtAxis, RenderView& view);
	void addSprite3D(RendererSprite* sprite,Vector3 vtx1, Vector3 vtx2,Vector3 vtx3, Vector3 vtx4,Vector4 color, float rotation, float scale, Vector2 size, BLEND_MODES blendMode,RenderView& view);


}