#include "draw.h"
#include "Lara.h"
#include "..\Renderer\Renderer11.h"
#include "camera.h"

BITE_INFO EnemyBites[9] =
{
	{ 0x14, 0xFFFFFFA1, 0xF0, 0xD },
	{ 0x30, 0, 0xB4, 0xFFFFFFF5 },
	{ 0xFFFFFFD0, 0, 0xB4, 0xE },
	{ 0xFFFFFFC9, 5, 0xE1, 0xE },
	{ 0xF, 0xFFFFFFC4, 0xC3, 0xD },
	{ 0xFFFFFFE2, 0xFFFFFFBF, 0xFA, 0x12 },
	{ 0, 0xFFFFFF92, 0x1E0, 0xD },
	{ 0xFFFFFFEC, 0xFFFFFFB0, 0xBE, 0xFFFFFFF6 },
	{ 0xA, 0xFFFFFFC4, 0xC8, 0xD }
};

Renderer11* g_Renderer;

int LightningCount;
int LightningRand;
int StormTimer;
int dLightningRand;
byte SkyStormColor[3];
byte SkyStormColor2[3];
short InterpolatedBounds[6];
LARGE_INTEGER PerformanceCount;
double LdFreq;
double LdSync;

int DrawPhaseGame()
{
	g_Renderer->Draw();
	Camera.numberFrames = g_Renderer->SyncRenderer();
	return Camera.numberFrames;
}

void UpdateStorm()
{
	if (LightningCount <= 0)
	{
		if (LightningRand < 4)
			LightningRand = 0;
		else
			LightningRand = LightningRand - (LightningRand >> 2);
	}
	else if (--LightningCount)
	{
		dLightningRand = rand() & 0x1FF;
		LightningRand = ((dLightningRand - LightningRand) >> 1) + LightningRand;
	}
	else
	{
		dLightningRand = 0;
		LightningRand = (rand() & 0x7F) + 400;
	}

	for (int i = 0; i < 3; i++)
	{
		SkyStormColor2[i] += LightningRand * SkyStormColor2[i] >> 8;
		SkyStormColor[i] = SkyStormColor2[i];
		if (SkyStormColor[i] > 255)
			SkyStormColor[i] = 255;
	}
}

short* GetBoundsAccurate(ITEM_INFO* item)
{
	int rate = 0;
	short* framePtr[2];
	
	int frac = GetFrame_D2(item, framePtr, &rate);
	if (frac == 0)
		return framePtr[0];
	else
	{
		for (int i = 0; i < 6; i++)
		{
			InterpolatedBounds[i] = *(framePtr[0]) + ((*(framePtr[1]) - *(framePtr[0])) * frac) / rate;
			framePtr[0]++;
			framePtr[1]++;
		}
		return InterpolatedBounds;
	}
}

short* GetBestFrame(ITEM_INFO* item)
{
	int rate = 0;
	short* framePtr[2];

	int frac = GetFrame_D2(item, framePtr, &rate);

	if (frac <= (rate >> 1))
		return framePtr[0];
	else
		return framePtr[1];
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

bool TIME_Reset()
{
	LARGE_INTEGER fq;

	QueryPerformanceCounter(&fq);

	LdSync = (double)fq.LowPart + (double)fq.HighPart * (double)0xffffffff;
	LdSync /= LdFreq;

	return true;
}

bool TIME_Init()
{
	LARGE_INTEGER fq;

	if (!QueryPerformanceFrequency(&fq))
		return false;

	LdFreq = (double)fq.LowPart + (double)fq.HighPart * (double)0xFFFFFFFF;
	LdFreq /= 60.0;

	TIME_Reset();

	return true;
}

int Sync()
{
	LARGE_INTEGER ct;
	double dCounter;
	
	QueryPerformanceCounter(&ct);
	
	dCounter = (double)ct.LowPart + (double)ct.HighPart * (double)0xFFFFFFFF;
	dCounter /= LdFreq;
	
	nFrames = long(dCounter) - long(LdSync);
	
	LdSync = dCounter;
	
	return nFrames;
}

void DrawAnimatingItem(ITEM_INFO* item)
{
	// TODO: to refactor
	// Empty stub because actually we disable items drawing when drawRoutine pointer is NULL in OBJECT_INFO
}

void _phd_ClipBoundingBox(short* frames)
{

}

void phd_PopMatrix()
{
	MatrixPtr -= 12;
	DxMatrixPtr -= 12;
}

void phd_RotBoundingBoxNoPersp(PHD_3DPOS* pos, short* bounds, short* tbounds)
{
	Matrix world = Matrix::CreateFromYawPitchRoll(
		TR_ANGLE_TO_RAD(pos->yRot),
		TR_ANGLE_TO_RAD(pos->xRot),
		TR_ANGLE_TO_RAD(pos->zRot)
	);

	Vector3 bMin = Vector3(bounds[0], bounds[2], bounds[4]);
	Vector3 bMax = Vector3(bounds[1], bounds[3], bounds[5]);

	bMin = Vector3::Transform(bMin, world);
	bMax = Vector3::Transform(bMax, world);

	tbounds[0] = bMin.x;
	tbounds[2] = bMin.y;
	tbounds[4] = bMin.z;
	tbounds[1] = bMax.x;
	tbounds[3] = bMax.y;
	tbounds[5] = bMax.z;
}

void GetLaraJointPosition(PHD_VECTOR* pos, int joint)
{
	if (joint > 14)
		joint = 14;

	Vector3 p = Vector3(pos->x, pos->y, pos->z);
	g_Renderer->GetLaraAbsBonePosition(&p, joint);

	pos->x = p.x;
	pos->y = p.y;
	pos->z = p.z;
}

void Inject_Draw()
{
	//INJECT(0x0048F9C0, phd_PushMatrix);
	INJECT(0x0042CF80, GetBoundsAccurate);
	INJECT(0x0042D020, GetBestFrame);
	INJECT(0x004D1A40, Sync);
	//INJECT(0x0042BE90, InitInterpolate);
	/*INJECT(0x0042BF00, phd_PopMatrix_I);
	INJECT(0x00490150, phd_RotYXZ);
	INJECT(0x0048FBE0, phd_RotX);
	INJECT(0x0048FCD0, phd_RotY);
	INJECT(0x0048FDC0, phd_RotZ); 
	INJECT(0x0042C030, phd_RotX_I);
	INJECT(0x0042BFC0, phd_RotY_I);
	INJECT(0x0042C0A0, phd_RotZ_I);
	INJECT(0x0048FB20, phd_TranslateRel);
	INJECT(0x0042C110, phd_TranslateRel_I);
	INJECT(0x0042C190, phd_TranslateRel_ID);
	INJECT(0x004903F0, phd_TranslateAbs);
	INJECT(0x0048FA40, phd_SetTrans);
	INJECT(0x0042BF50, phd_PushMatrix_I);
	INJECT(0x0048FA90, phd_PushUnitMatrix);
	INJECT(0x004904B0, phd_GetVectorAngles);
	INJECT(0x0048FEB0, phd_RotYXZpack);
	/*INJECT(0x0042C310, gar_RotYXZsuperpack);
	INJECT(0x0042C290, gar_RotYXZsuperpack_I);*/
}