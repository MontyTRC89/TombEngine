#include "framework.h"
#include "Renderer/Renderer11.h"
#include "Game/camera.h"
#include "Game/effects/tomb4fx.h"
#include "Specific/clock.h"
#include "Math/Math.h"
#include "Utils.h"
#include "VertexBuffer/VertexBuffer.h"
#include "RenderView/RenderView.h"
#include "Renderer/RendererRectangle.h"

namespace TEN::Renderer
{
	using namespace Utils;
	Renderer11 g_Renderer;

	Renderer11::Renderer11() : gameCamera({0, 0, 0}, {0, 0, 1}, {0, 1, 0}, 1, 1, 0, 1, 10, 90)
	{
		m_blinkColorDirection = 1;
	}

	Renderer11::~Renderer11()
	{
		FreeRendererData();
	}

	void Renderer11::FreeRendererData()
	{
		m_shadowLight = nullptr;

		ClearSceneItems();

		m_meshPointersToMesh.clear();
		m_moveableObjects.resize(0);
		m_staticObjects.resize(0);
		m_sprites.resize(0);
		m_rooms.resize(0);
		m_roomTextures.resize(0);
		m_moveablesTextures.resize(0);
		m_staticsTextures.resize(0);
		m_spritesTextures.resize(0);
		m_animatedTextures.resize(0);
		m_animatedTextureSets.resize(0);

		for (auto& mesh : m_meshes)
			delete mesh;
		m_meshes.resize(0);

		for (auto& item : m_items)
		{
			item.PrevRoomNumber = NO_ROOM;
			item.RoomNumber = NO_ROOM;
			item.ItemNumber = NO_ITEM;
			item.LightsToDraw.clear();
		}
	}

	void Renderer11::ClearSceneItems()
	{
		m_lines3DToDraw.clear();
		m_lines2DToDraw.clear();
		gameCamera.clear();
	}

	void Renderer11::Lock()
	{
		m_Locked = true;
	}

	int Renderer11::Synchronize()
	{
		// Sync the renderer
		int nf = Sync();
		if (nf < 2)
		{
			int i = 2 - nf;
			nf = 2;
			do
			{
				while (!Sync());
				i--;
			}
			while (i);
		}

		return nf;
	}

	void Renderer11::UpdateProgress(float value)
	{
		RenderLoadingScreen(value);
	}

	void Renderer11::RenderToCubemap(const RenderTargetCube& dest, const Vector3& pos, int roomNumer)
	{
		for (int i = 0; i < 6; i++)
		{
			auto renderView = RenderView(pos, RenderTargetCube::forwardVectors[i], RenderTargetCube::upVectors[i],
			                             dest.resolution, dest.resolution, Camera.RoomNumber, 10, 20480,
			                             90 * RADIAN);
			RenderSimpleScene(dest.RenderTargetView[i].Get(), dest.DepthStencilView[i].Get(), renderView);
			m_context->ClearState();
		}
	}

