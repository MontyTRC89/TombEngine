#pragma once

#include "vodoo.h"
#include "types.h"

#define PI 3.14159265358979323846f
#define RADIAN 0.01745329252f
#define SQUARE(x) ((x)*(x))
#define CLAMP(x, a, b) ((x)<(a)?(a):((x)>(b)?(b):(x)))
#define SIGN(x) ((0 < (x)) - ((x) < 0))
#define CLAMPADD(x, a, b) ((x)<(a)?((x)+(a)):((x)>(b)?((x)-(b)):0))
#define ONE_DEGREE 182
#define CLICK(x) ((x) * STEP_SIZE)
#define SECTOR(x) ((x) * WALL_SIZE)
#define TO_DEGREES(x) ((x) / 65536.0f * 360.0f)
#define TO_RAD(x) ((x) / 65536.0f * 360.0f * RADIAN)

//#define ATAN ((int(__cdecl*)(int, int)) 0x0048F8A0)

extern short rcossin_tbl[8192];

int phd_sin(short a);
int phd_cos(short a);
short ANGLE(double angle);
float ANGLEF(short angle);
// returns a float between 0-1
const float frand();
const float frandMinMax(float min, float max);
const float lerp(float v0, float v1, float t);
int mGetAngle(int x1, int y1, int x2, int y2);
int phd_atan(int dz, int dx);
void phd_GetVectorAngles(int x, int y, int z, short* angles);
void phd_RotBoundingBoxNoPersp(PHD_3DPOS* pos, short* bounds, short* tbounds);