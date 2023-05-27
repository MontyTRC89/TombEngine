#pragma once

struct ItemInfo;

namespace TEN::Hud
{
	struct TargetHighlightData
	{
		static constexpr auto COLOR_GREEN = Vector4(0.1f, 1.0f, 0.1f, 1.0f);
		static constexpr auto COLOR_RED	  = Vector4(1.0f, 0.1f, 0.1f, 1.0f);
		static constexpr auto COLOR_GRAY  = Vector4(0.5f, 0.5f, 0.5f, 1.0f);

		bool IsActive  = false;
		bool IsPrimary = false;

		Vector2 Position2D	  = Vector2::Zero;
		short	Orientation2D = 0;
		Vector4 Color		  = Vector4::Zero;
		Vector4 ColorTarget	  = Vector4::Zero;

		float Size			= 0.0f;
		float OpacityTarget = 0.0f;
		float Radius		= 0.0f;
		float RadiusTarget	= 0.0f;

		bool IsOffscreen() const;
		void Update(const Vector3& pos, bool isActive);
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
