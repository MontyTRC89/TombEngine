#pragma once
#include <cstddef>
#include <vector>
#include "Math/Math.h"
#include "Math/Math.h"

using std::vector;
using std::array;

class Vector3i;

namespace TEN::Effects::BOATFX
{


	struct WaveSegment
	{
		//std::array<Vector3, 4> Vertices = {};
		Vector3 Vertices[4];
		Vector3 Direction = Vector3::Zero;
		bool On = false;
		int Life = 0;
		float ScaleRate = 0.0f; // Maybe?
		int RoomNumber;
		float Opacity;
		int Age;
	};
	//const std::vector<WaveSegment>& GetParticles() const { return Segments; }
	extern std::array<WaveSegment, 32> Segments;

	struct Wave
	{
		//WaveSegment Segments[32];
		
		//std::vector<WaveSegment>& GetParticles() const { return Waves; }

		//std::array<WaveSegment,32> Segments ;
		unsigned int SegmentsNumMax = 0; // Maybe not necessary?

	};

	extern std::array<Wave, 1> Waves;

	/*struct WAKE_PTS
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

	extern std::vector<WAKE_PTS> WakePts;*/



	//extern int WakeFXRandomSeed;
	//extern float FloatSinCosTable[8192];
	extern Vector3i WakeFXPos[6];
	extern short WakeFXBuffer[1024];
	void SpawnWaveSegment(const Vector3& origin, const Vector3& target, int roomNumber, const Vector4& color);
	//void InitialiseFloatSinCosTable();
	void KayakUpdateWakeFX();
	//WAKE_PTS* TriggerWakeFX(const Vector3& origin, const Vector3& target, Vector4* color, char length);
	//void CalcSpline(Vector3i* pos, short* buffer, WAKE_PTS* wave);
	void ClearWaveSegment();
	WaveSegment& GetFreeWaveSegment();
	WaveSegment& GetPreviousSegment();
	//void TriggerLightningGlow(int x, int y, int z, byte size, byte r, byte g, byte b);
}