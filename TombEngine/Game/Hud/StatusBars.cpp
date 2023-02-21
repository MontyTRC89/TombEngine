#include "framework.h"
#include "Game/Hud/StatusBars.h"

#include "Game/camera.h"
#include "Game/collision/collide_room.h"
#include "Game/control/control.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Objects/game_object_ids.h"
#include "Renderer/Renderer11.h"
#include "Specific/clock.h"

using namespace TEN::Renderer;

extern TEN::Renderer::RendererHUDBar* g_AirBar;
extern TEN::Renderer::RendererHUDBar* g_HealthBar;
extern TEN::Renderer::RendererHUDBar* g_SprintBar;

namespace TEN::Hud
{
	void StatusBarsController::Update(ItemInfo& item)
	{
		constexpr auto FLASH_INTERVAL = 0.2f;

		if ((GameTimer % (int)round(FLASH_INTERVAL * FPS)) == 0)
			this->DoFlash = !DoFlash;

		this->UpdateHealthBar(item);
	}

	void StatusBarsController::Draw(ItemInfo& item) const
	{
		if (CurrentLevel == 0 || CinematicBarsHeight > 0)
			return;

		this->DrawAirBar(item);
		this->DrawHealthBar(item);
		this->DrawSprintBar(item);
	}

	void StatusBarsController::Clear()
	{
		*this = {};
	}

	void StatusBarsController::UpdateHealthBar(ItemInfo& item)
	{
		constexpr auto LIFE_MAX	  = 1.5f;
		constexpr auto LERP_ALPHA = 0.4f;

		const auto& player = *GetLaraInfo(&item);

		float health = std::clamp((float)item.HitPoints, 0.0f, LARA_HEALTH_MAX);
		this->HealthBar.TargetValue = health / LARA_HEALTH_MAX;

		if (HealthBar.Life > 0.0f)
			this->HealthBar.Life -= 1.0f;

		if (HealthBar.Value != HealthBar.TargetValue ||
			HealthBar.TargetValue <= 0.0f ||
			player.Control.HandStatus == HandStatus::WeaponReady)
		{
			this->HealthBar.Value = Lerp(HealthBar.Value, HealthBar.TargetValue, LERP_ALPHA);
			if (abs(HealthBar.Value - HealthBar.TargetValue) <= EPSILON)
				this->HealthBar.Value = HealthBar.TargetValue;

			this->HealthBar.Life = round(LIFE_MAX * FPS);

			// HACK: Hide health bar immediately when undrawing weapon.
			if (player.Control.HandStatus == HandStatus::WeaponReady &&
				player.Control.Weapon.GunType != LaraWeaponType::Torch)
			{
				this->HealthBar.Life = 0.0f;
			}
		}
	}

	void StatusBarsController::DrawStatusBar(float value, RendererHUDBar* rHudBar, GAME_OBJECT_ID textureID, int frame, bool isPoisoned) const
	{
		g_Renderer.DrawBar(value, rHudBar, textureID, frame, isPoisoned);
	}

	void StatusBarsController::DrawAirBar(ItemInfo& item) const
	{
		constexpr auto TEXTURE_ID = ID_AIR_BAR_TEXTURE;

		const auto& player = *GetLaraInfo(&item);

		// HACK: Unique handling for UPV.
		if (player.Vehicle == NO_ITEM || g_Level.Items[player.Vehicle].ObjectNumber != ID_UPV)
		{
			// Avoid showing air bar in these contexts.
			if (player.Control.WaterStatus != WaterStatus::Wade &&
				player.Control.WaterStatus != WaterStatus::TreadWater &&
				player.Control.WaterStatus != WaterStatus::Underwater &&
				!(TestEnvironment(ENV_FLAG_SWAMP, &item) && player.WaterSurfaceDist < -(CLICK(3) - 1)))
			{
				return;
			}
		}

		float air = std::clamp((float)player.Air, 0.0f, LARA_AIR_MAX);
		float value = air / LARA_AIR_MAX;

		if (air <= LARA_AIR_CRITICAL)
			value = DoFlash ? value : 0.0f;

		this->DrawStatusBar(value, g_AirBar, TEXTURE_ID, 0, false);
	}

	void StatusBarsController::DrawHealthBar(ItemInfo& item) const
	{
		constexpr auto TEXTURE_ID	  = ID_HEALTH_BAR_TEXTURE;
		constexpr auto CRITICAL_VALUE = LARA_HEALTH_CRITICAL / LARA_HEALTH_MAX;

		const auto& player = *GetLaraInfo(&item);

		if (HealthBar.Life > 0.0f || HealthBar.Value <= CRITICAL_VALUE ||
			(player.Control.HandStatus == HandStatus::WeaponReady &&
				player.Control.Weapon.GunType != LaraWeaponType::Torch) || // HACK: Torch is considered a weapon, so exclude it.
			player.PoisonPotency)
		{
			float value = HealthBar.Value;
			bool isPoisoned = (player.PoisonPotency != 0);

			if (HealthBar.Value <= CRITICAL_VALUE)
				value = (DoFlash || (HealthBar.Value != HealthBar.TargetValue)) ? value : 0.0f;

			this->DrawStatusBar(value, g_HealthBar, TEXTURE_ID, GlobalCounter, isPoisoned);
		}
	}

	void StatusBarsController::DrawSprintBar(ItemInfo& item) const
	{
		constexpr auto TEXTURE_ID = ID_DASH_BAR_TEXTURE;

		const auto& player = *GetLaraInfo(&item);

		if (player.SprintEnergy >= LARA_SPRINT_ENERGY_MAX)
			return;

		float sprintEnergy = std::clamp((float)player.SprintEnergy, 0.0f, LARA_SPRINT_ENERGY_MAX);
		float value = sprintEnergy / LARA_SPRINT_ENERGY_MAX;

		if (sprintEnergy <= LARA_SPRINT_ENERGY_CRITICAL)
			value = DoFlash ? value : 0.0f;

		this->DrawStatusBar(value, g_SprintBar, TEXTURE_ID, 0, false);
	}
}
