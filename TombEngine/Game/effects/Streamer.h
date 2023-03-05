#pragma once
#include "Math/Math.h"

using namespace TEN::Math;

struct ItemInfo;

namespace TEN::Effects::Streamer
{
	struct nStreamerSegment
	{
	private:
		static constexpr auto VERTEX_COUNT = 4;

	public:
		std::array<Vector3, VERTEX_COUNT> Vertices = {};

		AxisAngle Orientation = AxisAngle::Identity;

		float Life		= 0.0f;
		float Opacity	= 0.0f;
		float Width		= 0.0f;
		float ScaleRate = 0.0f;
		float FadeAlpha	= 0.0f;
	};

	class Streamer
	{
	private:
		// Constants
		static constexpr auto SEGMENT_COUNT_MAX = 128;

		// Components
		Vector3 AttachmentPoint = Vector3::Zero;
		std::vector<nStreamerSegment> Segments = {};

	public:
		// Utilities
		void AddSegment();
	};

	class StreamerModule
	{
	private:
		// Constants
		static constexpr auto STREAMER_COUNT_MAX = 8;

		// Components
		std::map<int, Streamer> Streamers = {}; // Key = tag.

	public:
		// Utilities
		void AddStreamer();
	};

	class StreamerController
	{
	private:
		// Constants
		static constexpr auto MODULE_COUNT_MAX = 32;

		// Components
		std::map<int, StreamerModule> Modules = {}; // Key = entity number.

	public:
		// Utilities
		void GrowStreamer(int entityID, int tag, const Vector3& pos, const AxisAngle& orient, float life, float scaleRate, float width, float fadeAlpha);

		void Update();
		void Draw() const;
		void Clear();
	};

	extern StreamerController StreamerEffect;

	// --------------

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

		StreamerType Type	   = StreamerType::Center;
		Vector3		 Direction = Vector3::Zero;


		float Life		= 0.0f;
		float Opacity	= 0.0f;
		float Width		= 0.0f;
		float ScaleRate = 0.0f;
		float FadeOut	= 0.0f;
	};

	extern std::array<std::vector<StreamerSegment>, (int)StreamerType::Count> Streamers;

	void SpawnStreamerSegment(const Vector3& pos, ItemInfo* item, int type, float width, float life, float fade);
	void SpawnStreamer(ItemInfo* item, int xOffset, int yOffset, int zOffset, int type, bool isOnWater, float width, float life, float fade);
	void UpdateStreamers();
}
