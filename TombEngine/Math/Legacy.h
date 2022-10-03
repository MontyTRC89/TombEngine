#pragma once
#include "Math/Containers/BoundingBox.h" // TODO: Including this here shouldn't be necessary.

struct PoseData;

constexpr auto FP_SHIFT	 = 16;
constexpr auto W2V_SHIFT = 14;

enum MatrixArrayValue
{
	M00, M01, M02, M03,
	M10, M11, M12, M13,
	M20, M21, M22, M23
};

struct ColorData
{
	byte r, g, b;
	byte cd;
};

struct Matrix3D
{
	short m00, m01, m02;
	short m10, m11, m12;
	short m20, m21, m22;
	short pad;
	int tx, ty, tz;
};

short ANGLE(float angle);
short FROM_DEGREES(float angle);
short FROM_RAD(float angle);
float TO_DEGREES(short angle);
float TO_RAD(short angle);

float phd_sin(short a);
float phd_cos(short a);
int	  phd_atan(int dz, int dx);

void phd_RotBoundingBoxNoPersp(PoseData* pose, BOUNDING_BOX* bounds, BOUNDING_BOX* tBounds);

void InterpolateAngle(short angle, short* rotation, short* outAngle, int shift);
void GetMatrixFromTrAngle(Matrix* matrix, short* framePtr, int index);