	RendererHudBar::RendererHudBar(ID3D11Device* devicePtr, const Vector2& pos, const Vector2& size, int borderSize, array<Vector4, COLOR_COUNT> colors)
	{
		constexpr auto VERTEX_COUNT		  = 5;
		constexpr auto UV_COUNT			  = 5;
		constexpr auto BORDER_UV_COUNT	  = 16;
		constexpr auto INDEX_COUNT		  = 12;
		constexpr auto BORDER_INDEX_COUNT = 56;

		auto barVertices = std::array<Vector3, VERTEX_COUNT>
		{
			Vector3(pos.x, HUD_ZERO_Y + pos.y, 0.5f),
			Vector3(pos.x + size.x, HUD_ZERO_Y + pos.y, 0.5f),
			Vector3(pos.x + (size.x * 0.5f), HUD_ZERO_Y + (pos.y + (size.y * 0.5f)), 0.5f),
			Vector3(pos.x, HUD_ZERO_Y + pos.y + size.y, 0.5f),
			Vector3(pos.x + size.x, HUD_ZERO_Y + (pos.y + size.y), 0.5f),
		};

		auto hudBorderSize = Vector2(
			borderSize * (SCREEN_SPACE_RES.x / SCREEN_SPACE_RES.y),
			borderSize);

		auto barBorderVertices = std::array<Vector3, 16>
		{
			// Top left
			Vector3(pos.x - hudBorderSize.x, HUD_ZERO_Y + (pos.y - hudBorderSize.y), 0.0f),
			Vector3(pos.x, HUD_ZERO_Y + (pos.y - hudBorderSize.y), 0.0f),
			Vector3(pos.x, HUD_ZERO_Y + pos.y, 0.0f),
			Vector3(pos.x - hudBorderSize.x, HUD_ZERO_Y + pos.y, 0.0f),

			// Top right
			Vector3(pos.x + size.x, HUD_ZERO_Y + (pos.y - hudBorderSize.y), 0.0f),
			Vector3(pos.x + size.x + hudBorderSize.x, HUD_ZERO_Y + (pos.y - hudBorderSize.y), 0.0f),
			Vector3(pos.x + size.x + hudBorderSize.x, HUD_ZERO_Y + pos.y, 0.0f),
			Vector3(pos.x + size.x, HUD_ZERO_Y + pos.y, 0.0f),

			// Bottom right
			Vector3(pos.x + size.x, HUD_ZERO_Y + (pos.y + size.y), 0.0f),
			Vector3(pos.x + size.x + hudBorderSize.x, HUD_ZERO_Y + (pos.y + size.y), 0.0f),
			Vector3(pos.x + size.x + hudBorderSize.x, HUD_ZERO_Y + ((pos.y + size.y) + hudBorderSize.y), 0.0f),
			Vector3(pos.x + size.x, HUD_ZERO_Y + ((pos.y + size.y) + hudBorderSize.y), 0.0f),

			// Bottom left
			Vector3(pos.x - hudBorderSize.x, HUD_ZERO_Y + (pos.y + size.y), 0.0f),
			Vector3(pos.x, HUD_ZERO_Y + (pos.y + size.y), 0.0f),
			Vector3(pos.x, HUD_ZERO_Y + ((pos.y + size.y) + hudBorderSize.y), 0.0f),
			Vector3(pos.x - hudBorderSize.x, HUD_ZERO_Y + ((pos.y + size.y) + hudBorderSize.y), 0.0f)
		};

		auto barUVs = std::array<Vector2, UV_COUNT>
		{
			Vector2::Zero,
			Vector2(1.0f, 0.0f),
			Vector2::One * 0.5f,
			Vector2(0.0f, 1.0f),
			Vector2::One,
		};
		auto barBorderUVs = std::array<Vector2, BORDER_UV_COUNT>
		{
			// Top left
			Vector2::Zero,
			Vector2(0.25f, 0.0f),
			Vector2::One * 0.25f,
			Vector2(0.0f, 0.25f),

			// Top right
			Vector2(0.75f, 0.0f),
			Vector2(1.0f, 0.0f),
			Vector2(1.0f, 0.25f),
			Vector2(0.75f, 0.25f),

			// Bottom right
			Vector2::One * 0.75f,
			Vector2(1.0f, 0.75f),
			Vector2::One,
			Vector2(0.75f, 1.0f),

			// Bottom left
			Vector2(0.0f, 0.75f),
			Vector2(0.25f, 0.75f),
			Vector2(0.25f, 1.0f),
			Vector2(0.0f, 1.0f)
		};

		auto barIndices = std::array<int, INDEX_COUNT>
		{
			2, 1, 0,
			2, 0, 3,
			2, 3, 4,
			2, 4, 1
		};
		auto barBorderIndices = std::array<int, BORDER_INDEX_COUNT>
		{
			// Top left
			0, 1, 3, 1, 2, 3,

			// Top center
			1, 4, 2, 4, 7, 2,

			// Top right
			4, 5, 7, 5, 6, 7,

			// Right
			7, 6, 8, 6, 9, 8,

			// Bottom right
			8, 9, 11, 9, 10, 11,

			// Bottom
			13, 8, 14, 8, 11, 14,

			// Bottom left
			12, 13, 15, 13, 14, 15,

			// Left
			3, 2, 12, 2, 13, 12,

			// Center
			2, 7, 13, 7, 8, 13
		};

		auto vertices = std::array<RendererVertex, VERTEX_COUNT>{};
		for (int i = 0; i < VERTEX_COUNT; i++)
		{
			vertices[i].Position = barVertices[i];
			vertices[i].Color = colors[i];
			vertices[i].UV = barUVs[i];
			vertices[i].Normal = Vector3::Zero;
			vertices[i].Bone = 0.0f;
		}

		this->InnerVertexBuffer = VertexBuffer(devicePtr, vertices.size(), vertices.data());
		this->InnerIndexBuffer = IndexBuffer(devicePtr, barIndices.size(), barIndices.data());

		auto borderVertices = std::array<RendererVertex, barBorderVertices.size()>{};
		for (int i = 0; i < barBorderVertices.size(); i++)
		{
			borderVertices[i].Position = barBorderVertices[i];
			borderVertices[i].Color = Vector4::One;
			borderVertices[i].UV = barBorderUVs[i];
			borderVertices[i].Normal = Vector3::Zero;
			borderVertices[i].Bone = 0.0f;
		}

		this->VertexBufferBorder = VertexBuffer(devicePtr, borderVertices.size(), borderVertices.data());
		this->IndexBufferBorder = IndexBuffer(devicePtr, barBorderIndices.size(), barBorderIndices.data());
	}

