#include "framework.h"
#include "Math/Legacy.h"

#include <cmath>

#include "Math/Constants.h"
#include "Math/Containers/BoundingBox.h"
#include "Math/Containers/PoseData.h"

short ANGLE(float angle)
{
	return (angle * (65536.0f / 360.0f));
}

short FROM_DEGREES(float angle)
{
	return (angle * (65536.0f / 360.0f));
}

short FROM_RAD(float angle)
{
	return ((angle / RADIAN) * (65536.0f / 360.0f));
}

float TO_DEGREES(short angle)
{
	return lround(angle * (360.0f / 65536.0f));
}

float TO_RAD(short angle)
{
	return ((angle * (360.0f / 65536.0f)) * RADIAN);
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

void phd_RotBoundingBoxNoPersp(PoseData* pose, BOUNDING_BOX* bounds, BOUNDING_BOX* tBounds)
{
	auto world = pose->Orientation.ToRotationMatrix();
	auto bMin = Vector3(bounds->X1, bounds->Y1, bounds->Z1);
	auto bMax = Vector3(bounds->X2, bounds->Y2, bounds->Z2);

	bMin = Vector3::Transform(bMin, world);
	bMax = Vector3::Transform(bMax, world);

	tBounds->X1 = bMin.x;
	tBounds->X2 = bMax.x;
	tBounds->Y1 = bMin.y;
	tBounds->Y2 = bMax.y;
	tBounds->Z1 = bMin.z;
	tBounds->Z2 = bMax.z;
}

void InterpolateAngle(short angle, short* rotation, short* outAngle, int shift)
{
	int deltaAngle = angle - *rotation;

	if (deltaAngle < -ANGLE(180.0f))
		deltaAngle += ANGLE(360.0f);
	else if (deltaAngle > ANGLE(180.0f))
		deltaAngle -= ANGLE(360.0f);

	if (outAngle)
		*outAngle = (short)deltaAngle;

	*rotation += short(deltaAngle >> shift);
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
		rotX = (rot0 & 0x3ff0) >> 4;
		rotY = ((rot1 & 0xfc00) >> 10) | ((rot0 & 0xf) << 6) & 0x3ff;
		rotZ = (rot1) & 0x3ff;

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
