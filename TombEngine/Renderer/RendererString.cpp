#include "framework.h"
#include "Renderer/Renderer.h"

#include "Specific/trutils.h"

namespace TEN::Renderer
{
	void Renderer::AddDebugString(const std::string& string, const Vector2& pos, const Color& color, float scale, int flags, RendererDebugPage page)
	{
		constexpr auto FLAGS = (int)PrintStringFlags::Outline | (int)PrintStringFlags::Center;

		if (DebugPage != page)
			return;

		AddString(string, pos, color, scale, FLAGS);
	}

	void Renderer::AddString(int x, int y, const std::string& string, D3DCOLOR color, int flags)
	{
		AddString(string, Vector2(x, y), Color(color), 1.0f, flags);
	}

	void Renderer::AddString(const std::string& string, const Vector2& pos, const Color& color, float scale, int flags)
	{
		constexpr auto BLINK_VALUE_MAX = 1.0f;
		constexpr auto BLINK_VALUE_MIN = 0.1f;
		constexpr auto BLINK_TIME_STEP = 0.2f;

		if (isLocked)
			return;

		if (string.empty())
			return;

		try
		{
			auto screenRes = GetScreenResolution();
			auto factor = Vector2(screenRes.x / SCREEN_SPACE_RES.x, screenRes.y / SCREEN_SPACE_RES.y);
			float uiScale = (screenRes.x > screenRes.y) ? factor.y : factor.x;
			float fontSpacing = gameFont->GetLineSpacing();
			float fontScale = REFERENCE_FONT_SIZE / fontSpacing;

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
				rString.Color = color.ToVector3();
				rString.Scale = (uiScale * fontScale) * scale;

				// Measure string.
				auto size = Vector2(gameFont->MeasureString(rString.String.c_str())) * rString.Scale;
				if (flags & (int)PrintStringFlags::Center)
				{
					rString.X = (pos.x * factor.x) - (size.x / 2.0f);
				}
				else if (flags & (int)PrintStringFlags::Right)
				{
					rString.X = (pos.x * factor.x) - size.x;
				}
				else
				{
					rString.X = pos.x * factor.x;
				}

				rString.Y = (pos.y * uiScale) + yOffset;

				if (flags & (int)PrintStringFlags::Blink)
				{
					rString.Color *= blinkColorValue;

					if (!isBlinkUpdated)
					{
						// Calculate blink increment based on sine wave.
						blinkColorValue = ((sin(blinkTime) + BLINK_VALUE_MAX) * 0.5f) + BLINK_VALUE_MIN;

						// Update blink time.
						blinkTime += BLINK_TIME_STEP;
						if (blinkTime > PI_MUL_2)
							blinkTime -= PI_MUL_2;

						isBlinkUpdated = true;
					}
				}

				yOffset += size.y;
				stringsToDraw.push_back(rString);
			}
		}
		catch (std::exception& ex)
		{
			TENLog(std::string("Unable to process string: '") + string + "'. Exception: " + std::string(ex.what()), LogLevel::Error);
		}
	}

	void Renderer::DrawAllStrings()
	{
		float shadowOffset = 1.5f / (REFERENCE_FONT_SIZE / gameFont->GetLineSpacing());

		spriteBatch->Begin();

		for (const auto& rString : stringsToDraw)
		{
			// Draw shadow.
			if (rString.Flags & (int)PrintStringFlags::Outline)
			{
				gameFont->DrawString(
					spriteBatch.get(), rString.String.c_str(),
					Vector2(rString.X + shadowOffset * rString.Scale, rString.Y + shadowOffset * rString.Scale),
					Vector4(0.0f, 0.0f, 0.0f, 1.0f) * ScreenFadeCurrent,
					0.0f, Vector4::Zero, rString.Scale);
			}

			// Draw string.
			gameFont->DrawString(
				spriteBatch.get(), rString.String.c_str(),
				Vector2(rString.X, rString.Y),
				Vector4(rString.Color.x, rString.Color.y, rString.Color.z, 1.0f) * ScreenFadeCurrent,
				0.0f, Vector4::Zero, rString.Scale);
		}

		spriteBatch->End();

		isBlinkUpdated = false;
		stringsToDraw.clear();
	}
}