	float Renderer11::CalculateFrameRate()
	{
		static int last_time = clock();
		static int count = 0;
		static float fps = 0.0f;

		count++;
		if (count == 10)
		{
			double t;
			time_t this_time;
			this_time = clock();
			t = (this_time - last_time) / static_cast<double>(CLOCKS_PER_SEC);
			last_time = this_time;
			fps = static_cast<float>(count / t);
			count = 0;
		}

		m_fps = fps;

		return fps;
	}

	void Renderer11::BindTexture(TEXTURE_REGISTERS registerType, TextureBase* texture, SAMPLER_STATES samplerType)
	{
		m_context->PSSetShaderResources(static_cast<UINT>(registerType), 1, texture->ShaderResourceView.GetAddressOf());

		ID3D11SamplerState* samplerState = nullptr;
		switch (samplerType)
		{
		case SAMPLER_ANISOTROPIC_CLAMP:
			samplerState = m_states->AnisotropicClamp();
			break;

		case SAMPLER_ANISOTROPIC_WRAP:
			samplerState = m_states->AnisotropicWrap();
			break;

		case SAMPLER_LINEAR_CLAMP:
			samplerState = m_states->LinearClamp();
			break;

		case SAMPLER_LINEAR_WRAP:
			samplerState = m_states->LinearWrap();
			break;

		case SAMPLER_POINT_WRAP:
			samplerState = m_states->PointWrap();
			break;

		case SAMPLER_SHADOW_MAP:
			samplerState = m_shadowSampler.Get();
			break;

		default:
			return;
		}

		m_context->PSSetSamplers(registerType, 1, &samplerState);
	}

	void Renderer11::BindRenderTargetAsTexture(TEXTURE_REGISTERS registerType, RenderTarget2D* target, SAMPLER_STATES samplerType)
	{
		m_context->PSSetShaderResources(static_cast<UINT>(registerType), 1, target->ShaderResourceView.GetAddressOf());

		ID3D11SamplerState* samplerState = nullptr;
		switch (samplerType)
		{
		case SAMPLER_ANISOTROPIC_CLAMP:
			samplerState = m_states->AnisotropicClamp();
			break;

		case SAMPLER_ANISOTROPIC_WRAP:
			samplerState = m_states->AnisotropicWrap();
			break;

		case SAMPLER_LINEAR_CLAMP:
			samplerState = m_states->LinearClamp();
			break;

		case SAMPLER_LINEAR_WRAP:
			samplerState = m_states->LinearWrap();
			break;

		case SAMPLER_POINT_WRAP:
			samplerState = m_states->PointWrap();
			break;

		case SAMPLER_SHADOW_MAP:
			samplerState = m_shadowSampler.Get();
			break;

		default:
			return;
		}

		m_context->PSSetSamplers(registerType, 1, &samplerState);
	}

