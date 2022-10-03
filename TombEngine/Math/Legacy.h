#pragma once
#include "Math/Containers/PoseData.h"

struct Vector3i;

constexpr auto FP_SHIFT	 = 16;
constexpr auto W2V_SHIFT = 14;

struct VECTOR
{
	int vx;
	int vy;
	int vz;
	int pad;
};

struct CVECTOR
{
	byte r;
	byte g;
	byte b;
	byte cd;
};

struct TR_VERTEX
{
	int x;
	int y;
	int z;
};

enum MATRIX_ARRAY_VALUE
{
	M00, M01, M02, M03,
	M10, M11, M12, M13,
	M20, M21, M22, M23
};

struct MATRIX3D
{
	short m00;
	short m01;
	short m02;
	short m10;
	short m11;
	short m12;
	short m20;
	short m21;
	short m22;
	short pad;
	int tx;
	int ty;
	int tz;
};

struct BOUNDING_BOX
{
	// Components
	short X1;
	short X2;
	short Y1;
	short Y2;
	short Z1;
	short Z2;

	// Utilities
	int Height() const;

	// Converters
	BoundingBox			ToDXBoundingBox() const;
	BoundingOrientedBox ToDXBoundingOrientedBox(PoseData pose) const;

	// Operators
	BOUNDING_BOX operator +(const PoseData& pose) const;
	BOUNDING_BOX operator *(float scale) const;
};

short ANGLE(float angle);
short FROM_DEGREES(float angle);
short FROM_RAD(float angle);
float TO_DEGREES(short angle);
float TO_RAD(short angle);

float phd_sin(short a);
float phd_cos(short a);
int	  phd_atan(int dz, int dx);

BoundingOrientedBox TO_DX_BBOX(PoseData pose, BOUNDING_BOX* bBox);

void phd_RotBoundingBoxNoPersp(PoseData* pose, BOUNDING_BOX* bounds, BOUNDING_BOX* tBounds);

void InterpolateAngle(short angle, short* rotation, short* outAngle, int shift);
void GetMatrixFromTrAngle(Matrix* matrix, short* framePtr, int index);
