#include "framework.h"
#include "Renderer/Renderer11.h"

#include "Specific/trutils.h"

namespace TEN::Renderer
{
	void Renderer11::AddString(int x, int y, const char* string, D3DCOLOR color, int flags)
	{
		AddString(std::string(string), Vector2(x, y), Color(color), 1.0f, flags);
	}

	void Renderer11::AddString(const std::string& string, const Vector2& pos, const Color& color, float scale, int flags)
	{
		if (m_Locked)
			return;

		if (string.empty())
			return;

		try
		{
			auto screenRes = GetScreenResolution();
			auto factor = Vector2(screenRes.x / SCREEN_COORDS.x, screenRes.y / SCREEN_COORDS.y);
			float UIScale = (screenRes.x > screenRes.y) ? factor.y : factor.x;
			float fontSpacing = m_gameFont->GetLineSpacing();
			float fontScale   = REFERENCE_FONT_SIZE / fontSpacing;

			auto stringLines = SplitString(string);
			float yOffset = 0.0f;
			for (const auto& line : stringLines)
			{
				// Prepare structure for renderer.
				RendererStringToDraw rString;
				rString.String = TEN::Utils::ToWString(line);
				rString.Flags = flags;
				rString.X = 0;
				rString.Y = 0;
				rString.Color = color.ToVector3() * UCHAR_MAX;
				rString.Scale = (UIScale * fontScale) * scale;

				// Measure string.
				auto size = Vector2(m_gameFont->MeasureString(rString.String.c_str()));
				float width = size.x * rString.Scale;

				rString.X = (flags & PRINTSTRING_CENTER) ? ((pos.x * factor.x) - (width / 2.0f)) : (pos.x * factor.x);
				rString.Y = (pos.y * UIScale) + yOffset;

				if (flags & PRINTSTRING_BLINK)
				{
					rString.Color = Vector3(m_blinkColorValue, m_blinkColorValue, m_blinkColorValue);

					if (!m_blinkUpdated)
					{
						m_blinkColorValue += m_blinkColorDirection * 16;
						m_blinkUpdated = true;

						if (m_blinkColorValue < 0)
						{
							m_blinkColorValue = 0;
							m_blinkColorDirection = 1;
						}

						if (m_blinkColorValue > UCHAR_MAX)
						{
							m_blinkColorValue = UCHAR_MAX;
							m_blinkColorDirection = -1;
						}
					}
				}

				m_strings.push_back(rString);

				yOffset += fontSpacing * 1.1f;
			}

		}
		catch (std::exception& ex)
		{
			TENLog(std::string("Unable to process string: '") + string + "'. Exception: " + std::string(ex.what()), LogLevel::Error);
		}
	}

	void Renderer11::DrawAllStrings()
	{
		float shadeOffset = 1.5f / (REFERENCE_FONT_SIZE / m_gameFont->GetLineSpacing());

		m_spriteBatch->Begin();

		for (const auto& rString : m_strings)
		{
			// Draw shadow.
			if (rString.Flags & PRINTSTRING_OUTLINE)
			{
				m_gameFont->DrawString(
					m_spriteBatch.get(), rString.String.c_str(),
					Vector2(rString.X + shadeOffset * rString.Scale, rString.Y + shadeOffset * rString.Scale),
					Vector4(0.0f, 0.0f, 0.0f, 1.0f) * ScreenFadeCurrent,
					0.0f, Vector4::Zero, rString.Scale);
			}

			// Draw string.
			m_gameFont->DrawString(
				m_spriteBatch.get(), rString.String.c_str(),
				Vector2(rString.X, rString.Y),
				Vector4(rString.Color.x / UCHAR_MAX, rString.Color.y / UCHAR_MAX, rString.Color.z / UCHAR_MAX, 1.0f) * ScreenFadeCurrent,
				0.0f, Vector4::Zero, rString.Scale);
		}

		m_spriteBatch->End();

		m_blinkUpdated = false;
		m_strings.clear();
	}
}
