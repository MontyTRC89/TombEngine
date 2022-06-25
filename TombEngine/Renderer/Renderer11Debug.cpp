#include "framework.h"
#include "Renderer/Renderer11.h"

namespace TEN::Renderer
{
	bool Renderer11::PrintDebugMessage(int x, int y, int alpha, byte r, byte g, byte b, LPCSTR Message)
	{
		return true;
	}

	void Renderer11::PrintDebugMessage(LPCSTR message, ...)
	{
		char buffer[255];
		ZeroMemory(buffer, 255);

		va_list args;
		va_start(args, message);
		_vsprintf_l(buffer, message, NULL, args);
		va_end(args);

		DrawString(10, m_currentY, buffer, 0xFFFFFFFF, PRINTSTRING_OUTLINE);

		m_currentY += 20;
	}
}
