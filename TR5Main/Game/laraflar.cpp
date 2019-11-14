#include "laraflar.h"
#include "..\Global\global.h"

void __cdecl DrawFlareMeshes()
{
	Lara.meshPtrs[13] = Meshes[Objects[ID_FLARE_ANIM].meshIndex + 26];
}

void Inject_LaraFlar()
{
	INJECT(0x004553B0, DrawFlareMeshes);
}