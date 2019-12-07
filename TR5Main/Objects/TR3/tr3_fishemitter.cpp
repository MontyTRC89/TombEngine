#include "../newobjects.h"
#include "../../Game/effects.h"

#define PIRAHNA_DAMAGE	4
#define X	0
#define Y	1
#define Z 	2
#define XYZ	3
#define MAX_FISH 8
#define OCB_FISH_LETAL			0x1000
#define OCB_FISH_EAT_CARCASS	0x2000

FISH_LEADER_INFO LeaderInfo[MAX_FISH];
FISH_INFO Fishes[MAX_FISH + (MAX_FISH * 24)];

byte FishRanges[1][3] = {
	{ 2 << 2, 5 << 2, 3 }
};

int PirahnaHitWait = false;
int CarcassItem = NO_ITEM;

/*uchar	temple_fish_ranges[3][3] = {
								{	1 << 2,1 << 2,2	},
								{	1 << 2,4 << 2,2	},
								{	1 << 2,7 << 2,4	},
};

uchar	quadchase_fish_ranges[8][3] = {
								{	1 << 2,3 << 2,1	},
								{	0 << 2,3 << 2,2	},
								{	2 << 2,1 << 2,2	},
								{	1 << 2,2 << 2,1	},
								{	1 << 2,4 << 2,2	},
								{	1 << 2,6 << 2,1	},
								{	3 << 2,1 << 2,1	},
								{	4 << 2,1 << 2,1	},
};

uchar	house_fish_ranges[7][3] = {
								{	1 << 2,1 << 2,1	},	// Unused.
								{	4 << 2,2 << 2,2	},
								{	6 << 2,2 << 2,2	},
								{	2 << 2,4 << 2,2	},
								{	2 << 2,3 << 2,1	},
								{	5 << 2,2 << 2,2	},
								{	4 << 2,2 << 2,1	},
};

uchar	shore_fish_ranges[3][3] = {
								{	3 << 2,3 << 2,6	},
								{	3 << 2,5 << 2,6	},
								{	5 << 2,1 << 2,8	},
};

uchar	crash_fish_ranges[1][3] = {
								{	5 << 2,1 << 2,6	},
};

uchar	rapids_fish_ranges[2][3] = {
								{	4 << 2,4 << 2,8	},
								{	1 << 2,2 << 2,5	},
};*/

void SetupShoal(int shoalNumber)
{
	//if (CurrentLevel == LV_JUNGLE)
	//{
		LeaderInfo[shoalNumber].xRange = (FishRanges[shoalNumber][0] + 2) << 8;
		LeaderInfo[shoalNumber].zRange = (FishRanges[shoalNumber][1] + 2) << 8;
		LeaderInfo[shoalNumber].yRange = (FishRanges[shoalNumber][2]) << 8;
	//}
	/*else if (CurrentLevel == LV_TEMPLE)
	{
		LeaderInfo[shoalNumber].xRange = (temple_fish_ranges[shoalNumber][0] + 2) << 8;
		LeaderInfo[shoalNumber].zRange = (temple_fish_ranges[shoalNumber][1] + 2) << 8;
		LeaderInfo[shoalNumber].yRange = (temple_fish_ranges[shoalNumber][2]) << 8;
	}
	else if (CurrentLevel == LV_QUADBIKE)
	{
		LeaderInfo[shoalNumber].xRange = (quadchase_fish_ranges[shoalNumber][0] + 2) << 8;
		LeaderInfo[shoalNumber].zRange = (quadchase_fish_ranges[shoalNumber][1] + 2) << 8;
		LeaderInfo[shoalNumber].yRange = (quadchase_fish_ranges[shoalNumber][2]) << 8;
	}
	else if (CurrentLevel == LV_GYM)
	{
		LeaderInfo[shoalNumber].xRange = (house_fish_ranges[shoalNumber][0] + 2) << 8;
		LeaderInfo[shoalNumber].zRange = (house_fish_ranges[shoalNumber][1] + 2) << 8;
		LeaderInfo[shoalNumber].yRange = (house_fish_ranges[shoalNumber][2]) << 8;
	}
	else if (CurrentLevel == LV_SHORE)
	{
		LeaderInfo[shoalNumber].xRange = (shore_fish_ranges[shoalNumber][0] + 2) << 8;
		LeaderInfo[shoalNumber].zRange = (shore_fish_ranges[shoalNumber][1] + 2) << 8;
		LeaderInfo[shoalNumber].yRange = (shore_fish_ranges[shoalNumber][2]) << 8;
	}
	else if (CurrentLevel == LV_CRASH)
	{
		LeaderInfo[shoalNumber].xRange = (crash_fish_ranges[shoalNumber][0] + 2) << 8;
		LeaderInfo[shoalNumber].zRange = (crash_fish_ranges[shoalNumber][1] + 2) << 8;
		LeaderInfo[shoalNumber].yRange = (crash_fish_ranges[shoalNumber][2]) << 8;
	}
	else if (CurrentLevel == LV_RAPIDS)
	{
		LeaderInfo[shoalNumber].xRange = (rapids_fish_ranges[shoalNumber][0] + 2) << 8;
		LeaderInfo[shoalNumber].zRange = (rapids_fish_ranges[shoalNumber][1] + 2) << 8;
		LeaderInfo[shoalNumber].yRange = (rapids_fish_ranges[shoalNumber][2]) << 8;
	}
	else
	{
		LeaderInfo[shoalNumber].xRange = 1 << 8;
		LeaderInfo[shoalNumber].zRange = 1 << 8;
		LeaderInfo[shoalNumber].yRange = 1 << 8;
	}*/
}


