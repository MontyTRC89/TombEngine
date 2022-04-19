#pragma once
#include <cstddef>
#include <vector>
#include "Specific/phd_global.h"
#include "Specific/trmath.h"

using std::vector;

struct PHD_VECTOR;
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

	struct LIGHTNING_INFO
	{
		PHD_VECTOR pos1;
		PHD_VECTOR pos2;
		PHD_VECTOR pos3;
		PHD_VECTOR pos4;
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

	extern int LightningRandomSeed;
	extern float FloatSinCosTable[8192];
	extern PHD_VECTOR LightningPos[6];
	extern short LightningBuffer[1024];

	extern std::vector<LIGHTNING_INFO> Lightning;

	void InitialiseFloatSinCosTable();
	void UpdateLightning();
	void TriggerLightning(PHD_VECTOR* src, PHD_VECTOR* dest, char amplitude, byte r, byte g, byte b, byte life, char flags, char width, char segments);
	void CalcLightningSpline(PHD_VECTOR* pos, short* buffer, LIGHTNING_INFO* arc);
	void TriggerLightningGlow(int x, int y, int z, byte size, byte r, byte g, byte b);
}