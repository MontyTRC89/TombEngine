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
	constexpr auto NUM_WAKE_SPRITES = 256;
	constexpr auto NUM_WAKE_DIRECTION = 3;

	struct WaveSegment
	{
		//std::array<Vector3, 4> Vertices = {}; //NOTE: this cunfuses me, I will use that later when everything is set up and works
		Vector3 Vertices[4];
		Vector3i Direction = Vector3::Zero;
		EulerAngles Orientation;
		bool On = false;
		int Life = 0;
		float ScaleRate = 0.0f; 
		float Opacity = 0.5f;
		float width = 1.0f;
		int StreamerID;
		int FadeOut;
		int PreviousID;
	};

	extern WaveSegment Segments[NUM_WAKE_SPRITES][NUM_WAKE_DIRECTION];

	void SpawnWaveSegment(const Vector3& origin, ItemInfo* Item, int waveDirection, float width, int life, float fade);
	void DoWakeEffect(ItemInfo* Item, int xOffset, int yOffset, int zOffset, int waveDirection, bool OnWaterint, float width, int life, float fade);

	void UpdateWakeFX();

	int GetPreviousSegment(int waveDirection);
	WaveSegment&  GetFreeWaveSegment(int waveDirection);

}