void SetupFish(int leader, ITEM_INFO* item)
{
	int fishXRange = LeaderInfo[leader].xRange;
	int fishYRange = LeaderInfo[leader].yRange;
	int fishZRange = LeaderInfo[leader].zRange;

	Fishes[leader].x = 0;
	Fishes[leader].y = 0;
	Fishes[leader].z = 0;
	Fishes[leader].angle = 0;
	Fishes[leader].speed = (GetRandomControl() & 63) + 8;
	Fishes[leader].swim = GetRandomControl() & 63;

	for (int i = 0; i < 24; i++)
	{
		Fishes[MAX_FISH + (leader * 24) + i].x = (GetRandomControl() % (fishXRange << 1)) - fishXRange;
		Fishes[MAX_FISH + (leader * 24) + i].y = (GetRandomControl() % fishYRange);
		Fishes[MAX_FISH + (leader * 24) + i].destY = (GetRandomControl() % fishYRange);
		Fishes[MAX_FISH + (leader * 24) + i].z = (GetRandomControl() % (fishZRange << 1)) - fishZRange;

		Fishes[MAX_FISH + (leader * 24) + i].angle = GetRandomControl() & 4095;
		Fishes[MAX_FISH + (leader * 24) + i].speed = (GetRandomControl() & 31) + 32;
		Fishes[MAX_FISH + (leader * 24) + i].swim = GetRandomControl() & 63;
	}

	LeaderInfo[leader].on = 1;
	LeaderInfo[leader].angle = 0;
	LeaderInfo[leader].speed = (GetRandomControl() & 127) + 32;
	LeaderInfo[leader].angleTime = 0;
	LeaderInfo[leader].speedTime = 0;
}

