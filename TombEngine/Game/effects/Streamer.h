#pragma once

struct ItemInfo;

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
	private:
		static constexpr auto VERTEX_COUNT = 4;

	public:
		std::array<Vector3, VERTEX_COUNT> Vertices = {};

		bool On			= false;
		int	 StreamerID = 0;
		int	 PreviousID = 0;

		Vector3 Direction = Vector3::Zero;

		float Life		= 0.0f;
		float Opacity	= 0.0f;
		float Width		= 0.0f;
		float ScaleRate = 0.0f;
		int	  FadeOut	= 0;
	};

	extern StreamerSegment Segments[STREAMER_SEGMENT_COUNT_MAX][STREAMER_DIRECTION_COUNT];

	void SpawnStreamerSegment(const Vector3& pos, ItemInfo* item, int waveDirection, float width, int life, float fade);
	void SpawnStreamer(ItemInfo* item, int xOffset, int yOffset, int zOffset, int waveDirection, bool isOnWater, float width, float life, float fade);
	void UpdateStreamers();
}
