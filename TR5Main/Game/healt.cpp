#include "healt.h"
#include "draw.h"
#include "pickup.h"

short PickupX;
short PickupY;
short CurrentPickup;
DISPLAY_PICKUP Pickups[NUM_PICKUPS];
short PickupVel;

void DrawHealtBar(int percentual)
{
	g_Renderer->DrawHealthBar(percentual);
}

void DrawAirBar(int percentual)
{
	g_Renderer->DrawAirBar(percentual);
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
	int i = 0;
	do
	{
		if (Pickups[pickupIndex].life > 0)
			break;
		pickupIndex = (pickupIndex + 1) & 7;
		i++;
	} while (i < 8);

	CurrentPickup = pickupIndex;
	if (i != 8)
		return g_Renderer->DrawPickup(Pickups[CurrentPickup].objectNumber);

	CurrentPickup = 0;

	return 0;
}


void AddDisplayPickup(short objectNumber)
{
	for (int i = 0; i < 8; i++)
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
	for (int i = 0; i < 8; i++)
	{
		DISPLAY_PICKUP* pickup = &Pickups[i];
		pickup->life = -1;
	}

	PickupX = 128;
	PickupY = 128;
	PickupVel = 0;
	CurrentPickup = 0;
}

void Inject_Healt()
{
	INJECT(0x004B1950, DrawHealtBar);
	INJECT(0x004B18E0, DrawAirBar);
}