	void Renderer11::BindRoomLights(std::vector<RendererLight*>& lights)
	{
		for (int i = 0; i < lights.size(); i++)
		{ 
			memcpy(&m_stRoom.RoomLights[i], lights[i], sizeof(ShaderLight));
		}
		m_stRoom.NumRoomLights = lights.size();
	}

	void Renderer11::BindStaticLights(std::vector<RendererLight*>& lights)
	{
		for (int i = 0; i < lights.size(); i++)
		{
			memcpy(&m_stStatic.Lights[i], lights[i], sizeof(ShaderLight));
		}
		m_stStatic.NumLights = lights.size();
	}

	void Renderer11::BindInstancedStaticLights(std::vector<RendererLight*>& lights, int instanceID)
	{
		for (int i = 0; i < lights.size(); i++)
		{
			memcpy(&m_stInstancedStaticMeshBuffer.StaticMeshes[instanceID].Lights[i], lights[i], sizeof(ShaderLight));
		} 
		m_stInstancedStaticMeshBuffer.StaticMeshes[instanceID].NumLights = lights.size();
	}

	void Renderer11::BindMoveableLights(std::vector<RendererLight*>& lights, int roomNumber, int prevRoomNumber, float fade)
	{
		int numLights = 0;
		for (int i = 0; i < lights.size(); i++)
		{
			float fadedCoeff = 1.0f;

			// Interpolate lights which don't affect neighbor rooms
			if (!lights[i]->AffectNeighbourRooms && roomNumber != NO_ROOM && lights[i]->RoomNumber != NO_ROOM)
			{
				if (lights[i]->RoomNumber == roomNumber)
					fadedCoeff = fade;
				else if (lights[i]->RoomNumber == prevRoomNumber)
					fadedCoeff = 1.0f - fade;
				else
					continue;
			}

			if (fadedCoeff == 0.0f)
				continue;

			memcpy(&m_stItem.Lights[numLights], lights[i], sizeof(ShaderLight));
			m_stItem.Lights[numLights].Intensity *= fadedCoeff;
			numLights++;
		}

		m_stItem.NumLights = numLights;
	}

	void Renderer11::BindConstantBufferVS(CONSTANT_BUFFERS constantBufferType, ID3D11Buffer** buffer)
	{
		m_context->VSSetConstantBuffers(static_cast<UINT>(constantBufferType), 1, buffer);
	}

	void Renderer11::BindConstantBufferPS(CONSTANT_BUFFERS constantBufferType, ID3D11Buffer** buffer)
	{
		m_context->PSSetConstantBuffers(static_cast<UINT>(constantBufferType), 1, buffer);
	}

