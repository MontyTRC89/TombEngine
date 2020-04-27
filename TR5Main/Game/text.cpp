#include "text.h"
#include "draw.h"

void PrintString(int x, int y, int unk1, char* string, int unk2)
{
	g_Renderer->PrintString(x, y, string, 0xFFFFFFFF, 0);
}
