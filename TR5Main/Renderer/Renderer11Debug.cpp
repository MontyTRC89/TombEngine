#include "framework.h"
#include "Renderer11.h"
namespace  T5M::Renderer {
	bool Renderer11::printDebugMessage(int x, int y, int alpha, byte r, byte g, byte b, LPCSTR Message) {
		return true;
	}

	void Renderer11::printDebugMessage(LPCSTR message, ...) {
		char buffer[255];
		ZeroMemory(buffer, 255);

		va_list args;
		va_start(args, message);
		_vsprintf_l(buffer, message, NULL, args);
		va_end(args);

		PrintString(10, m_currentY, buffer, 0xFFFFFFFF, PRINTSTRING_OUTLINE);

		m_currentY += 20;
	}

}
