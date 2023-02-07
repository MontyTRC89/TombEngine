#pragma once
#include "Game/Hud/PickupSummary.h"

struct ItemInfo;

namespace TEN::Hud
{
	class PickupSummaryController;

	class HudController
	{
	public:
		// Components
		PickupSummaryController PickupSummary = {};

		// Utilities
		void Update();
		void Draw() const;
		void Clear();
	};

	extern HudController g_Hud;

	void UpdateBars(ItemInfo* item);
	void DrawHud(ItemInfo* item);
}
