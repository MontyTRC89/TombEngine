#include "framework.h"
#include "Specific/EulerAngle.h"
#include "Specific/trmath.h"

#include <cmath>
#include "Specific/prng.h"

using namespace TEN::Math::Random;

float FROM_DEGREES(float angle)//
{
	return angle * (M_PI / 180.0f);
}

float TO_DEGREES(float angle)//
{
	return angle * (M_PI / 180.0f);
}

const float lerp(float v0, float v1, float t)
{
	return (1 - t) * v0 + t * v1;
}

const Vector3 getRandomVector()
{
	auto vector = Vector3(GenerateFloat(-1, 1), GenerateFloat(-1, 1), GenerateFloat(-1, 1));
	vector.Normalize();
	return vector;
}

const Vector3 getRandomVectorInCone(const Vector3& direction, const float angleDegrees)
{
	float x = GenerateFloat(-angleDegrees, angleDegrees) * RADIAN;
	float y = GenerateFloat(-angleDegrees, angleDegrees) * RADIAN;
	float z = GenerateFloat(-angleDegrees, angleDegrees) * RADIAN;
	auto matrix = Matrix::CreateRotationX(x) * Matrix::CreateRotationY(y) * Matrix::CreateRotationZ(z);
	
	auto result = direction.TransformNormal(direction, matrix);
	result.Normalize();
	return result;
}

int mGetAngle(int x1, int y1, int x2, int y2)
{
	return (65536 - (short)atan2(x2 - x1, y2 - y1)) % 65536;
}

void phd_GetVectorAngles(int x, int y, int z, float* angles)
{
	const float angle = atan2(x, z);

	auto vector = Vector3(x, y, z);
	const auto matrix = Matrix::CreateRotationY(-angle);
	Vector3::Transform(vector, matrix, vector);

	angles[0] = angle;
	angles[1] = -atan2(y, vector.z);
}

int phd_Distance(PHD_3DPOS* first, PHD_3DPOS* second)
{
	return (int)round(Vector3::Distance(first->Position.ToVector3(), second->Position.ToVector3()));
}

void phd_RotBoundingBoxNoPersp(PHD_3DPOS* pos, BOUNDING_BOX* bounds, BOUNDING_BOX* tbounds)
{
	auto world = Matrix::CreateFromYawPitchRoll(
		pos->Orientation.y,
		pos->Orientation.x,
		pos->Orientation.z
	);

	auto bMin = Vector3(bounds->X1, bounds->Y1, bounds->Z1);
	auto bMax = Vector3(bounds->X2, bounds->Y2, bounds->Z2);

	bMin = Vector3::Transform(bMin, world);
	bMax = Vector3::Transform(bMax, world);

	tbounds->X1 = bMin.x;
	tbounds->X2 = bMax.x;
	tbounds->Y1 = bMin.y;
	tbounds->Y2 = bMax.y;
	tbounds->Z1 = bMin.z;
	tbounds->Z2 = bMax.z;
}

void InterpolateAngle(short angle, short* rotation, short* outAngle, int shift)
{
	int deltaAngle = angle - *rotation;

	if (deltaAngle < EulerAngle::DegToRad(-180.0f))
		deltaAngle += EulerAngle::DegToRad(360.0f);
	else if (deltaAngle > EulerAngle::DegToRad(180.0f))
		deltaAngle -= EulerAngle::DegToRad(360.0f);

	if (outAngle)
		*outAngle = static_cast<short>(deltaAngle);

	*rotation += static_cast<short>(deltaAngle >> shift);
}

BoundingOrientedBox TO_DX_BBOX(PHD_3DPOS pos, BOUNDING_BOX* box)
{
	auto boxCentre = Vector3((box->X2 + box->X1) / 2.0f, (box->Y2 + box->Y1) / 2.0f, (box->Z2 + box->Z1) / 2.0f);
	auto boxExtent = Vector3((box->X2 - box->X1) / 2.0f, (box->Y2 - box->Y1) / 2.0f, (box->Z2 - box->Z1) / 2.0f);
	auto rotation = Quaternion::CreateFromYawPitchRoll(pos.Orientation.y, pos.Orientation.x, pos.Orientation.z);

	BoundingOrientedBox result;
	BoundingOrientedBox(boxCentre, boxExtent, Vector4::UnitY).Transform(result, 1, rotation, Vector3(pos.Position.x, pos.Position.y, pos.Position.z));
	return result;
}

__int64 FP_Mul(__int64 a, __int64 b)
{
	return (int)((((__int64)a * (__int64)b)) >> FP_SHIFT);
}

__int64 FP_Div(__int64 a, __int64 b)
{
	return (int)(((a) / (b >> 8)) << 8);
}

void FP_VectorMul(Vector3Int* v, int scale, Vector3Int* result)
{
	result->x = FP_FromFixed(v->x * scale);
	result->y = FP_FromFixed(v->y * scale);
	result->z = FP_FromFixed(v->z * scale);
}

int FP_DotProduct(Vector3Int* a, Vector3Int* b)
{
	return ((a->x * b->x) + (a->y * b->y) + (a->z * b->z)) >> W2V_SHIFT;
}

void FP_CrossProduct(Vector3Int* a, Vector3Int* b, Vector3Int* result)
{
	result->x = ((a->y * b->z) - (a->z * b->y)) >> W2V_SHIFT;
	result->y = ((a->z * b->x) - (a->x * b->z)) >> W2V_SHIFT;
	result->z = ((a->x * b->y) - (a->y * b->x)) >> W2V_SHIFT;
}

void FP_GetMatrixAngles(MATRIX3D* m, short* angles)
{
	short yaw = atan2(m->m22, m->m02);
	short pitch = atan2(sqrt((m->m22 * m->m22) + (m->m02 * m->m02)), m->m12);
	
	int sy = sin(yaw);
	int cy = cos(yaw);
	short roll = atan2(((cy * m->m00) - (sy * m->m20)), ((sy * m->m21) - (cy * m->m01)));

	if ((m->m12 >= 0 && pitch > 0) ||
		(m->m12 < 0 && pitch < 0))
	{
		pitch = -pitch;
	}

	angles[0] = pitch;
	angles[1] = yaw;
	angles[2] = roll;
}

__int64 FP_ToFixed(__int64 value)
{
	return (value << FP_SHIFT);
}

__int64 FP_FromFixed(__int64 value)
{
	return (value >> FP_SHIFT);
}

Vector3Int* FP_Normalise(Vector3Int* v)
{
	long a = v->x >> FP_SHIFT;
	long b = v->y >> FP_SHIFT;
	long c = v->z >> FP_SHIFT;

	if (a == 0 && b == 0 && c == 0)	
		return v;

	a = a * a;
	b = b * b;
	c = c * c;
	long d = (a + b + c);
	long e = sqrt(abs(d));

	e <<= FP_SHIFT;

	long mod = FP_Div(FP_ONE << 8, e);
	mod >>= 8;

	v->x = FP_Mul(v->x, mod);
	v->y = FP_Mul(v->y, mod);
	v->z = FP_Mul(v->z, mod);

	return v;
}
