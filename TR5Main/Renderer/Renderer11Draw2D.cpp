#include "Renderer11.h"

RendererHUDBar* g_HealthBar;
RendererHUDBar* g_AirBar;
RendererHUDBar* g_DashBar;

bool Renderer11::initialiseBars()
{
	array<Vector4, 9> healthColors = {
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
	g_HealthBar = new RendererHUDBar(m_device, 0, 0, 800, 600, 3, healthColors);
	return true;
}
bool Renderer11::DrawBar(float percent,const RendererHUDBar* const bar)
{
	UINT strides = 0;
	UINT offset = 0;
	m_context->ClearDepthStencilView(m_currentRenderTarget->DepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);	
	m_context->IASetInputLayout(m_inputLayout);
	m_context->IASetVertexBuffers(0, 1, &bar->vertexBuffer->Buffer, &strides, &offset);
	m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_context->IASetIndexBuffer(bar->indexBuffer->Buffer, DXGI_FORMAT_R32_SINT, 0);
	m_context->VSSetConstantBuffers(0, 1, &m_cbHUD);
	m_context->VSSetShader(m_vsHUD,NULL,0);
	m_context->PSSetShader(m_psHUD, NULL,0);
	m_context->OMSetBlendState(m_states->Opaque(), NULL,0xFFFFFFFF);
	m_context->OMSetDepthStencilState(m_states->DepthNone(),0);
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
	drawFullScreenQuad(m_binocularsTexture->ShaderResourceView, Vector3::One, false);
	m_context->OMSetBlendState(m_states->Opaque(), NULL, 0xFFFFFFFF);

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
	m_spriteBatch->Draw(m_whiteTexture->ShaderResourceView, rect, color);
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
