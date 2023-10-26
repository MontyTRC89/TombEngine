#include "framework.h"
#include "Renderer/Renderer.h"

#include "Game/camera.h"
#include "Game/effects/tomb4fx.h"
#include "Math/Math.h"
#include "Renderer/Structures/RendererRectangle.h"
#include "Renderer/RenderView/RenderView.h"
#include "Renderer/Utils.h"
#include "Renderer/Graphics/VertexBuffer.h"
#include "Scripting/Include/Flow/ScriptInterfaceFlowHandler.h"
#include "Specific/clock.h"
#include "Graphics/RenderTargetCube.h"

namespace TEN::Renderer
{
	using namespace Utils;
	Renderer g_Renderer;

	Renderer::Renderer() : gameCamera({0, 0, 0}, {0, 0, 1}, {0, 1, 0}, 1, 1, 0, 1, 10, 90)
	{
	}

	Renderer::~Renderer()
	{
		FreeRendererData();
	}

	void Renderer::FreeRendererData()
	{
		shadowLight = nullptr;

		ClearSceneItems();

		moveableObjects.resize(0);
		staticObjects.resize(0);
		sprites.resize(0);
		rooms.resize(0);
		roomTextures.resize(0);
		moveablesTextures.resize(0);
		staticTextures.resize(0);
		spritesTextures.resize(0);
		animatedTextures.resize(0);
		animatedTextureSets.resize(0);

		for (auto& mesh : meshes)
			delete mesh;
		meshes.resize(0);

		for (auto& item : items)
		{
			item.PrevRoomNumber = NO_ROOM;
			item.RoomNumber = NO_ROOM;
			item.ItemNumber = NO_ITEM;
			item.LightsToDraw.clear();
		}
	}

	void Renderer::ClearSceneItems()
	{
		lines3DToDraw.clear();
		lines2DToDraw.clear();
		gameCamera.Clear();
	}

	void Renderer::Lock()
	{
		isLocked = true;
	}

	int Renderer::Synchronize()
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

	void Renderer::UpdateProgress(float value)
	{
		RenderLoadingScreen(value);
	}

	void Renderer::RenderToCubemap(const RenderTargetCube& dest, const Vector3& pos, int roomNumer)
	{
		for (int i = 0; i < 6; i++)
		{
			auto renderView = RenderView(pos, RenderTargetCube::forwardVectors[i], RenderTargetCube::upVectors[i],
			                             dest.Resolution, dest.Resolution, Camera.pos.RoomNumber, 10, 20480,
			                             90 * RADIAN);
			RenderSimpleScene(dest.RenderTargetView[i].Get(), dest.DepthStencilView[i].Get(), renderView);
			context->ClearState();
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

		auto vertices = std::array<Vertex, VERTEX_COUNT>{};
		for (int i = 0; i < VERTEX_COUNT; i++)
		{
			vertices[i].Position = barVertices[i];
			vertices[i].Color = colors[i];
			vertices[i].UV = barUVs[i];
			vertices[i].Normal = Vector3::Zero;
			vertices[i].Bone = 0.0f;
		}

		InnerVertexBuffer = VertexBuffer(devicePtr, (int)vertices.size(), vertices.data());
		InnerIndexBuffer = IndexBuffer(devicePtr, (int)barIndices.size(), barIndices.data());

		auto borderVertices = std::array<Vertex, barBorderVertices.size()>{};
		for (int i = 0; i < barBorderVertices.size(); i++)
		{
			borderVertices[i].Position = barBorderVertices[i];
			borderVertices[i].Color = Vector4::One;
			borderVertices[i].UV = barBorderUVs[i];
			borderVertices[i].Normal = Vector3::Zero;
			borderVertices[i].Bone = 0.0f;
		}

		VertexBufferBorder = VertexBuffer(devicePtr, (int)borderVertices.size(), borderVertices.data());
		IndexBufferBorder = IndexBuffer(devicePtr, (int)barBorderIndices.size(), barBorderIndices.data());
	}

	float Renderer::CalculateFrameRate()
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
			t = (this_time - last_time) / (double)CLOCKS_PER_SEC;
			last_time = this_time;
			fps = float(count / t);
			count = 0;
		}

		fps = fps;

