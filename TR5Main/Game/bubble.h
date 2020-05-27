#pragma once
#include <d3d11.h>
#include <SimpleMath.h>
#include <vector>
#include "types.h"
#include "constants.h"
#include "effect2.h"

constexpr float MAX_BUBBLES = 256;
constexpr int BUBBLE_FLAG_BIG_SIZE = 0x1;
constexpr int BUBBLE_FLAG_CLUMP = 0x2;
constexpr int BUBBLE_FLAG_HIGH_AMPLITUDE = 0x4;

struct BUBBLE_STRUCT
{
	DirectX::SimpleMath::Vector4 color;
	DirectX::SimpleMath::Vector4 sourceColor;
	DirectX::SimpleMath::Vector4 destinationColor;
	DirectX::SimpleMath::Vector3 worldPositionCenter; // goes straight up
	DirectX::SimpleMath::Vector3 worldPosition; // actual position with wave motion
	DirectX::SimpleMath::Vector3 amplitude;
	DirectX::SimpleMath::Vector3 wavePeriod;
	DirectX::SimpleMath::Vector3 waveSpeed;
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

void UpdateBubbles();
int GetFreeBubble();//8BEAC(<), 8DEF0(<) (F)
void CreateBubble(PHD_VECTOR* pos, short roomNum, int unk1, int unk2, int flags, int xv, int yv, int zv);//8BF14(<), 8DF58(<) (F)
