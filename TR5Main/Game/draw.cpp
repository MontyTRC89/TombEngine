#include "framework.h"
#include "draw.h"
#include "Lara.h"
#include "camera.h"
#include "level.h"
#include "Renderer11.h"

using TEN::Renderer::g_Renderer;

BITE_INFO EnemyBites[9] =
{
	{ 20, -95, 240, 13 },
	{ 48, 0, 180, -11 },
	{ -48, 0, 180, 14 },
	{ -48, 5, 225, 14 },
	{ 15, -60, 195, 13 },
	{ -30, -65, 250, 18 },
	{ 0, -110, 480, 13 },
	{ -20, -80, 190, -10 },
	{ 10, -60, 200, 13 }
};

BOUNDING_BOX InterpolatedBounds;

int DrawPhaseGame()
{
	g_Renderer.Draw();
	Camera.numberFrames = g_Renderer.SyncRenderer();
	return Camera.numberFrames;
}

BOUNDING_BOX* GetBoundsAccurate(ITEM_INFO* item)
{
	int rate = 0;
	ANIM_FRAME* framePtr[2];
	
	int frac = GetFrame_D2(item, framePtr, &rate);
	if (frac == 0)
		return &framePtr[0]->boundingBox;
	else
	{
		InterpolatedBounds.X1 = framePtr[0]->boundingBox.X1 + (framePtr[1]->boundingBox.X1 - framePtr[0]->boundingBox.X1) * frac / rate;
		InterpolatedBounds.X2 = framePtr[0]->boundingBox.X2 + (framePtr[1]->boundingBox.X2 - framePtr[0]->boundingBox.X2) * frac / rate;
		InterpolatedBounds.Y1 = framePtr[0]->boundingBox.Y1 + (framePtr[1]->boundingBox.Y1 - framePtr[0]->boundingBox.Y1) * frac / rate;
		InterpolatedBounds.Y2 = framePtr[0]->boundingBox.Y2 + (framePtr[1]->boundingBox.Y2 - framePtr[0]->boundingBox.Y2) * frac / rate;
		InterpolatedBounds.Z1 = framePtr[0]->boundingBox.Z1 + (framePtr[1]->boundingBox.Z1 - framePtr[0]->boundingBox.Z1) * frac / rate;
		InterpolatedBounds.Z2 = framePtr[0]->boundingBox.Z2 + (framePtr[1]->boundingBox.Z2 - framePtr[0]->boundingBox.Z2) * frac / rate;

		return &InterpolatedBounds;
	}
}

ANIM_FRAME* GetBestFrame(ITEM_INFO* item)
{
	int rate = 0;
	ANIM_FRAME* framePtr[2];

	int frac = GetFrame_D2(item, framePtr, &rate);

	if (frac <= (rate >> 1))
		return framePtr[0];
	else
		return framePtr[1];
}

int GetFrame_D2(ITEM_INFO* item, ANIM_FRAME* framePtr[], int* rate)
{
	ANIM_STRUCT *anim;
	int frm;
	int first, second;
	int interp, rat;

	frm = item->frameNumber;
	anim = &g_Level.Anims[item->animNumber];
	framePtr[0] = framePtr[1] = &g_Level.Frames[anim->framePtr];
	rat = *rate = anim->interpolation & 0x00ff;
	frm -= anim->frameBase; 
	first = frm / rat;
	interp = frm % rat;
	framePtr[0] += first;				  // Get Frame pointers
	framePtr[1] = framePtr[0] + 1;               // and store away
	if (interp == 0)
		return(0);
	second = first * rat + rat;
	if (second>anim->frameEnd)                       // Clamp KeyFrame to End if need be
		*rate = anim->frameEnd - (second - rat);
	return(interp);
}

void DrawAnimatingItem(ITEM_INFO* item)
{
	// TODO: to refactor
	// Empty stub because actually we disable items drawing when drawRoutine pointer is NULL in OBJECT_INFO
}

void GetLaraJointPosition(PHD_VECTOR* pos, int LM_enum)
{
	if (LM_enum >= NUM_LARA_MESHES)
		LM_enum = LM_HEAD;

	Vector3 p = Vector3(pos->x, pos->y, pos->z);
	g_Renderer.getLaraAbsBonePosition(&p, LM_enum);

	pos->x = p.x;
	pos->y = p.y;
	pos->z = p.z;
}

void ClampRotation(PHD_3DPOS* pos, short angle, short rot)
{
	if (angle <= rot)
	{
		if (angle >= -rot)
			pos->yRot += angle;
		else
			pos->yRot -= rot;
	}
	else
	{
		pos->yRot += rot;
	}
}
