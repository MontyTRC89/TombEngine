#include "draw.h"
#include "..\Renderer\Renderer.h"

Renderer* g_Renderer;

__int32 __cdecl DrawPhaseGame()
{
	// Control routines uses joints calculated here for getting Lara joint positions
	CalcLaraMatrices(0);
	phd_PushUnitMatrix();
	CalcLaraMatrices(1);

	// Calls my new rock & roll renderer :)
	g_Renderer->Draw();

	// We need to pop the matrix stack or the game will crash
	MatrixPtr -= 12;
	DxMatrixPtr -= 48;

	return 1;
}

void __cdecl DoBootScreen(__int32 language)
{
	//printf("DoBootScreen\n");
	//DrawFullScreenImage((char*)"load.bmp");
}

void Inject_Draw()
{
	INJECT(0x0042A400, DrawPhaseGame);
	INJECT(0x004B8A80, DoBootScreen);
}