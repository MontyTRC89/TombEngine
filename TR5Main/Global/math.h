#pragma once

#include "vodoo.h"

#define PI 3.14159265358979323846f
#define RADIAN 0.01745329252f

#define rcossin_tbl	ARRAY_(0x0050B46C, short, [1000])

#define SQUARE(x) ((x)*(x))
#define CLAMP(x, a, b) ((x)<(a)?(a):((x)>(b)?(b):(x)))
#define SIGN(x) ((0 < (x)) - ((x) < 0))
#define CLAMPADD(x, a, b) ((x)<(a)?((x)+(a)):((x)>(b)?((x)-(b)):0))

#define ONE_DEGREE 182
short ANGLE(short ang);
#define TR_ANGLE_TO_DEGREES(x) ((x) / 65536.0 * 360.0)
#define TR_ANGLE_TO_RAD(x) ((x) / 65536.0 * 360.0 * RADIAN)

#define SQRT_ASM ((int (__cdecl*)(int)) 0x0048F980)
#define ATAN ((int (__cdecl*)(int, int)) 0x0048F8A0)
#define SIN(x) (4 * rcossin_tbl[((int)(x) >> 3) & 0x1FFE])
#define COS(x) (4 * rcossin_tbl[(((int)(x) >> 3) & 0x1FFE) + 1])

#define CalculateObjectLighting ((void(__cdecl*)(ITEM_INFO*,short*)) 0x0042CD50)
#define mGetAngle ((int(__cdecl*)(int, int, int, int)) 0x0048F290)