#include "framework.h"
#include "Game/Hud/Hud.h"

#include "Game/Hud/PickupSummary.h"
#include "Game/Hud/StatusBars.h"
#include "Game/items.h"

using namespace TEN::Math;

namespace TEN::Hud
{
	HudController g_Hud = {};

	void HudController::Update(ItemInfo& item)
	{
		this->PickupSummary.Update();
		this->StatusBars.Update(item);
	}

	void HudController::Draw(ItemInfo& item) const
	{
		this->PickupSummary.Draw();
		this->StatusBars.Draw(item);
	}

	void HudController::Clear()
	{
		this->PickupSummary.Clear();
		this->StatusBars.Clear();
	}
}
