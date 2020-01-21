#include "draw.h"
#include "Lara.h"
#include "..\Renderer\Renderer11.h"
#include "camera.h"

Renderer11* g_Renderer;

int DrawPhaseGame()
{
	// Control routines uses joints calculated here for getting Lara joint positions
	CalcLaraMatrices(0);
	phd_PushUnitMatrix();
	CalcLaraMatrices(1);

	// Calls my new rock & roll renderer :)
	g_Renderer->Draw();
	Camera.numberFrames = g_Renderer->SyncRenderer();

	// We need to pop the matrix stack or the game will crash
	phd_PopMatrix();

	return Camera.numberFrames;
}

void DrawAnimatingItem(ITEM_INFO* item)
{
	/*OBJECT_INFO* obj;
	CREATURE_INFO* creature;
	int* bones;
	int clip, i, poppush, frac, rate, bit;
	short* frames[2];
	short* extra_rotation;
	short* rotation1, *rotation2;
	short** mesh;

	creature = (CREATURE_INFO*)item->data;
	frac = GetFrame_D2(item, frames, &rate);
	obj = &Objects[item->objectNumber];

	//if (obj->shadowSize)
	//	S_PrintShadow(obj->shadowSize, frames[0], item, NULL);

	phd_PushMatrix();
	phd_TranslateAbs(item->pos.xPos, item->pos.yPos, item->pos.zPos);
	phd_RotYXZ(item->pos.yRot, item->pos.xRot, item->pos.zRot);

	// checking objectMip there, deleted !

	sub_42B4C0(item, frames[0]);
	if (phd_ClipBoundingBox(frames[0]))
	{
		//CalculateObjectLighting(item, frames[0]);

		if (item->data == NULL)
			extra_rotation = NullRotations;
		else
			extra_rotation = (short*)item->data;

		mesh = &Meshes[Objects[item->objectNumber].meshIndex];
		bones = &Bones[obj->boneIndex];
		bit = 1;

		if (!frac)
		{
			phd_TranslateRel((int)*(frames[0] + 6), (int)*(frames[0] + 7), (int)*(frames[0] + 8)); // can be [0][6] etc.. ?
			rotation1 = (short*)(frames[0] + 9);
			gar_RotYXZsuperpack(&rotation1, 0);

			if (item->meshBits & bit)
				phd_PutPolygons(*mesh);

			for (i = (obj->nmeshes - 1); i > 0; i--, bones += 4, mesh += 2)
			{
				poppush = *bones;

				if (poppush & 1)
				{
					phd_PopMatrix();
					phd_PopDxMatrix();
				}

				if (poppush & 2)
					phd_PushMatrix();

				phd_TranslateRel(*(bones + 1), *(bones + 2), *(bones + 3));
				gar_RotYXZsuperpack(&rotation1, 0);

				if (extra_rotation && (poppush & (ROT_X | ROT_Y | ROT_Z)))
				{
					if (poppush & ROT_Y)
						phd_RotY(*(extra_rotation++));
					if (poppush & ROT_X)
						phd_RotX(*(extra_rotation++));
					if (poppush & ROT_Z)
						phd_RotZ(*(extra_rotation++));
				}

				bit <<= 1;
				if (item->meshBits & bit)
					phd_PutPolygons(*mesh);
			}
		}
		else
		{
			InitInterpolate(frac, rate);
			phd_TranslateRel_ID((int)*(frames[0] + 6), (int)*(frames[0] + 7), (int)*(frames[0] + 8), (int)*(frames[1] + 6), (int)*(frames[1] + 7), (int)*(frames[1] + 8));
			rotation1 = (short*)(frames[0] + 9);
			rotation2 = (short*)(frames[1] + 9);
			gar_RotYXZsuperpack_I(&rotation1, &rotation2, 0);

			if (item->meshBits & bit)
				phd_PutPolygons_I(*mesh);

			for (i = (obj->nmeshes - 1); i > 0; i--, bones += 4, mesh += 2)
			{
				poppush = *bones;
				if (poppush & 1)
					phd_PopMatrix_I();

				if (poppush & 2)
					phd_PushMatrix_I();

				phd_TranslateRel_I(*(bones + 1), *(bones + 2), *(bones + 3));
				gar_RotYXZsuperpack_I(&rotation1, &rotation2, 0);

				if (extra_rotation && (poppush & (ROT_X | ROT_Y | ROT_Z)))
				{
					if (poppush & ROT_Y)
						phd_RotY_I(*(extra_rotation++));
					if (poppush & ROT_X)
						phd_RotX_I(*(extra_rotation++));
					if (poppush & ROT_Z)
						phd_RotZ_I(*(extra_rotation++));
				}

				bit <<= 1;
				if (item->meshBits & bit)
					phd_PutPolygons_I(*mesh);
			}
		}
	}

	PhdRight = PhdWidth;
	PhdLeft = 0;
	PhdTop = 0;
	PhdBottom = PhdHeight;
	phd_PopMatrix();
	phd_PopDxMatrix();*/
}

int GetFrame_D2(ITEM_INFO* item, short* framePtr[], int* rate)
{
	ANIM_STRUCT *anim;
	int frm;
	int first, second;
	int frame_size;
	int interp, rat;

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

void _InitInterpolate(void)
{
}

void _phd_PushMatrix(void)
{
}

void _phd_PushMatrix_I(void)
{
}

void _phd_PushUnitMatrix(void)
{
}

void _phd_RotYXZ(short ry, short rx, short rz)
{
}

void _phd_RotY(short ry)
{
}

void _phd_RotY_I(short ry)
{
}

void _phd_RotX(short rx)
{
}

void _phd_RotX_I(short rx)
{
}

void _phd_RotZ(short rz)
{
}

void _phd_RotZ_I(short rz)
{
}

void _phd_PutPolygons(void)
{
}

void _phd_PutPolygons_I(void)
{
}

void _phd_TranslateRel(int x, int y, int z)
{
}

void _phd_TranslateRel_I(int x, int y, int z)
{
}

void _phd_TranslateRel_ID(int x1, int y1, int z1, int x2, int y2, int z2)
{
}

void _phd_TranslateAbs(int x, int y, int z)
{
}

void _phd_RotYXZpack(short ry, short rx, short rz)
{
}

void _gar_RotYXZsuperpack(short** pproc, int skip)
{
}

void _gar_RotYXZsuperpack_I(short** pproc1, short** pproc2, int skip)
{
}

void _phd_ClipBoundingBox(short* frames)
{
}

void _phd_DxTranslateRel(int x, int y, int z)
{
}

void _phd_DxTranslateAbs(int x, int y, int z)
{
}

void _phd_DxRotY(short ry)
{
}

void _phd_DxRotX(short rx)
{
}

void _phd_DxRotZ(short rz)
{
}

void _phd_DxRotYXZ(short ry, short rx, short rz)
{
}

void _phd_DxRotYXZpack(int rangle)
{
}

void phd_PopMatrix(void)
{
	MatrixPtr -= 12;
	DxMatrixPtr -= 48;
}

void _phd_PopMatrix_I(void)
{
	MatrixPtr -= 12;
	DxMatrixPtr -= 48;
}

void Inject_Draw()
{
	//INJECT(0x0042B900, DrawAnimatingItem);
}