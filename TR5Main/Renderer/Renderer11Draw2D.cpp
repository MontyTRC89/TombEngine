#include "framework.h"
#include "Renderer11.h"
#include "camera.h"
#include "spotcam.h"

RendererHUDBar* g_HealthBar;
RendererHUDBar* g_AirBar;
RendererHUDBar* g_DashBar;
RendererHUDBar* g_MusicVolumeBar;
RendererHUDBar* g_SFXVolumeBar;

bool Renderer11::initialiseBars()
{
	std::array<Vector4, 9> healthColors = {
		//top
		Vector4(82 / 255.0f,0,0,1),
		Vector4(36 / 255.0f,46 / 255.0f,0,1),
		Vector4(0,82 / 255.0f,0,1),
		//center
		Vector4(159 / 255.0f,0,0,1),
		Vector4(78 / 255.0f,81 / 255.0f,0,1),
		Vector4(0,158 / 255.0f,0,1),
		//bottom
		Vector4(82 / 255.0f,0,0,1),
		Vector4(36 / 255.0f,46 / 255.0f,0,1),
		Vector4(0,82 / 255.0f,0,1),
	};

	std::array<Vector4, 9> airColors = {
		//top
		Vector4(0 ,0,90 / 255.0f,1),
		Vector4(0 / 255.0f,28 / 255.0f,84 / 255.0f,1),
		Vector4(0 ,47 / 255.0f,96/255.0f,1),
		//center
		Vector4(0,3 / 255,153 / 255.0f,1),
		Vector4(0,39 / 255,155 / 255.0f,1),
		Vector4(0,78 / 255.0f,159/255.0f,1),
		//bottom
		Vector4(0 ,0,90 / 255.0f,1),
		Vector4(0 / 255.0f,28 / 255.0f,84 / 255.0f,1),
		Vector4(0 ,47 / 255.0f,96 / 255.0f,1),
	};

	std::array<Vector4, 9> dashColors = {
		//top
		Vector4(78 / 255.0f,4 / 255.0f,0,1),
		Vector4(161 / 255.0f,25 / 255.0f,84 / 255.0f,1),
		Vector4(136 / 255.0f,117 / 255.0f,5 / 255.0f,1),
		//center
		Vector4(211 / 255.0f,29 / 255.0f,23 / 255.0f,1),
		Vector4(245 / 255.0f,119 / 255,24 / 255.0f,1),
		Vector4(207 / 255.0f,183 / 255.0f,27 / 255.0f,1),
		//bottom
		Vector4(78 / 255.0f,4 / 255.0f,0,1),
		Vector4(161 / 255.0f,25 / 255.0f,84 / 255.0f,1),
		Vector4(136 / 255.0f,117 / 255.0f,5 / 255.0f,1),
	};
	std::array<Vector4, 9> soundSettingColors = {
		//top
		Vector4(0.18f,0.3f,0.72f,1),
		Vector4(0.18f,0.3f,0.72f,1),
		Vector4(0.18f,0.3f,0.72f,1),
		//center
		Vector4(0.18f,0.3f,0.72f,1),
		Vector4(0.18f,0.3f,0.72f,1),
		Vector4(0.18f,0.3f,0.72f,1),
		//bottom
		Vector4(0.18f,0.3f,0.72f,1),
		Vector4(0.18f,0.3f,0.72f,1),
		Vector4(0.18f,0.3f,0.72f,1),
	};
	g_HealthBar = new RendererHUDBar(m_device, 20, 32, 150, 8, 1, healthColors);
	g_AirBar = new RendererHUDBar(m_device, 630, 32, 150, 8, 1, airColors);
	g_DashBar = new RendererHUDBar(m_device, 630, 32 + 8 + 4, 150, 8, 1, dashColors);
	g_MusicVolumeBar = new RendererHUDBar(m_device, 400, 212, 150, 8, 1, soundSettingColors);
	g_SFXVolumeBar = new RendererHUDBar(m_device, 400, 230, 150, 8, 1, soundSettingColors);
	return true;
}
bool Renderer11::DrawBar(float percent,const RendererHUDBar* const bar)
{
	UINT strides = sizeof(RendererVertex);
	UINT offset = 0;
	float color[] = { 0,0,0,1.0f };
	m_context->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 0.0f, 0xFF);
	m_context->IASetInputLayout(m_inputLayout);
	m_context->IASetVertexBuffers(0, 1, bar->vertexBufferBorder.Buffer.GetAddressOf(), &strides, &offset);
	m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_context->IASetIndexBuffer(bar->indexBufferBorder.Buffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	m_context->VSSetConstantBuffers(0, 1, &m_cbHUD);
	m_context->VSSetShader(m_vsHUD, NULL, 0);
	m_context->PSSetShaderResources(0, 1, m_HUDBarBorderTexture.ShaderResourceView.GetAddressOf());
	ID3D11SamplerState* sampler = m_states->LinearClamp();
	m_context->PSSetSamplers(0, 1, &sampler);
	m_context->PSSetShader(m_psHUDTexture, NULL, 0);
	m_context->OMSetBlendState(m_states->Opaque(), NULL, 0xFFFFFFFF);
	m_context->OMSetDepthStencilState(m_states->DepthNone(), NULL);
	m_context->RSSetState(m_states->CullNone());
	m_context->DrawIndexed(56, 0, 0);

	
	m_context->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 0.0f, 0xFF);
	m_context->IASetInputLayout(m_inputLayout);
	m_context->IASetVertexBuffers(0, 1, bar->vertexBuffer.Buffer.GetAddressOf(), &strides, &offset);
	m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_context->IASetIndexBuffer(bar->indexBuffer.Buffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	m_stHUDBar.Percent = percent;
	updateConstantBuffer(m_cbHUDBar, &m_stHUDBar, sizeof(CHUDBarBuffer));
	m_context->VSSetConstantBuffers(0, 1, &m_cbHUD);
	m_context->PSSetConstantBuffers(0, 1, &m_cbHUDBar);
	m_context->VSSetShader(m_vsHUD,NULL,0);
	m_context->PSSetShader(m_psHUDBarColor, NULL,0);
	m_context->OMSetBlendState(m_states->Opaque(), NULL,0xFFFFFFFF);
	m_context->OMSetDepthStencilState(m_states->DepthNone(),NULL);
	m_context->RSSetState(m_states->CullNone());
	m_context->DrawIndexed(24, 0, 0);
	

	return true;
}

