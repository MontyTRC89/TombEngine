#pragma once

#include "vodoo.h"

#define PI 3.14159265358979323846f
#define RADIAN 0.01745329252f

#define rcossin_tbl	ARRAY_(0x0050B46C, __int16, [1000])

#define SQUARE(x) ((x)*(x))
#define CLAMP(x, a, b) ((x)<(a)?(a):((x)>(b)?(b):(x)))
#define SIGN(x) ((0 < (x)) - ((x) < 0))
#define CLAMPADD(x, a, b) ((x)<(a)?((x)+(a)):((x)>(b)?((x)-(b)):0))

#define ONE_DEGREE 182
#define ANGLE(x) ((x) * 65536.0 / 360.0)
#define TR_ANGLE_TO_DEGREES(x) ((x) / 65536.0 * 360.0)
#define TR_ANGLE_TO_RAD(x) ((x) / 65536.0 * 360.0 * RADIAN)

#define SQRT_ASM ((int (__cdecl*)(int)) 0x0048F980)
#define ATAN ((__int32 (__cdecl*)(__int32, __int32)) 0x0048F8A0)
#define SIN(x) (4 * rcossin_tbl[((int)(x) >> 3) & 0x1FFE])
#define COS(x) (4 * rcossin_tbl[(((int)(x) >> 3) & 0x1FFE) + 1])

#define InitInterpolate ((void(__cdecl*)(__int32,__int32)) 0x0042BE90)
#define phd_PushMatrix ((void(__cdecl*)(void)) 0x0048F9C0)
#define phd_PushMatrix_I ((void(__cdecl*)(void)) 0x0042BF50)
#define phd_PushUnitMatrix ((void (__cdecl*)()) 0x0048FA90)
#define phd_GetVectorAngles ((void(__cdecl*)(__int32, __int32, __int32, __int16*)) 0x004904B0)
#define phd_RotYXZ ((void(__cdecl*)(__int32, __int32, __int32)) 0x00490150)
#define phd_PutPolygons ((void(__cdecl*)(__int16*)) 0x004B3F00)
#define phd_PutPolygons_I ((void(__cdecl*)(__int16*)) 0x0042C3F0)
#define phd_TranslateRel ((void(__cdecl*)(__int32, __int32, __int32)) 0x0048FB20)
#define phd_TranslateRel_I ((void(__cdecl*)(__int32, __int32, __int32)) 0x0042C110)
#define phd_TranslateRel_ID ((void(__cdecl*)(__int32,__int32,__int32,__int32,__int32,__int32)) 0x0042C190)
#define phd_TranslateAbs ((void(__cdecl*)(__int32, __int32, __int32)) 0x004903F0)
#define gar_RotYXZsuperpack ((void(__cdecl*)(__int16**,__int32)) 0x0042C310)
#define gar_RotYXZsuperpack_I ((void(__cdecl*)(__int16**,__int16**,__int32)) 0x0042C290)
#define phd_ClipBoundingBox ((int(__cdecl*)(__int16*)) 0x004B7EB0) // int S_GetObjectBounds(frames[0])
#define phd_PopMatrix() MatrixPtr -= 12
#define phd_PopMatrix_I ((void(__cdecl*)(void)) 0x0042BF00)
#define phd_PopDxMatrix() DxMatrixPtr -= 48
#define CalculateObjectLighting ((void(__cdecl*)(ITEM_INFO*,__int16*)) 0x0042CD50)
#define phd_RotY ((void(__cdecl*)(__int16)) 0x0048FCD0)
#define phd_RotX ((void(__cdecl*)(__int16)) 0x0048FBE0)
#define phd_RotZ ((void(__cdecl*)(__int16)) 0x0048FDC0)
#define phd_RotY_I ((void(__cdecl*)(__int16)) 0x0042BFC0)
#define phd_RotX_I ((void(__cdecl*)(__int16)) 0x0042C030)
#define phd_RotZ_I ((void(__cdecl*)(__int16)) 0x0042C0A0)
#define mGetAngle ((__int32(__cdecl*)(__int32, __int32, __int32, __int32)) 0x0048F290)