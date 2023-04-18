#include "framework.h"
#include "Game/Hud/Hud.h"

#include "Game/Hud/PickupSummary.h"
#include "Game/Hud/StatusBars.h"
#include "Game/items.h"

using namespace TEN::Math;


// DEBUG
#include "Renderer11.h"
using TEN::Renderer::g_Renderer;

namespace TEN::Hud
{
	HudController g_Hud = {};

	void HudController::Update(const ItemInfo& item)
	{
		PickupSummary.Update();
		StatusBars.Update(item);
	}

	void HudController::Draw(const ItemInfo& item) const
	{
		// ----------------Debug
		static short orient2D = 0;
		orient2D += ANGLE(1.0f);

		auto pos = SCREEN_SPACE_RES / 3;
		g_Renderer.DrawSpriteIn2DSpace(
			ID_MISC_SPRITES, 1,
			pos, 0, Vector4(1.0f, 1.0f, 1.0f, 0.5f), Vector2(300.0f));

		//------------------

		PickupSummary.Draw();
		StatusBars.Draw(item);
	}

	void HudController::Clear()
	{
		PickupSummary.Clear();
		StatusBars.Clear();
	}
}
