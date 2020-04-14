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

#define DxIMStack	ARRAY_(0x00E6D860, int, [768])
#define DxIMptr		VAR_U_(0x00E6D834, int*)
#define IMStack		ARRAY_(0x00E6CB00, int, [768])
#define IMptr		VAR_U_(0x00E6E468, int*)
#define IM_Rate		VAR_U_(0x00E6E464, int)
#define IM_Frac		VAR_U_(0x00E6D734, int)

void InitInterpolate(int frac, int rate)
{
	IM_Rate = rate;
	IM_Frac = frac;
	IMptr = IMStack;
	DxIMptr = DxIMStack;
	memcpy(IMStack, MatrixPtr, 48);
	memcpy(DxIMStack, DxMatrixPtr, 48);
}

void phd_PushMatrix(void)
{
	memcpy((MatrixPtr + 12), MatrixPtr, 48);
	MatrixPtr += 12;
	DxMatrixPtr += 12;
}

void phd_PopMatrix(void)
{
	MatrixPtr -= 12;
	DxMatrixPtr -= 12;
}

void phd_PopMatrix_I(void)
{
	MatrixPtr -= 12;
	DxMatrixPtr -= 12;
	IMptr -= 12;
	DxIMptr -= 12;
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

void phd_RotYXZ(short ry, short rx, short rz)
{
	int	sina, cosa, r0, r1;
	int* mptr = MatrixPtr;
	
	if (ry)
	{
		sina = SIN(ry);
		cosa = COS(ry);
		r0 = *(mptr + M00) * cosa - *(mptr + M02) * sina;
		r1 = *(mptr + M02) * cosa + *(mptr + M00) * sina;
		*(mptr + M00) = r0 >> W2V_SHIFT;
		*(mptr + M02) = r1 >> W2V_SHIFT;
		r0 = *(mptr + M10) * cosa - *(mptr + M12) * sina;
		r1 = *(mptr + M12) * cosa + *(mptr + M10) * sina;
		*(mptr + M10) = r0 >> W2V_SHIFT;
		*(mptr + M12) = r1 >> W2V_SHIFT;
		r0 = *(mptr + M20) * cosa - *(mptr + M22) * sina;
		r1 = *(mptr + M22) * cosa + *(mptr + M20) * sina;
		*(mptr + M20) = r0 >> W2V_SHIFT;
		*(mptr + M22) = r1 >> W2V_SHIFT;
	}

	if (rx)
	{
		sina = SIN(rx);
		cosa = COS(rx);
		r0 = *(mptr + M01) * cosa + *(mptr + M02) * sina;
		r1 = *(mptr + M02) * cosa - *(mptr + M01) * sina;
		*(mptr + M01) = r0 >> W2V_SHIFT;
		*(mptr + M02) = r1 >> W2V_SHIFT;
		r0 = *(mptr + M11) * cosa + *(mptr + M12) * sina;
		r1 = *(mptr + M12) * cosa - *(mptr + M11) * sina;
		*(mptr + M11) = r0 >> W2V_SHIFT;
		*(mptr + M12) = r1 >> W2V_SHIFT;
		r0 = *(mptr + M21) * cosa + *(mptr + M22) * sina;
		r1 = *(mptr + M22) * cosa - *(mptr + M21) * sina;
		*(mptr + M21) = r0 >> W2V_SHIFT;
		*(mptr + M22) = r1 >> W2V_SHIFT;
	}

	if (rz)
	{
		sina = SIN(rz);
		cosa = COS(rz);
		r0 = *(mptr + M00) * cosa + *(mptr + M01) * sina;
		r1 = *(mptr + M01) * cosa - *(mptr + M00) * sina;
		*(mptr + M00) = r0 >> W2V_SHIFT;
		*(mptr + M01) = r1 >> W2V_SHIFT;
		r0 = *(mptr + M10) * cosa + *(mptr + M11) * sina;
		r1 = *(mptr + M11) * cosa - *(mptr + M10) * sina;
		*(mptr + M10) = r0 >> W2V_SHIFT;
		*(mptr + M11) = r1 >> W2V_SHIFT;
		r0 = *(mptr + M20) * cosa + *(mptr + M21) * sina;
		r1 = *(mptr + M21) * cosa - *(mptr + M20) * sina;
		*(mptr + M20) = r0 >> W2V_SHIFT;
		*(mptr + M21) = r1 >> W2V_SHIFT;
	}
}

void phd_RotY(short ry)
{
	int	sina, cosa, r0, r1;
	int* mptr = MatrixPtr;

	if (ry)
	{
		sina = SIN(ry);
		cosa = COS(ry);
		r0 = *(mptr + M00) * cosa - *(mptr + M02) * sina;
		r1 = *(mptr + M02) * cosa + *(mptr + M00) * sina;
		*(mptr + M00) = r0 >> W2V_SHIFT;
		*(mptr + M02) = r1 >> W2V_SHIFT;
		r0 = *(mptr + M10) * cosa - *(mptr + M12) * sina;
		r1 = *(mptr + M12) * cosa + *(mptr + M10) * sina;
		*(mptr + M10) = r0 >> W2V_SHIFT;
		*(mptr + M12) = r1 >> W2V_SHIFT;
		r0 = *(mptr + M20) * cosa - *(mptr + M22) * sina;
		r1 = *(mptr + M22) * cosa + *(mptr + M20) * sina;
		*(mptr + M20) = r0 >> W2V_SHIFT;
		*(mptr + M22) = r1 >> W2V_SHIFT;
	}
}

void phd_RotY_I(short ry)
{
	phd_RotY(ry);

	int* mptr = MatrixPtr;
	int* dxptr = DxMatrixPtr;
	
	MatrixPtr = IMptr;
	DxMatrixPtr = DxIMptr;
	
	phd_RotY(ry);
	
	MatrixPtr = mptr;
	DxMatrixPtr = dxptr;
}

void phd_RotX(short rx)
{
	int	sina, cosa, r0, r1;
	int* mptr = MatrixPtr;

	if (rx)
	{
		sina = SIN(rx);
		cosa = COS(rx);
		r0 = *(mptr + M01) * cosa + *(mptr + M02) * sina;
		r1 = *(mptr + M02) * cosa - *(mptr + M01) * sina;
		*(mptr + M01) = r0 >> W2V_SHIFT;
		*(mptr + M02) = r1 >> W2V_SHIFT;
		r0 = *(mptr + M11) * cosa + *(mptr + M12) * sina;
		r1 = *(mptr + M12) * cosa - *(mptr + M11) * sina;
		*(mptr + M11) = r0 >> W2V_SHIFT;
		*(mptr + M12) = r1 >> W2V_SHIFT;
		r0 = *(mptr + M21) * cosa + *(mptr + M22) * sina;
		r1 = *(mptr + M22) * cosa - *(mptr + M21) * sina;
		*(mptr + M21) = r0 >> W2V_SHIFT;
		*(mptr + M22) = r1 >> W2V_SHIFT;
	}
}

void phd_RotX_I(short rx)
{
	phd_RotX(rx);

	int* mptr = MatrixPtr;
	int* dxptr = DxMatrixPtr;

	MatrixPtr = IMptr;
	DxMatrixPtr = DxIMptr;

	phd_RotX(rx);

	MatrixPtr = mptr;
	DxMatrixPtr = dxptr;
}

void phd_RotZ(short rz)
{
	int	sina, cosa, r0, r1;
	int* mptr = MatrixPtr;

	if (rz)
	{
		sina = SIN(rz);
		cosa = COS(rz);
		r0 = *(mptr + M00) * cosa + *(mptr + M01) * sina;
		r1 = *(mptr + M01) * cosa - *(mptr + M00) * sina;
		*(mptr + M00) = r0 >> W2V_SHIFT;
		*(mptr + M01) = r1 >> W2V_SHIFT;
		r0 = *(mptr + M10) * cosa + *(mptr + M11) * sina;
		r1 = *(mptr + M11) * cosa - *(mptr + M10) * sina;
		*(mptr + M10) = r0 >> W2V_SHIFT;
		*(mptr + M11) = r1 >> W2V_SHIFT;
		r0 = *(mptr + M20) * cosa + *(mptr + M21) * sina;
		r1 = *(mptr + M21) * cosa - *(mptr + M20) * sina;
		*(mptr + M20) = r0 >> W2V_SHIFT;
		*(mptr + M21) = r1 >> W2V_SHIFT;
	}
}

void phd_RotZ_I(short rz)
{
	phd_RotZ(rz);

	int* mptr = MatrixPtr;
	int* dxptr = DxMatrixPtr;

	MatrixPtr = IMptr;
	DxMatrixPtr = DxIMptr;

	phd_RotZ(rz);

	MatrixPtr = mptr;
	DxMatrixPtr = dxptr;
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

void Inject_Draw()
{
	INJECT(0x0048F9C0, phd_PushMatrix);
	INJECT(0x0042CF80, GetBoundsAccurate);
	INJECT(0x0042D020, GetBestFrame);
	INJECT(0x004D1A40, Sync);
	INJECT(0x0042BE90, InitInterpolate);
	INJECT(0x0042BF00, phd_PopMatrix_I);
	INJECT(0x00490150, phd_RotYXZ);
	INJECT(0x0048FBE0, phd_RotX);
	INJECT(0x0048FCD0, phd_RotY);
	INJECT(0x0048FDC0, phd_RotZ); 
	INJECT(0x0042C030, phd_RotX_I);
	INJECT(0x0042BFC0, phd_RotY_I);
	INJECT(0x0042C0A0, phd_RotZ_I);
}