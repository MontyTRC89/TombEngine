#include "framework.h"
#include "Renderer/Renderer11.h"
#include "Game/camera.h"
#include "Game/spotcam.h"
#include "Specific/setup.h"
#include "Game/control/control.h"
#include "Game/effects/weather.h"

using namespace TEN::Effects::Environment;

TEN::Renderer::RendererHUDBar* g_HealthBar;
TEN::Renderer::RendererHUDBar* g_AirBar;
TEN::Renderer::RendererHUDBar* g_DashBar;
TEN::Renderer::RendererHUDBar* g_MusicVolumeBar;
TEN::Renderer::RendererHUDBar* g_SFXVolumeBar;
TEN::Renderer::RendererHUDBar* g_LoadingBar;

namespace TEN::Renderer
{
	void Renderer11::InitialiseBars()
	{
		std::array<Vector4, 5> healthColors = 
		{
			//top
			Vector4(82 / 255.0f,0,0,1),
			Vector4(0,82 / 255.0f,0,1),
			//center
			Vector4(78 / 255.0f,81 / 255.0f,0,1),
			//bottom
			Vector4(82 / 255.0f,0,0,1),
			Vector4(0,82 / 255.0f,0,1),
		};

		std::array<Vector4, 5> airColors = 
		{
			//top
			Vector4(0 ,0,90 / 255.0f,1),
			Vector4(0 ,47 / 255.0f,96 / 255.0f,1),
			//center
			Vector4(0,39 / 255,155 / 255.0f,1),
			//bottom
			Vector4(0 ,0,90 / 255.0f,1),
			Vector4(0 ,47 / 255.0f,96 / 255.0f,1),
		};

		std::array<Vector4, 5> loadingColors = 
		{
			//top
			Vector4(0 ,0,90 / 255.0f,1),
			Vector4(0 ,47 / 255.0f,96 / 255.0f,1),
			//center
			Vector4(0,39 / 255,155 / 255.0f,1),
			//bottom
			Vector4(0 ,0,90 / 255.0f,1),
			Vector4(0 ,47 / 255.0f,96 / 255.0f,1),
		};

		std::array<Vector4, 5> dashColors =
		{
			//top
			Vector4(78 / 255.0f,4 / 255.0f,0,1),
			Vector4(136 / 255.0f,117 / 255.0f,5 / 255.0f,1),
			//center
			Vector4(245 / 255.0f,119 / 255,24 / 255.0f,1),
			//bottom
			Vector4(78 / 255.0f,4 / 255.0f,0,1),
			Vector4(136 / 255.0f,117 / 255.0f,5 / 255.0f,1),
		};
		std::array<Vector4, 5> soundSettingColors = 
		{
			//top
			Vector4(0.18f,0.3f,0.72f,1),
			Vector4(0.18f,0.3f,0.72f,1),
			//center
			Vector4(0.18f,0.3f,0.72f,1),
			//bottom
			Vector4(0.18f,0.3f,0.72f,1),
			Vector4(0.18f,0.3f,0.72f,1),
		};

		g_HealthBar = new RendererHUDBar(m_device.Get(), 20, 32, 150, 8, 1, healthColors);
		g_AirBar = new RendererHUDBar(m_device.Get(), 630, 32, 150, 8, 1, airColors);
		g_DashBar = new RendererHUDBar(m_device.Get(), 630, 32 + 8 + 4, 150, 8, 1, dashColors);
		g_MusicVolumeBar = new RendererHUDBar(m_device.Get(), 400, 194, 150, 8, 1, soundSettingColors);
		g_SFXVolumeBar = new RendererHUDBar(m_device.Get(), 400, 212, 150, 8, 1, soundSettingColors);
		g_LoadingBar = new RendererHUDBar(m_device.Get(), 325, 400, 150, 8, 1, airColors);
	}

	void Renderer11::DrawBar(float percent, const RendererHUDBar* const bar, GAME_OBJECT_ID textureSlot, int frame, bool poison)
	{
		UINT strides = sizeof(RendererVertex);
		UINT offset = 0;
		float color[] = { 0,0,0,1.0f };
	
		m_context->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 0.0f, 0xFF);
		
