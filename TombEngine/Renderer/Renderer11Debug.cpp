#include "framework.h"
#include "Renderer/Renderer11.h"

namespace TEN::Renderer
{
	void Renderer11::ResetDebugVariables()
	{
		m_timeUpdate = 0;
		m_timeDraw = 0;
		m_timeFrame = 0;
		m_numDrawCalls = 0;
		m_numRoomsDrawCalls = 0;
		m_numMoveablesDrawCalls = 0;
		m_numStaticsDrawCalls = 0;
		m_numSpritesDrawCalls = 0;
		m_numTransparentDrawCalls = 0;
		m_numRoomsTransparentDrawCalls = 0;
		m_numMoveablesTransparentDrawCalls = 0;
		m_numStaticsTransparentDrawCalls = 0;
		m_numSpritesTransparentDrawCalls = 0;
		m_biggestRoomIndexBuffer = 0;
		m_numPolygons = 0;
	}

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

		AddString(10, m_currentY, buffer, 0xFFFFFFFF, PRINTSTRING_OUTLINE);

		m_currentY += 20;
	}
}
