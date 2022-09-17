#pragma once

struct BOUNDING_BOX;
struct MATRIX3D;
struct PoseData;
struct Vector3i;

short ANGLE(float angle);
short FROM_DEGREES(float angle);
short FROM_RAD(float angle);
float TO_DEGREES(short angle);
float TO_RAD(short angle);

float phd_sin(short a);
float phd_cos(short a);
int phd_atan(int dz, int dx);

BoundingOrientedBox TO_DX_BBOX(PoseData pos, BOUNDING_BOX* box);

const Vector3 GetRandomVector();
const Vector3 GetRandomVectorInCone(const Vector3& direction, const float angleDegrees);
int mGetAngle(int x1, int y1, int x2, int y2);
void phd_RotBoundingBoxNoPersp(PoseData* pos, BOUNDING_BOX* bounds, BOUNDING_BOX* tbounds);

void InterpolateAngle(short angle, short* rotation, short* outAngle, int shift);
void GetMatrixFromTrAngle(Matrix* matrix, short* framePtr, int index);

constexpr auto FP_SHIFT = 16;
constexpr auto FP_ONE = (1 << FP_SHIFT);
constexpr auto W2V_SHIFT = 14;
