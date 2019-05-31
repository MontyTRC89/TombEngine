#include "draw.h"
#include "lara.h"
#include "..\Renderer\Renderer11.h"

Renderer11* g_Renderer;

__int32 __cdecl DrawPhaseGame()
{
	// Control routines uses joints calculated here for getting Lara joint positions
	CalcLaraMatrices(0);
	phd_PushUnitMatrix();
	CalcLaraMatrices(1);

	// Calls my new rock & roll renderer :)
	g_Renderer->Draw();
	Camera.numberFrames = g_Renderer->SyncRenderer();

	// We need to pop the matrix stack or the game will crash
	MatrixPtr -= 12;
	DxMatrixPtr -= 48;

	return Camera.numberFrames;
}

__int32 __cdecl GetFrame_D2(ITEM_INFO* item, __int16* framePtr[], __int32* rate)
{
	ANIM_STRUCT *anim;
	int			frm;
	int			first, second;
	int			frame_size;
	int			interp, rat;

	frm = item->frameNumber;
	anim = &Anims[item->animNumber];
	framePtr[0] = framePtr[1] = anim->framePtr;
	rat = *rate = anim->interpolation & 0x00ff;
	frame_size = anim->interpolation >> 8;
	frm -= anim->frameBase;
	first = frm / rat;
	interp = frm % rat;
	framePtr[0] += first * frame_size;				  // Get Frame pointers
	framePtr[1] = framePtr[0] + frame_size;               // and store away
	if (interp == 0)
		return(0);
	second = first * rat + rat;
	if (second>anim->frameEnd)                       // Clamp KeyFrame to End if need be
		*rate = anim->frameEnd - (second - rat);
	return(interp);
}

void Inject_Draw()
{

}