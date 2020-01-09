#include "Renderer11.h"

bool Renderer11::DrawBar(int x, int y, int w, int h, int percent, int color1, int color2)
{
	byte r1 = (color1 >> 16) & 0xFF;
	byte g1 = (color1 >> 8) & 0xFF;
	byte b1 = (color1 >> 0) & 0xFF;

	byte r2 = (color2 >> 16) & 0xFF;
	byte g2 = (color2 >> 8) & 0xFF;
	byte b2 = (color2 >> 0) & 0xFF;

	float factorX = ScreenWidth / 800.0f;
	float factorY = ScreenHeight / 600.0f;

	int realX = x * factorX;
	int realY = y * factorY;
	int realW = w * factorX;
	int realH = h * factorY;

	int realPercent = percent / 100.0f * realW;

	for (int i = 0; i < realH; i++)
		AddLine2D(realX, realY + i, realX + realW, realY + i, 0, 0, 0, 255);

	for (int i = 0; i < realH; i++)
		AddLine2D(realX, realY + i, realX + realPercent, realY + i, r1, g1, b1, 255);

	AddLine2D(realX, realY, realX + realW, realY, 255, 255, 255, 255);
	AddLine2D(realX, realY + realH, realX + realW, realY + realH, 255, 255, 255, 255);
	AddLine2D(realX, realY, realX, realY + realH, 255, 255, 255, 255);
	AddLine2D(realX + realW, realY, realX + realW, realY + realH + 1, 255, 255, 255, 255);

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
