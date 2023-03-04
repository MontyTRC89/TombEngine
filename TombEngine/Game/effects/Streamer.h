#pragma once

struct ItemInfo;

namespace TEN::Effects::Streamer
{
	constexpr auto STREAMER_SEGMENT_COUNT_MAX = 256;

	enum class StreamerType
	{
		Center,
		Left,
		Right,

		Count
	};

	struct StreamerSegment
	{
	private:
		static constexpr auto VERTEX_COUNT = 4;

	public:
		std::array<Vector3, VERTEX_COUNT> Vertices = {};

		bool		 On			   = false;
		int			 PreviousIndex = 0;
		StreamerType Type		   = StreamerType::Center;

		Vector3 Direction = Vector3::Zero;

		float Life		= 0.0f;
		float Opacity	= 0.0f;
		float Width		= 0.0f;
		float ScaleRate = 0.0f;
		int	  FadeOut	= 0;
	};

	extern StreamerSegment Segments[STREAMER_SEGMENT_COUNT_MAX][(int)StreamerType::Count];

	void SpawnStreamerSegment(const Vector3& pos, ItemInfo* item, int type, float width, int life, float fade);
	void SpawnStreamer(ItemInfo* item, int xOffset, int yOffset, int zOffset, int type, bool isOnWater, float width, float life, float fade);
	void UpdateStreamers();
}
