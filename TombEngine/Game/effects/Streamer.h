#pragma once
#include "Math/Math.h"

using namespace TEN::Math;

struct ItemInfo;

namespace TEN::Effects::Streamer
{
	class Streamer
	{
	private:
		// Constants
		static constexpr auto SEGMENT_COUNT_MAX = 128;

	public:
		struct StreamerSegment
		{
			static constexpr auto VERTEX_COUNT = 2;
			static constexpr auto OPACITY_MAX  = 0.8f;

			std::array<Vector3, VERTEX_COUNT> Vertices = {};

			AxisAngle Orientation = AxisAngle::Identity;
			Vector4	  Color		  = Vector4::Zero;

			float Life		 = 0.0f;
			float LifeMax	 = 0.0f;
			float OpacityMax = 0.0f;
			float ScaleRate	 = 0.0f;

			void Update();
		};

		// Components
		bool IsBroken = false;
		std::vector<StreamerSegment> Segments = {};

		// Utilities
		void AddSegment(const Vector3& pos, const Vector3& direction, short orient2D, const Vector4& color, float width, float life, float scaleRate, unsigned int segmentCount);
		void Update();

	private:
		// Helpers
		StreamerSegment& GetNewSegment();
	};

	class StreamerModule
	{
	private:
		// Constants
		static constexpr auto STREAMER_COUNT_MAX = 8;
		static constexpr auto POOL_COUNT_MAX	 = 8;

	public:
		// Components
		std::map<int, std::vector<Streamer>> Pools = {}; // Key = tag.

		// Utilities
		void AddStreamer(int tag, const Vector3& pos, const Vector3& direction, short orient2D, const Vector4& color, float width, float life, float scaleRate);
		void Update();

	private:
		// Helpers
		Streamer& GetStreamer(std::vector<Streamer>& pool);
		void	  ClearInactiveStreamers(std::vector<Streamer>& pool);
		void	  ClearInactivePools();
	};

	class StreamerController
	{
	private:
		// Constants
		static constexpr auto MODULE_COUNT_MAX = 64;

	public:
		// Components
		std::map<int, StreamerModule> Modules = {}; // Key = entity number.

		// Utilities
		void Spawn(int entityID, int tag, const Vector3& pos, const Vector3& direction, short orient2D, const Vector4& color, float width, float life, float scaleRate);
		void Update();
		void Clear();

	private:
		void ClearInactiveModules();
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

	struct StreamerSegmentOld
	{
	private:
		static constexpr auto VERTEX_COUNT = 2;

	public:
		std::array<Vector3, VERTEX_COUNT> Vertices = {};

		StreamerType Type	   = StreamerType::Center;
		Vector3		 Direction = Vector3::Zero;

		float Life		= 0.0f;
		float Opacity	= 0.0f;
		float ScaleRate = 0.0f;
		float FadeOut	= 0.0f;
	};

	extern std::array<std::vector<StreamerSegmentOld>, (int)StreamerType::Count> Streamers;

	void SpawnStreamerSegment(const Vector3& pos, ItemInfo* item, int type, float width, float life, float fade);
	void SpawnStreamer(ItemInfo* item, int xOffset, int yOffset, int zOffset, int type, bool isOnWater, float width, float life, float fade);
	void UpdateStreamers();
}