void Renderer11::AddLine2D(int x1, int y1, int x2, int y2, byte r, byte g, byte b, byte a)
{
	RendererLine2D* line = &m_lines2DBuffer[m_nextLine2D++];

	line->Vertices[0] = Vector2(x1, y1);
	line->Vertices[1] = Vector2(x2, y2);
	line->Color = Vector4(r, g, b, a);

	m_lines2DToDraw.push_back(line);
}

bool Renderer11::drawOverlays()
{
	if (!BinocularRange && !SpotcamOverlay)
		return true;

	m_context->OMSetBlendState(m_states->AlphaBlend(), NULL, 0xFFFFFFFF);
	drawFullScreenQuad(m_binocularsTexture.ShaderResourceView.Get(), Vector3::One, false);
	m_context->OMSetBlendState(m_states->Opaque(), NULL, 0xFFFFFFFF);

	if (LaserSight)
	{
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

		m_context->VSSetShader(m_vsFullScreenQuad, NULL, 0);
		m_context->PSSetShader(m_psFullScreenQuad, NULL, 0);

		m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_context->IASetInputLayout(m_inputLayout);

		m_primitiveBatch->Begin();
		m_primitiveBatch->DrawQuad(vertices[0], vertices[1], vertices[2], vertices[3]);
		m_primitiveBatch->End();
	}

	return true;
}

bool Renderer11::drawColoredQuad(int x, int y, int w, int h, Vector4 color)
{
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

	AddLine2D(rect.left + shiftW, rect.top + shiftH, rect.right - shiftW, rect.top + shiftH, 128, 128, 128, 128);
	AddLine2D(rect.right - shiftW, rect.top + shiftH, rect.right - shiftW, rect.bottom - shiftH, 128, 128, 128, 128);
	AddLine2D(rect.left + shiftW, rect.bottom - shiftH, rect.right - shiftW, rect.bottom - shiftH, 128, 128, 128, 128);
	AddLine2D(rect.left + shiftW, rect.top + shiftH, rect.left + shiftW, rect.bottom - shiftH, 128, 128, 128, 128);

	m_context->OMSetDepthStencilState(m_states->DepthDefault(), 0);

	return true;
}
