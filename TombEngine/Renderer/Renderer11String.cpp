#include "framework.h"
#include "Renderer/Renderer11.h"
#include "Specific/trutils.h"

namespace TEN::Renderer 
{
	void Renderer11::AddString(int x, int y, const char* string, D3DCOLOR color, int flags)
	{
		if (m_Locked)
			return;

		if (string == nullptr)
			return;

		try
		{
			float factorX = m_screenWidth / REFERENCE_RES_WIDTH;
			float factorY = m_screenHeight / REFERENCE_RES_HEIGHT;
			float UIScale = m_screenWidth > m_screenHeight ? factorY : factorX;
			float fontSpacing = m_gameFont->GetLineSpacing();
			float fontScale   = REFERENCE_FONT_SIZE / fontSpacing;

			float currentY = 0;

			auto lines = TEN::Utils::SplitString(string);

			for (auto line : lines)
			{
				auto cLine = line.c_str();

				// Convert the string to wstring
				int sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, cLine, line.size(), NULL, 0);
				std::wstring wstr(sizeNeeded, 0);
				MultiByteToWideChar(CP_UTF8, 0, cLine, strlen(cLine), &wstr[0], sizeNeeded);

				// Prepare the structure for the renderer
				RendererStringToDraw str;
				str.String = wstr;
				str.Flags = flags;
				str.X = 0;
				str.Y = 0;
				str.Color = Vector3((color >> 16) & 0xFF, (color >> 8) & 0xFF, color & 0xFF);
				str.Scale = UIScale * fontScale;

				// Measure the string
				Vector2 size = m_gameFont->MeasureString(wstr.c_str());
				float width = size.x * str.Scale;

				str.X = (flags & PRINTSTRING_CENTER) ? (float)x * factorX - (width / 2.0f) : (float)x * factorX;
				str.Y = y * UIScale + currentY;

				if (flags & PRINTSTRING_BLINK)
				{
					str.Color = Vector3(m_blinkColorValue, m_blinkColorValue, m_blinkColorValue);

					if (!m_blinkUpdated)
					{
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

				currentY += fontSpacing * 1.1f;
			}

		}
		catch (std::exception& ex)
		{
			TENLog(std::string("Unable to process string: '") + string +
				"'. Exception: " + std::string(ex.what()), LogLevel::Error);
		}
	}

	void Renderer11::DrawAllStrings()
	{
		float shadeOffset = 1.5f / (REFERENCE_FONT_SIZE / m_gameFont->GetLineSpacing());

		m_spriteBatch->Begin();

		for (int i = 0; i < m_strings.size(); i++)
		{
			RendererStringToDraw* str = &m_strings[i];

			// Draw shadow if needed
			if (str->Flags & PRINTSTRING_OUTLINE)
				m_gameFont->DrawString(m_spriteBatch.get(), str->String.c_str(), Vector2(str->X + shadeOffset * str->Scale, str->Y + shadeOffset * str->Scale),
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
