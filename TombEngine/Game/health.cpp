#include "framework.h"
#include "Game/health.h"

#include "Game/animation.h"
#include "Game/camera.h"
#include "Game/control/control.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/Lara/lara_tests.h"
#include "Game/pickup/pickup.h"
#include "Renderer/Renderer11.h"
#include "Specific/level.h"

using namespace TEN::Renderer;

short PickupX;
short PickupY;
short PickupVel;
short CurrentPickup;
DisplayPickup Pickups[MAX_COLLECTED_PICKUPS];

float HealthBarOldValue = LARA_HEALTH_MAX;
float HealthBarValue = HealthBarOldValue;
float HealthBarMutateAmount = 0;
int   HealthBarTimer = 40;
bool  EnableSmoothHealthBar = true;

extern RendererHUDBar* g_HealthBar;
extern RendererHUDBar* g_DashBar;
extern RendererHUDBar* g_AirBar;

void DrawHUD(ItemInfo* item)
{
	static bool doFlash = false;
	if ((GameTimer & 0x07) == 0x07)
		doFlash = !doFlash;

	if (CurrentLevel == 0 || CinematicBarsHeight > 0)
		return;

	DrawSprintBar(LaraItem);
	DrawHealthBar(LaraItem, doFlash);
	DrawAirBar(LaraItem, doFlash);
	DrawAllPickups();
}

void DrawHealthBarOverlay(ItemInfo* item, int value)
{
	const auto& lara = *GetLaraInfo(item);

	if (CurrentLevel)
		g_Renderer.DrawBar(value, g_HealthBar, ID_HEALTH_BAR_TEXTURE, GlobalCounter, Lara.PoisonPotency);
}

void DrawHealthBar(ItemInfo* item, float value)
{
	const auto& lara = *GetLaraInfo(item);

	g_Renderer.DrawBar(value, g_HealthBar, ID_HEALTH_BAR_TEXTURE, GlobalCounter, lara.PoisonPotency);
}

void DrawHealthBar(ItemInfo* item, bool doFlash)
{
	const auto& lara = *GetLaraInfo(item);

	// Flash when at critical capacity and bar is not transitioning.
	if (HealthBarValue <= LARA_HEALTH_CRITICAL)
	{
		if (!BinocularRange)
		{
			if (doFlash || HealthBarMutateAmount)
				DrawHealthBar(item, HealthBarValue / LARA_HEALTH_MAX);
			else
				DrawHealthBar(item, 0.0f);
		}
		else
		{
			if (doFlash || HealthBarMutateAmount)
				DrawHealthBarOverlay(item, HealthBarValue / LARA_HEALTH_MAX);
			else
				DrawHealthBarOverlay(item, 0);
		}
	}
	else if (HealthBarTimer > 0 || HealthBarValue <= 0 ||
			 lara.Control.HandStatus == HandStatus::WeaponReady &&
			 lara.Control.Weapon.GunType != LaraWeaponType::Torch ||
			 lara.PoisonPotency)
	{
		if (!BinocularRange)
			DrawHealthBar(item, HealthBarValue / LARA_HEALTH_MAX);
		else
			DrawHealthBarOverlay(item, HealthBarValue / LARA_HEALTH_MAX);
	}
}

void DrawAirBar(float value)
{
	g_Renderer.DrawBar(value, g_AirBar,ID_AIR_BAR_TEXTURE, 0, 0);
}

void DrawAirBar(ItemInfo* item, bool doFlash)
{
	const auto& lara = *GetLaraInfo(item);

	if (lara.Air == LARA_AIR_MAX || item->HitPoints <= 0)
		return;

	if (lara.Vehicle == NO_ITEM ||
		g_Level.Items[lara.Vehicle].ObjectNumber != ID_UPV)
	{
		if (lara.Control.WaterStatus != WaterStatus::Underwater &&
			lara.Control.WaterStatus != WaterStatus::TreadWater &&
			!(TestEnvironment(ENV_FLAG_SWAMP, item) && lara.WaterSurfaceDist < -(CLICK(3) - 1)))
			return;
	}

	float air = lara.Air;
	if (air < 0.0f)
	{
		air = 0.0f;
	}
	else if (air > LARA_AIR_MAX)
	{
		air = LARA_AIR_MAX;
	}

	if (air <= LARA_AIR_CRITICAL)
	{
		if (doFlash)
			DrawAirBar(air / LARA_AIR_MAX);
		else
			DrawAirBar(0.0f);
	}
	else
	{
		DrawAirBar(air / LARA_AIR_MAX);
	}
}

