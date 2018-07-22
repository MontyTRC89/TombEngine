#include "text.h"
#include "draw.h"

void __cdecl PrintString(__int32 x, __int32 y, __int32 unk1, char* string, __int32 unk2)
{
	g_Renderer->PrintString(x, y, string, 0xFFFFFFFF, 0);
}

void Inject_Text()
{
	INJECT(0x00480BC0, PrintString);
}