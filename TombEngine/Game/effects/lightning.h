#pragma once
#include <cstddef>
#include <vector>
#include "Specific/phd_global.h"
#include "Specific/trmath.h"

using std::vector;

struct Vector3Int;
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
		Vector3Int pos1;
		Vector3Int pos2;
		Vector3Int pos3;
		Vector3Int pos4;
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
	extern Vector3Int LightningPos[6];
	extern short LightningBuffer[1024];

	void InitialiseFloatSinCosTable();
	void UpdateLightning();
	void TriggerLightning(Vector3Int* src, Vector3Int* dest, byte amplitude, byte r, byte g, byte b, byte life, char flags, char width, char segments);
	void CalcLightningSpline(Vector3Int* pos, short* buffer, LIGHTNING_INFO* arc);
	void TriggerLightningGlow(int x, int y, int z, byte size, byte r, byte g, byte b);
}