	void Renderer11::SetBlendMode(BLEND_MODES blendMode, bool force)
	{
		if (blendMode != lastBlendMode || force)
		{
			switch (blendMode)
			{
			case BLENDMODE_ALPHABLEND:
				m_context->OMSetBlendState(m_states->NonPremultiplied(), nullptr, 0xFFFFFFFF);
				break;

			case BLENDMODE_ALPHATEST:
				m_context->OMSetBlendState(m_states->Opaque(), nullptr, 0xFFFFFFFF);
				break;

			case BLENDMODE_OPAQUE:
				m_context->OMSetBlendState(m_states->Opaque(), nullptr, 0xFFFFFFFF);
				break;

			case BLENDMODE_SUBTRACTIVE:
				m_context->OMSetBlendState(m_subtractiveBlendState.Get(), nullptr, 0xFFFFFFFF);
				break;

			case BLENDMODE_ADDITIVE:
				m_context->OMSetBlendState(m_states->Additive(), nullptr, 0xFFFFFFFF);
				break;

			case BLENDMODE_SCREEN:
				m_context->OMSetBlendState(m_screenBlendState.Get(), nullptr, 0xFFFFFFFF);
				break;

			case BLENDMODE_LIGHTEN:
				m_context->OMSetBlendState(m_lightenBlendState.Get(), nullptr, 0xFFFFFFFF);
				break;

			case BLENDMODE_EXCLUDE:
				m_context->OMSetBlendState(m_excludeBlendState.Get(), nullptr, 0xFFFFFFFF);
				break;
			}

			m_stBlending.BlendMode = static_cast<unsigned int>(blendMode);
			m_cbBlending.updateData(m_stBlending, m_context.Get());
			BindConstantBufferPS(CB_BLENDING, m_cbBlending.get());

			lastBlendMode = blendMode;
		}

		switch (blendMode)
		{
		case BLENDMODE_OPAQUE:
		case BLENDMODE_ALPHATEST:
			SetDepthState(DEPTH_STATE_WRITE_ZBUFFER);
			break;

		default:
			SetDepthState(DEPTH_STATE_READ_ONLY_ZBUFFER);
			break;
		}
	}

	void Renderer11::SetDepthState(DEPTH_STATES depthState, bool force)
	{
		if (depthState != lastDepthState || force)
		{
			switch (depthState)
			{
			case DEPTH_STATE_READ_ONLY_ZBUFFER:
				m_context->OMSetDepthStencilState(m_states->DepthRead(), 0xFFFFFFFF);
				break;

			case DEPTH_STATE_WRITE_ZBUFFER:
				m_context->OMSetDepthStencilState(m_states->DepthDefault(), 0xFFFFFFFF);
				break;

			case DEPTH_STATE_NONE:
				m_context->OMSetDepthStencilState(m_states->DepthNone(), 0xFFFFFFFF);
				break;

			}

			lastDepthState = depthState;
		}
	}

	void Renderer11::SetCullMode(CULL_MODES cullMode, bool force)
	{
		if (m_numDebugPage == RENDERER_DEBUG_PAGE::WIREFRAME_MODE)
		{
			m_context->RSSetState(m_states->Wireframe());
			return;
		}

		if (cullMode != lastCullMode || force)
		{
			switch (cullMode)
			{
			case CULL_MODE_NONE:
				m_context->RSSetState(m_cullNoneRasterizerState.Get());
				break;

			case CULL_MODE_CCW:
				m_context->RSSetState(m_cullCounterClockwiseRasterizerState.Get());
				break;

			case CULL_MODE_CW:
				m_context->RSSetState(m_cullClockwiseRasterizerState.Get());
				break;
			}

			lastCullMode = cullMode;
		}
	}

	void Renderer11::SetAlphaTest(ALPHA_TEST_MODES mode, float threshold, bool force)
	{
		if (m_stBlending.AlphaTest != static_cast<int>(mode) ||
			m_stBlending.AlphaThreshold != threshold ||
			force)
		{
			m_stBlending.AlphaTest = static_cast<int>(mode);
			m_stBlending.AlphaThreshold = threshold;
			m_cbBlending.updateData(m_stBlending, m_context.Get());
			BindConstantBufferPS(CB_BLENDING, m_cbBlending.get());
		}
	}

	void Renderer11::SetScissor(RendererRectangle s)
	{
		D3D11_RECT rects;
		rects.left = s.left;
		rects.top = s.top;
		rects.right = s.right;
		rects.bottom = s.bottom;

		m_context->RSSetScissorRects(1, &rects);
	}

	void Renderer11::ResetScissor()
	{
		D3D11_RECT rects[1];
		rects[0].left = 0;
		rects[0].right = m_screenWidth;
		rects[0].top = 0;
		rects[0].bottom = m_screenHeight;

		m_context->RSSetScissorRects(1, rects);
	}
}
