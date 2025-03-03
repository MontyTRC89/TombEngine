#pragma once

#include "Math/Math.h"
#include "Renderer/RendererEnums.h"

using namespace TEN::Math;

struct ItemInfo;

namespace TEN::Effects::Streamer
{
	enum class StreamerFeatherType
	{
		None,
		Center,
		Left,
		Right
	};

	class Streamer
	{
	private:
		// Constants

		static constexpr auto SEGMENT_COUNT_MAX = 128;

		struct StreamerSegment
		{
			static constexpr auto VERTEX_COUNT = 2;

			std::array<Vector3, VERTEX_COUNT> Vertices	  = {}; // TODO: Refactor vertices to be calculated by renderer instead of by internal effect logic.
			AxisAngle						  Orientation = AxisAngle::Identity; // TODO: Interpolate?
			Vector4							  Color		  = Vector4::Zero;
			Vector4							  ColorStart  = Vector4::Zero;
			Vector4							  ColorEnd	  = Vector4::Zero;

			int	  Life		 = 0; // Time in game frames.
			int	  LifeMax	 = 0; // Time in game frames.
			float Velocity	 = 0.0f;
			float ExpRate	 = 0.0f;
			short Rotation	 = 0;

			std::array<Vector3, VERTEX_COUNT> PrevVertices = {};
			Vector4							  PrevColor	   = Vector4::Zero;

			void InitializeVertices(const Vector3& pos, float width);
			void Update();

		private:
			void TransformVertices(float vel, float expRate);

			void StoreInterpolationData()
			{
				PrevColor = Color;
				PrevVertices[0] = Vertices[0];
				PrevVertices[1] = Vertices[1];
			}
		};

		// Fields

		std::vector<StreamerSegment> _segments	  = {};
		StreamerFeatherType			 _featherType = StreamerFeatherType::None;
		BlendMode					 _blendMode	  = BlendMode::AlphaBlend;
		bool						 _isBroken	  = false;


	public:
		// Constructors
		
		Streamer(StreamerFeatherType featherType, BlendMode blendMode);

		// Getters
		
		const std::vector<StreamerSegment>& GetSegments() const;
		StreamerFeatherType					GetFeatherType() const;
		BlendMode							GetBlendMode() const;

		// Inquirers
		
		bool IsBroken() const;

		// Utilities

		void AddSegment(const Vector3& pos, const Vector3& dir, short orient, const Color& colorStart, const Color& colorEnd,
						float width, float life, float vel, float expRate, short rot, unsigned int segmentCount);
		void Update();

	private:
		// Helpers

		StreamerSegment& GetNewSegment();
	};

	class StreamerGroup
	{
	private:
		// Constants

		static constexpr auto POOL_COUNT_MAX	 = 64;
		static constexpr auto STREAMER_COUNT_MAX = 4;

		// Fields

		std::unordered_map<int, std::vector<Streamer>> _pools = {}; // Key = tag.

	public:

		// Getters

		const std::unordered_map<int, std::vector<Streamer>>& GetPools() const;

		// Utilities

		void AddStreamer(int tag, const Vector3& pos, const Vector3& dir, short orient, const Color& colorStart, const Color& colorEnd,
						 float width, float life, float vel, float expRate, short rot,
						 StreamerFeatherType featherType, BlendMode blendMode);
		void Update();

	private:
		// Helpers

		std::vector<Streamer>& GetPool(int tag);
		Streamer&			   GetStreamerIteration(int tag, StreamerFeatherType featherType, BlendMode blendMode);
		void				   ClearInactivePools();
		void				   ClearInactiveStreamers(int tag);
	};

	class StreamerEffectController
	{
	private:
		// Constants

		static constexpr auto GROUP_COUNT_MAX = 64;

		// Fields

		std::unordered_map<int, StreamerGroup> _groups = {}; // Key = item number.

	public:
		// Getters
		
		const std::unordered_map<int, StreamerGroup>& GetGroups() const;

		// Utilities

		// TODO: Use seconds.
		void Spawn(int itemNumber, int tag, const Vector3& pos, const Vector3& dir, short orient, const Color& colorStart, const Color& colorEnd,
				   float width, float life, float vel, float expRate, short rot,
				   StreamerFeatherType featherType = StreamerFeatherType::None, BlendMode blendMode = BlendMode::AlphaBlend);
		void Update();
		void Clear();

	private:
		// Helpers

		StreamerGroup& GetGroup(int itemNumber);
		void		   ClearInactiveGroups();
	};

	extern StreamerEffectController StreamerEffect;
}
