#include "framework.h"
#include "Game/health.h"

#include "Game/animation.h"
#include "Game/camera.h"
#include "Game/control/control.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_tests.h"
#include "Game/pickup/pickup.h"
#include "Renderer/Renderer11.h"
#include "Specific/level.h"

using namespace TEN::Renderer;
short PickupX;
short PickupY;
short CurrentPickup;
DISPLAY_PICKUP Pickups[MAX_COLLECTED_PICKUPS];
short PickupVel;
int OldHitPoints = LARA_HEALTH_MAX;
int HealthBarTimer = 40;
float HealthBar = OldHitPoints;
float MutateAmount = 0;
int FlashState = 0;
int FlashCount = 0;
int PoisonFlag = 0;
extern RendererHUDBar* g_HealthBar;
extern RendererHUDBar* g_DashBar;
extern RendererHUDBar* g_AirBar;

bool EnableSmoothHealthBar = true;

void DrawHealthBarOverlay(int value)
{
	if (CurrentLevel)
	{
		int color2 = 0;
		if (Lara.poisoned)
			color2 = 0xA0A000;
		else
			color2 = 0xA00000;

		g_Renderer.drawBar(value, ::g_HealthBar, ID_HEALTH_BAR_TEXTURE, GlobalCounter, Lara.poisoned);
	}
}

void DrawHealthBar(float value)
{
	if (CurrentLevel)
		g_Renderer.drawBar(value, ::g_HealthBar, ID_HEALTH_BAR_TEXTURE, GlobalCounter, Lara.poisoned);
}

void UpdateHealthBar(ITEM_INFO* item, int flash)
{
	auto HitPoints = item->HitPoints;

	if (HitPoints < 0)
		HitPoints = 0;
	else if (HitPoints > LARA_HEALTH_MAX)
		HitPoints = LARA_HEALTH_MAX;

	// OPT: smoothly transition health bar display.
	if (EnableSmoothHealthBar)
	{
		if (OldHitPoints != HitPoints)
		{
			MutateAmount += OldHitPoints - HitPoints;
			OldHitPoints = HitPoints;
			HealthBarTimer = 40;
		}

		if (HealthBar - MutateAmount < 0)
			MutateAmount = HealthBar;
		else if (HealthBar - MutateAmount > LARA_HEALTH_MAX)
			MutateAmount = HealthBar - LARA_HEALTH_MAX;

		HealthBar -= MutateAmount / 3;
		MutateAmount -= MutateAmount / 3;

		if (MutateAmount > -0.5f && MutateAmount < 0.5f)
		{
			MutateAmount = 0;
			HealthBar = HitPoints;
		}
	}

	// OG: discretely transition health bar display.
	else
	{
		if (OldHitPoints != HitPoints)
		{
			OldHitPoints = HitPoints;
			HealthBar = HitPoints;
			HealthBarTimer = 40;
		}
	}

	if (HealthBarTimer < 0)
		HealthBarTimer = 0;

	// Flash when at 1/4 capacity AND HP bar is not transitioning.
	if (HealthBar <= LARA_HEALTH_MAX / 4)
	{
		if (!BinocularRange)
		{
			if (flash)
				DrawHealthBar(HealthBar / LARA_HEALTH_MAX);
			else
				DrawHealthBar(0);
		}
		else
		{
			if (flash)
				DrawHealthBarOverlay(HealthBar / LARA_HEALTH_MAX);
			else
				DrawHealthBarOverlay(0);
		}
	}
	else if (HealthBarTimer > 0 ||
		HealthBar <= 0 ||
		Lara.Control.HandStatus == HandStatus::WeaponReady && Lara.Control.WeaponControl.GunType != WEAPON_TORCH ||
		Lara.poisoned >= 256)
	{
		if (!BinocularRange)
			DrawHealthBar(HealthBar / LARA_HEALTH_MAX);
		else
			DrawHealthBarOverlay(HealthBar / LARA_HEALTH_MAX);
	}

	if (PoisonFlag)
		PoisonFlag--;
}

void DrawAirBar(float value)
{
	if (CurrentLevel)
		g_Renderer.drawBar(value, ::g_AirBar,ID_AIR_BAR_TEXTURE,0,0);
}

void UpdateAirBar(ITEM_INFO* item, int flash)
{
	if (Lara.Air == LARA_AIR_MAX || item->HitPoints <= 0)
		return;

	if (Lara.Vehicle == NO_ITEM ||
		g_Level.Items[Lara.Vehicle].ObjectNumber != ID_UPV)
	{
		if (Lara.Control.WaterStatus != WaterStatus::Underwater &&
			Lara.Control.WaterStatus != WaterStatus::WaterSurface &&
			!(TestEnvironment(ENV_FLAG_SWAMP, item) && Lara.WaterSurfaceDist < -(STOP_SIZE + STEP_SIZE - 1)))
			return;
	}

	int air = Lara.Air;
	if (air < 0)
		air = 0;
	else if (air > LARA_AIR_MAX)
		air = LARA_AIR_MAX;
	if (air <= (LARA_AIR_MAX / 4))
	{
		if (flash)
			DrawAirBar(air / LARA_AIR_MAX);
		else
			DrawAirBar(0);
	}
	else
		DrawAirBar(air / LARA_AIR_MAX);
}

void DrawSprintBar(float value)
{
	if (CurrentLevel)
		g_Renderer.drawBar(value, ::g_DashBar, ID_DASH_BAR_TEXTURE, 0, 0);
}

void UpdateSprintBar()
{
	if (Lara.SprintEnergy < LARA_SPRINT_MAX)
		DrawSprintBar(Lara.SprintEnergy / LARA_SPRINT_MAX);
}

void DrawAllPickups()
{
	DISPLAY_PICKUP* pickup = &Pickups[CurrentPickup];

	if (pickup->life > 0)
	{
		if (PickupX > 0)
			PickupX += -PickupX >> 3;
		else
			pickup->life--;
	}
	else if (pickup->life == 0)
	{
		if (PickupX < 128)
		{
			if (PickupVel < 16)
				PickupVel++;

			PickupX += PickupVel;
		}
		else
		{
			pickup->life = -1;
			PickupVel = 0;
		}
	}
	else
	{
		int i;
		for (i = 0; i < MAX_COLLECTED_PICKUPS; i++)
		{
			if (Pickups[CurrentPickup].life > 0)
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

	g_Renderer.drawPickup(Pickups[CurrentPickup].objectNumber);
}


void AddDisplayPickup(GAME_OBJECT_ID objectNumber)
{
	DISPLAY_PICKUP* pickup = Pickups;

	for (int i = 0; i < MAX_COLLECTED_PICKUPS; i++)
	{
		if (pickup->life < 0)
		{
			pickup->life = 45;
			pickup->objectNumber = objectNumber;

			break;
		}

		pickup++;
	}

	// No free slot found, so just pickup the object ithout displaying it
	PickedUpObject(objectNumber, 0);
}

void InitialisePickupDisplay()
{
	for (int i = 0; i < MAX_COLLECTED_PICKUPS; i++)
		Pickups[i].life = -1;

	PickupX = 128;
	PickupY = 128;
	PickupVel = 0;
	CurrentPickup = 0;
}

int FlashIt()
{
	if (FlashCount)
		FlashCount--;
	else
	{
		FlashState ^= 1;
		FlashCount = 5;
	}

	return FlashState;
}
