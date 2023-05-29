#pragma once

struct ItemInfo;

namespace TEN::Hud
{
	struct CrosshairData
	{
	private:
		struct SegmentData
		{
			Vector2 PosOffset2D	   = Vector2::Zero;
			short	OrientOffset2D = 0;
		};

	public:
		static constexpr auto COLOR_GREEN	= Vector4(0.0f, 0.9f, 0.1f, 9.0f);
		static constexpr auto COLOR_RED		= Vector4(1.0f, 0.1f, 0.1f, 9.0f);
		static constexpr auto COLOR_GRAY	= Vector4(0.5f, 0.5f, 0.5f, 6.0f);
		static constexpr auto SEGMENT_COUNT = 4;

		bool IsActive  = false;
		bool IsPrimary = false;

		Vector2 Position2D	  = Vector2::Zero;
		short	Orientation2D = 0;
		Vector4 Color		  = Vector4::Zero;
		Vector4 ColorTarget	  = Vector4::Zero;

		float Size		   = 0.0f;
		float RadiusScalar = 0.0f;

		std::array<SegmentData, SEGMENT_COUNT> Segments = {};

		Vector2 Get2DPositionOffset(short orientOffset2D) const;
		bool	IsOffscreen() const;
		void	Update(const Vector3& cameraPos, bool doPulse, bool isActive);
	};

	class TargetHighlighterController
	{
	private:
		// Members
		std::unordered_map<int, CrosshairData> Crosshairs = {}; // Key = entity ID.

	public:
		// Setters
		void SetPrimary(int entityID);
		void SetPeripheral(int entityID);

		// Utilities
		void Update(std::vector<int> entityIds);
		void Update(const ItemInfo& playerItem);
		void Draw() const;
		void Clear();

	private:
		// Helpers
		CrosshairData& GetNewCrosshair(int entityID);
		void		   AddCrosshair(int entityID, const Vector3& pos);
		void		   ClearInactiveCrosshairs();

		void DrawDebug() const;
	};
}
