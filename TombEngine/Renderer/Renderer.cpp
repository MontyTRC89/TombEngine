#include "framework.h"
#include "Renderer/Renderer.h"
#include "Game/camera.h"
#include "Game/effects/tomb4fx.h"
#include "Math/Math.h"
#include "Renderer/Structures/RendererRectangle.h"
#include "Renderer/RenderView.h"
#include "Renderer/RendererUtils.h"
#include "Renderer/Graphics/VertexBuffer.h"
#include "Renderer/Structures/RendererHudBar.h"
#include "Scripting/Include/Flow/ScriptInterfaceFlowHandler.h"
#include "Specific/clock.h"
#include "Graphics/RenderTargetCube.h"

namespace TEN::Renderer
{
	using namespace TEN::Renderer::Structures;
	using namespace Utils;
	Renderer g_Renderer;

	Renderer::Renderer() : _gameCamera({0, 0, 0}, {0, 0, 1}, {0, 1, 0}, 1, 1, 0, 1, 10, 90)
	{
	}

	Renderer::~Renderer()
	{
		FreeRendererData();
	}

	void Renderer::FreeRendererData()
	{
		_shadowLight = nullptr;

		ClearSceneItems();

		_moveableObjects.resize(0);
		_staticObjects.resize(0);
		_sprites.resize(0);
		_rooms.resize(0);
		_roomTextures.resize(0);
		_moveablesTextures.resize(0);
		_staticTextures.resize(0);
		_spritesTextures.resize(0);
		_animatedTextures.resize(0);
		_animatedTextureSets.resize(0);

		for (auto& mesh : _meshes)
			delete mesh;
		_meshes.resize(0);

		for (auto& item : _items)
		{
			item.PrevRoomNumber = NO_VALUE;
			item.RoomNumber = NO_VALUE;
			item.ItemNumber = NO_VALUE;
			item.LightsToDraw.clear();
		}
	}

	void Renderer::ClearSceneItems()
	{
		_lines2DToDraw.clear();
		_lines3DToDraw.clear();
		_triangles3DToDraw.clear();
		_gameCamera.Clear();
	}

	void Renderer::Lock()
	{
		_isLocked = true;
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
			//RenderSimpleScene(dest.RenderTargetView[i].Get(), dest.DepthStencilView[i].Get(), renderView);
			_context->ClearState();
		}
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

		_fps = fps;