		return fps;
	}

	void Renderer::BindTexture(TextureRegister registerType, TextureBase* texture, SamplerStateRegister samplerType)
	{
		context->PSSetShaderResources((UINT)registerType, 1, texture->ShaderResourceView.GetAddressOf());

		if (g_GameFlow->IsPointFilterEnabled() && samplerType != SamplerStateRegister::ShadowMap)
		{
			samplerType = SamplerStateRegister::PointWrap;
		}

		ID3D11SamplerState* samplerState = nullptr;
		switch (samplerType)
		{
		case SamplerStateRegister::AnisotropicClamp:
			samplerState = renderStates->AnisotropicClamp();
			break;

		case SamplerStateRegister::AnisotropicWrap:
			samplerState = renderStates->AnisotropicWrap();
			break;

		case SamplerStateRegister::LinearClamp:
			samplerState = renderStates->LinearClamp();
			break;

		case SamplerStateRegister::LinearWrap:
			samplerState = renderStates->LinearWrap();
			break;

		case SamplerStateRegister::PointWrap:
			samplerState = renderStates->PointWrap();
			break;

		case SamplerStateRegister::ShadowMap:
			samplerState = m_shadowSampler.Get();
			break;

		default:
			return;
		}

		context->PSSetSamplers((UINT)registerType, 1, &samplerState);
	}

	void Renderer::BindRenderTargetAsTexture(TextureRegister registerType, RenderTarget2D* target, SamplerStateRegister samplerType)
	{
		context->PSSetShaderResources((UINT)registerType, 1, target->ShaderResourceView.GetAddressOf());

		ID3D11SamplerState* samplerState = nullptr;
		switch (samplerType)
		{
		case SamplerStateRegister::AnisotropicClamp:
			samplerState = renderStates->AnisotropicClamp();
			break;

		case SamplerStateRegister::AnisotropicWrap:
			samplerState = renderStates->AnisotropicWrap();
			break;

		case SamplerStateRegister::LinearClamp:
			samplerState = renderStates->LinearClamp();
			break;

		case SamplerStateRegister::LinearWrap:
			samplerState = renderStates->LinearWrap();
			break;

		case SamplerStateRegister::PointWrap:
			samplerState = renderStates->PointWrap();
			break;

		case SamplerStateRegister::ShadowMap:
			samplerState = m_shadowSampler.Get();
			break;

		default:
			return;
		}

		context->PSSetSamplers((UINT)registerType, 1, &samplerState);
	}

	void Renderer::BindRoomLights(std::vector<RendererLight*>& lights)
	{
		for (int i = 0; i < lights.size(); i++)
			memcpy(&stRoom.RoomLights[i], lights[i], sizeof(ShaderLight));
		
		stRoom.NumRoomLights = (int)lights.size();
	}

	void Renderer::BindStaticLights(std::vector<RendererLight*>& lights)
	{
		for (int i = 0; i < lights.size(); i++)
			memcpy(&stStatic.Lights[i], lights[i], sizeof(ShaderLight));
		
		stStatic.NumLights = (int)lights.size();
	}

	void Renderer::BindInstancedStaticLights(std::vector<RendererLight*>& lights, int instanceID)
	{
		for (int i = 0; i < lights.size(); i++)
			memcpy(&stInstancedStaticMeshBuffer.StaticMeshes[instanceID].Lights[i], lights[i], sizeof(ShaderLight));

		stInstancedStaticMeshBuffer.StaticMeshes[instanceID].NumLights = (int)lights.size();
	}

	void Renderer::BindMoveableLights(std::vector<RendererLight*>& lights, int roomNumber, int prevRoomNumber, float fade)
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

			memcpy(&stItem.Lights[numLights], lights[i], sizeof(ShaderLight));
			stItem.Lights[numLights].Intensity *= fadedCoeff;
			numLights++;
		}

		stItem.NumLights = numLights;
	}

	void Renderer::BindConstantBufferVS(ConstantBufferRegister constantBufferType, ID3D11Buffer** buffer)
	{
		context->VSSetConstantBuffers(static_cast<UINT>(constantBufferType), 1, buffer);
	}

	void Renderer::BindConstantBufferPS(ConstantBufferRegister constantBufferType, ID3D11Buffer** buffer)
	{
		context->PSSetConstantBuffers(static_cast<UINT>(constantBufferType), 1, buffer);
	}

	void Renderer::SetBlendMode(BlendMode blendMode, bool force)
	{
		if (blendMode != lastBlendMode || force)
		{
			switch (blendMode)
			{
			case BlendMode::AlphaBlend:
				context->OMSetBlendState(renderStates->NonPremultiplied(), nullptr, 0xFFFFFFFF);
				break;

			case BlendMode::AlphaTest:
				context->OMSetBlendState(renderStates->Opaque(), nullptr, 0xFFFFFFFF);
				break;

			case BlendMode::Opaque:
				context->OMSetBlendState(renderStates->Opaque(), nullptr, 0xFFFFFFFF);
				break;

			case BlendMode::Subtractive:
				context->OMSetBlendState(subtractiveBlendState.Get(), nullptr, 0xFFFFFFFF);
				break;

			case BlendMode::Additive:
				context->OMSetBlendState(renderStates->Additive(), nullptr, 0xFFFFFFFF);
				break;

			case BlendMode::Screen:
				context->OMSetBlendState(screenBlendState.Get(), nullptr, 0xFFFFFFFF);
				break;

			case BlendMode::Lighten:
				context->OMSetBlendState(lightenBlendState.Get(), nullptr, 0xFFFFFFFF);
				break;

			case BlendMode::Exclude:
				context->OMSetBlendState(excludeBlendState.Get(), nullptr, 0xFFFFFFFF);
				break;
			}

			stBlending.BlendMode = static_cast<unsigned int>(blendMode);
			cbBlending.updateData(stBlending, context.Get());
			BindConstantBufferPS(ConstantBufferRegister::Blending, cbBlending.get());

			lastBlendMode = blendMode;
		}

		switch (blendMode)
		{
		case BlendMode::Opaque:
		case BlendMode::AlphaTest:
			SetDepthState(DepthState::Write);
			break;

		default:
			SetDepthState(DepthState::Read);
			break;
		}
	}

	void Renderer::SetDepthState(DepthState depthState, bool force)
	{
		if (depthState != lastDepthState || force)
		{
			switch (depthState)
			{
			case DepthState::Read:
				context->OMSetDepthStencilState(renderStates->DepthRead(), 0xFFFFFFFF);
				break;

			case DepthState::Write:
				context->OMSetDepthStencilState(renderStates->DepthDefault(), 0xFFFFFFFF);
				break;

			case DepthState::None:
				context->OMSetDepthStencilState(renderStates->DepthNone(), 0xFFFFFFFF);
				break;

			}

			lastDepthState = depthState;
		}
	}

	void Renderer::SetCullMode(CullMode cullMode, bool force)
	{
		if (DebugPage == RendererDebugPage::WireframeMode)
		{
			context->RSSetState(renderStates->Wireframe());
			return;
		}

		if (cullMode != lastCullMode || force)
		{
			switch (cullMode)
			{
			case CullMode::None:
				context->RSSetState(cullNoneRasterizerState.Get());
				break;

			case CullMode::CounterClockwise:
				context->RSSetState(cullCounterClockwiseRasterizerState.Get());
				break;

			case CullMode::Clockwise:
				context->RSSetState(cullClockwiseRasterizerState.Get());
				break;
			}

			lastCullMode = cullMode;
		}
	}

	void Renderer::SetAlphaTest(AlphaTestModes mode, float threshold, bool force)
	{
		if (stBlending.AlphaTest != (int)mode ||
			stBlending.AlphaThreshold != threshold ||
			force)
		{
			stBlending.AlphaTest = (int)mode;
			stBlending.AlphaThreshold = threshold;
			cbBlending.updateData(stBlending, context.Get());
			BindConstantBufferPS(ConstantBufferRegister::Blending, cbBlending.get());
		}
	}

	void Renderer::SetScissor(RendererRectangle s)
	{
		D3D11_RECT rects;
		rects.left = s.Left;
		rects.top = s.Top;
		rects.right = s.Right;
		rects.bottom = s.Bottom;

		context->RSSetScissorRects(1, &rects);
	}

	void Renderer::ResetScissor()
	{
		D3D11_RECT rects[1];
		rects[0].left = 0;
		rects[0].right = screenWidth;
		rects[0].top = 0;
		rects[0].bottom = screenHeight;

		context->RSSetScissorRects(1, rects);
	}
}
