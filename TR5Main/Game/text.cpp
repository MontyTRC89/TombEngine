#include "framework.h"
#include "Game/text.h"
#include "Game/animation.h"
#include "Renderer/Renderer11.h"

namespace TEN::Renderer {
	void PrintString(int x, int y, int unk1, char* string, int unk2) {
		g_Renderer.drawString(x, y, string, D3DCOLOR_RGBA(0xFF, 0xFF, 0xFF, 255), 0);
	}
}
