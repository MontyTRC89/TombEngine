#include "framework.h"
#include "text.h"
#include "draw.h"
#include "Renderer11.h"
namespace T5M::Renderer {
	void PrintString(int x, int y, int unk1, char* string, int unk2) {
		g_Renderer.drawString(x, y, string, D3DCOLOR_RGBA(0xFF, 0xFF, 0xFF, 255), 0);
	}
}
