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
		//std::array<Vector3, 4> Vertices = {}; //NOTE: this cunfuses me, I will use that later when everything is set up and works
		Vector3 Vertices[4];
		Vector3i Direction = Vector3::Zero;
		Pose pos;
		bool On = false;
		int Life = 0;
		float ScaleRate = 0.0f; 
		int RoomNumber;
		float Opacity;
		int Age;
		float width = 36.0f;
		int PrevSegment;
	};

	extern std::array<WaveSegment, 64> Segments;

	struct Wave
	{
	//NOTE: struct not needed. delete later when everything else work
		std::array<WaveSegment, 64> Segments;
		unsigned int SegmentsNumMax = 0; 

	};

	extern std::array<Wave, 1> Waves;


	extern Vector3i WakeFXPos[6];
	extern short WakeFXBuffer[1024];
	void SpawnWaveSegment(const Vector3& origin, const Vector3& target, ItemInfo* kayakItem, const Vector4& color, int Velocity);

	void KayakUpdateWakeFX();

	void ClearWaveSegment();
	WaveSegment& GetFreeWaveSegment();
	WaveSegment& GetPreviousSegment();

	Vector3 RotatePoint(const Vector3& point, const EulerAngles& rotation);
}