void ControlFish(short itemNumber)
{
	int pirahnaAttack = 0;
	int angle = 0;

	ITEM_INFO* item = &Items[itemNumber];
	ITEM_INFO* enemy = item;

	if (!TriggerActive(item))
		return;

	int leader = item->hitPoints;
	if (!LeaderInfo[leader].on)
		SetupFish(leader, item);

	if (item->triggerFlags & OCB_FISH_LETAL)  
	{
		if ((item->triggerFlags == OCB_FISH_EAT_CARCASS) == 0)
			pirahnaAttack = (LaraItem->roomNumber == item->roomNumber);
		else
		{
			if (CarcassItem != -1)
				pirahnaAttack = 2;
			else
				pirahnaAttack = (LaraItem->roomNumber == item->roomNumber);
		}
	}
	else
		pirahnaAttack = 0;

	if (PirahnaHitWait)
		PirahnaHitWait--;

	FISH_INFO* fish = (FISH_INFO*)&Fishes[leader];

	if (pirahnaAttack)
	{
		if (pirahnaAttack == 1)
			enemy = LaraItem;
		else
			enemy = &Items[CarcassItem];

		LeaderInfo[leader].angle = fish->angle = (-(mGetAngle(fish->x + item->pos.xPos, fish->z + item->pos.zPos, enemy->pos.xPos, enemy->pos.zPos) + 0x4000) >> 4) & 4095;
		LeaderInfo[leader].speed = (GetRandomControl() & 63) + 192;
	}

	int diff = fish->angle - LeaderInfo[leader].angle;

	if (diff > 2048)
		diff -= 4096;
	else if (diff < -2048)
		diff += 4096;

	if (diff > 128)
	{
		fish->angAdd -= 4;
		if (fish->angAdd < -120)
			fish->angAdd = -120;
	}
	else	if (diff < -128)
	{
		fish->angAdd += 4;
		if (fish->angAdd > 120)
			fish->angAdd = 120;
	}
	else
	{
		fish->angAdd -= fish->angAdd >> 2;
		if (abs(fish->angAdd) < 4)
			fish->angAdd = 0;
	}

	fish->angle += fish->angAdd;

	if (diff > 1024)
		fish->angle += fish->angAdd >> 2;
	fish->angle &= 4095;

	diff = fish->speed - LeaderInfo[leader].speed;

	if (diff < -4)
	{
		diff = fish->speed + (GetRandomControl() & 3) + 1;
		if (diff < 0)
			diff = 0;
		fish->speed = diff;

	}
	else	if (diff > 4)
	{
		diff = fish->speed - (GetRandomControl() & 3) - 1;
		if (diff > 255)
			diff = 255;
		fish->speed = diff;
	}

	fish->swim += fish->speed >> 4;
	fish->swim &= 63;

	int x = fish->x;
	int z = fish->z;
	
	x += -fish->speed * SIN(fish->angle << 4) >> W2V_SHIFT;  //   -(((rcossin_tbl[(fish->angle << 1)]) * (fish->speed)) >> 13);
	z += fish->speed * COS(fish->angle << 4) >> W2V_SHIFT;  //   (((rcossin_tbl[(fish->angle << 1) + 1]) * (fish->speed)) >> 13);
	
	if (pirahnaAttack == 0)
	{
		int fishXRange = LeaderInfo[leader].xRange;
		int fishZRange = LeaderInfo[leader].zRange;
		if (z < -fishZRange)
		{
			z = -fishZRange;
			if (fish->angle < 2048)
				LeaderInfo[leader].angle = fish->angle - ((GetRandomControl() & 127) + 128);
			else
				LeaderInfo[leader].angle = fish->angle + ((GetRandomControl() & 127) + 128);
			LeaderInfo[leader].angleTime = (GetRandomControl() & 15) + 8;
			LeaderInfo[leader].speedTime = 0;
		}
		else if (z > fishZRange)
		{
			z = fishZRange;
			if (fish->angle > 3072)
				LeaderInfo[leader].angle = fish->angle - ((GetRandomControl() & 127) + 128);
			else
				LeaderInfo[leader].angle = fish->angle + ((GetRandomControl() & 127) + 128);
			LeaderInfo[leader].angleTime = (GetRandomControl() & 15) + 8;
			LeaderInfo[leader].speedTime = 0;
		}

		if (x < -fishXRange)
		{
			x = -fishXRange;
			if (fish->angle < 1024)
				LeaderInfo[leader].angle = fish->angle - ((GetRandomControl() & 127) + 128);
			else
				LeaderInfo[leader].angle = fish->angle + ((GetRandomControl() & 127) + 128);
			LeaderInfo[leader].angleTime = (GetRandomControl() & 15) + 8;
			LeaderInfo[leader].speedTime = 0;
		}
		else if (x > fishXRange)
		{
			x = fishXRange;
			if (fish->angle < 3072)
				LeaderInfo[leader].angle = fish->angle - ((GetRandomControl() & 127) + 128);
			else
				LeaderInfo[leader].angle = fish->angle + ((GetRandomControl() & 127) + 128);
			LeaderInfo[leader].angleTime = (GetRandomControl() & 15) + 8;
			LeaderInfo[leader].speedTime = 0;
		}

		if ((GetRandomControl() & 15) == 0)
			LeaderInfo[leader].angleTime = 0;

		if (LeaderInfo[leader].angleTime)
			LeaderInfo[leader].angleTime--;
		else
		{
			LeaderInfo[leader].angleTime = (GetRandomControl() & 15) + 8;
			int angAdd = ((GetRandomControl() & 63) + 16) - 8 - 32;
			if ((GetRandomControl() & 3) == 0)
				LeaderInfo[leader].angle += angAdd << 5;
			else
				LeaderInfo[leader].angle += angAdd;
			LeaderInfo[leader].angle &= 4095;
		}

		if (LeaderInfo[leader].speedTime)
			LeaderInfo[leader].speedTime--;
		else
		{
			LeaderInfo[leader].speedTime = (GetRandomControl() & 31) + 32;
			if ((GetRandomControl() & 7) == 0)
				LeaderInfo[leader].speed = (GetRandomControl() & 127) + 128;
			else if ((GetRandomControl() & 3) == 0)
				LeaderInfo[leader].speed += (GetRandomControl() & 127) + 32;
			else if (LeaderInfo[leader].speed > 140)
				LeaderInfo[leader].speed -= (GetRandomControl() & 31) + 48;
			else
			{
				LeaderInfo[leader].speedTime = (GetRandomControl() & 3) + 4;
				LeaderInfo[leader].speed += (GetRandomControl() & 31) - 15;
			}
		}

	}

	int ftx = x;
	int ftz = z;

	fish->x = x;
	fish->z = z;

	fish = (FISH_INFO*)&Fishes[MAX_FISH + (leader * 24)];

	for (int i = 0; i < 24; i++)
	{
		if (item->flags & OCB_FISH_LETAL)
		{
			PHD_3DPOS pos;
			pos.xPos = item->pos.xPos + fish->x;
			pos.yPos = item->pos.yPos + fish->y;
			pos.zPos = item->pos.zPos + fish->z;
			if (FishNearLara(&pos, 256, (pirahnaAttack < 2) ? LaraItem : enemy))
			{
				if (PirahnaHitWait == 0)
				{
					DoBloodSplat(item->pos.xPos + fish->x, item->pos.yPos + fish->y, item->pos.zPos + fish->z, 0, 0, (pirahnaAttack < 2) ? LaraItem->roomNumber : enemy->roomNumber);
					PirahnaHitWait = 8;
				}
				if (pirahnaAttack != 2)
					LaraItem->hitPoints -= PIRAHNA_DAMAGE;
			}
		}

		angle = (-(mGetAngle(fish->x, fish->z, ftx, ftz) + 0x4000) >> 4) & 4095;
		int dx = fish->x - ftx + ((24 - i) << 7);
		int dz = fish->z - ftz - ((24 - i) << 7);

		dx *= dx;
		dz *= dz;

		diff = fish->angle - angle;

		if (diff > 2048)
			diff -= 4096;
		else if (diff < -2048)
			diff += 4096;

		if (diff > 128)
		{
			fish->angAdd -= 4;
			if (fish->angAdd < -92 - (i >> 1))
				fish->angAdd = -92 - (i >> 1);
		}
		else	if (diff < -128)
		{
			fish->angAdd += 4;
			if (fish->angAdd > 92 + (i >> 1))
				fish->angAdd = 92 + (i >> 1);
		}
		else
		{
			fish->angAdd -= fish->angAdd >> 2;
			if (abs(fish->angAdd) < 4)
				fish->angAdd = 0;
		}

		fish->angle += fish->angAdd;

		if (diff > 1024)
			fish->angle += fish->angAdd >> 2;
		fish->angle &= 4095;

		if ((dx + dz) < (0x100000 + ((i << 7) * (i << 7))))
		{
			if (fish->speed > 32 + (i << 1))
				fish->speed -= fish->speed >> 5;
		}
		else
		{
			if (fish->speed < 160 + (i >> 1))
				fish->speed += (GetRandomControl() & 3) + 1 + (i >> 1);
			if (fish->speed > 160 + (i >> 1) - (i << 2))
				fish->speed = 160 + (i >> 1) - (i << 2);
		}

		if (GetRandomControl() & 1)
			fish->speed -= GetRandomControl() & 1;
		else
			fish->speed += GetRandomControl() & 1;

		if (fish->speed < 32)
			fish->speed = 32;
		else if (fish->speed > 200)
			fish->speed = 200;

		fish->swim += (fish->speed >> 4) + (fish->speed >> 5);
		fish->swim &= 63;

		x = fish->x - fish->speed * SIN(fish->angle << 4) >> W2V_SHIFT; //   (((rcossin_tbl[(fish->angle << 1)])* (fish->speed)) >> 13);
		z = fish->z + fish->speed * COS(fish->angle << 4) >> W2V_SHIFT; //

		if (z < -32000)
			z = -32000;
		else if (z > 32000)
			z = 32000;
		if (x < -32000)
			x = -32000;
		else if (x > 32000)
			x = 32000;

		fish->x = x;
		fish->z = z;

		if (pirahnaAttack == 0)
		{
			if (abs(fish->y - fish->destY) < 16)
				fish->destY = GetRandomControl() % LeaderInfo[leader].yRange;
		}
		else
		{
			int y = enemy->pos.yPos - item->pos.yPos;
			if (abs(fish->y - fish->destY) < 16)
				fish->destY = y + (GetRandomControl() & 255); 
		}

		fish->y += (fish->destY - fish->y) >> 4;
		fish++;
	}
}

bool FishNearLara(PHD_3DPOS* pos, int distance, ITEM_INFO* item)
{
	int x = pos->xPos - item->pos.xPos;
	int y = abs(pos->yPos - item->pos.yPos);
	int z = pos->zPos - item->pos.zPos;

	if (x < -distance || x > distance || z < -distance || z > distance || y < -WALL_SIZE * 3 || y > WALL_SIZE * 3)
		return false;

	if ((SQUARE(x) + SQUARE(z)) > SQUARE(distance))
		return false;

	if (y > distance)
		return false;

	return true;
}

