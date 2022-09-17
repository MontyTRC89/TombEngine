#include "framework.h"
#include "Math/Legacy.h"

#include <cmath>

#include "Math/Constants.h"
#include "Math/Containers/PoseData.h"
#include "Math/Containers/Vector3i.h"
#include "Math/Random.h"
#include "Specific/phd_global.h"

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

float phd_sin(short a)
{
	return sin(TO_RAD(a));
}

float phd_cos(short a)
{
	return cos(TO_RAD(a));
}

int phd_atan(int x, int y)
{
	return FROM_RAD(atan2(y, x));
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

int mGetAngle(int x1, int y1, int x2, int y2)
{
	return (65536 - phd_atan(x2 - x1, y2 - y1)) % 65536;
}

void phd_RotBoundingBoxNoPersp(PoseData* pos, BOUNDING_BOX* bounds, BOUNDING_BOX* tbounds)
{
	auto world = pos->Orientation.ToRotationMatrix();
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

		*matrix = Matrix::CreateFromYawPitchRoll(
			rotY * (360.0f / 1024.0f) * RADIAN,
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

BoundingOrientedBox TO_DX_BBOX(PoseData pos, BOUNDING_BOX* box)
{
	auto boxCentre = Vector3((box->X2 + box->X1) / 2.0f, (box->Y2 + box->Y1) / 2.0f, (box->Z2 + box->Z1) / 2.0f);
	auto boxExtent = Vector3((box->X2 - box->X1) / 2.0f, (box->Y2 - box->Y1) / 2.0f, (box->Z2 - box->Z1) / 2.0f);
	auto rotation = pos.Orientation.ToQuaternion();

	BoundingOrientedBox result;
	BoundingOrientedBox(boxCentre, boxExtent, Vector4::UnitY).Transform(result, 1, rotation, Vector3(pos.Position.x, pos.Position.y, pos.Position.z));
	return result;
}
