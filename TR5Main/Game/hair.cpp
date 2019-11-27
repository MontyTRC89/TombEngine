#include "hair.h"
#include "..\Global\global.h"

void __cdecl j_HairControl(short a, short b, short c)
{
	HairControl(a, b, c);
}

void Inject_Hair()
{
	INJECT(0x00403102, j_HairControl);
}