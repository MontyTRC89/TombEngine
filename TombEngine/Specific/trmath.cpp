#include "framework.h"
#include "Specific/trmath.h"

#include <cmath>
#include "Specific/prng.h"

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

Vector3Shrt GetVectorAngles(int x, int y, int z)
{
	const float angle = atan2(x, z);

	auto vector = Vector3(x, y, z);
	const auto matrix = Matrix::CreateRotationY(-angle);
	Vector3::Transform(vector, matrix, vector);

	return Vector3Shrt(
		FROM_RAD(-atan2(y, vector.z)),
		FROM_RAD(angle),
		0
	);
}

Vector3Shrt GetOrientBetweenPoints(Vector3Int origin, Vector3Int target)
{
	return GetVectorAngles(target.x - origin.x, target.y - origin.y, target.z - origin.z);
}

int phd_Distance(PHD_3DPOS* first, PHD_3DPOS* second)
{
	return (int)round(Vector3::Distance(first->Position.ToVector3(), second->Position.ToVector3()));
}

void phd_RotBoundingBoxNoPersp(PHD_3DPOS* pos, BOUNDING_BOX* bounds, BOUNDING_BOX* tbounds)
{
	auto world = Matrix::CreateFromYawPitchRoll(
		TO_RAD(pos->Orientation.y),
		TO_RAD(pos->Orientation.x),
		TO_RAD(pos->Orientation.z)
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

	if (deltaAngle < -ANGLE(180.0f))
		deltaAngle += ANGLE(360.0f);
	else if (deltaAngle > ANGLE(180.0f))
		deltaAngle -= ANGLE(360.0f);

	if (outAngle)
		*outAngle = static_cast<short>(deltaAngle);

	*rotation += static_cast<short>(deltaAngle >> shift);
}

void GetMatrixFromTrAngle(Matrix* matrix, short* framePtr, int index)
{
	short* ptr = &framePtr[0];

	ptr += 9;
	for (int i = 0; i < index; i++)
		ptr += ((*ptr & 0xc000) == 0 ? 2 : 1);

	int rot0 = *ptr++;
	int frameMode = (rot0 & 0xc000);

	int rot1;
	int rotX;
	int rotY;
	int rotZ;

	switch (frameMode)
	{
	case 0:
		rot1 = *ptr++;
		rotX = ((rot0 & 0x3ff0) >> 4);
		rotY = (((rot1 & 0xfc00) >> 10) | ((rot0 & 0xf) << 6) & 0x3ff);
		rotZ = ((rot1) & 0x3ff);

		*matrix = Matrix::CreateFromYawPitchRoll(rotY * (360.0f / 1024.0f) * RADIAN,
			rotX * (360.0f / 1024.0f) * RADIAN,
			rotZ * (360.0f / 1024.0f) * RADIAN);
		break;

	case 0x4000:
		*matrix = Matrix::CreateRotationX((rot0 & 0xfff) * (360.0f / 4096.0f) * RADIAN);
		break;

	case 0x8000:
		*matrix = Matrix::CreateRotationY((rot0 & 0xfff) * (360.0f / 4096.0f) * RADIAN);
		break;

	case 0xc000:
		*matrix = Matrix::CreateRotationZ((rot0 & 0xfff) * (360.0f / 4096.0f) * RADIAN);
		break;
	}
}

BoundingOrientedBox TO_DX_BBOX(PHD_3DPOS pos, BOUNDING_BOX* box)
{
	auto boxCentre = Vector3((box->X2 + box->X1) / 2.0f, (box->Y2 + box->Y1) / 2.0f, (box->Z2 + box->Z1) / 2.0f);
	auto boxExtent = Vector3((box->X2 - box->X1) / 2.0f, (box->Y2 - box->Y1) / 2.0f, (box->Z2 - box->Z1) / 2.0f);
	auto rotation = Quaternion::CreateFromYawPitchRoll(TO_RAD(pos.Orientation.y), TO_RAD(pos.Orientation.x), TO_RAD(pos.Orientation.z));

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
	short yaw = phd_atan(m->m22, m->m02);
	short pitch = phd_atan(sqrt((m->m22 * m->m22) + (m->m02 * m->m02)), m->m12);
	
	int sy = phd_sin(yaw);
	int cy = phd_cos(yaw);
	short roll = phd_atan(((cy * m->m00) - (sy * m->m20)), ((sy * m->m21) - (cy * m->m01)));

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

Vector3 TranslateVector(Vector3& vector, short angle, float forward, float up, float right)
{
	if (forward == 0.0f && up == 0.0f && right == 0.0f)
		return vector;

	float sinAngle = phd_sin(angle);
	float cosAngle = phd_cos(angle);

	return Vector3(
		vector.x + (forward * sinAngle) + (right * cosAngle),
		vector.y + up,
		vector.z + (forward * cosAngle) - (right * sinAngle)
	);
}

Vector3Int TranslateVector(Vector3Int& vector, short angle, float forward, float up, float right)
{
	auto newVector = TranslateVector(vector.ToVector3(), angle, forward, up, right);
	return Vector3Int(
		(int)round(newVector.x),
		(int)round(newVector.y),
		(int)round(newVector.z)
	);
}

Vector3 TranslateVector(Vector3& vector, Vector3Shrt& orient, float distance)
{
	if (distance == 0.0f)
		return vector;

	float sinX = phd_sin(orient.x);
	float cosX = phd_cos(orient.x);
	float sinY = phd_sin(orient.y);
	float cosY = phd_cos(orient.y);

	return Vector3(
		vector.x + distance * (sinY * cosX),
		vector.y - distance * sinX,
		vector.z + distance * (cosY * cosX)
	);
}

Vector3Int TranslateVector(Vector3Int& vector, Vector3Shrt& orient, float distance)
{
	auto newVector = TranslateVector(vector.ToVector3(), orient, distance);
	return Vector3Int(
		(int)round(newVector.x),
		(int)round(newVector.y),
		(int)round(newVector.z)
	);
}

Vector3 TranslateVector(Vector3& vector, Vector3& target, float distance)
{
	if (distance == 0.0f)
		return vector;

	float distanceBetween = Vector3::Distance(vector, target);
	if (distance > distanceBetween)
		return target;

	auto direction = target - vector;
	direction.Normalize();
	return (vector + (direction * distance));
}

Vector3Int TranslateVector(Vector3Int& vector, Vector3Int& target, float distance)
{
	auto newVector = TranslateVector(vector.ToVector3(), target.ToVector3(), distance);
	return Vector3Int(
		(int)round(newVector.x),
		(int)round(newVector.y),
		(int)round(newVector.z)
	);
}