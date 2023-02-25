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

extern TEN::Renderer::RendererHudBar* g_AirBar;
extern TEN::Renderer::RendererHudBar* g_HealthBar;
extern TEN::Renderer::RendererHudBar* g_SprintBar;

namespace TEN::Hud
{
	void StatusBarsController::Update(ItemInfo& item)
	{
		constexpr auto FLASH_INTERVAL = 0.2f;

		// Update flash.
		if ((GameTimer % (int)round(FLASH_INTERVAL * FPS)) == 0)
			this->DoFlash = !DoFlash;

		// Update bars.
		this->UpdateAirBar(item);
		this->UpdateHealthBar(item);
		this->UpdateSprintBar(item);
	}

	void StatusBarsController::Draw(ItemInfo& item) const
	{
		// Avoid drawing in title level and during cutscenes.
		if (CurrentLevel == 0 || CinematicBarsHeight > 0)
			return;

		const auto& player = *GetLaraInfo(&item);
		bool isPoisoned = (player.PoisonPotency != 0);

		// Draw bars.
		this->DrawAirBar();
		this->DrawHealthBar(isPoisoned);
		this->DrawSprintBar();
	}

	void StatusBarsController::Clear()
	{
		*this = {};
	}

	void StatusBarsController::UpdateAirBar(ItemInfo& item)
	{
		const auto& player = *GetLaraInfo(&item);

		// Update life.
		if (AirBar.Life > 0.0f)
			this->AirBar.Life -= 1.0f;

		// Update opacity.
		float alpha = std::clamp(AirBar.Life, 0.0f, STATUS_BAR_LIFE_START_FADING) / STATUS_BAR_LIFE_START_FADING;
		this->AirBar.Opacity = Lerp(0.0f, 1.0f, alpha);

		// Update target value.
		float air = std::clamp((float)player.Air, 0.0f, LARA_AIR_MAX);
		this->AirBar.TargetValue = air / LARA_AIR_MAX;

		// Update value.
		this->AirBar.Value = Lerp(AirBar.Value, AirBar.TargetValue, STATUS_BAR_VALUE_LERP_ALPHA);
		if (abs(AirBar.Value - AirBar.TargetValue) <= EPSILON)
			this->AirBar.Value = AirBar.TargetValue;

		// Set max life according to context.
		if (AirBar.Value != AirBar.TargetValue ||
			player.Control.WaterStatus == WaterStatus::Wade ||
			player.Control.WaterStatus == WaterStatus::TreadWater ||
			player.Control.WaterStatus == WaterStatus::Underwater)
		{
			this->AirBar.Life = round(STATUS_BAR_LIFE_MAX * FPS);
		}

		// HACK: Special case for UPV as it sets player.Control.WaterStatus to WaterStatus::Dry.
		if (player.Vehicle != NO_ITEM)
		{
			if (g_Level.Items[player.Vehicle].ObjectNumber != ID_UPV)
				this->AirBar.Life = round(STATUS_BAR_LIFE_MAX * FPS);
		}
	}

	void StatusBarsController::UpdateHealthBar(ItemInfo& item)
	{
		const auto& player = *GetLaraInfo(&item);

		// Update life.
		if (HealthBar.Life > 0.0f)
			this->HealthBar.Life -= 1.0f;

		// Update opacity.
		float alpha = std::clamp(HealthBar.Life, 0.0f, STATUS_BAR_LIFE_START_FADING) / STATUS_BAR_LIFE_START_FADING;
		this->HealthBar.Opacity = Lerp(0.0f, 1.0f, alpha);

		// Update target value.
		float health = std::clamp((float)item.HitPoints, 0.0f, LARA_HEALTH_MAX);
		this->HealthBar.TargetValue = health / LARA_HEALTH_MAX;

		// Update value.
		this->HealthBar.Value = Lerp(HealthBar.Value, HealthBar.TargetValue, STATUS_BAR_VALUE_LERP_ALPHA);
		if (abs(HealthBar.Value - HealthBar.TargetValue) <= EPSILON)
			this->HealthBar.Value = HealthBar.TargetValue;

		// Set max life according to context.
		if (HealthBar.Value != HealthBar.TargetValue ||
			health <= LARA_HEALTH_CRITICAL ||
			player.PoisonPotency != 0 ||
			player.Control.HandStatus == HandStatus::WeaponDraw ||
			(player.Control.HandStatus == HandStatus::WeaponReady &&
				player.Control.Weapon.GunType != LaraWeaponType::Torch)) // HACK: Exclude torch.
		{
			this->HealthBar.Life = round(STATUS_BAR_LIFE_MAX * FPS);
		}

		// HACK: Special case for weapon undraw.
		if (player.Control.HandStatus == HandStatus::WeaponUndraw && health > LARA_HEALTH_CRITICAL)
			this->HealthBar.Life = 0.0f;
	}

