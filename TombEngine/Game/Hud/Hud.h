#pragma once
#include "Game/Hud/PickupSummary.h"
#include "Game/Hud/StatusBars.h"

struct ItemInfo;

namespace TEN::Hud
{
	class HudController
	{
	public:
		// Members
		StatusBarsController	StatusBars	  = {};
		PickupSummaryController PickupSummary = {};

		// Utilities
		void Update(const ItemInfo& item);
		void Draw(const ItemInfo& item) const;
		void Clear();
	};

	extern HudController g_Hud;
}
