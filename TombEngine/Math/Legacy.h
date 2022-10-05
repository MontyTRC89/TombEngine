#pragma once
#include "Math/Containers/BoundingBox.h" // TODO: Including this here shouldn't be necessary.

struct PoseData;

constexpr auto FP_SHIFT	 = 16;
constexpr auto W2V_SHIFT = 14;

struct ColorData
{
	byte r, g, b;
	byte cd;
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
