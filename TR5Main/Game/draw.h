#pragma once

#include "..\Global\global.h"

///#define DrawAnimatingItem ((void (__cdecl*)(ITEM_INFO*)) 0x0042B900)
#define GetBoundsAccurate ((short* (__cdecl*)(ITEM_INFO*)) 0x0042CF80)
#define GetBestFrame ((short* (__cdecl*)(ITEM_INFO*)) 0x0042D020)
#define CalcLaraMatrices ((void (__cdecl*)(int)) 0x0041E120)
#define Sync ((int (__cdecl*)()) 0x004D1A40)
//#define GetRoomBounds ((void (__cdecl*)()) 0x0042D4F0) 
#define UpdateStorm ((void (__cdecl*)()) 0x0042A310)  
#define IsRoomOutside ((int (__cdecl*)(int, int, int)) 0x00418E90) 
#define DrawBaddieGunFlash ((void (__cdecl*)(ITEM_INFO*)) 0x00466880)
#define sub_42B4C0 ((void (__cdecl*)(ITEM_INFO*,short*)) 0x0042B4C0)

#define InitInterpolate ((void(__cdecl*)(int,int)) 0x0042BE90)
#define phd_PushMatrix ((void(__cdecl*)(void)) 0x0048F9C0)
#define phd_PushMatrix_I ((void(__cdecl*)(void)) 0x0042BF50)
#define phd_PushUnitMatrix ((void (__cdecl*)()) 0x0048FA90)
#define phd_GetVectorAngles ((void(__cdecl*)(int, int, int, short*)) 0x004904B0)
#define phd_RotYXZ ((void(__cdecl*)(short, short, short)) 0x00490150)
#define phd_PutPolygons ((void(__cdecl*)(short*)) 0x004B3F00)
#define phd_PutPolygons_I ((void(__cdecl*)(short*)) 0x0042C3F0)
#define phd_TranslateRel ((void(__cdecl*)(int, int, int)) 0x0048FB20)
#define phd_TranslateRel_I ((void(__cdecl*)(int, int, int)) 0x0042C110)
#define phd_TranslateRel_ID ((void(__cdecl*)(int,int,int,int,int,int)) 0x0042C190)
#define phd_TranslateAbs ((void(__cdecl*)(int, int, int)) 0x004903F0)
#define phd_RotYXZpack ((void(__cdecl*)(int)) 0x0048FEB0)
#define gar_RotYXZsuperpack ((void(__cdecl*)(short**,int)) 0x0042C310)
#define gar_RotYXZsuperpack_I ((void(__cdecl*)(short**,short**,int)) 0x0042C290)
#define phd_ClipBoundingBox ((int(__cdecl*)(short*)) 0x004B7EB0) // int S_GetObjectBounds(frames[0])
#define phd_PopMatrix_I ((void(__cdecl*)(void)) 0x0042BF00)
#define phd_RotY ((void(__cdecl*)(short)) 0x0048FCD0)
#define phd_RotX ((void(__cdecl*)(short)) 0x0048FBE0)
#define phd_RotZ ((void(__cdecl*)(short)) 0x0048FDC0)
#define phd_RotY_I ((void(__cdecl*)(short)) 0x0042BFC0)
#define phd_RotX_I ((void(__cdecl*)(short)) 0x0042C030)
#define phd_RotZ_I ((void(__cdecl*)(short)) 0x0042C0A0)

#define phd_DxTranslateRel ((void(__cdecl*)(int, int, int)) 0x00490790)
#define phd_DxTranslateAbs ((void(__cdecl*)(int, int, int)) 0x00490610)
#define phd_DxRotX ((void(__cdecl*)(short)) 0x00490810)
#define phd_DxRotY ((void(__cdecl*)(short)) 0x004908E0)
#define phd_DxRotZ ((void(__cdecl*)(short)) 0x004909B0)
#define phd_DxRotYXZ ((void(__cdecl*)(int, int, int)) 0x00490AF0)
#define phd_DxRotYXZpack ((void(__cdecl*)(int)) 0x00490A80)

#define mGetAngle ((int(__cdecl*)(int, int, int, int)) 0x0048F290)
#define phd_GenerateW2V ((void(__cdecl*)(PHD_3DPOS*)) 0x0048F330)

int DrawPhaseGame();
void DrawAnimatingItem(ITEM_INFO* item);
int GetFrame_D2(ITEM_INFO* item, short* framePtr[], int* rate);
void CollectRooms(short roomNumber);
void GetRoomBounds();
void SetRoomBounds(short* door, int roomNumber, ROOM_INFO* parent);

/* phd function (DX11 version) */
// TODO: phd_XX function after this lign is used with DX11 one, when it's finished, delete the #define one and delete this sign: "_" at the beginning of the new function. (delete the unused if needed)

void _InitInterpolate(void);
void _phd_PushMatrix(void);
void _phd_PushMatrix_I(void);
void _phd_PushUnitMatrix(void);
void _phd_RotYXZ(short ry, short rx, short rz);
void _phd_RotY(short ry);
void _phd_RotY_I(short ry);
void _phd_RotX(short rx);
void _phd_RotX_I(short rx);
void _phd_RotZ(short rz);
void _phd_RotZ_I(short rz);
void _phd_PutPolygons(void);
void _phd_PutPolygons_I(void);
void _phd_TranslateRel(int x, int y, int z);
void _phd_TranslateRel_I(int x, int y, int z);
void _phd_TranslateRel_ID(int x1, int y1, int z1, int x2, int y2, int z2);
void _phd_TranslateAbs(int x, int y, int z);
void _phd_RotYXZpack(short ry, short rx, short rz);
void _gar_RotYXZsuperpack(short** pproc, int skip);
void _gar_RotYXZsuperpack_I(short** pproc1, short** pproc2, int skip);
void _phd_ClipBoundingBox(short* frames);

void _phd_DxTranslateRel(int x, int y, int z);
void _phd_DxTranslateAbs(int x, int y, int z);
void _phd_DxRotY(short ry);
void _phd_DxRotX(short rx);
void _phd_DxRotZ(short rz);
void _phd_DxRotYXZ(short ry, short rx, short rz);
void _phd_DxRotYXZpack(int rangle);

void phd_PopMatrix(void);
void _phd_PopMatrix_I(void);

extern Renderer11* g_Renderer;

void Inject_Draw();