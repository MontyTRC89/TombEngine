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
		_numSortedRoomsDrawCalls = 0;
		_numMoveablesDrawCalls = 0;
		_numSortedMoveablesDrawCalls = 0;
		_numStaticsDrawCalls = 0;
		_numInstancedStaticsDrawCalls = 0;
		_numSortedStaticsDrawCalls = 0;
		_numSpritesDrawCalls = 0;
		_numInstancedSpritesDrawCalls = 0;
		_numSortedSpritesDrawCalls = 0;

		_numLinesDrawCalls = 0;

		_numTriangles = 0;
		_numSortedTriangles = 0;

		_numShadowMapDrawCalls = 0;
		_numDebrisDrawCalls = 0;
		_numEffectsDrawCalls = 0;

		_numDotProducts = 0;
		_numCheckPortalCalls = 0;
		_numGetVisibleRoomsCalls = 0;

		_currentY;
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
