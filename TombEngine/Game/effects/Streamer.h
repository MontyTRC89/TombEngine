#pragma once
#include "Math/Math.h"

class Vector3i;

namespace TEN::Effects::Streamer
{
	constexpr auto STREAMER_SEGMENT_COUNT_MAX = 256;
	constexpr auto STREAMER_DIRECTION_COUNT = 3;

	enum class WaveDirection
	{
		WAVE_DIRECTION_CENTRAL = 0,
		WAVE_DIRECTION_LEFT = 1,
		WAVE_DIRECTION_RIGHT = 2
	};

	struct StreamerSegment
	{
		//std::array<Vector3, 4> Vertices = {}; // NOTE: this confuses me, will use later when everything is set up and works.
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

	extern StreamerSegment Segments[STREAMER_SEGMENT_COUNT_MAX][STREAMER_DIRECTION_COUNT];

	void SpawnStreamerSegment(const Vector3& origin, ItemInfo* item, int waveDirection, float width, int life, float fade);
	void SpawnStreamer(ItemInfo* Item, int xOffset, int yOffset, int zOffset, int waveDirection, bool OnWaterint, float width, int life, float fade);
	void UpdateStreamers();
}
