#pragma once
#include <cstddef>
#include <vector>
#include "Math/Math.h"
#include "Math/Math.h"

using std::vector;

class Vector3i;

namespace TEN::Effects::BOATFX
{


	struct WAKE_PTS
	{
		int x[2];
		int y;
		int z[2];
		short xvel[2];
		short zvel[2];
		byte life;
		byte pad[3];
		bool IsActive;
		int length;
		int width;
		Vector3 Normal = Vector3::Zero;
		Vector3i pos1;
		Vector3i pos2;
		Vector3i pos3;
		Vector3i pos4;
		byte r;
		byte g;
		byte b;
	};

	extern std::vector<WAKE_PTS> WakePts;

	//extern int WakeFXRandomSeed;
	//extern float FloatSinCosTable[8192];
	extern Vector3i WakeFXPos[6];
	extern short WakeFXBuffer[1024];

	//void InitialiseFloatSinCosTable();
	void KayakUpdateWakeFX();
	WAKE_PTS* TriggerWakeFX(const Vector3& origin, const Vector3& target, Vector4* color, char length);
	void CalcSpline(Vector3i* pos, short* buffer, WAKE_PTS* wave);
	//void TriggerLightningGlow(int x, int y, int z, byte size, byte r, byte g, byte b);
}