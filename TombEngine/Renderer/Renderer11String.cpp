#include "framework.h"
#include "Renderer/Renderer11.h"

namespace TEN::Renderer {
	void Renderer11::DrawString(int x, int y, const char* string, D3DCOLOR color, int flags)
	{
		float factorX = ScreenWidth / REFERENCE_RES_WIDTH;
		float factorY = ScreenHeight / REFERENCE_RES_HEIGHT;

		RECT rect = { 0, 0, 0, 0 };

		// Convert the string to wstring
		int sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, string, strlen(string), NULL, 0);
		std::wstring wstr(sizeNeeded, 0);
		MultiByteToWideChar(CP_UTF8, 0, string, strlen(string), &wstr[0], sizeNeeded);

		// Prepare the structure for the renderer
		RendererStringToDraw str;
		str.String = wstr;
		str.Flags = flags;
		str.X = 0;
		str.Y = 0;
		str.Color = Vector3((color >> 16) & 0xFF, (color >> 8) & 0xFF, color & 0xFF);
		str.Scale = ScreenWidth > ScreenHeight ? factorY : factorX;

		// Measure the string
		Vector2 size = m_gameFont->MeasureString(wstr.c_str());

		if (flags & PRINTSTRING_CENTER)
		{
			int width = size.x * str.Scale;
			rect.left = x * factorX - width / 2;
			rect.right = x * factorX + width / 2;
			rect.top += y * str.Scale;
			rect.bottom += y * str.Scale;
		}
		else
		{
			rect.left = x * factorX;
			rect.right += x * factorX;
			rect.top = y * str.Scale;
			rect.bottom += y * str.Scale;
		}

		str.X = rect.left;
		str.Y = rect.top;

		if (flags & PRINTSTRING_BLINK)
		{
			if (flags & PRINTSTRING_DONT_UPDATE_BLINK)
			{
				str.Color = Vector3(192);
			}
			else if (!m_blinkUpdated)
			{
				str.Color = Vector3(m_blinkColorValue, m_blinkColorValue, m_blinkColorValue);
				m_blinkColorValue += m_blinkColorDirection * 16;
				m_blinkUpdated = true;

				if (m_blinkColorValue < 0)
				{
					m_blinkColorValue = 0;
					m_blinkColorDirection = 1;
				}

				if (m_blinkColorValue > 255)
				{
					m_blinkColorValue = 255;
					m_blinkColorDirection = -1;
				}
			}
		}

		m_strings.push_back(str);
	}

	void Renderer11::DrawAllStrings()
	{
		m_spriteBatch->Begin();

		for (int i = 0; i < m_strings.size(); i++)
		{
			RendererStringToDraw* str = &m_strings[i];

			// Draw shadow if needed
			if (str->Flags & PRINTSTRING_OUTLINE)
				m_gameFont->DrawString(m_spriteBatch.get(), str->String.c_str(), Vector2(str->X + 2 * str->Scale, str->Y + 2 * str->Scale),
					Vector4(0.0f, 0.0f, 0.0f, 1.0f) * ScreenFadeCurrent,
					0.0f, Vector4::Zero, str->Scale);

			// Draw string
			m_gameFont->DrawString(m_spriteBatch.get(), str->String.c_str(), Vector2(str->X, str->Y),
				Vector4(str->Color.x / 255.0f, str->Color.y / 255.0f, str->Color.z / 255.0f, 1.0f) * ScreenFadeCurrent,
				0.0f, Vector4::Zero, str->Scale);
		}

		m_spriteBatch->End();

		m_blinkUpdated = false;
		m_strings.clear();
	}
}
