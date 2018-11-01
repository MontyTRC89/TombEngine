#include "larafire.h"
#include "items.h"
#include <stdio.h>

WEAPON_INFO Weapons[NUM_WEAPONS] =
{
	/* No weapons */
	{ { ANGLE(0), ANGLE(0), ANGLE(0), ANGLE(0) },{ ANGLE(0), ANGLE(0), ANGLE(0), ANGLE(0) },{ ANGLE(0), ANGLE(0), ANGLE(0), ANGLE(0) }, 0x0000, 0x0000, 0x0000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x0000 },
	/* Pistols */
	{ { 54616, ANGLE(60), 54616, ANGLE(60) },{ 34596, ANGLE(60), 50976, ANGLE(80) },{ 54616, ANGLE(170), 50976, ANGLE(80) }, 0x071C, 0x05B0, 0x028A, 0x2000, 100, 0x09, 0x03, 0x00, 0x0008 },
	/* Revolver */
	{ { 54616, ANGLE(60), 54616, ANGLE(60) },{ 63716, ANGLE(10), 50976, ANGLE(80) },{ ANGLE(0), ANGLE(0), ANGLE(0), ANGLE(0) }, 0x071C, 0x02D8, 0x028A, 0x2000, 0x15, 0x10, 0x03, 0x00, 0x0079 },
	/* Uzis */
	{ { 54616, ANGLE(60), 54616, ANGLE(60) },{ 34596, ANGLE(60), 50976, ANGLE(80) },{ 54616, ANGLE(170), 50976, ANGLE(80) }, 0x071C, 0x05B0, 0x028A, 0x2000, 0x01, 0x03, 0x03, 0x00, 0x002B },
	/* Shotgun */
	{ { 54616, ANGLE(60), 55526, ANGLE(55) },{ 50976, ANGLE(80), 53706, ANGLE(65) },{ 50976, ANGLE(80), 53706, ANGLE(65) }, 0x071C, 0x0000, 0x01F4, 0x2000, 0x03, 0x09, 0x03, 0x0A, 0x002D },
	/* HK */
	{ { 54616, ANGLE(60), 55526, ANGLE(55) },{ 50976, ANGLE(80), 53706, ANGLE(65) },{ 50976, ANGLE(80), 53706, ANGLE(65) }, 0x071C, 0x02D8, 0x01F4, 0x3000, 0x04, 0x00, 0x03, 0x10, 0x0000 },
	/* Crossbow */
	{ { 54616, ANGLE(60), 55526, ANGLE(55) },{ 50976, ANGLE(80), 53706, ANGLE(65) },{ 50976, ANGLE(80), 53706, ANGLE(65) }, 0x071C, 0x05B0, 0x01F4, 0x2000, 0x05, 0x00, 0x02, 0x0A, 0x0000 },
	/* Flare */
	{ { ANGLE(0), ANGLE(0), ANGLE(0), ANGLE(0) },{ ANGLE(0), ANGLE(0), ANGLE(0), ANGLE(0) },{ ANGLE(0), ANGLE(0), ANGLE(0), ANGLE(0) }, 0x0000, 0x0000, 0x0000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x0000 },
	/* Flare 2 */
	{ { 60076, ANGLE(30), 55526, ANGLE(55) },{ 60076, ANGLE(30), 55526, ANGLE(55) },{ 60076, ANGLE(30), 55526, ANGLE(55) }, 0x071C, 0x05B0, 0x0190, 0x2000, 0x03, 0x00, 0x02, 0x00, 0x002B },
	/* Grenade launcher */
	{ { -ANGLE(60), ANGLE(60), -ANGLE(55), ANGLE(55) },{ -ANGLE(80), ANGLE(80), -ANGLE(65), ANGLE(65) },{ -ANGLE(80), ANGLE(80), -ANGLE(65), ANGLE(65) }, ANGLE(10), ANGLE(8), 500, 8 * WALL_SIZE, 20, 0, 2, 10, 0 }, 
	/* Harpoon gun */
	{ { -ANGLE(60), ANGLE(60), -ANGLE(65), ANGLE(65) },{ -ANGLE(20), ANGLE(20), -ANGLE(75), ANGLE(75) },{ -ANGLE(80), ANGLE(80), -ANGLE(75), ANGLE(75) }, ANGLE(10), ANGLE(8), 500, 8 * WALL_SIZE, 6, 0, 2, 10, 0 },
	/* Rocket launcher */
	{ { -ANGLE(60), ANGLE(60), -ANGLE(55), ANGLE(55) },{ -ANGLE(80), ANGLE(80), -ANGLE(65), ANGLE(65) },{ -ANGLE(80), ANGLE(80), -ANGLE(65), ANGLE(65) }, ANGLE(10), ANGLE(8), 500, 8 * WALL_SIZE, 30, 0, 2, 12, 77 }
};

void __cdecl SmashItem(__int16 itemNum)
{
	/*ITEM_INFO* item = &Items[itemNum];
	__int16 objectNumber = item->objectNumber;
	printf("SmashItem\n");
	if (objectNumber >= ID_SMASH_OBJECT1 && objectNumber <= ID_SMASH_OBJECT8)
		SmashObject(itemNum);
	else if (objectNumber == ID_BELL_SWITCH)
	{
		if (item->status != ITEM_ACTIVE)
		{
			item->status = ITEM_ACTIVE;
			AddActiveItem(itemNum);
		}
	}*/
}

void Inject_LaraFire()
{
	//INJECT(0x00453A90, SmashItem);
}