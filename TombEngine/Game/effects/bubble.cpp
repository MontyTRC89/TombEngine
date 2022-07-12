#include "framework.h"
#include "Game/effects/bubble.h"

#include "Game/collision/collide_room.h"
#include "Game/control/control.h"
#include "Objects/objectslist.h"
#include "Specific/level.h"
#include "Specific/setup.h"
#include "Specific/prng.h"
#include "Specific/trmath.h"

using std::vector;
using namespace TEN::Math::Random;

extern vector<BUBBLE_STRUCT> Bubbles = vector<BUBBLE_STRUCT>(MAX_BUBBLES);

void DisableBubbles()
{
	for (int i = 0; i < MAX_BUBBLES; i++)
	{
		auto* bubble = &Bubbles[i];
		bubble->active = false;
	}
}

void UpdateBubbles()
{
	for (int i = 0; i < MAX_BUBBLES; i++)
	{
		auto* bubble = &Bubbles[i];

		if (!bubble->active)
			continue;

		bubble->age++;
		float alpha = bubble->age / 15.0f;
		alpha = fmin(alpha, 1.0f);

		bubble->size = Lerp(0.0f, bubble->destinationSize, alpha);
		bubble->color = Vector4::Lerp(bubble->sourceColor, bubble->destinationColor, alpha);
		int ceilingHeight = g_Level.Rooms[bubble->roomNumber].maxceiling;
		short roomNumber = bubble->roomNumber;

		auto probe = GetCollision(bubble->worldPosition.x, bubble->worldPosition.y, bubble->worldPosition.z, bubble->roomNumber);
		FloorInfo* floor = GetFloor(bubble->worldPosition.x, bubble->worldPosition.y, bubble->worldPosition.z, &roomNumber);

		if (bubble->worldPosition.y > probe.Position.Floor || !floor)
		{
			bubble->active = 0;
			continue;
		}

		if (!TestEnvironment(ENV_FLAG_WATER, probe.RoomNumber))
		{
			SetupRipple(bubble->worldPosition.x, g_Level.Rooms[bubble->roomNumber].maxceiling, bubble->worldPosition.z, (GetRandomControl() & 0xF) + 48, RIPPLE_FLAG_SHORT_INIT);
			bubble->active = false;
			continue;
		}

		if (probe.Position.Ceiling == NO_HEIGHT || bubble->worldPosition.y <= probe.Position.Ceiling)
		{
			bubble->active = false;
			continue;
		}

		bubble->wavePeriod += bubble->waveSpeed;
		bubble->worldPositionCenter.y -= bubble->speed;
		bubble->worldPosition = bubble->worldPositionCenter + bubble->amplitude * Vector3(sin(bubble->wavePeriod.x), sin(bubble->wavePeriod.y), sin(bubble->wavePeriod.z));
	}
}

int GetFreeBubble()
{
	int oldestLifeIndex = 0;
	int oldestAge = 0;

	for (int i = 0; i < MAX_BUBBLES; i++)
	{
		auto* bubble = &Bubbles[i];

		if (!bubble->active)
			return i;

		if (oldestAge < bubble->age)
		{
			oldestAge = bubble->age;
			oldestLifeIndex = i;
		}
	}

	// In case we don't find any inactive bubble, take the oldest one.
	return oldestLifeIndex;
}

void CreateBubble(Vector3Int* pos, short roomNum, int unk1, int unk2, int flags, int xv, int yv, int zv)
{
	if (g_Level.Rooms[roomNum].flags & ENV_FLAG_WATER)
	{
		auto* bubble = &Bubbles[GetFreeBubble()];

		bubble->active = true;
		bubble->size = 0;
		bubble->age = 0;
		bubble->speed = flags & BUBBLE_FLAG_CLUMP ? GenerateFloat(8, 16) : GenerateFloat(8, 12);
		bubble->sourceColor = Vector4(0, 0, 0, 1);

		float shade = GenerateFloat(0.3f, 0.8f);

		bubble->destinationColor = Vector4(shade, shade, shade, 1);
		bubble->color = bubble->sourceColor;
		bubble->destinationSize = flags & BUBBLE_FLAG_BIG_SIZE ? GenerateFloat(256, 512) : GenerateFloat(32, 128);
		bubble->spriteNum = flags & BUBBLE_FLAG_CLUMP ? SPR_UNKNOWN1 : SPR_BUBBLES;
		bubble->rotation = 0;
		bubble->worldPosition = Vector3(pos->x, pos->y, pos->z);

		float maxAmplitude = (flags & BUBBLE_FLAG_HIGH_AMPLITUDE) ? 256 : 32;

		bubble->amplitude = Vector3(GenerateFloat(-maxAmplitude, maxAmplitude), GenerateFloat(-maxAmplitude, maxAmplitude), GenerateFloat(-maxAmplitude, maxAmplitude));
		bubble->worldPositionCenter = bubble->worldPosition;
		bubble->wavePeriod = Vector3::Zero;
		bubble->waveSpeed = Vector3(1 / GenerateFloat(8, 16), 1 / GenerateFloat(8, 16), 1 / GenerateFloat(8, 16));
		bubble->roomNumber = roomNum;
	}
}