	void StatusBarsController::UpdateSprintBar(ItemInfo& item)
	{
		const auto& player = *GetLaraInfo(&item);

		// Update life.
		if (SprintBar.Life > 0.0f)
			this->SprintBar.Life -= 1.0f;

		// Update opacity.
		float alpha = std::clamp(SprintBar.Life, 0.0f, STATUS_BAR_LIFE_START_FADING) / STATUS_BAR_LIFE_START_FADING;
		this->SprintBar.Opacity = Lerp(0.0f, 1.0f, alpha);

		// Update target value.
		float sprintEnergy = std::clamp((float)player.SprintEnergy, 0.0f, LARA_SPRINT_ENERGY_MAX);
		this->SprintBar.TargetValue = sprintEnergy / LARA_SPRINT_ENERGY_MAX;

		// Update value.
		this->SprintBar.Value = Lerp(SprintBar.Value, SprintBar.TargetValue, STATUS_BAR_VALUE_LERP_ALPHA);
		if (abs(SprintBar.Value - SprintBar.TargetValue) <= EPSILON)
			this->SprintBar.Value = SprintBar.TargetValue;

		// Set max life according to context.
		if (SprintBar.Value != SprintBar.TargetValue ||
			SprintBar.Value != 1.0f)
		{
			this->SprintBar.Life = round(STATUS_BAR_LIFE_MAX * FPS);
		}
	}

	void StatusBarsController::DrawStatusBar(float value, float criticalValue, const RendererHudBar& rHudBar, GAME_OBJECT_ID textureID, int frame, bool isPoisoned) const
	{
		if (value <= criticalValue)
			value = DoFlash ? value : 0.0f;

		g_Renderer.DrawBar(value, rHudBar, textureID, frame, isPoisoned);
	}

	void StatusBarsController::DrawAirBar() const
	{
		constexpr auto TEXTURE_ID	  = ID_AIR_BAR_TEXTURE;
		constexpr auto CRITICAL_VALUE = LARA_AIR_CRITICAL / LARA_AIR_MAX;

		if (AirBar.Life <= 0.0f)
			return;

		this->DrawStatusBar(AirBar.Value, CRITICAL_VALUE, *g_AirBar, TEXTURE_ID, 0, false);
	}

	void StatusBarsController::DrawHealthBar(bool isPoisoned) const
	{
		constexpr auto TEXTURE_ID	  = ID_HEALTH_BAR_TEXTURE;
		constexpr auto CRITICAL_VALUE = LARA_HEALTH_CRITICAL / LARA_HEALTH_MAX;

		if (HealthBar.Life <= 0.0f)
			return;

		this->DrawStatusBar(HealthBar.Value, CRITICAL_VALUE, *g_HealthBar, TEXTURE_ID, GlobalCounter, isPoisoned);
	}

	void StatusBarsController::DrawSprintBar() const
	{
		constexpr auto TEXTURE_ID	  = ID_DASH_BAR_TEXTURE;
		constexpr auto CRITICAL_VALUE = LARA_SPRINT_ENERGY_CRITICAL / LARA_SPRINT_ENERGY_MAX;

		if (SprintBar.Life <= 0.0f)
			return;

		this->DrawStatusBar(SprintBar.Value, CRITICAL_VALUE, *g_SprintBar, TEXTURE_ID, 0, false);
	}
}
