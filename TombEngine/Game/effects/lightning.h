#pragma once
#include <cstddef>
#include <vector>
#include "Math/Math.h"

using std::vector;

class Vector3i;
struct ENERGY_ARC;

namespace TEN::Effects::Lightning
{
	constexpr auto MAX_CURL_LASER_SEGMENTS = 56;

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

	struct HelicalLaser
	{
		Pose pos;
		Vector3i pos1;
		Vector3i pos4;
		short life;
		short coil;
		short spin, spinadd;
		short length, dlength;
		unsigned char segments = MAX_CURL_LASER_SEGMENTS;
		short size;
		int r, g, b;
		char fadein;
	};

	extern std::vector<LIGHTNING_INFO> Lightning;
	extern std::vector<HelicalLaser>   HelicalLasers;

	extern int LightningRandomSeed;
	extern Vector3i LightningPos[6];
	extern short LightningBuffer[1024];

	LIGHTNING_INFO* TriggerLightning(Vector3i* origin, Vector3i* target, byte amplitude, byte r, byte g, byte b, byte life, char flags, char width, char segments);
	void			TriggerLightningGlow(int x, int y, int z, byte size, byte r, byte g, byte b);
	void			SpawnHelicalLaser(const Vector3& origin, const Vector3& target);

	void UpdateLightning();
	void UpdateHelicalLasers();

	void CalcLightningSpline(Vector3i* pos, short* buffer, LIGHTNING_INFO* arc);
	int LSpline(int x, int* knots, int nk);
}
