#include "bubble.h"
using namespace std;
extern vector<BUBBLE_STRUCT> Bubbles = vector<BUBBLE_STRUCT>(MAX_BUBBLES);

void UpdateBubbles()
{
	for (int i = 0; i < MAX_BUBBLES; i++)
	{
		BUBBLE_STRUCT* bubble = &Bubbles[i];
		if (!bubble->active) {
			continue;
		}
		bubble->age++;
		float alpha = bubble->age / 15.0f;
		alpha = fmin(alpha, 1.0f);

		bubble->size = lerp(0, bubble->destinationSize, alpha);
		bubble->color = Vector4::Lerp(bubble->sourceColor, bubble->destinationColor, alpha);
		int ceilingHeight = Rooms[bubble->roomNumber].maxceiling;
		short roomNumber = bubble->roomNumber;


		FLOOR_INFO* floor = GetFloor(bubble->worldPosition.x, bubble->worldPosition.y, bubble->worldPosition.z, &roomNumber);
		int height = GetFloorHeight(floor, bubble->worldPosition.x, bubble->worldPosition.y, bubble->worldPosition.z);

		if (bubble->worldPosition.y > height || !floor)
		{
			bubble->active = 0;
			continue;
		}

		if (!(Rooms[roomNumber].flags & ENV_FLAG_WATER))
		{
			SetupRipple(bubble->worldPosition.x, Rooms[bubble->roomNumber].maxceiling, bubble->worldPosition.z, (GetRandomControl() & 0xF) + 48, RIPPLE_FLAG_SHORT_LIFE + RIPPLE_FLAG_RAND_ROT);
			bubble->active = false;
			continue;
		}

		int ceiling = GetCeiling(floor, bubble->worldPosition.x, bubble->worldPosition.y, bubble->worldPosition.z);
		if (ceiling == NO_HEIGHT || bubble->worldPosition.y <= ceiling)
		{
			bubble->active = false;
			continue;
		}

		bubble->wavePeriod += bubble->waveSpeed;
		bubble->worldPositionCenter.y -= bubble->speed;
		bubble->worldPosition = bubble->worldPositionCenter + bubble->amplitude * Vector3(sin(bubble->wavePeriod.x), sin(bubble->wavePeriod.y), sin(bubble->wavePeriod.z));
	}
}

int GetFreeBubble() //8BEAC(<), 8DEF0(<) (F)
{
	int oldestAgeIndex = 0;
	int oldestAge = 0;
	for (int i = 0; i < MAX_BUBBLES; i++) {
		BUBBLE_STRUCT* bub = &Bubbles[i];
		if (!bub->active) {
			return i;
		}
		if (oldestAge < bub->age) {
			oldestAge = bub->age;
			oldestAgeIndex = i;
		}
	}
	//incase we dont find any non-active bubble, take the one with the oldest age
	return oldestAgeIndex;
}

void CreateBubble(PHD_VECTOR* pos, short roomNum, int unk1, int unk2, int flags, int xv, int yv, int zv) //8BF14(<), 8DF58(<) (F)
{
	if (Rooms[roomNum].flags & ENV_FLAG_WATER)
	{
		BUBBLE_STRUCT* bubble = &Bubbles[GetFreeBubble()];
		bubble->active = true;
		bubble->size = 0;
		bubble->age = 0;
		bubble->speed = flags & BUBBLE_FLAG_CLUMP ? frandMinMax(32, 48) : frandMinMax(16, 32);
		bubble->sourceColor = Vector4(0, 0, 0, 1);
		float shade = frandMinMax(0.3, 0.8);
		bubble->destinationColor = Vector4(shade, shade, shade, 1);
		bubble->color = bubble->sourceColor;
		bubble->destinationSize = flags & BUBBLE_FLAG_BIG_SIZE ? frandMinMax(256, 512) : frandMinMax(96, 128);
		bubble->spriteNum = flags & BUBBLE_FLAG_CLUMP ? SPR_UNKNOWN1 : SPR_BUBBLES;
		bubble->rotation = frandMinMax(-1, 1) * PI;
		bubble->worldPosition = Vector3(pos->x, pos->y, pos->z);
		float maxAmplitude = flags & BUBBLE_FLAG_HIGH_AMPLITUDE ? 256 : 32;
		bubble->amplitude = Vector3(frandMinMax(-maxAmplitude, maxAmplitude), frandMinMax(-maxAmplitude, maxAmplitude), frandMinMax(-maxAmplitude, maxAmplitude));
		bubble->worldPositionCenter = bubble->worldPosition;
		bubble->wavePeriod = Vector3::Zero;
		bubble->waveSpeed = Vector3(1 / frandMinMax(8, 16), 1 / frandMinMax(8, 16), 1 / frandMinMax(8, 16));
		bubble->roomNumber = roomNum;
	}
}

void Inject_Bubble() {
	INJECT(0x00483540, UpdateBubbles);
	INJECT(0x004832C0, GetFreeBubble);
	INJECT(0x00483350, CreateBubble);
}