		m_context->IASetInputLayout(m_inputLayout.Get());
		m_context->IASetVertexBuffers(0, 1, bar->VertexBufferBorder.Buffer.GetAddressOf(), &strides, &offset);
		m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_context->IASetIndexBuffer(bar->IndexBufferBorder.Buffer.Get(), DXGI_FORMAT_R32_UINT, 0);
		
		m_context->VSSetShader(m_vsHUD.Get(), NULL, 0);
		m_context->PSSetShader(m_psHUDTexture.Get(), NULL, 0);

		SetBlendMode(BLENDMODE_OPAQUE);
		SetDepthState(DEPTH_STATE_NONE);
		SetCullMode(CULL_MODE_NONE);

		BindConstantBufferVS(CB_HUD, m_cbHUD.get());
		BindTexture(TEXTURE_HUD, m_sprites[Objects[ID_BAR_BORDER_GRAPHIC].meshIndex].Texture, SAMPLER_LINEAR_CLAMP);

		DrawIndexedTriangles(56, 0, 0);

		m_context->PSSetShaderResources(0, 1, m_sprites[Objects[textureSlot].meshIndex].Texture->ShaderResourceView.GetAddressOf());

		m_context->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 0.0f, 0xFF);
		
		m_context->IASetInputLayout(m_inputLayout.Get());
		m_context->IASetVertexBuffers(0, 1, bar->InnerVertexBuffer.Buffer.GetAddressOf(), &strides, &offset);
		m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_context->IASetIndexBuffer(bar->InnerIndexBuffer.Buffer.Get(), DXGI_FORMAT_R32_UINT, 0);
		
		m_context->VSSetShader(m_vsHUD.Get(), NULL, 0);
		m_context->PSSetShader(m_psHUDBarColor.Get(), NULL, 0);

		m_stHUDBar.Percent = percent;
		m_stHUDBar.Poisoned = poison;
		m_stHUDBar.Frame = frame;
		m_cbHUDBar.updateData(m_stHUDBar, m_context.Get());
		BindConstantBufferVS(CB_HUD_BAR, m_cbHUDBar.get());
		BindConstantBufferPS(CB_HUD_BAR, m_cbHUDBar.get());

		BindTexture(TEXTURE_HUD, m_sprites[Objects[textureSlot].meshIndex].Texture, SAMPLER_LINEAR_CLAMP);

		DrawIndexedTriangles(12, 0, 0);
	}

	void Renderer11::DrawLoadingBar(float percentage)
	{
		UINT strides = sizeof(RendererVertex);
		UINT offset = 0;
		float color[] = { 0,0,0,1.0f };
		
		m_context->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 0.0f, 0xFF);
	
		m_context->IASetInputLayout(m_inputLayout.Get());
		m_context->IASetVertexBuffers(0, 1, g_LoadingBar->VertexBufferBorder.Buffer.GetAddressOf(), &strides, &offset);
		m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_context->IASetIndexBuffer(g_LoadingBar->IndexBufferBorder.Buffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	
		m_context->VSSetShader(m_vsHUD.Get(), NULL, 0);
		m_context->PSSetShader(m_psHUDTexture.Get(), NULL, 0);

		SetBlendMode(BLENDMODE_OPAQUE);
		SetDepthState(DEPTH_STATE_NONE);
		SetCullMode(CULL_MODE_NONE);

		BindConstantBufferVS(CB_HUD, m_cbHUD.get());
		BindTexture(TEXTURE_HUD, &loadingBarBorder, SAMPLER_LINEAR_CLAMP);

		DrawIndexedTriangles(56, 0, 0);

		m_context->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 0.0f, 0xFF);
		
		m_context->IASetInputLayout(m_inputLayout.Get());
		m_context->IASetVertexBuffers(0, 1, g_LoadingBar->InnerVertexBuffer.Buffer.GetAddressOf(), &strides, &offset);
		m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_context->IASetIndexBuffer(g_LoadingBar->InnerIndexBuffer.Buffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	
		m_context->VSSetShader(m_vsHUD.Get(), NULL, 0);
		m_context->PSSetShader(m_psHUDBarColor.Get(), NULL, 0);
		
		m_stHUDBar.Percent = percentage / 100.0f;
		m_stHUDBar.Poisoned = false;
		m_stHUDBar.Frame = 0;
		m_cbHUDBar.updateData(m_stHUDBar, m_context.Get());
		BindConstantBufferVS(CB_HUD_BAR, m_cbHUDBar.get());
		BindConstantBufferPS(CB_HUD_BAR, m_cbHUDBar.get());

		BindTexture(TEXTURE_HUD, &loadingBarInner, SAMPLER_LINEAR_CLAMP);

		DrawIndexedTriangles(12, 0, 0);
	}

	void Renderer11::AddLine2D(int x1, int y1, int x2, int y2, byte r, byte g, byte b, byte a) {
		RendererLine2D line;

		line.Vertices[0] = Vector2(x1, y1);
		line.Vertices[1] = Vector2(x2, y2);
		line.Color = Vector4(r, g, b, a);

		m_lines2DToDraw.push_back(line);
	}

	void Renderer11::DrawOverlays(RenderView& view)
	{
		auto flashColor = Weather.FlashColor();
		if (flashColor != Vector3::Zero)
		{
			SetBlendMode(BLENDMODE_ADDITIVE);
			DrawFullScreenQuad(m_whiteTexture.ShaderResourceView.Get(), flashColor);
		}

		if (CurrentLevel == 0)
			return;

		if (!BinocularRange && !SpotcamOverlay)
			return;

		SetBlendMode(BLENDMODE_ALPHABLEND);

		if (BinocularRange && !LaserSight)
		{
			DrawFullScreenQuad(m_sprites[Objects[ID_BINOCULAR_GRAPHIC].meshIndex].Texture->ShaderResourceView.Get(), Vector3::One);
		}
		else if (BinocularRange && LaserSight)
		{
			DrawFullScreenQuad(m_sprites[Objects[ID_LASER_SIGHT_GRAPHIC].meshIndex].Texture->ShaderResourceView.Get(), Vector3::One);

			SetBlendMode(BLENDMODE_OPAQUE);

			// Draw the aiming point
			RendererVertex vertices[4];

			vertices[0].Position.x = -4.0f / ScreenWidth;
			vertices[0].Position.y = 4.0f / ScreenHeight;
			vertices[0].Position.z = 0.0f;
			vertices[0].UV.x = 0.0f;
			vertices[0].UV.y = 0.0f;
			vertices[0].Color = Vector4(1.0f, 0.0f, 0.0f, 1.0f);

			vertices[1].Position.x = 4.0f / ScreenWidth;
			vertices[1].Position.y = 4.0f / ScreenHeight;
			vertices[1].Position.z = 0.0f;
			vertices[1].UV.x = 1.0f;
			vertices[1].UV.y = 0.0f;
			vertices[1].Color = Vector4(1.0f, 0.0f, 0.0f, 1.0f);

			vertices[2].Position.x = 4.0f / ScreenWidth;
			vertices[2].Position.y = -4.0f / ScreenHeight;
			vertices[2].Position.z = 0.0f;
			vertices[2].UV.x = 1.0f;
			vertices[2].UV.y = 1.0f;
			vertices[2].Color = Vector4(1.0f, 0.0f, 0.0f, 1.0f);

			vertices[3].Position.x = -4.0f / ScreenWidth;
			vertices[3].Position.y = -4.0f / ScreenHeight;
			vertices[3].Position.z = 0.0f;
			vertices[3].UV.x = 0.0f;
			vertices[3].UV.y = 1.0f;
			vertices[3].Color = Vector4(1.0f, 0.0f, 0.0f, 1.0f);

			m_context->VSSetShader(m_vsFullScreenQuad.Get(), NULL, 0);
			m_context->PSSetShader(m_psFullScreenQuad.Get(), NULL, 0);

			m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			m_context->IASetInputLayout(m_inputLayout.Get());

			m_primitiveBatch->Begin();
			m_primitiveBatch->DrawQuad(vertices[0], vertices[1], vertices[2], vertices[3]);
			m_primitiveBatch->End();
		}
		else
		{
			// TODO: Vignette goes here! -- Lwmte, 21.08.21
		}
	}
}
