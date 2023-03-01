#pragma once
#include "Game/Lara/lara.h"

enum GAME_OBJECT_ID : short;
struct ItemInfo;

namespace TEN::Renderer
{
	struct RendererHudBar;
}

namespace TEN::Hud
{
	struct StatusBar
	{
		float Value		  = 0.0f;
		float TargetValue = 0.0f;
		float Life		  = 0.0f;
		float Opacity	  = 0.0f; // TODO: Opacity in renderer.
	};

	class StatusBarsController
	{
	private:
		// Constants
		static constexpr auto STATUS_BAR_LIFE_MAX		   = 0.75f;
		static constexpr auto STATUS_BAR_LIFE_START_FADING = 0.2f;
		static constexpr auto STATUS_BAR_VALUE_LERP_ALPHA  = 0.3f;

		// Components
		StatusBar AirBar	= {};
		StatusBar HealthBar = {};
		StatusBar SprintBar = {};
		bool	  DoFlash	= false;

	public:
		// Utilities
		void Initialize(ItemInfo& item);
		void Update(ItemInfo& item);
		void Draw(ItemInfo& item) const;
		void Clear();

	private:
		// Update helpers
		void InitializeStatusBar(StatusBar& bar, float statusValue, float statusValueMax);
		void UpdateStatusBar(StatusBar& bar, float statusValue, float statusValueMax);
		void UpdateAirBar(ItemInfo& item);
		void UpdateHealthBar(ItemInfo& item);
		void UpdateSprintBar(ItemInfo& item);

		// Draw helpers
		void DrawStatusBar(float value, float criticalValue, const TEN::Renderer::RendererHudBar& rHudBar, GAME_OBJECT_ID textureID, int frame, bool isPoisoned) const;
		void DrawAirBar() const;
		void DrawHealthBar(bool isPoisoned) const;
		void DrawSprintBar() const;
	};
}
