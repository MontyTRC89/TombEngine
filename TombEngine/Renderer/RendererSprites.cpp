#include "framework.h"
#include "Renderer/Structures/RendererSprite.h"
#include "Renderer/Structures/RendererSpriteBucket.h"
#include "Renderer/Renderer.h"

namespace TEN::Renderer 
{
	using namespace TEN::Renderer::Structures;

	void Renderer::AddSpriteBillboard(RendererSprite* sprite, const Vector3& pos, const Vector4& color, float orient2D, float scale,
										Vector2 size, BlendMode blendMode, bool isSoftParticle, RenderView& view, SpriteRenderType renderType)
	{
		if (_isLocked)
			return;

		if (scale <= 0.0f)
			scale = 1.0f;

		size.x *= scale;
		size.y *= scale;

		RendererSpriteToDraw spr = {};

		spr.Type = SpriteType::Billboard;
		spr.Sprite = sprite;
		spr.pos = pos;
		spr.Rotation = orient2D;
		spr.Scale = scale;
		spr.Width = size.x;
		spr.Height = size.y;
		spr.BlendMode = blendMode;
		spr.SoftParticle = isSoftParticle;
		spr.c1 = color;
		spr.c2 = color;
		spr.c3 = color;
		spr.c4 = color;
		spr.color = color;
		spr.renderType = renderType;

		view.SpritesToDraw.push_back(spr);
	}

	void Renderer::AddSpriteBillboardConstrained(RendererSprite* sprite, const Vector3& pos, const Vector4 &color, float orient2D,
												   float scale, Vector2 size, BlendMode blendMode, const Vector3& constrainAxis,
												   bool softParticles, RenderView& view, SpriteRenderType renderType)
	{
		if (_isLocked)
			return;

		if (scale <= 0.0f)
			scale = 1.0f;

		size.x *= scale;
		size.y *= scale;

		RendererSpriteToDraw spr = {};

		spr.Type = SpriteType::CustomBillboard;
		spr.Sprite = sprite;
		spr.pos = pos;
		spr.Rotation = orient2D;
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
		spr.renderType = renderType;

		view.SpritesToDraw.push_back(spr);
	}

	void Renderer::AddSpriteBillboardConstrainedLookAt(RendererSprite* sprite, const Vector3& pos, const Vector4& color, float orient2D,
														 float scale, Vector2 size, BlendMode blendMode, const Vector3& lookAtAxis,
														 bool isSoftParticle, RenderView& view, SpriteRenderType renderType)
	{
		if (_isLocked)
			return;

		if (scale <= 0.0f)
			scale = 1.0f;

		size.x *= scale;
		size.y *= scale;

		RendererSpriteToDraw spr = {};

		spr.Type = SpriteType::LookAtBillboard;
		spr.Sprite = sprite;
		spr.pos = pos;
		spr.Rotation = orient2D;
		spr.Scale = scale;
		spr.Width = size.x;
		spr.Height = size.y;
		spr.BlendMode = blendMode;
		spr.LookAtAxis = lookAtAxis;
		spr.SoftParticle = isSoftParticle;
		spr.c1 = color;
		spr.c2 = color;
		spr.c3 = color;
		spr.c4 = color;
		spr.color = color;
		spr.renderType = renderType;

		view.SpritesToDraw.push_back(spr);
	}

	void Renderer::AddQuad(RendererSprite* sprite, const Vector3& vertex0, const Vector3& vertex1, const Vector3& vertex2, const Vector3& vertex3,
							 const Vector4 color, float orient2D, float scale, Vector2 size, BlendMode blendMode, bool softParticles,
							 RenderView& view)
	{
		AddQuad(sprite, vertex0, vertex1, vertex2, vertex3, color, color, color, color, orient2D, scale, size, blendMode, softParticles, view, SpriteRenderType::Default);
	}

