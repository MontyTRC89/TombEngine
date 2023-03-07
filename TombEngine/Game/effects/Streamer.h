#pragma once
#include "Math/Math.h"

using namespace TEN::Math;

struct ItemInfo;

namespace TEN::Effects::Streamer
{
	enum class StreamerFlags
	{
		FadeLeft,
		FadeRight
	};

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

			AxisAngle Orientation = AxisAngle::Identity;
			Vector4	  Color		  = Vector4::Zero;

			float Life		 = 0.0f;
			float LifeMax	 = 0.0f;
			float OpacityMax = 0.0f;
			float Velocity	 = 0.0f;
			float ScaleRate	 = 0.0f;
			int	  Flags		 = 0;

			std::array<Vector3, VERTEX_COUNT> Vertices = {};

			void InitializeVertices(const Vector3& pos, float width);
			void Update();

		private:
			void TransformVertices(float vel, float scaleRate);
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
		static constexpr auto POOL_COUNT_MAX	 = 8;
		static constexpr auto STREAMER_COUNT_MAX = 8;

	public:
		// Components
		std::map<int, std::vector<Streamer>> Pools = {}; // Key = tag.

		// Utilities
		void AddStreamer(int tag, const Vector3& pos, const Vector3& direction, short orient2D, const Vector4& color, float width, float life, float scaleRate);
		void Update();

	private:
		// Helpers
		std::vector<Streamer>& GetPool(int tag);
		Streamer&			   GetStreamer(int tag);
		void				   ClearInactivePools();
		void				   ClearInactiveStreamers(int tag);
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
		// Helpers
		StreamerModule& GetModule(int entityNumber);
		void			ClearInactiveModules();
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
