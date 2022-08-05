#include "framework.h"
#include "Specific/trmath.h"

#include <cmath>
#include "Specific/prng.h"

using namespace TEN::Math::Random;
const Vector3 GetRandomVector()
{
	auto vector = Vector3(GenerateFloat(-1, 1), GenerateFloat(-1, 1), GenerateFloat(-1, 1));
	vector.Normalize();
	return vector;
}

const Vector3 GetRandomVectorInCone(const Vector3& direction, const float angleDegrees)
{
	float x = GenerateFloat(-angleDegrees, angleDegrees) * RADIAN;
	float y = GenerateFloat(-angleDegrees, angleDegrees) * RADIAN;
	float z = GenerateFloat(-angleDegrees, angleDegrees) * RADIAN;
	auto matrix = Matrix::CreateRotationX(x) * Matrix::CreateRotationY(y) * Matrix::CreateRotationZ(z);
	
	auto result = direction.TransformNormal(direction, matrix);
	result.Normalize();
	return result;
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

const float Lerp(float v0, float v1, float t)
{
	return (1.0f - t) * v0 + t * v1;
}

const float Smoothstep(float edge0, float edge1, float x)
{
	x = std::clamp(x, edge0, edge1);

	// Don't process if input value is the same as one of edges
	if (x == edge0)
		return edge0;
	else if (x == edge1)
		return edge1;

	// Scale, bias and saturate x to 0..1 range
	x = std::clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);

	// Evaluate polynomial
	return x * x * x * (x * (x * 6 - 15) + 10);
}

const float Smoothstep(float x)
{
	return Smoothstep(0.0f, 1.0f, x);
}

const float Luma(Vector3& color)
{
	// Use Rec.709 trichromat formula to get perceptive luma value
	return (float)((color.x * 0.2126f) + (color.y * 0.7152f) + (color.z * 0.0722f));
}

const Vector4 Screen(Vector4& ambient, Vector4& tint)
{
	auto result = Screen(Vector3(ambient.x, ambient.y, ambient.z), Vector3(tint.x, tint.y, tint.z));
	return Vector4(result.x, result.y, result.z, ambient.w * tint.w);
}

const Vector3 Screen(Vector3& ambient, Vector3& tint)
{
	auto luma = Luma(tint);

	auto multiplicative = ambient * tint;
	auto additive = ambient + tint;

	auto R = (float)Lerp(multiplicative.x, additive.x, luma);
	auto G = (float)Lerp(multiplicative.y, additive.y, luma);
	auto B = (float)Lerp(multiplicative.z, additive.z, luma);

	return Vector3(R, G, B);
}
