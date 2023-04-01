#pragma once
#include "Game/Hud/PickupSummary.h"
#include "Game/Hud/StatusBars.h"

struct ItemInfo;

namespace TEN::Hud
{
	class HudController
	{
	public:
		// Components
		StatusBarsController	StatusBars	  = {};
		PickupSummaryController PickupSummary = {};

		// Utilities
		void Update(ItemInfo& item);
		void Draw(ItemInfo& item) const;
		void Clear();
	};

	extern HudController g_Hud;
}
