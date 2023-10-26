#include "framework.h"
#include "Renderer/Renderer.h"

namespace TEN::Renderer
{
	void Renderer::ResetDebugVariables()
	{
		timeUpdate = 0;
		timeDraw = 0;
		timeFrame = 0;
		timeRoomsCollector = 0;
		numDrawCalls = 0;
		numRoomsDrawCalls = 0;
		numMoveablesDrawCalls = 0;
		numStaticsDrawCalls = 0;
		numSpritesDrawCalls = 0;
		numTransparentDrawCalls = 0;
		numRoomsTransparentDrawCalls = 0;
		numMoveablesTransparentDrawCalls = 0;
		numStaticsTransparentDrawCalls = 0;
		numSpritesTransparentDrawCalls = 0;
		biggestRoomIndexBuffer = 0;
		numPolygons = 0;
		numCheckPortalCalls = 0;
		numGetVisibleRoomsCalls = 0;
		numDotProducts = 0;
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

		AddString(10, currentY, buffer, 0xFFFFFFFF, PRINTSTRING_OUTLINE);

		currentY += 20;
	}
}
