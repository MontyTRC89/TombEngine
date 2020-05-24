#include "healt.h"
#include "draw.h"
#include "pickup.h"
#include "lara.h"
#include "camera.h"
#include "level.h"
#include "control.h"

short PickupX;
short PickupY;
short CurrentPickup;
DISPLAY_PICKUP Pickups[MAX_COLLECTED_PICKUPS];
short PickupVel;
int OldHitPoints = 1000;
int HealtBarTimer = 40;
int FlashState = 0;
int FlashCount = 0;
int PoisonFlag = 0;
int DashTimer = 0;
extern RendererHUDBar* g_HealthBar;
extern RendererHUDBar* g_DashBar;
extern RendererHUDBar* g_AirBar;

void DrawHealthBarOverlay(int value)
{
	if (CurrentLevel)
	{
		int color2 = 0;
		if (Lara.poisoned || Lara.gassed)
			color2 = 0xA0A000;
		else
			color2 = 0xA00000;
		g_Renderer->DrawBar(value, g_HealthBar);
	}
}

void DrawHealthBar(float value)
{
	if (CurrentLevel)
	{
		//int color2;
		//if (Lara.poisoned || Lara.gassed)
		//	color2 = 0xA0A000;
		//else
		//	color2 = 0xA00000;
		g_Renderer->DrawBar(value,g_HealthBar);
	}
}

void UpdateHealtBar(int flash)
{
	int hitPoints = LaraItem->hitPoints;

	if (hitPoints < 0)
		hitPoints = 0;
	else if (hitPoints > 1000)
		hitPoints = 1000;

	if (OldHitPoints != hitPoints)
	{
		OldHitPoints = hitPoints;
		HealtBarTimer = 40;
	}

	if (HealtBarTimer < 0)
		HealtBarTimer = 0;

	if (hitPoints <= 1000 / 4)
	{
		if (!BinocularRange)
		{
			if (flash)
				DrawHealthBar(hitPoints / 1000.0f);
			else
				DrawHealthBar(0);
		}
		else 
		{
			if (flash)
				DrawHealthBarOverlay(hitPoints / 1000.0f);
			else
				DrawHealthBarOverlay(0);
		}
	}
	else if ((HealtBarTimer > 0)
		|| (hitPoints <= 0)
		|| (Lara.gunStatus == LG_READY && Lara.gunType != WEAPON_TORCH)
		|| (Lara.poisoned >= 256))
	{
		if (!BinocularRange && !SniperOverlay)
		{
			DrawHealthBar(hitPoints / 1000.0f);
		}
		else
		{
			DrawHealthBarOverlay(hitPoints / 1000.0f);
		}
	}

	if (PoisonFlag)
		PoisonFlag--;
}

void DrawAirBar(float value)
{
	if (CurrentLevel)
	{
		g_Renderer->DrawBar(value, g_AirBar);
	}
}

void UpdateAirBar(int flash)
{
	if (Lara.air == 1800 || LaraItem->hitPoints <= 0)
		return;

	if ((Lara.Vehicle == NO_ITEM)
		|| (Items[Lara.Vehicle].objectNumber != ID_UPV))
	{
		if ((Lara.waterStatus != LW_UNDERWATER)
			&& (Lara.waterStatus != LW_SURFACE)
			&& (!((Rooms[LaraItem->roomNumber].flags & ENV_FLAG_SWAMP)
				&& (Lara.waterSurfaceDist < -775))))
			return;
	}

	int air = Lara.air;
	if (air < 0)
		air = 0;
	else if (air > 1800)
		air = 1800;
	if (air <= 450)
	{
		if (flash)
			DrawAirBar(air/ 1800.0f);
		else
			DrawAirBar(0);
	}
	else
		DrawAirBar(air / 1800.0f);

	if (Lara.gassed)
	{
		if (Lara.dpoisoned < 2048)
			Lara.dpoisoned += 2;
		Lara.gassed = false;
	}
}

void DrawDashBar(int value)
{
	if (CurrentLevel)
	{
		g_Renderer->DrawBar(value, g_DashBar);
	}
}

int DrawAllPickups()
{
	if (Pickups[CurrentPickup].life > 0)
	{
		if (PickupX > 0)
		{
			PickupX += -PickupX >> 5;
			return g_Renderer->DrawPickup(Pickups[CurrentPickup].objectNumber);
		}
		else
		{
			Pickups[CurrentPickup].life--;
			return g_Renderer->DrawPickup(Pickups[CurrentPickup].objectNumber);
		}
	}
	else if (Pickups[CurrentPickup].life == 0)
	{
		if (PickupX < 128)
		{
			if (PickupVel < 16)
				PickupVel++;
			PickupX += PickupVel >> 2;
			return g_Renderer->DrawPickup(Pickups[CurrentPickup].objectNumber);
		}
		else
		{
			Pickups[CurrentPickup].life = -1;
			PickupVel = 0;
			return g_Renderer->DrawPickup(Pickups[CurrentPickup].objectNumber);
		}
	}

	int pickupIndex = CurrentPickup;
	int i;
	for (i = 0; i < MAX_COLLECTED_PICKUPS; ++i)
	{
		if (Pickups[pickupIndex].life > 0)
			break;
		pickupIndex = pickupIndex + 1 & MAX_COLLECTED_PICKUPS - 1;
	}

	CurrentPickup = pickupIndex;
	if (i != MAX_COLLECTED_PICKUPS)
		return g_Renderer->DrawPickup(Pickups[CurrentPickup].objectNumber);

	CurrentPickup = 0;

	return 0;
}


void AddDisplayPickup(short objectNumber)
{
	for (int i = 0; i < MAX_COLLECTED_PICKUPS; i++)
	{
		DISPLAY_PICKUP* pickup = &Pickups[i];
		if (pickup->life < 0)
		{
			pickup->life = 45;
			pickup->objectNumber = objectNumber;
			PickedUpObject(objectNumber);
			return;
		}
	}

	// No free slot found, so just pickup the object ithout displaying it
	PickedUpObject(objectNumber);
}

void InitialisePickupDisplay()
{
	for (int i = 0; i < MAX_COLLECTED_PICKUPS; i++)
	{
		DISPLAY_PICKUP* pickup = &Pickups[i];
		pickup->life = -1;
	}

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
