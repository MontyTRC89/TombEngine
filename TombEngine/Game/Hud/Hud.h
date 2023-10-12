#pragma once
#include "Game/Hud/PickupSummary.h"
#include "Game/Hud/StatusBars.h"
#include "Game/Hud/TargetHighlighter.h"

struct ItemInfo;

namespace TEN::Hud
{
	class HudController
	{
	public:
		// Members
		StatusBarsController		StatusBars		  = {};
		PickupSummaryController		PickupSummary	  = {};
		TargetHighlighterController TargetHighlighter = {};

		// Utilities
		void Update(const ItemInfo& playerItem);
		void Draw(const ItemInfo& playerItem) const;
		void Clear();
	};

	extern HudController g_Hud;
}
