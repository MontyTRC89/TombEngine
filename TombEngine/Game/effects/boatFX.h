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
		EulerAngles Orientation;
		bool On = false;
		int Life = 0;
		float ScaleRate = 0.0f; 
		int RoomNumber = 0;
		float Opacity = 0.5f;
		//int Age;
		float width = 1.0f;
		int PrevSegment;
		int ID;
	};

	extern WaveSegment Segments[64][3];



	void SpawnWaveSegment(const Vector3& origin, ItemInfo* Item, int waveDirection);

	void KayakUpdateWakeFX();
	void DoWakeEffect(ItemInfo* Item, int xOffset, int zOffset, int waveDirection, bool OnWater);

	WaveSegment&  GetFreeWaveSegment(int waveDirection);
	WaveSegment&  GetPreviousSegment(int waveDirection);
}
