#include "hair.h"
#include "..\Global\global.h"

void __cdecl j_HairControl(__int16 a, __int16 b, __int16 c)
{
	HairControl(a, b, c);
}

void Inject_Hair()
{
	INJECT(0x00403102, j_HairControl);
}