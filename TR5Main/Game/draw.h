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

/*/
extern Matrix IMStack[64];
extern Matrix* IMptr;
extern int IM_Rate;
extern int IM_Frac;
extern Matrix MatrixStack[64];
extern Matrix* MatrixPtr;
extern Matrix* W2VMatrix;*/

#define phd_PushMatrix ((void(__cdecl*)(void)) 0x0048F9C0)
#define phd_RotYXZ ((void(__cdecl*)(short, short, short)) 0x00490150)
#define phd_TranslateRel ((void(__cdecl*)(int, int, int)) 0x0048FB20)
#define phd_TranslateRel_I ((void(__cdecl*)(int, int, int)) 0x0042C110)
#define phd_TranslateRel_ID ((void(__cdecl*)(int,int,int,int,int,int)) 0x0042C190)
#define phd_TranslateAbs ((void(__cdecl*)(int, int, int)) 0x004903F0)
#define phd_PopMatrix_I ((void(__cdecl*)(void)) 0x0042BF00)
#define phd_RotY ((void(__cdecl*)(short)) 0x0048FCD0)
#define phd_RotX ((void(__cdecl*)(short)) 0x0048FBE0)
#define phd_RotZ ((void(__cdecl*)(short)) 0x0048FDC0)
#define phd_RotY_I ((void(__cdecl*)(short)) 0x0042BFC0)
#define phd_RotX_I ((void(__cdecl*)(short)) 0x0042C030)
#define phd_RotZ_I ((void(__cdecl*)(short)) 0x0042C0A0)
#define phd_SetTrans ((void (__cdecl*)(int, int, int)) 0x0048FA40)
#define phd_PushMatrix_I ((void(__cdecl*)(void)) 0x0042BF50)
#define phd_PushUnitMatrix ((void (__cdecl*)()) 0x0048FA90)
#define phd_GetVectorAngles ((void(__cdecl*)(int, int, int, short*)) 0x004904B0)
#define phd_RotYXZpack ((void(__cdecl*)(int)) 0x0048FEB0)
#define gar_RotYXZsuperpack ((void(__cdecl*)(short**,int)) 0x0042C310)
#define gar_RotYXZsuperpack_I ((void(__cdecl*)(short**,short**,int)) 0x0042C290)
#define phd_GenerateW2V ((void(__cdecl*)(PHD_3DPOS*)) 0x0048F330)

int DrawPhaseGame();
int GetFrame_D2(ITEM_INFO* item, short* framePtr[], int* rate);
void UpdateStorm();
short* GetBoundsAccurate(ITEM_INFO* item);
short* GetBestFrame(ITEM_INFO* item);
int Sync();
bool TIME_Init();
bool TIME_Reset();
void DrawAnimatingItem(ITEM_INFO* item);
//void InitInterpolate(int frac, int rate);
void phd_PopMatrix();
void phd_RotBoundingBoxNoPersp(PHD_3DPOS* pos, short* bounds, short* tbounds);
void GetLaraJointPosition(PHD_VECTOR* pos, int joint);
//void phd_GenerateW2V(PHD_3DPOS* viewpos);

void Inject_Draw();