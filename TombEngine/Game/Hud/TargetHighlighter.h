#pragma once

struct ItemInfo;

namespace TEN::Hud
{
	struct CrosshairData
	{
		static constexpr auto COLOR_GREEN	= Vector4(0.0f, 0.8f, 0.2f, 0.9f);
		static constexpr auto COLOR_RED		= Vector4(1.0f, 0.1f, 0.1f, 0.9f);
		static constexpr auto COLOR_GRAY	= Vector4(0.5f, 0.5f, 0.5f, 0.6f);
		static constexpr auto SEGMENT_COUNT = 4;

	private:
		struct SegmentData
		{
			Vector2 PosOffset2D	   = Vector2::Zero;
			short	OrientOffset2D = 0;
		};

	public:
		bool IsActive  = false;
		bool IsPrimary = false;

		Vector2 Position2D	  = Vector2::Zero;
		short	Orientation2D = 0;
		Vector4 Color		  = Vector4::Zero;
		Vector4 ColorTarget	  = Vector4::Zero;

		float Size		  = 0.0f;
		float RadiusScale = 0.0f;
		float PulseScale  = 0.0f;

		std::array<SegmentData, SEGMENT_COUNT> Segments = {};

		bool	IsOffscreen() const;
		float	GetSize(float cameraDist) const;
		float	GetRadius() const;
		Vector2 Get2DPositionOffset(short orientOffset2D) const;

		void SetPrimary();
		void SetPeripheral();
		void Update(const Vector3& cameraPos, bool doPulse, bool isActive);
	};

	class TargetHighlighterController
	{
	private:
		// Members
		std::unordered_map<int, CrosshairData> Crosshairs = {}; // Key = entity ID.

	public:
		// Utilities
		void Update(const ItemInfo& playerItem);
		void Draw() const;
		void Clear();

	private:
		// Update helpers
		void Update(const std::vector<int>& entityIds);

		// Misc. helpers
		CrosshairData& GetNewCrosshair(int entityID);
		void		   AddCrosshair(int entityID, const Vector3& pos);
		void		   ClearInactiveCrosshairs();

		void DrawDebug() const;
	};
}
