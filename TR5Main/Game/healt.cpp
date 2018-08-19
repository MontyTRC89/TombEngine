#include "healt.h"
#include "draw.h"

void __cdecl DrawHealtBar(__int32 percentual)
{
	g_Renderer->DrawHealthBar(percentual);
}

void __cdecl DrawAirBar(__int32 percentual)
{
	g_Renderer->DrawAirBar(percentual);
}

void Inject_Healt()
{
	INJECT(0x004B1950, DrawHealtBar);
	INJECT(0x004B18E0, DrawAirBar);
}
