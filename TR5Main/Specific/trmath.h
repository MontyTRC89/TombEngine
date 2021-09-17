#pragma once
#include "Specific\phd_global.h"

struct VectorInt2
{
	int x;
	int y;
};

struct VectorInt3
{
	int x;
	int y;
	int z;
};

constexpr auto PI = 3.14159265358979323846f;
constexpr auto RADIAN = 0.01745329252f;
constexpr auto ONE_DEGREE = 182;
constexpr auto PREDICTIVE_SCALE_FACTOR = 14;
constexpr auto WALL_SIZE = 1024;
constexpr auto STEP_SIZE = WALL_SIZE / 4;
constexpr auto STOP_SIZE = WALL_SIZE / 2;
constexpr auto STEPUP_HEIGHT = ((STEP_SIZE * 3) / 2);
constexpr auto SWIM_DEPTH = 730;
constexpr auto WADE_DEPTH = STEPUP_HEIGHT;
constexpr auto BAD_JUMP_CEILING = ((STEP_SIZE * 3) / 4);
constexpr auto SLOPE_DIFFERENCE = 60;
constexpr auto NO_HEIGHT  = (-0x7F00);
constexpr auto MAX_HEIGHT = (-0x7FFF);
constexpr auto DEEP_WATER = 0x7FFF;

constexpr auto SQUARE = [](auto x) { return x * x; };
constexpr auto CLICK = [](auto x) { return STEP_SIZE * x; };
constexpr auto SECTOR = [](auto x) { return WALL_SIZE * x; };
constexpr auto MESH_BITS = [](auto x) { return 1 << x; };

short ANGLE(float angle);
short FROM_DEGREES(float angle);
short FROM_RAD(float angle);
float TO_DEGREES(short angle);
float TO_RAD(short angle); 

BoundingOrientedBox TO_DX_BBOX(PHD_3DPOS pos, BOUNDING_BOX* box);

float phd_sin(short a);
float phd_cos(short a);

const float lerp(float v0, float v1, float t);
const Vector3 getRandomVector();
const Vector3 getRandomVectorInCone(const Vector3& direction,const float angleDegrees);
int mGetAngle(int x1, int y1, int x2, int y2);
int phd_atan(int dz, int dx);
void phd_GetVectorAngles(int x, int y, int z, short* angles);
void phd_RotBoundingBoxNoPersp(PHD_3DPOS* pos, BOUNDING_BOX* bounds, BOUNDING_BOX* tbounds);

void InterpolateAngle(short angle, short* rotation, short* outAngle, int shift);
void GetMatrixFromTrAngle(Matrix* matrix, short* frameptr, int index);