		return fps;
	}

	void Renderer::BindTexture(TextureRegister registerType, TextureBase* texture, SamplerStateRegister samplerType)
	{
		_context->PSSetShaderResources((UINT)registerType, 1, texture->ShaderResourceView.GetAddressOf());

		if (g_GameFlow->IsPointFilterEnabled() && samplerType != SamplerStateRegister::ShadowMap)
		{
			samplerType = SamplerStateRegister::PointWrap;
		}

		ID3D11SamplerState* samplerState = nullptr;
		switch (samplerType)
		{
		case SamplerStateRegister::AnisotropicClamp:
			samplerState = _renderStates->AnisotropicClamp();
			break;

		case SamplerStateRegister::AnisotropicWrap:
			samplerState = _renderStates->AnisotropicWrap();
			break;

		case SamplerStateRegister::LinearClamp:
			samplerState = _renderStates->LinearClamp();
			break;

		case SamplerStateRegister::LinearWrap:
			samplerState = _renderStates->LinearWrap();
			break;

		case SamplerStateRegister::PointWrap:
			samplerState = _renderStates->PointWrap();
			break;

		case SamplerStateRegister::ShadowMap:
			samplerState = _shadowSampler.Get();
			break;

		default:
			return;
		}

		_context->PSSetSamplers((UINT)registerType, 1, &samplerState);
	}

	void Renderer::BindRenderTargetAsTexture(TextureRegister registerType, RenderTarget2D* target, SamplerStateRegister samplerType)
	{
		_context->PSSetShaderResources((UINT)registerType, 1, target->ShaderResourceView.GetAddressOf());
		  
		ID3D11SamplerState* samplerState = nullptr;
		switch (samplerType)
		{
		case SamplerStateRegister::AnisotropicClamp:
			samplerState = _renderStates->AnisotropicClamp();
			break;

		case SamplerStateRegister::AnisotropicWrap:
			samplerState = _renderStates->AnisotropicWrap();
			break;

		case SamplerStateRegister::LinearClamp:
			samplerState = _renderStates->LinearClamp();
			break;

		case SamplerStateRegister::LinearWrap:
			samplerState = _renderStates->LinearWrap();
			break;

		case SamplerStateRegister::PointWrap:
			samplerState = _renderStates->PointWrap();
			break;

		case SamplerStateRegister::ShadowMap:
			samplerState = _shadowSampler.Get();
			break;

		default:
			return;
		}

		_context->PSSetSamplers((UINT)registerType, 1, &samplerState);
	} 

	void Renderer::BindRoomLights(std::vector<RendererLight*>& lights)
	{
		for (int i = 0; i < lights.size(); i++)
			memcpy(&_stRoom.RoomLights[i], lights[i], sizeof(ShaderLight));
		
		_stRoom.NumRoomLights = (int)lights.size();
	}

	void Renderer::BindStaticLights(std::vector<RendererLight*>& lights)
	{
		for (int i = 0; i < lights.size(); i++)
			memcpy(&_stStatic.Lights[i], lights[i], sizeof(ShaderLight));
		
		_stStatic.NumLights = (int)lights.size();
	}

	void Renderer::BindInstancedStaticLights(std::vector<RendererLight*>& lights, int instanceID)
	{
		for (int i = 0; i < lights.size(); i++)
			memcpy(&_stInstancedStaticMeshBuffer.StaticMeshes[instanceID].Lights[i], lights[i], sizeof(ShaderLight));

		_stInstancedStaticMeshBuffer.StaticMeshes[instanceID].NumLights = (int)lights.size();
	}

	void Renderer::BindMoveableLights(std::vector<RendererLight*>& lights, int roomNumber, int prevRoomNumber, float fade)
	{
		int numLights = 0;
		for (int i = 0; i < lights.size(); i++)
		{
			float fadedCoeff = 1.0f;

			// Interpolate lights which don't affect neighbor rooms
			if (!lights[i]->AffectNeighbourRooms && roomNumber != NO_VALUE && lights[i]->RoomNumber != NO_VALUE)
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

			memcpy(&_stItem.Lights[numLights], lights[i], sizeof(ShaderLight));
			_stItem.Lights[numLights].Intensity *= fadedCoeff;
			numLights++;
		}

		_stItem.NumLights = numLights;
	}

	void Renderer::BindConstantBufferVS(ConstantBufferRegister constantBufferType, ID3D11Buffer** buffer)
	{
		_context->VSSetConstantBuffers(static_cast<UINT>(constantBufferType), 1, buffer);
	}

	void Renderer::BindConstantBufferPS(ConstantBufferRegister constantBufferType, ID3D11Buffer** buffer)
	{
		_context->PSSetConstantBuffers(static_cast<UINT>(constantBufferType), 1, buffer);
	}

	void Renderer::SetBlendMode(BlendMode blendMode, bool force)
	{
		if (blendMode != _lastBlendMode || force)
		{
			switch (blendMode)
			{
			case BlendMode::AlphaBlend:
				_context->OMSetBlendState(_renderStates->NonPremultiplied(), nullptr, 0xFFFFFFFF);
				break;

			case BlendMode::AlphaTest:
				_context->OMSetBlendState(_renderStates->Opaque(), nullptr, 0xFFFFFFFF);
				break;

			case BlendMode::Opaque:
				_context->OMSetBlendState(_renderStates->Opaque(), nullptr, 0xFFFFFFFF);
				break;

			case BlendMode::Subtractive:
				_context->OMSetBlendState(_subtractiveBlendState.Get(), nullptr, 0xFFFFFFFF);
				break;

			case BlendMode::Additive:
				_context->OMSetBlendState(_renderStates->Additive(), nullptr, 0xFFFFFFFF);
				break;

			case BlendMode::Screen:
				_context->OMSetBlendState(_screenBlendState.Get(), nullptr, 0xFFFFFFFF);
				break;

			case BlendMode::Lighten:
				_context->OMSetBlendState(_lightenBlendState.Get(), nullptr, 0xFFFFFFFF);
				break;

			case BlendMode::Exclude:
				_context->OMSetBlendState(_excludeBlendState.Get(), nullptr, 0xFFFFFFFF);
				break;
			}

			_stBlending.BlendMode = static_cast<unsigned int>(blendMode);
			_cbBlending.UpdateData(_stBlending, _context.Get());
			
			_lastBlendMode = blendMode;
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
		if (depthState != _lastDepthState || force)
		{
			switch (depthState)
			{
			case DepthState::Read:
				_context->OMSetDepthStencilState(_renderStates->DepthRead(), 0xFFFFFFFF);
				break;

			case DepthState::Write:
				_context->OMSetDepthStencilState(_renderStates->DepthDefault(), 0xFFFFFFFF);
				break;

			case DepthState::None:
				_context->OMSetDepthStencilState(_renderStates->DepthNone(), 0xFFFFFFFF);
				break;

			}

			_lastDepthState = depthState;
		}
	}

	void Renderer::SetCullMode(CullMode cullMode, bool force)
	{ 
		if (_debugPage == RendererDebugPage::WireframeMode)
		{
			if (!_doingFullscreenPass)
			{
				_context->RSSetState(_renderStates->Wireframe());
				return;
			}
			else
			{
				force = true;
			}
		}

		if (cullMode != _lastCullMode || force)
		{
			switch (cullMode)
			{
			case CullMode::None:
				_context->RSSetState(_cullNoneRasterizerState.Get());
				break;

			case CullMode::CounterClockwise:
				_context->RSSetState(_cullCounterClockwiseRasterizerState.Get());
				break;

			case CullMode::Clockwise:
				_context->RSSetState(_cullClockwiseRasterizerState.Get());
				break;
			}

			_lastCullMode = cullMode;
		}
	}

	void Renderer::SetAlphaTest(AlphaTestMode mode, float threshold, bool force)
	{
		if (_stBlending.AlphaTest != (int)mode ||
			_stBlending.AlphaThreshold != threshold ||
			force)
		{
			_stBlending.AlphaTest = (int)mode;
			_stBlending.AlphaThreshold = threshold;
			_cbBlending.UpdateData(_stBlending, _context.Get());
			BindConstantBufferPS(ConstantBufferRegister::Blending, _cbBlending.get());
		}
	}

	void Renderer::SetScissor(RendererRectangle s)
	{
		D3D11_RECT rects;
		rects.left = s.Left;
		rects.top = s.Top;
		rects.right = s.Right;
		rects.bottom = s.Bottom;

		_context->RSSetScissorRects(1, &rects);
	}

	void Renderer::ResetScissor()
	{
		D3D11_RECT rects[1];
		rects[0].left = 0;
		rects[0].right = _screenWidth;
		rects[0].top = 0;
		rects[0].bottom = _screenHeight;

		_context->RSSetScissorRects(1, rects);
	}
}