	void Renderer::AddQuad(RendererSprite* sprite, const Vector3& vertex0, const Vector3& vertex1, const Vector3& vertex2, const Vector3& vertex3,
							 const Vector4& color0, const Vector4& color1, const Vector4& color2, const Vector4& color3, float orient2D,
							 float scale, Vector2 size, BlendMode blendMode, bool isSoftParticle, RenderView& view, SpriteRenderType renderType)
	{
		if (_isLocked)
			return;

		if (scale <= 0.0f)
			scale = 1.0f;

		size.x *= scale;
		size.y *= scale;

		RendererSpriteToDraw spr = {};

		spr.Type = SpriteType::ThreeD;
		spr.Sprite = sprite;
		spr.vtx1 = vertex0;
		spr.vtx2 = vertex1;
		spr.vtx3 = vertex2;
		spr.vtx4 = vertex3;
		spr.c1 = color0;
		spr.c2 = color1;
		spr.c3 = color2;
		spr.c4 = color3;
		spr.Rotation = orient2D;
		spr.Scale = scale;
		spr.Width = size.x;
		spr.Height = size.y;
		spr.BlendMode = blendMode;
		spr.pos = (vertex0 + vertex1 + vertex2 + vertex3) / 4.0f;
		spr.SoftParticle = isSoftParticle;
		spr.renderType = renderType;

		view.SpritesToDraw.push_back(spr);
	}

	void Renderer::AddColoredQuad(const Vector3& vertex0, const Vector3& vertex1, const Vector3& vertex2, const Vector3& vertex3,
									const Vector4& color, BlendMode blendMode, RenderView& view)
	{
		AddColoredQuad(vertex0, vertex1, vertex2, vertex3, color, color, color, color, blendMode, view, SpriteRenderType::Default);
	}

	void Renderer::AddColoredQuad(const Vector3& vertex0, const Vector3& vertex1, const Vector3& vertex2, const Vector3& vertex3,
									const Vector4& color0, const Vector4& color1, const Vector4& color2, const Vector4& color3,
									BlendMode blendMode, RenderView& view, SpriteRenderType renderType)
	{
		if (_isLocked)
			return;

		auto sprite = RendererSpriteToDraw{};

		sprite.Type = SpriteType::ThreeD;
		sprite.Sprite = &_whiteSprite;
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
		sprite.renderType = renderType;

		view.SpritesToDraw.push_back(sprite);
	}

