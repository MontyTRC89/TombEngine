#pragma once

#include "vodoo.h"

#define rcossin_tbl	ARRAY_(0x0050B46C, __int16, [1000])

#define SQUARE(x) ((x)*(x))
#define CLAMP(x, a, b) ((x)<(a)?(a):((x)>(b)?(b):(x)))
#define SIGN(x) ((0 < (x)) - ((x) < 0))
#define CLAMPADD(x, a, b) ((x)<(a)?((x)+(a)):((x)>(b)?((x)-(b)):0))

#define ONE_DEGREE 182
#define ANGLE(x) ((x) * 65536.0 / 360.0)
#define SECTOR(x) ((x) * WALL_SIZE)
#define TR_ANGLE_TO_DEGREES(x) ((x) / 65536.0 * 360.0)
#define TR_ANGLE_TO_RAD(x) ((x) / 65536.0 * 360.0 * RADIAN)

#define SQRT_ASM ((__int32 (__cdecl*)(__int32)) 0x0048F980)
#define ATAN ((__int32 (__cdecl*)(__int32, __int32)) 0x0048F8A0)
#define SIN(x) (4 * rcossin_tbl[((__int32)(x) >> 3) & 0x1FFE])
#define COS(x) (4 * rcossin_tbl[(((__int32)(x) >> 3) & 0x1FFE) + 1])

#define phd_GetVectorAngles ((void(__cdecl*)(__int32, __int32, __int32, __int16*)) 0x004904B0)
#define phd_RotYXZ ((void(__cdecl*)(__int32, __int32, __int32)) 0x00490150)
#define phd_TranslateRel ((void(__cdecl*)(__int32, __int32, __int32)) 0x0048FB20)
#define mGetAngle ((__int32(__cdecl*)(__int32, __int32, __int32, __int32)) 0x0048F290)
#define phd_GenerateW2V ((void(__cdecl*)(PHD_3DPOS*)) 0x0048F330)

#define PI 3.14159265358979323846f
#define RADIAN 0.01745329252f