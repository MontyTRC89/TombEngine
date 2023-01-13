#pragma once
#include <cstddef>
#include <vector>
#include "Math/Math.h"

using std::vector;

class Vector3i;
struct ENERGY_ARC;

namespace TEN::Effects::Lightning
{
	enum LightningFlags
	{
		LI_SPLINE	= (1 << 0),
		LI_MOVEEND	= (1 << 1),
		LI_THINOUT	= (1 << 2),
		LI_THININ	= (1 << 3),
		LI_SPARKEND = (1 << 4),
	};

	struct ElectricArc
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
		unsigned int NumSegments = 0;

		Vector3 Origin		  = Vector3::Zero;
		Vector3 Target		  = Vector3::Zero;
		short	Orientation2D = 0;
		Vector4 Color		  = Vector4::Zero;
		Vector3 LightPosition = Vector3::Zero; // TODO: Use a light cone instead?

		float Life		= 0.0f;
		float Scale		= 0.0f;
		float Length	= 0.0f;
		float LengthEnd = 0.0f;
		float FadeIn	= 0.0f;
		short Rotation	= 0;
		short Coil		= 0;

		int r, g, b;
	};

	extern std::vector<ElectricArc>	 Lightning;
	extern std::vector<HelicalLaser> HelicalLasers;

	extern Vector3i LightningPos[6];
	extern short	LightningBuffer[1024];

	ElectricArc* TriggerLightning(Vector3i* origin, Vector3i* target, byte amplitude, byte r, byte g, byte b, byte life, char flags, char width, char segments);
	void			TriggerLightningGlow(int x, int y, int z, byte size, byte r, byte g, byte b);
	void			SpawnHelicalLaser(const Vector3& origin, const Vector3& target);

	void UpdateLightning();
	void UpdateHelicalLasers();

	void CalcLightningSpline(Vector3i* pos, short* buffer, const ElectricArc& arc);
	int LSpline(int x, int* knots, int nk);
}
