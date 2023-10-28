#include "framework.h"
#include "Renderer/Renderer.h"

namespace TEN::Renderer
{
	void Renderer::ResetDebugVariables()
	{
		_timeUpdate = 0;
		_timeDraw = 0;
		_timeFrame = 0;
		_timeRoomsCollector = 0;
		_numDrawCalls = 0;
		_numRoomsDrawCalls = 0;
		_numMoveablesDrawCalls = 0;
		_numStaticsDrawCalls = 0;
		_numSpritesDrawCalls = 0;
		_numTransparentDrawCalls = 0;
		_numRoomsTransparentDrawCalls = 0;
		_numMoveablesTransparentDrawCalls = 0;
		_numStaticsTransparentDrawCalls = 0;
		_numSpritesTransparentDrawCalls = 0;
		_biggestRoomIndexBuffer = 0;
		_numPolygons = 0;
		_numCheckPortalCalls = 0;
		_numGetVisibleRoomsCalls = 0;
		_numDotProducts = 0;
	}

	bool Renderer::PrintDebugMessage(int x, int y, int alpha, byte r, byte g, byte b, LPCSTR Message)
	{
		return true;
	}

	void Renderer::PrintDebugMessage(LPCSTR message, ...)
	{
		char buffer[255];
		ZeroMemory(buffer, 255);

		va_list args;
		va_start(args, message);
		_vsprintf_l(buffer, message, NULL, args);
		va_end(args);

		AddString(10, _currentY, buffer, 0xFFFFFFFF, (int)PrintStringFlags::Outline);

		_currentY += 20;
	}
}
