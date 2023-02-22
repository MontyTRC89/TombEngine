#pragma once
#include "Game/Lara/lara.h"

enum GAME_OBJECT_ID : short;
struct ItemInfo;

namespace TEN::Renderer
{
	struct RendererHUDBar;
}

namespace TEN::Hud
{
	struct StatusBar
	{
		float Life		  = 0.0f;
		float Opacity	  = 0.0f; // TODO: Opacity in renderer.
		float Value		  = 0.0f;
		float TargetValue = 0.0f;
	};

	class StatusBarsController
	{
	private:
		// Components
		StatusBar AirBar	= {};
		StatusBar HealthBar = {};
		StatusBar SprintBar = {};
		bool	  DoFlash	= false;

	public:
		// Utilities
		void Update(ItemInfo& item);
		void Draw(ItemInfo& item) const;
		void Clear();

	private:
		// Update helpers
		void UpdateAirBar(ItemInfo& item);
		void UpdateHealthBar(ItemInfo& item);
		void UpdateSprintBar(ItemInfo& item);

		// Draw helpers
		void DrawStatusBar(float value, float criticalValue, TEN::Renderer::RendererHUDBar* rHudBarPtr, GAME_OBJECT_ID textureID, int frame, bool isPoisoned) const;
		void DrawAirBar(ItemInfo& item) const;
		void DrawHealthBar(ItemInfo& item) const;
		void DrawSprintBar(ItemInfo& item) const;
	};
}
