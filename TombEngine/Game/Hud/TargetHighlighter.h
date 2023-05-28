#pragma once

struct ItemInfo;

namespace TEN::Hud
{
	struct TargetHighlightData
	{
	private:
		struct SegmentData
		{
			Vector3 Offset2D	   = Vector3::Zero;
			short	OrientOffset2D = 0;
		};

	public:
		static constexpr auto SEGMENT_COUNT		= 4;
		static constexpr auto RADIUS_SCALAR_MAX = 10.0f;
		static constexpr auto COLOR_GREEN		= Vector4(0.1f, 1.0f, 0.1f, 9.0f);
		static constexpr auto COLOR_RED			= Vector4(1.0f, 0.1f, 0.1f, 9.0f);
		static constexpr auto COLOR_GRAY		= Vector4(0.5f, 0.5f, 0.5f, 6.0f);

		bool IsActive  = false;
		bool IsPrimary = false;

		Vector2 Position2D	  = Vector2::Zero;
		short	Orientation2D = 0;
		Vector4 Color		  = Vector4::Zero;
		Vector4 ColorTarget	  = Vector4::Zero;

		float Size				 = 0.0f;
		float RadiusScalar		 = 0.0f;
		float RadiusScalarTarget = 0.0f;

		std::array<SegmentData, SEGMENT_COUNT> Segments = {}; // TODO

		bool IsOffscreen() const;
		void Update(const Vector3& cameraPos, bool isActive);
	};

	class TargetHighlighterController
	{
	private:
		// Members
		std::unordered_map<int, TargetHighlightData> TargetHighlights = {}; // Key = entity ID.

	public:
		// Setters
		void SetPrimary(std::vector<int> entityIds);
		void SetPrimary(int entityID);
		void SetPeripheral(std::vector<int> entityIds);
		void SetPeripheral(int entityID);

		// Utilities
		void Update(std::vector<int> entityIds);
		void Update(const ItemInfo& playerItem);
		void Draw() const;
		void Clear();

	private:
		// Helpers
		std::vector<int>	 GetTargetEntityIds(const ItemInfo& playerItem);
		TargetHighlightData& GetNewTargetHighlight(int entityID);
		void				 AddTargetHighlight(int entityID, const Vector3& pos);
		void				 ClearInactiveTargetHighlights();

		void DrawDebug() const;
	};
}
