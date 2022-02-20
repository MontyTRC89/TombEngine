#include "framework.h"
#include "Renderer/Renderer11.h"
#include "Game/camera.h"
#include "Game/effects/tomb4fx.h"
#include "Specific/clock.h"
#include "Specific/trmath.h"
#include "Utils.h"
#include "VertexBuffer/VertexBuffer.h"
#include "RenderView/RenderView.h"

using std::vector;

namespace TEN::Renderer
{
	using namespace Utils;
	using std::array;
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
		shadowLight = nullptr;

		ClearSceneItems();

		m_meshPointersToMesh.clear();
		m_moveableObjects.clear();
		m_staticObjects.clear();
		m_sprites.clear();
		m_rooms.clear();
		m_roomTextures.clear();
		m_moveablesTextures.clear();
		m_staticsTextures.clear();
		m_spritesTextures.clear();
		m_animatedTextures.clear();
		m_animatedTextureSets.clear();
	}

	void Renderer11::ClearSceneItems()
	{
		m_lines3DToDraw.clear();
		m_lines2DToDraw.clear();
		gameCamera.clear();
	}

	int Renderer11::SyncRenderer()
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
			                             dest.resolution, dest.resolution, Camera.pos.roomNumber, 10, 20480,
			                             90 * RADIAN);
			RenderSimpleScene(dest.RenderTargetView[i].Get(), dest.DepthStencilView[i].Get(), renderView);
			m_context->ClearState();
		}
	}

	RendererHUDBar::RendererHUDBar(ID3D11Device* m_device, int x, int y, int w, int h, int borderSize,
	                               array<Vector4, 5> colors)
	{
		array<Vector3, 9> barVertices = {
			Vector3(x, HUD_ZERO_Y + y, 0.5),
			Vector3(x + w, HUD_ZERO_Y + y, 0.5),
			Vector3(x + (w / 2.0f), HUD_ZERO_Y + (y + h / 2.0f), 0.5),
			Vector3(x, HUD_ZERO_Y + y + h, 0.5),
			Vector3(x + w, HUD_ZERO_Y + y + h, 0.5),

		};
		const float HUD_BORDER_WIDTH = borderSize * (REFERENCE_RES_WIDTH / REFERENCE_RES_HEIGHT);
		const float HUD_BORDER_HEIGT = borderSize;
		array<Vector3, 16> barBorderVertices = {
			//top left
			Vector3(x - HUD_BORDER_WIDTH, HUD_ZERO_Y + y - HUD_BORDER_HEIGT, 0),
			Vector3(x, HUD_ZERO_Y + y - HUD_BORDER_HEIGT, 0),
			Vector3(x, HUD_ZERO_Y + y, 0),
			Vector3(x - HUD_BORDER_WIDTH, HUD_ZERO_Y + y, 0),
			//top right
			Vector3(x + w, HUD_ZERO_Y + y - HUD_BORDER_HEIGT, 0),
			Vector3(x + w + HUD_BORDER_WIDTH, HUD_ZERO_Y + y - HUD_BORDER_HEIGT, 0),
			Vector3(x + w + HUD_BORDER_WIDTH, HUD_ZERO_Y + y, 0),
			Vector3(x + w, HUD_ZERO_Y + y, 0),
			//bottom right
			Vector3(x + w, HUD_ZERO_Y + y + h, 0),
			Vector3(x + w + HUD_BORDER_WIDTH, HUD_ZERO_Y + y + h, 0),
			Vector3(x + w + HUD_BORDER_WIDTH, HUD_ZERO_Y + y + h + HUD_BORDER_HEIGT, 0),
			Vector3(x + w, HUD_ZERO_Y + y + h + HUD_BORDER_HEIGT, 0),
			//bottom left
			Vector3(x - HUD_BORDER_WIDTH, HUD_ZERO_Y + y + h, 0),
			Vector3(x, HUD_ZERO_Y + y + h, 0),
			Vector3(x, HUD_ZERO_Y + y + h + HUD_BORDER_HEIGT, 0),
			Vector3(x - HUD_BORDER_WIDTH, HUD_ZERO_Y + y + h + HUD_BORDER_HEIGT, 0)
		};

		array<Vector2, 5> barUVs = {
			Vector2(0, 0),
			Vector2(1, 0),
			Vector2(0.5, 0.5),
			Vector2(0, 1),
			Vector2(1, 1),
		};
		array<Vector2, 16> barBorderUVs = {
			//top left
			Vector2(0, 0),
			Vector2(0.25, 0),
			Vector2(0.25, 0.25),
			Vector2(0, 0.25),
			//top right
			Vector2(0.75, 0),
			Vector2(1, 0),
			Vector2(1, 0.25),
			Vector2(0.75, 0.25),
			//bottom right
			Vector2(0.75, 0.75),
			Vector2(1, 0.75),
			Vector2(1, 1),
			Vector2(0.75, 1),
			//bottom left
			Vector2(0, 0.75),
			Vector2(0.25, 0.75),
			Vector2(0.25, 1),
			Vector2(0, 1),
		};

		array<int, 12> barIndices = {
			2, 1, 0,
			2, 0, 3,
			2, 3, 4,
			2, 4, 1
		};
		array<int, 56> barBorderIndices = {
			//top left
			0, 1, 3, 1, 2, 3,
			//top center
			1, 4, 2, 4, 7, 2,
			//top right
			4, 5, 7, 5, 6, 7,
			//right
			7, 6, 8, 6, 9, 8,
			//bottom right
			8, 9, 11, 9, 10, 11,
			//bottom
			13, 8, 14, 8, 11, 14,
			//bottom left
			12, 13, 15, 13, 14, 15,
			//left
			3, 2, 12, 2, 13, 12,
			//center
			2, 7, 13, 7, 8, 13
		};
		array<RendererVertex, 5> vertices;
		for (int i = 0; i < 5; i++)
		{
			vertices[i].Position = barVertices[i];
			vertices[i].Color = colors[i];
			vertices[i].UV = barUVs[i];
			vertices[i].Normal = Vector3(0, 0, 0);
			vertices[i].Bone = 0.0f;
		}

		InnerVertexBuffer = VertexBuffer(m_device, vertices.size(), vertices.data());
		InnerIndexBuffer = IndexBuffer(m_device, barIndices.size(), barIndices.data());

		array<RendererVertex, barBorderVertices.size()> verticesBorder;
		for (int i = 0; i < barBorderVertices.size(); i++)
		{
			verticesBorder[i].Position = barBorderVertices[i];
			verticesBorder[i].Color = Vector4(1, 1, 1, 1);
			verticesBorder[i].UV = barBorderUVs[i];
			verticesBorder[i].Normal = Vector3(0, 0, 0);
			verticesBorder[i].Bone = 0.0f;
		}
		VertexBufferBorder = VertexBuffer(m_device, verticesBorder.size(), verticesBorder.data());
		IndexBufferBorder = IndexBuffer(m_device, barBorderIndices.size(), barBorderIndices.data());
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

	void Renderer11::BindConstantBufferVS(CONSTANT_BUFFERS constantBufferType, ID3D11Buffer** buffer)
	{
		m_context->VSSetConstantBuffers(static_cast<UINT>(constantBufferType), 1, buffer);
	}

	void Renderer11::BindConstantBufferPS(CONSTANT_BUFFERS constantBufferType, ID3D11Buffer** buffer)
	{
		m_context->PSSetConstantBuffers(static_cast<UINT>(constantBufferType), 1, buffer);
	}

	void Renderer11::SetBlendMode(BLEND_MODES blendMode)
	{
		if (blendMode != lastBlendMode)
		{
			switch (blendMode)
			{
			case BLENDMODE_ALPHABLEND:
				m_context->OMSetBlendState(m_states->NonPremultiplied(), NULL, 0xFFFFFFFF);
				SetDepthState(DEPTH_STATE_READ_ONLY_ZBUFFER);
				break;

			case BLENDMODE_ALPHATEST:
				m_context->OMSetBlendState(m_states->Opaque(), NULL, 0xFFFFFFFF);
				SetDepthState(DEPTH_STATE_WRITE_ZBUFFER);
				break;

			case BLENDMODE_OPAQUE:
				m_context->OMSetBlendState(m_states->Opaque(), NULL, 0xFFFFFFFF);
				SetDepthState(DEPTH_STATE_WRITE_ZBUFFER);
				break;

			case BLENDMODE_SUBTRACTIVE:
				m_context->OMSetBlendState(m_subtractiveBlendState.Get(), NULL, 0xFFFFFFFF);
				SetDepthState(DEPTH_STATE_READ_ONLY_ZBUFFER);
				break;

			case BLENDMODE_ADDITIVE:
				m_context->OMSetBlendState(m_states->Additive(), NULL, 0xFFFFFFFF);
				SetDepthState(DEPTH_STATE_READ_ONLY_ZBUFFER);
				break;

			case BLENDMODE_SCREEN:
				m_context->OMSetBlendState(m_screenBlendState.Get(), NULL, 0xFFFFFFFF);
				SetDepthState(DEPTH_STATE_READ_ONLY_ZBUFFER);
				break;

			case BLENDMODE_LIGHTEN:
				m_context->OMSetBlendState(m_lightenBlendState.Get(), NULL, 0xFFFFFFFF);
				SetDepthState(DEPTH_STATE_READ_ONLY_ZBUFFER);
				break;

			case BLENDMODE_EXCLUDE:
				m_context->OMSetBlendState(m_excludeBlendState.Get(), NULL, 0xFFFFFFFF);
				SetDepthState(DEPTH_STATE_READ_ONLY_ZBUFFER);
				break;

			}

			lastBlendMode = blendMode;
		}
	}

	void Renderer11::SetDepthState(DEPTH_STATES depthState)
	{
		if (depthState != lastDepthState)
		{
			switch (depthState)
			{
			case DEPTH_STATE_READ_ONLY_ZBUFFER:
				m_context->OMSetDepthStencilState(m_states->DepthRead(), 0xFFFFFFFF);
				break;

			case DEPTH_STATE_WRITE_ZBUFFER:
				m_context->OMSetDepthStencilState(m_states->DepthDefault(), 0xFFFFFFFF);
				break;

			}

			lastDepthState = depthState;
		}
	}

	void Renderer11::SetCullMode(CULL_MODES cullMode)
	{
		if (cullMode != lastCullMode)
		{
			switch (cullMode)
			{
			case CULL_MODE_NONE:
				m_context->RSSetState(m_states->CullNone());
				break;

			case CULL_MODE_CCW:
				m_context->RSSetState(m_states->CullCounterClockwise());
				break;

			case CULL_MODE_CW:
				m_context->RSSetState(m_states->CullClockwise());
				break;

			}

			lastCullMode = cullMode;
		}
	}
}
