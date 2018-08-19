#include "effect2.h"
#include "draw.h"

void __cdecl TriggerDynamics(__int32 x, __int32 y, __int32 z, __int16 falloff, byte r, byte g, byte b)
{
	g_Renderer->AddDynamicLight(x, y, z, falloff, r, g, b);
}

void Inject_Effect2()
{
	INJECT(0x00431240, TriggerDynamics);
}