	void Renderer::SortAndPrepareSprites(RenderView& view)
	{
		if (view.SpritesToDraw.empty())
		{
			return;
		}

		_spriteBuckets.clear();

		// Sort sprites by sprite and blend mode for faster batching.
		std::sort(
			view.SpritesToDraw.begin(),
			view.SpritesToDraw.end(),
			[](RendererSpriteToDraw& rDrawSprite0, RendererSpriteToDraw& rDrawSprite1)
			{
				if (rDrawSprite0.Sprite != rDrawSprite1.Sprite)
				{
					return (rDrawSprite0.Sprite > rDrawSprite1.Sprite);
				}
				else if (rDrawSprite0.BlendMode != rDrawSprite1.BlendMode)
				{
					return (rDrawSprite0.BlendMode > rDrawSprite1.BlendMode);
				}
				else
				{
					return (rDrawSprite0.Type > rDrawSprite1.Type);
				}
			}
		);

		// Group sprites to draw in buckets for instancing (billboards only).
		RendererSpriteBucket currentSpriteBucket;

		currentSpriteBucket.Sprite = view.SpritesToDraw[0].Sprite;
		currentSpriteBucket.BlendMode = view.SpritesToDraw[0].BlendMode;
		currentSpriteBucket.IsBillboard = view.SpritesToDraw[0].Type != SpriteType::ThreeD;
		currentSpriteBucket.IsSoftParticle = view.SpritesToDraw[0].SoftParticle;
		currentSpriteBucket.RenderType = view.SpritesToDraw[0].renderType;

		for (auto& rDrawSprite : view.SpritesToDraw)
		{
			bool isBillboard = rDrawSprite.Type != SpriteType::ThreeD;

			if (rDrawSprite.Sprite != currentSpriteBucket.Sprite ||
				rDrawSprite.BlendMode != currentSpriteBucket.BlendMode ||
				rDrawSprite.SoftParticle != currentSpriteBucket.IsSoftParticle ||
				rDrawSprite.renderType != currentSpriteBucket.RenderType ||
				currentSpriteBucket.SpritesToDraw.size() == INSTANCED_SPRITES_BUCKET_SIZE ||
				isBillboard != currentSpriteBucket.IsBillboard)
			{
				_spriteBuckets.push_back(currentSpriteBucket);

				currentSpriteBucket.Sprite = rDrawSprite.Sprite;
				currentSpriteBucket.BlendMode = rDrawSprite.BlendMode;
				currentSpriteBucket.IsBillboard = isBillboard;
				currentSpriteBucket.IsSoftParticle = rDrawSprite.SoftParticle;
				currentSpriteBucket.RenderType = rDrawSprite.renderType;
				currentSpriteBucket.SpritesToDraw.clear();
			}

			// HACK: Prevent sprites like Explosionsmoke which have BlendMode::Subtractive from having laser effects.
			if (DoesBlendModeRequireSorting(rDrawSprite.BlendMode) && currentSpriteBucket.RenderType != SpriteRenderType::Default)
			{
				// If blend mode requires sorting, save sprite for later.
				int distance = (rDrawSprite.pos - Camera.pos.ToVector3()).Length();
				RendererTransparentFace face;
				face.type = TransparentFaceType::Sprite;
				face.info.sprite = &rDrawSprite;
				face.distance = distance;
				face.info.world = GetWorldMatrixForSprite(&rDrawSprite, view);
				face.info.blendMode = rDrawSprite.BlendMode;

				for (int j = 0; j < view.RoomsToDraw.size(); j++)
				{
					int roomNumber = view.RoomsToDraw[j]->RoomNumber;
					if (g_Level.Rooms[roomNumber].Active() && IsPointInRoom(Vector3i(rDrawSprite.pos), roomNumber))
					{
						view.RoomsToDraw[j]->TransparentFacesToDraw.push_back(face);
						break;
					}
				}
			}
			// Add sprite to current bucket.
			else
			{
				currentSpriteBucket.SpritesToDraw.push_back(rDrawSprite);
			}
		}

		_spriteBuckets.push_back(currentSpriteBucket);
	}

