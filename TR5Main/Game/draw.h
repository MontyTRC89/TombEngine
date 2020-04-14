#pragma once

#include "..\Global\global.h"

#define CalcLaraMatrices ((void (__cdecl*)(int)) 0x0041E120)
#define IsRoomOutside ((int (__cdecl*)(int, int, int)) 0x00418E90) 
#define sub_42B4C0 ((void (__cdecl*)(ITEM_INFO*,short*)) 0x0042B4C0)

#define gar_RotYXZsuperpack ((void(__cdecl*)(short**,int)) 0x0042C310)
#define gar_RotYXZsuperpack_I ((void(__cdecl*)(short**,short**,int)) 0x0042C290)
#define phd_RotBoundingBoxNoPersp ((void (__cdecl*)(short*, short*)) 0x0042E240)

//#define InitInterpolate ((void(__cdecl*)(int,int)) 0x0042BE90)
//#define Sync ((int (__cdecl*)()) 0x004D1A40)
//#define GetBoundsAccurate ((short* (__cdecl*)(ITEM_INFO*)) 0x0042CF80)
//#define GetBestFrame ((short* (__cdecl*)(ITEM_INFO*)) 0x0042D020)
//#define phd_PushMatrix ((void(__cdecl*)(void)) 0x0048F9C0)
//#define phd_RotYXZ ((void(__cdecl*)(short, short, short)) 0x00490150)
//#define phd_TranslateRel ((void(__cdecl*)(int, int, int)) 0x0048FB20)
//#define phd_TranslateRel_I ((void(__cdecl*)(int, int, int)) 0x0042C110)
//#define phd_TranslateRel_ID ((void(__cdecl*)(int,int,int,int,int,int)) 0x0042C190)
//#define phd_TranslateAbs ((void(__cdecl*)(int, int, int)) 0x004903F0)
//#define phd_PopMatrix_I ((void(__cdecl*)(void)) 0x0042BF00)
//#define phd_RotY ((void(__cdecl*)(short)) 0x0048FCD0)
//#define phd_RotX ((void(__cdecl*)(short)) 0x0048FBE0)
//#define phd_RotZ ((void(__cdecl*)(short)) 0x0048FDC0)
//#define phd_RotY_I ((void(__cdecl*)(short)) 0x0042BFC0)
//#define phd_RotX_I ((void(__cdecl*)(short)) 0x0042C030)
//#define phd_RotZ_I ((void(__cdecl*)(short)) 0x0042C0A0)
//#define phd_SetTrans ((void (__cdecl*)(int, int, int)) 0x0048FA40)
//#define phd_PushMatrix_I ((void(__cdecl*)(void)) 0x0042BF50)
//#define phd_PushUnitMatrix ((void (__cdecl*)()) 0x0048FA90)
//#define phd_GetVectorAngles ((void(__cdecl*)(int, int, int, short*)) 0x004904B0)
//#define phd_RotYXZpack ((void(__cdecl*)(int)) 0x0048FEB0)

#define mGetAngle ((int(__cdecl*)(int, int, int, int)) 0x0048F290)
#define phd_GenerateW2V ((void(__cdecl*)(PHD_3DPOS*)) 0x0048F330)

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

/* phd function (DX11 version) */
// TODO: phd_XX function after this lign is used with DX11 one, when it's finished, delete the #define one and delete this sign: "_" at the beginning of the new function. (delete the unused if needed)

void _gar_RotYXZsuperpack(short** pproc, int skip);
void _gar_RotYXZsuperpack_I(short** pproc1, short** pproc2, int skip);

extern Renderer11* g_Renderer;
extern BITE_INFO EnemyBites[9];

void Inject_Draw();