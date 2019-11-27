#include "text.h"
#include "draw.h"

void __cdecl PrintString(int x, int y, int unk1, char* string, int unk2)
{
	g_Renderer->PrintString(x, y, string, 0xFFFFFFFF, 0);
}

void Inject_Text()
{
	INJECT(0x00480BC0, PrintString);
}