	void Renderer::DrawSprites(RenderView& view, RendererPass rendererPass)
	{
		if (view.SpritesToDraw.empty())
		{
			return;
		}

		BindRenderTargetAsTexture(TextureRegister::DepthMap, &_depthMap, SamplerStateRegister::LinearClamp);

		SetDepthState(DepthState::Read);
		SetCullMode(CullMode::None);

		_context->VSSetShader(_vsInstancedSprites.Get(), nullptr, 0);

		// Set up vertex buffer and parameters.
		UINT stride = sizeof(Vertex);
		UINT offset = 0;
		_context->IASetVertexBuffers(0, 1, _quadVertexBuffer.Buffer.GetAddressOf(), &stride, &offset);

		for (auto& spriteBucket : _spriteBuckets)
		{
			if (spriteBucket.SpritesToDraw.size() == 0 || !spriteBucket.IsBillboard)
			{
				continue;
			}

			// Prepare constant buffer for instanced sprites.
			for (int i = 0; i < spriteBucket.SpritesToDraw.size(); i++)
			{
				auto& rDrawSprite = spriteBucket.SpritesToDraw[i];

				_stInstancedSpriteBuffer.Sprites[i].World = GetWorldMatrixForSprite(&rDrawSprite, view);
				_stInstancedSpriteBuffer.Sprites[i].Color = rDrawSprite.color;
				_stInstancedSpriteBuffer.Sprites[i].IsBillboard = 1;
				_stInstancedSpriteBuffer.Sprites[i].IsSoftParticle = rDrawSprite.SoftParticle ? 1 : 0;

				// NOTE: Strange packing due to particular HLSL 16 byte alignment requirements.
				_stInstancedSpriteBuffer.Sprites[i].UV[0].x = rDrawSprite.Sprite->UV[0].x;
				_stInstancedSpriteBuffer.Sprites[i].UV[0].y = rDrawSprite.Sprite->UV[1].x;
				_stInstancedSpriteBuffer.Sprites[i].UV[0].z = rDrawSprite.Sprite->UV[2].x;
				_stInstancedSpriteBuffer.Sprites[i].UV[0].w = rDrawSprite.Sprite->UV[3].x;
				_stInstancedSpriteBuffer.Sprites[i].UV[1].x = rDrawSprite.Sprite->UV[0].y;
				_stInstancedSpriteBuffer.Sprites[i].UV[1].y = rDrawSprite.Sprite->UV[1].y;
				_stInstancedSpriteBuffer.Sprites[i].UV[1].z = rDrawSprite.Sprite->UV[2].y;
				_stInstancedSpriteBuffer.Sprites[i].UV[1].w = rDrawSprite.Sprite->UV[3].y;
			}

			BindTexture(TextureRegister::ColorMap, spriteBucket.Sprite->Texture, SamplerStateRegister::LinearClamp);

			_cbInstancedSpriteBuffer.updateData(_stInstancedSpriteBuffer, _context.Get());

			// Draw sprites with instancing.
			DrawInstancedTriangles(4, (unsigned int)spriteBucket.SpritesToDraw.size(), 0);

			_numSpritesDrawCalls++;
		}

		// Draw 3D sprites.
		SetDepthState(DepthState::Read);
		SetCullMode(CullMode::None);

		_context->VSSetShader(_vsSprites.Get(), nullptr, 0);

		stride = sizeof(Vertex);
		offset = 0;
		_context->IASetVertexBuffers(0, 1, _quadVertexBuffer.Buffer.GetAddressOf(), &stride, &offset);

		for (auto& spriteBucket : _spriteBuckets)
		{
			if (spriteBucket.SpritesToDraw.empty() || spriteBucket.IsBillboard)
			{
				continue;
			}

			_stSprite.IsSoftParticle = spriteBucket.IsSoftParticle ? 1 : 0;
			_stSprite.RenderType = (int)spriteBucket.RenderType;

			_cbSprite.updateData(_stSprite, _context.Get());

			SetBlendMode(spriteBucket.BlendMode);
			BindTexture(TextureRegister::ColorMap, spriteBucket.Sprite->Texture, SamplerStateRegister::LinearClamp);

			if (spriteBucket.BlendMode == BlendMode::AlphaTest)
			{
				SetAlphaTest(AlphaTestMode::GreatherThan, ALPHA_TEST_THRESHOLD, true);
			}
			else
			{
				SetAlphaTest(AlphaTestMode::None, 0);
			}

			_primitiveBatch->Begin();

			for (auto& rDrawSprite : spriteBucket.SpritesToDraw)
			{
				auto vertex0 = Vertex{};
				vertex0.Position = rDrawSprite.vtx1;
				vertex0.UV = rDrawSprite.Sprite->UV[0];
				vertex0.Color = rDrawSprite.c1;

				auto vertex1 = Vertex{};
				vertex1.Position = rDrawSprite.vtx2;
				vertex1.UV = rDrawSprite.Sprite->UV[1];
				vertex1.Color = rDrawSprite.c2;

				auto vertex2 = Vertex{};
				vertex2.Position = rDrawSprite.vtx3;
				vertex2.UV = rDrawSprite.Sprite->UV[2];
				vertex2.Color = rDrawSprite.c3;

				auto vertex3 = Vertex{};
				vertex3.Position = rDrawSprite.vtx4;
				vertex3.UV = rDrawSprite.Sprite->UV[3];
				vertex3.Color = rDrawSprite.c4;

				_primitiveBatch->DrawTriangle(vertex0, vertex1, vertex3);
				_primitiveBatch->DrawTriangle(vertex1, vertex2, vertex3);
			}

			_primitiveBatch->End();

			_numSpritesDrawCalls++;
			_numDrawCalls++;
		}
	}

}

