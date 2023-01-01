#pragma once
#include <cstddef>
#include <vector>
#include "Math/Math.h"
#include "Math/Math.h"

using std::vector;

class Vector3i;
struct ENERGY_ARC;

namespace TEN::Effects::Lightning
{
	enum LightningFlags
	{
		LI_SPLINE = 1,
		LI_MOVEEND = 2,
		LI_THINOUT = 4,
		LI_THININ = 8,
		LI_SPARKEND = 16
	};

	struct TWOGUNINFO
	{
		Pose pos;
		short life;
		short coil;
		short spin, spinadd;
		short length, dlength;
		short size;
		int r, g, b;
		char fadein;
	};

	extern TWOGUNINFO twogun[2];

	struct LIGHTNING_INFO
	{
		Vector3i pos1;
		Vector3i pos2;
		Vector3i pos3;
		Vector3i pos4;
		signed char interpolation[9];
		byte r;
		byte g;
		byte b;
		byte life;
		byte amplitude;
		byte flags;
		byte width;
		byte segments;

		int sAmplitude;
		int segmentSize;
		int direction;
		int rotation;
		int type;
		int sLife;
	};

	extern std::vector<LIGHTNING_INFO> Lightning;

	extern int LightningRandomSeed;
	extern float FloatSinCosTable[8192];
	extern Vector3i LightningPos[6];
	extern short LightningBuffer[1024];

	void InitialiseFloatSinCosTable();
	void UpdateLightning();
	void TriggerLaserBeam(Vector3i pos1, Vector3i pos2, EulerAngles orient);
	void UpdateTwogunLasers();
	void TriggerLightning(Vector3i* src, Vector3i* dest, byte amplitude, byte r, byte g, byte b, byte life, char flags, char width, char segments);
	void CalcLightningSpline(Vector3i* pos, short* buffer, LIGHTNING_INFO* arc);
	void TriggerLightningGlow(int x, int y, int z, byte size, byte r, byte g, byte b);
}