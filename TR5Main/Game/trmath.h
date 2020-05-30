#pragma once
#include "phd_global.h"

constexpr auto PI = 3.14159265358979323846f;
constexpr auto RADIAN = 0.01745329252f;
constexpr auto ONE_DEGREE = 182;
constexpr auto PREDICTIVE_SCALE_FACTOR = 14;
constexpr auto W2V_SHIFT = 14; // Shift scale of View.Frame to World.Frame
constexpr auto NODE_SHIFT = 15;
constexpr auto W2V_SCALE = (1 << W2V_SHIFT); // Scale of View Frame to World Frame
constexpr auto WALL_SHIFT = 10;
constexpr auto STEP_SIZE = 256;
constexpr auto WALL_SIZE = 1024;
constexpr auto STEPUP_HEIGHT = ((STEP_SIZE * 3) / 2);
constexpr auto BAD_JUMP_CEILING = ((STEP_SIZE * 3) / 4);

#define SQUARE(x) ((x)*(x))
#define CLAMP(x, a, b) ((x)<(a)?(a):((x)>(b)?(b):(x)))
#define SIGN(x) ((0 < (x)) - ((x) < 0))
#define CLAMPADD(x, a, b) ((x)<(a)?((x)+(a)):((x)>(b)?((x)-(b)):0))
#define CLICK(x) ((x) * STEP_SIZE)
#define SECTOR(x) ((x) * WALL_SIZE)
#define HIDWORD(l) ((DWORD)(((DWORDLONG)(l)>>32)&0xFFFFFFFF))

short ANGLE(float angle);
float TO_DEGREES(short angle);
float TO_RAD(short angle);

extern short rcossin_tbl[8192];

int phd_sin(short a);
int phd_cos(short a);

// returns a float between 0-1
const float frand();
const float frandMinMax(float min, float max);
const float lerp(float v0, float v1, float t);
int mGetAngle(int x1, int y1, int x2, int y2);
int phd_atan(int dz, int dx);
void phd_GetVectorAngles(int x, int y, int z, short* angles);
void phd_RotBoundingBoxNoPersp(PHD_3DPOS* pos, short* bounds, short* tbounds);