#pragma once

#include "..\Global\global.h"

#define CalcLaraMatrices ((void (__cdecl*)(int)) 0x0041E120)
#define mGetAngle ((int(__cdecl*)(int, int, int, int)) 0x0048F290)

extern Renderer11* g_Renderer;
extern BITE_INFO EnemyBites[9];
extern int LightningCount;
extern int LightningRand;
extern int StormTimer;
extern int dLightningRand;
extern byte SkyStormColor[3];
extern byte SkyStormColor2[3];

int DrawPhaseGame();
int GetFrame_D2(ITEM_INFO* item, short* framePtr[], int* rate);
void UpdateStorm();
short* GetBoundsAccurate(ITEM_INFO* item);
short* GetBestFrame(ITEM_INFO* item);
int Sync();
bool TIME_Init();
bool TIME_Reset();
void DrawAnimatingItem(ITEM_INFO* item);
void phd_PushMatrix();
void InitInterpolate(int frac, int rate);
void phd_PopMatrix();
void phd_PopMatrix_I();
void phd_RotYXZ(short ry, short rx, short rz);
void phd_RotX(short ry);
void phd_RotY(short ry);
void phd_RotZ(short ry);
void phd_RotY_I(short ry);
void phd_RotX_I(short rx);
void phd_RotZ_I(short rz);
void phd_TranslateRel(int x, int y, int z);
void phd_TranslateRel_I(int x, int y, int z);
void phd_TranslateRel_ID(int x1, int y1, int z1, int x2, int y2, int z2);
void phd_TranslateAbs(int x, int y, int z);
void phd_SetTrans(int x, int y, int z);
void phd_PushMatrix_I();
void phd_PushUnitMatrix();
void phd_GetVectorAngles(int x, int y, int z, short* angles);
void phd_RotYXZpack(int rots);
void gar_RotYXZsuperpack(short** framePtr, int skip);
void gar_RotYXZsuperpack_I(short** framePtr1, short** framePtr2, int skip);
void phd_RotBoundingBoxNoPersp(PHD_3DPOS* pos, short* bounds, short* tbounds);

void Inject_Draw();