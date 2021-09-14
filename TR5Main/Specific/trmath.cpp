#include "framework.h"
#include "Specific\trmath.h"
#include <cmath>
#include "Specific\prng.h"

using namespace TEN::Math::Random;

short ANGLE(float angle)
{
	return angle * 65536.0f / 360.0f;
}

short FROM_DEGREES(float angle)
{
	return angle * 65536.0f / 360.0f;
}

short FROM_RAD(float angle)
{
	return angle / RADIAN * 65536.0f / 360.0f;
}

float TO_DEGREES(short angle)
{
	return lround(angle * 360.0f / 65536.0f);
}

float TO_RAD(short angle)
{
	return angle * 360.0f / 65536.0f * RADIAN;
}

const float lerp(float v0, float v1, float t)
{
	return (1 - t) * v0 + t * v1;
}

const Vector3 getRandomVector()
{
	Vector3 v = {generateFloat(-1,1),generateFloat(-1,1),generateFloat(-1,1)};
	v.Normalize();
	return v;
}

const Vector3 getRandomVectorInCone(const Vector3& direction, const float angleDegrees)
{
	float x = generateFloat(-angleDegrees, angleDegrees) * RADIAN;
	float y = generateFloat(-angleDegrees, angleDegrees) * RADIAN;
	float z = generateFloat(-angleDegrees, angleDegrees) * RADIAN;
	Matrix m = Matrix::CreateRotationX(x)* Matrix::CreateRotationY(y) * Matrix::CreateRotationZ(z);
	Vector3 result = direction.TransformNormal(direction, m);
	result.Normalize();
	return result;
}

float phd_sin(short a)
{
	return sin(TO_RAD(a));
}

float phd_cos(short a)
{
	return cos(TO_RAD(a));
}

int mGetAngle(int x1, int y1, int x2, int y2)
{
	return (65536 - phd_atan(x2 - x1, y2 - y1)) % 65536;
}

int phd_atan(int x, int y)
{
	return FROM_RAD(atan2(y, x));
}

void phd_GetVectorAngles(int x, int y, int z, short* angles)
{
	const auto angle = atan2(x, z);

	auto vector = Vector3(x, y, z);
	const auto matrix = Matrix::CreateRotationY(-angle);
	Vector3::Transform(vector, matrix, vector);

	angles[0] = FROM_RAD(angle);
	angles[1] = FROM_RAD(-atan2(y, vector.z));
}

void phd_RotBoundingBoxNoPersp(PHD_3DPOS* pos, BOUNDING_BOX* bounds, BOUNDING_BOX* tbounds)
{
	Matrix world = Matrix::CreateFromYawPitchRoll(
		TO_RAD(pos->yRot),
		TO_RAD(pos->xRot),
		TO_RAD(pos->zRot)
	);

	Vector3 bMin = Vector3(bounds->X1, bounds->Y1, bounds->Z1);
	Vector3 bMax = Vector3(bounds->X2, bounds->Y2, bounds->Z2);

	bMin = Vector3::Transform(bMin, world);
	bMax = Vector3::Transform(bMax, world);

	tbounds->X1 = bMin.x;
	tbounds->X2 = bMax.x;
	tbounds->Y1 = bMin.y;
	tbounds->Y2 = bMax.y;
	tbounds->Z1 = bMin.z;
	tbounds->Z2 = bMax.z;
}

BoundingOrientedBox TO_DX_BBOX(PHD_3DPOS pos, BOUNDING_BOX* box)
{
	Vector3 boxCentre = Vector3((box->X2 + box->X1) / 2.0f, (box->Y2 + box->Y1) / 2.0f, (box->Z2 + box->Z1) / 2.0f);
	Vector3 boxExtent = Vector3((box->X2 - box->X1) / 2.0f, (box->Y2 - box->Y1) / 2.0f, (box->Z2 - box->Z1) / 2.0f);
	Quaternion rotation = Quaternion::CreateFromYawPitchRoll(TO_RAD(pos.yRot), TO_RAD(pos.xRot), TO_RAD(pos.zRot));

	BoundingOrientedBox result;
	BoundingOrientedBox(boxCentre, boxExtent, Vector4::UnitY).Transform(result, 1, rotation, Vector3(pos.xPos, pos.yPos, pos.zPos));
	return result;
}