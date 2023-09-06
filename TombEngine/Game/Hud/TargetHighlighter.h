#pragma once

struct ItemInfo;

namespace TEN::Hud
{
	struct CrosshairData
	{
		static constexpr auto COLOR_RED		= Vector4(1.0f, 0.2f, 0.2f, 0.9f);
		static constexpr auto COLOR_GRAY	= Vector4(0.5f, 0.5f, 0.5f, 0.5f);
		static constexpr auto SEGMENT_COUNT = 4;

	private:
		struct SegmentData
		{
			Vector2 PosOffset	 = Vector2::Zero;
			short	OrientOffset = 0;
		};

	public:
		bool IsActive  = false;
		bool IsPrimary = false;

		Vector2 Position	= Vector2::Zero;
		short	Orientation = 0;
		float	Size		= 0.0f;
		Vector4 Color		= Vector4::Zero;
		Vector4 ColorTarget = Vector4::Zero;

		float RadiusScale = 0.0f;
		float PulseScale  = 0.0f;

		std::array<SegmentData, SEGMENT_COUNT> Segments = {};

		bool	IsOffscreen() const;
		float	GetSize(float cameraDist) const;
		float	GetRadius() const;
		Vector2 GetPositionOffset(short orientOffset) const;

		void SetPrimary();
		void SetPeripheral();
		void Update(const Vector3& targetPos, bool doPulse, bool isActive);
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
		void Update(const std::vector<int>& itemNumbers);

		// Misc. helpers
		CrosshairData& GetNewCrosshair(int itemNumber);
		void		   AddCrosshair(int itemNumber, const Vector3& targetPos);
		void		   ClearInactiveCrosshairs();

		void DrawDebug() const;
	};
}