void DrawSprintBar(float value)
{
	g_Renderer.DrawBar(value, g_DashBar, ID_DASH_BAR_TEXTURE, 0, 0);
}

void DrawSprintBar(ItemInfo* item)
{
	const auto& lara = *GetLaraInfo(item);

	if (lara.SprintEnergy < LARA_SPRINT_ENERGY_MAX)
		DrawSprintBar(lara.SprintEnergy / LARA_SPRINT_ENERGY_MAX);
}

void DrawAllPickups()
{
	auto& pickup = Pickups[CurrentPickup];

	if (pickup.Life > 0)
	{
		if (PickupX > 0)
			PickupX += -PickupX >> 3;
		else
			pickup.Life--;
	}
	else if (pickup.Life == 0)
	{
		if (PickupX < 128)
		{
			if (PickupVel < 16)
				PickupVel++;

			PickupX += PickupVel;
		}
		else
		{
			pickup.Life = -1;
			PickupVel = 0;
		}
	}
	else
	{
		int i;
		for (i = 0; i < MAX_COLLECTED_PICKUPS; i++)
		{
			if (Pickups[CurrentPickup].Life > 0)
				break;

			CurrentPickup++;
			CurrentPickup &= (MAX_COLLECTED_PICKUPS - 1);
		}

		if (i == MAX_COLLECTED_PICKUPS)
		{
			CurrentPickup = 0;
			return;
		}
	}

	g_Renderer.DrawPickup(Pickups[CurrentPickup].ObjectNumber);
}

void AddDisplayPickup(GAME_OBJECT_ID objectNumber)
{
	for (auto& pickup : Pickups)
	{
		if (pickup.Life < 0)
		{
			pickup.Life = 45;
			pickup.ObjectNumber = objectNumber;
			break;
		}
	}

	// No free slot found; pickup the object without displaying it.
	PickedUpObject(objectNumber);
}

void InitialisePickupDisplay()
{
	for (auto& pickup : Pickups)
		pickup.Life = -1;

	PickupX = 128;
	PickupY = 128;
	PickupVel = 0;
	CurrentPickup = 0;
}

void UpdateBars(ItemInfo* item)
{
	if (HealthBarTimer)
		HealthBarTimer--;

	const auto& lara = *GetLaraInfo(item);

	float hitPoints = item->HitPoints;

	if (hitPoints < 0)
	{
		hitPoints = 0;
	}
	else if (hitPoints > LARA_HEALTH_MAX)
	{
		hitPoints = LARA_HEALTH_MAX;
	}

	// Smoothly transition health bar display.
	if (EnableSmoothHealthBar)
	{
		if (HealthBarOldValue != hitPoints)
		{
			HealthBarMutateAmount += HealthBarOldValue - hitPoints;
			HealthBarOldValue = hitPoints;
			HealthBarTimer = 40;
		}

		if (HealthBarValue - HealthBarMutateAmount < 0)
		{
			HealthBarMutateAmount = HealthBarValue;
		}
		else if (HealthBarValue - HealthBarMutateAmount > LARA_HEALTH_MAX)
		{
			HealthBarMutateAmount = HealthBarValue - LARA_HEALTH_MAX;
		}

		HealthBarValue -= HealthBarMutateAmount / 3;
		HealthBarMutateAmount -= HealthBarMutateAmount / 3;

		if (HealthBarMutateAmount > -0.5f && HealthBarMutateAmount < 0.5f)
		{
			HealthBarMutateAmount = 0;
			HealthBarValue = hitPoints;
		}
	}
	// Discretely transition health bar display.
	else
	{
		if (HealthBarOldValue != hitPoints)
		{
			HealthBarOldValue = hitPoints;
			HealthBarValue = hitPoints;
			HealthBarTimer = 40;
		}
	}
}
