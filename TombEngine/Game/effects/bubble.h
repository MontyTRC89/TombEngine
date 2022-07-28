#pragma once
#include "Game/effects/effects.h"

constexpr float MAX_BUBBLES = 256;
constexpr int BUBBLE_FLAG_BIG_SIZE = 0x1;
constexpr int BUBBLE_FLAG_CLUMP = 0x2;
constexpr int BUBBLE_FLAG_HIGH_AMPLITUDE = 0x4;

struct BUBBLE_STRUCT
{
	Vector4 color;
	Vector4 sourceColor;
	Vector4 destinationColor;
	Vector3 worldPositionCenter; // goes straight up
	Vector3 worldPosition; // actual position with wave motion
	Vector3 amplitude;
	Vector3 wavePeriod;
	Vector3 waveSpeed;
	float speed;
	float size;
	float destinationSize;
	float rotation;
	int roomNumber;
	int spriteNum;
	int age;
	bool active;
};
extern std::vector<BUBBLE_STRUCT> Bubbles;

void DisableBubbles();
void UpdateBubbles();
int GetFreeBubble();//8BEAC(<), 8DEF0(<) (F)
void CreateBubble(Vector3Int* pos, short roomNum, int unk1, int unk2, int flags, int xv, int yv, int zv);//8BF14(<), 8DF58(<) (F)
