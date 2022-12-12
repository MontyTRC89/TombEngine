#include "framework.h"
#include "Math/Legacy.h"

#include "Math/Constants.h"

float phd_sin(short x)
{
	return sin(TO_RAD(x));
}

float phd_cos(short x)
{
	return cos(TO_RAD(x));
}

// NOTE: Order of parameters is inverted!
int phd_atan(int y, int x)
{
	return FROM_RAD(atan2(x, y));
}

void InterpolateAngle(short angle, short& rotation, short& outAngle, int shift)
{
	int deltaAngle = angle - rotation;

	if (deltaAngle < ANGLE(-180.0f))
		deltaAngle += ANGLE(360.0f);
	else if (deltaAngle > ANGLE(180.0f))
		deltaAngle -= ANGLE(360.0f);

	if (outAngle)
		outAngle = (short)deltaAngle;

	rotation += short(deltaAngle >> shift);
}

void GetMatrixFromTrAngle(Matrix& matrix, short* framePtr, int index)
{
	short* ptr = &framePtr[0];

	ptr += 9;
	for (int i = 0; i < index; i++)
		ptr += ((*ptr & 0xc000) == 0) ? 2 : 1;

	int rot0 = *ptr++;
	int frameMode = rot0 & 0xc000;

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

		matrix = Matrix::CreateFromYawPitchRoll(
			rotY * (360.0f / 1024.0f) * RADIAN,
			rotX * (360.0f / 1024.0f) * RADIAN,
			rotZ * (360.0f / 1024.0f) * RADIAN);

		break;

	case 0x4000:
		matrix = Matrix::CreateRotationX((rot0 & 0xfff) * (360.0f / 4096.0f) * RADIAN);
		break;

	case 0x8000:
		matrix = Matrix::CreateRotationY((rot0 & 0xfff) * (360.0f / 4096.0f) * RADIAN);
		break;

	case 0xc000:
		matrix = Matrix::CreateRotationZ((rot0 & 0xfff) * (360.0f / 4096.0f) * RADIAN);
		break;
	}
}
