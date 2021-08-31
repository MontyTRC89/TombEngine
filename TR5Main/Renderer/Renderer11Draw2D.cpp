#include "framework.h"
#include "Renderer11.h"
#include "camera.h"
#include "spotcam.h"
#include "lara.h"
#include "control.h"

TEN::Renderer::RendererHUDBar* g_HealthBar;
TEN::Renderer::RendererHUDBar* g_AirBar;
TEN::Renderer::RendererHUDBar* g_DashBar;
TEN::Renderer::RendererHUDBar* g_MusicVolumeBar;
TEN::Renderer::RendererHUDBar* g_SFXVolumeBar;

namespace TEN::Renderer {

	void Renderer11::initialiseBars()
	{
		std::array<Vector4, 5> healthColors = {
			//top
			Vector4(82 / 255.0f,0,0,1),
			Vector4(0,82 / 255.0f,0,1),
			//center
			Vector4(78 / 255.0f,81 / 255.0f,0,1),
			//bottom
			Vector4(82 / 255.0f,0,0,1),
			Vector4(0,82 / 255.0f,0,1),
		};

		std::array<Vector4, 5> airColors = {
			//top
			Vector4(0 ,0,90 / 255.0f,1),
			Vector4(0 ,47 / 255.0f,96 / 255.0f,1),
			//center
			Vector4(0,39 / 255,155 / 255.0f,1),
			//bottom
			Vector4(0 ,0,90 / 255.0f,1),
			Vector4(0 ,47 / 255.0f,96 / 255.0f,1),
		};

		std::array<Vector4, 5> dashColors = {
			//top
			Vector4(78 / 255.0f,4 / 255.0f,0,1),
			Vector4(136 / 255.0f,117 / 255.0f,5 / 255.0f,1),
			//center
			Vector4(245 / 255.0f,119 / 255,24 / 255.0f,1),
			//bottom
			Vector4(78 / 255.0f,4 / 255.0f,0,1),
			Vector4(136 / 255.0f,117 / 255.0f,5 / 255.0f,1),
		};
		std::array<Vector4, 5> soundSettingColors = {
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
	}
	void Renderer11::drawBar(float percent, const RendererHUDBar* const bar,GAME_OBJECT_ID textureSlot,int frame, bool poison) {
		UINT strides = sizeof(RendererVertex);
		UINT offset = 0;
		float color[] = { 0,0,0,1.0f };
		m_context->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 0.0f, 0xFF);
		m_context->IASetInputLayout(m_inputLayout.Get());
		m_context->IASetVertexBuffers(0, 1, bar->vertexBufferBorder.Buffer.GetAddressOf(), &strides, &offset);
		m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_context->IASetIndexBuffer(bar->indexBufferBorder.Buffer.Get(), DXGI_FORMAT_R32_UINT, 0);
		m_context->VSSetConstantBuffers(0, 1, m_cbHUD.get());
		m_context->VSSetShader(m_vsHUD.Get(), NULL, 0);
		m_context->PSSetShaderResources(0, 1, m_sprites[Objects[ID_BAR_BORDER_GRAPHIC].meshIndex].Texture->ShaderResourceView.GetAddressOf());
		ID3D11SamplerState* sampler = m_states->LinearClamp();
		m_context->PSSetSamplers(0, 1, &sampler);
		m_context->PSSetShader(m_psHUDTexture.Get(), NULL, 0);
		m_context->OMSetBlendState(m_states->Opaque(), NULL, 0xFFFFFFFF);
		m_context->OMSetDepthStencilState(m_states->DepthNone(), NULL);
		m_context->RSSetState(m_states->CullNone());
		m_context->DrawIndexed(56, 0, 0);
		m_context->PSSetShaderResources(0, 1, m_sprites[Objects[textureSlot].meshIndex].Texture->ShaderResourceView.GetAddressOf());


		m_context->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 0.0f, 0xFF);
		m_context->IASetInputLayout(m_inputLayout.Get());
		m_context->IASetVertexBuffers(0, 1, bar->vertexBuffer.Buffer.GetAddressOf(), &strides, &offset);
		m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_context->IASetIndexBuffer(bar->indexBuffer.Buffer.Get(), DXGI_FORMAT_R32_UINT, 0);
		m_stHUDBar.Percent = percent;
		m_stHUDBar.Poisoned = poison;
		m_stHUDBar.Frame = frame;
		m_cbHUDBar.updateData(m_stHUDBar, m_context.Get());
		m_context->VSSetConstantBuffers(0, 1, m_cbHUD.get());
		m_context->PSSetConstantBuffers(0, 1, m_cbHUDBar.get());
		m_context->VSSetShader(m_vsHUD.Get(), NULL, 0);
		m_context->PSSetShader(m_psHUDBarColor.Get(), NULL, 0);
		m_context->OMSetBlendState(m_states->Opaque(), NULL, 0xFFFFFFFF);
		m_context->OMSetDepthStencilState(m_states->DepthNone(), NULL);
		m_context->RSSetState(m_states->CullNone());
		m_context->DrawIndexed(12, 0, 0);
	}

	void Renderer11::addLine2D(int x1, int y1, int x2, int y2, byte r, byte g, byte b, byte a) {
		RendererLine2D* line = &m_lines2DBuffer[m_nextLine2D++];

		line->Vertices[0] = Vector2(x1, y1);
		line->Vertices[1] = Vector2(x2, y2);
		line->Color = Vector4(r, g, b, a);

		m_lines2DToDraw.push_back(line);
	}

	void Renderer11::drawOverlays(RenderView& view)
	{
		if (CurrentLevel == 0)
			return;

		if (!BinocularRange && !SpotcamOverlay)
			return;

		m_context->OMSetBlendState(m_states->AlphaBlend(), NULL, 0xFFFFFFFF);

		if (BinocularRange && !LaserSight)
		{
			drawFullScreenQuad(m_sprites[Objects[ID_BINOCULAR_GRAPHIC].meshIndex].Texture->ShaderResourceView.Get(), Vector3::One, false);
		}
		else if (BinocularRange && LaserSight)
		{
			drawFullScreenQuad(m_sprites[Objects[ID_LASER_SIGHT_GRAPHIC].meshIndex].Texture->ShaderResourceView.Get(), Vector3::One, false);

			m_context->OMSetBlendState(m_states->Opaque(), NULL, 0xFFFFFFFF);

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

	void Renderer11::drawColoredQuad(int x, int y, int w, int h, DirectX::SimpleMath::Vector4 color) {
		float factorW = ScreenWidth / 800.0f;
		float factorH = ScreenHeight / 600.0f;

		RECT rect;
		rect.top = y * factorH;
		rect.left = x * factorW;
		rect.bottom = (y + h) * factorH;
		rect.right = (x + w) * factorW;

		m_spriteBatch->Begin(SpriteSortMode_BackToFront, m_states->AlphaBlend(), NULL, m_states->DepthRead());
		m_spriteBatch->Draw(m_whiteTexture.ShaderResourceView.Get(), rect, color);
		m_spriteBatch->End();

		int shiftW = 4 * factorW;
		int shiftH = 4 * factorH;

		addLine2D(rect.left + shiftW, rect.top + shiftH, rect.right - shiftW, rect.top + shiftH, 128, 128, 128, 128);
		addLine2D(rect.right - shiftW, rect.top + shiftH, rect.right - shiftW, rect.bottom - shiftH, 128, 128, 128, 128);
		addLine2D(rect.left + shiftW, rect.bottom - shiftH, rect.right - shiftW, rect.bottom - shiftH, 128, 128, 128, 128);
		addLine2D(rect.left + shiftW, rect.top + shiftH, rect.left + shiftW, rect.bottom - shiftH, 128, 128, 128, 128);

		m_context->OMSetDepthStencilState(m_states->DepthDefault(), 0);

	}

}
