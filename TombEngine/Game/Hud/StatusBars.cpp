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
	constexpr auto STATUS_BAR_LERP_ALPHA = 0.4f;

	void StatusBarsController::Update(ItemInfo& item)
	{
		constexpr auto FLASH_INTERVAL = 0.2f;

		if ((GameTimer % (int)round(FLASH_INTERVAL * FPS)) == 0)
			this->DoFlash = !DoFlash;

		this->UpdateAirBar(item);
		this->UpdateHealthBar(item);
		this->UpdateSprintBar(item);
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

	void StatusBarsController::UpdateAirBar(ItemInfo& item)
	{
		constexpr auto LIFE_MAX = 1.0f / FPS;

		const auto& player = *GetLaraInfo(&item);

		float air = std::clamp((float)player.Air, 0.0f, LARA_AIR_MAX);
		this->AirBar.TargetValue = air / LARA_AIR_MAX;

		if (AirBar.Life > 0.0f)
			this->AirBar.Life -= 1.0f;

		this->AirBar.Value = Lerp(AirBar.Value, AirBar.TargetValue, STATUS_BAR_LERP_ALPHA);
		if (abs(AirBar.Value - AirBar.TargetValue) <= EPSILON)
			this->AirBar.Value = AirBar.TargetValue;

		if (AirBar.Value != AirBar.TargetValue ||
			player.Control.WaterStatus == WaterStatus::Wade ||
			player.Control.WaterStatus == WaterStatus::TreadWater ||
			player.Control.WaterStatus == WaterStatus::Underwater)
		{
			this->AirBar.Life = round(LIFE_MAX * FPS);
		}

		// HACK: Special case for UPV.
		if (player.Vehicle != NO_ITEM)
		{
			if (g_Level.Items[player.Vehicle].ObjectNumber != ID_UPV)
				this->AirBar.Life = round(LIFE_MAX * FPS);
		}
	}

	void StatusBarsController::UpdateHealthBar(ItemInfo& item)
	{
		constexpr auto LIFE_MAX = 1.5f;

		const auto& player = *GetLaraInfo(&item);

		float health = std::clamp((float)item.HitPoints, 0.0f, LARA_HEALTH_MAX);
		this->HealthBar.TargetValue = health / LARA_HEALTH_MAX;

		if (HealthBar.Life > 0.0f)
			this->HealthBar.Life -= 1.0f;

		if (HealthBar.Value != HealthBar.TargetValue ||
			HealthBar.TargetValue <= 0.0f ||
			player.Control.HandStatus == HandStatus::WeaponReady)
		{
			this->HealthBar.Value = Lerp(HealthBar.Value, HealthBar.TargetValue, STATUS_BAR_LERP_ALPHA);
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

	void StatusBarsController::UpdateSprintBar(ItemInfo& item)
	{
		// TODO
	}

	void StatusBarsController::DrawStatusBar(float value, RendererHUDBar* rHudBar, GAME_OBJECT_ID textureID, int frame, bool isPoisoned) const
	{
		g_Renderer.DrawBar(value, rHudBar, textureID, frame, isPoisoned);
	}

	void StatusBarsController::DrawAirBar(ItemInfo& item) const
	{
		constexpr auto TEXTURE_ID	  = ID_AIR_BAR_TEXTURE;
		constexpr auto CRITICAL_VALUE = LARA_AIR_CRITICAL / LARA_AIR_MAX;

		if (AirBar.Life <= 0.0f)
			return;

		const auto& player = *GetLaraInfo(&item);

		float value = AirBar.Value;
		if (AirBar.Value <= CRITICAL_VALUE)
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
			bool isPoisoned = (player.PoisonPotency != 0);

			float value = HealthBar.Value;
			if (HealthBar.Value <= CRITICAL_VALUE)
				value = DoFlash ? value : 0.0f;

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
