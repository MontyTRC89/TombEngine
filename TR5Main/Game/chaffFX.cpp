#include "chaffFX.h"
#include "bubble.h"
#include "tomb4fx.h"
#include "control.h"
#include "lara.h"
#include "draw.h"
#include "../Specific/level.h"

#define	MAX_TRIGGER_RANGE	0x4000

void TriggerChaffEffects()
{
	PHD_VECTOR vect;
	vect.x = 8;
	vect.y = 36;
	vect.z = 32;
	GetLaraJointPosition(&vect, LM_LHAND);

	PHD_VECTOR pos;
	pos.x = vect.x;
	pos.y = vect.y;
	pos.z = vect.z;

	vect.x = 8;
	vect.y = 36;
	vect.z = 1024 + (GetRandomDraw() & 255);
	GetLaraJointPosition(&vect, LM_LHAND);

	PHD_VECTOR vel;
	vel.x = vect.x - pos.x;
	vel.y = vect.y - pos.y;
	vel.z = vect.z - pos.z;

	TriggerChaffEffects(LaraItem, &pos, &vel, LaraItem->speed, (bool)(Rooms[LaraItem->roomNumber].flags & ENV_FLAG_WATER));
}

void TriggerChaffEffects(ITEM_INFO* Item)
{
	Matrix world
		= Matrix::CreateTranslation(-6, 6, 32)
		* Matrix::CreateFromYawPitchRoll(TO_RAD(Item->pos.yRot), TO_RAD(Item->pos.xRot), TO_RAD(Item->pos.zRot));

	PHD_VECTOR pos;
	pos.x = Item->pos.xPos + world.Translation().x;
	pos.y = Item->pos.yPos + world.Translation().y;
	pos.z = Item->pos.zPos + world.Translation().z;

	world
		= Matrix::CreateTranslation(-6, 6, 32)
		* Matrix::CreateTranslation((GetRandomDraw() & 127) - 64, (GetRandomDraw() & 127) - 64, (GetRandomDraw() & 511) + 512)
		* Matrix::CreateFromYawPitchRoll(TO_RAD(Item->pos.yRot), TO_RAD(Item->pos.xRot), TO_RAD(Item->pos.zRot));

	PHD_VECTOR vel;
	vel.x = world.Translation().x;
	vel.y = world.Translation().y;
	vel.z = world.Translation().z;

	TriggerChaffEffects(Item, &pos, &vel, Item->speed, (bool)(Rooms[Item->roomNumber].flags & ENV_FLAG_WATER));
}

void TriggerChaffEffects(ITEM_INFO* item, PHD_VECTOR* pos, PHD_VECTOR* vel, int speed, bool isUnderwater)
{
	int numSparks = (int)frandMinMax(2, 5);
	for (int i = 0; i < numSparks; i++)
	{
		long	dx, dz;

		dx = item->pos.xPos - pos->x;
		dz = item->pos.zPos - pos->z;

		if (dx < -MAX_TRIGGER_RANGE || dx > MAX_TRIGGER_RANGE || dz < -MAX_TRIGGER_RANGE || dz > MAX_TRIGGER_RANGE)
			return;

		CVECTOR color;
		color.r = 255;
		color.g = (GetRandomDraw() & 127) + 64;
		color.b = 192 - color.g;

		TriggerChaffSparkles(pos, vel, &color);
		if (isUnderwater)
		{
			TriggerChaffBubbles(pos, item->roomNumber);
		}
		else
		{
			if (speed > 32)
				TriggerChaffSmoke(pos, vel, 64 - speed, TRUE, (bool)(Rooms[item->roomNumber].flags & ENV_FLAG_WIND));
			else
			{
				if (GetRandomControl() & 3)
					speed = 0;
				else
					speed = ((GetRandomControl() & 0xF) + (GetRandomControl() & 0x10)) << 6;

				TriggerChaffSmoke(pos, vel, speed, FALSE, (bool)(Rooms[item->roomNumber].flags & ENV_FLAG_WIND));
			}
		}
	}
}


void TriggerChaffSparkles (PHD_VECTOR* pos, PHD_VECTOR* vel, CVECTOR* color)
{
	SPARKS* sparkle;

	sparkle = &Sparks[GetFreeSpark()];

	sparkle->on = true;

	sparkle->sR = 255;
	sparkle->sG = 255;
	sparkle->sB = 255;

	sparkle->dR = color->r;
	sparkle->dG = color->g;
	sparkle->dB = color->b;

	sparkle->colFadeSpeed = 3;
	sparkle->fadeToBlack = 5;
	sparkle->sLife = sparkle->life = 10;
	sparkle->transType = COLADD;
	sparkle->dynamic = true;

	sparkle->x = pos->x + (GetRandomDraw() & 7) - 3;
	sparkle->y = pos->y + (GetRandomDraw() & 7) - 3;
	sparkle->z = pos->z + (GetRandomDraw() & 7) - 3;
	sparkle->xVel = vel->x + ((GetRandomDraw() & 255) - 128);
	sparkle->yVel = vel->y + ((GetRandomDraw() & 255) - 128);
	sparkle->zVel = vel->z + ((GetRandomDraw() & 255) - 128);
	sparkle->friction = 2 | (2 << 4);
	sparkle->scalar = 1;
	sparkle->size = sparkle->sSize = (GetRandomDraw() & 3) + 4;
	sparkle->dSize = (GetRandomDraw() & 1) + 1;
	sparkle->gravity = sparkle->maxYvel = 0;
	sparkle->flags = SP_SCALE;
}


void TriggerChaffSmoke(PHD_VECTOR* pos, PHD_VECTOR* vel, int speed, bool moving, bool wind)
{
	SMOKE_SPARKS* smoke;

	int rnd = 0;
	BYTE trans, size;
	
	smoke = &SmokeSparks[GetFreeSmokeSpark()];

	smoke->on = true;

	smoke->sShade = 0;
	if (moving)
	{
		trans = (speed << 7) >> 5;
		smoke->dShade = trans;
	}
	else
		smoke->dShade = 64 + (GetRandomDraw() & 7);

	smoke->colFadeSpeed = 4 + (GetRandomDraw() & 3);
	smoke->fadeToBlack = 4;

	rnd = (GetRandomControl() & 3) - (speed >> 12) + 20;
	if (rnd < 9)
	{
		smoke->life = 9;
		smoke->sLife = 9;
	}
	else
	{
		smoke->life = rnd;
		smoke->sLife = rnd;
	}

	smoke->transType = COLADD;
	
	smoke->x = pos->x + (GetRandomControl() & 7) - 3;
	smoke->y = pos->y + (GetRandomControl() & 7) - 3;
	smoke->z = pos->z + (GetRandomControl() & 7) - 3;
	smoke->xVel = vel->x + ((GetRandomDraw() & 63) - 32);
	smoke->yVel = vel->y;
	smoke->zVel = vel->z + ((GetRandomDraw() & 63) - 32);
	smoke->friction = 4;

	if (GetRandomControl() & 1)
	{
		smoke->flags = SP_EXPDEF | SP_ROTATE | SP_DEF | SP_SCALE;
		smoke->rotAng = (GetRandomControl() & 0xFFF);
		if (GetRandomControl() & 1)
			smoke->rotAdd = (GetRandomControl() & 7) - 24;
		else
			smoke->rotAdd = (GetRandomControl() & 7) + 24;
	}
	else
	{
		smoke->flags = SP_EXPDEF | SP_DEF | SP_SCALE;
	}

	if (wind)
		smoke->flags |= SP_WIND;

	smoke->scalar = 1;
	smoke->gravity = (GetRandomControl() & 3) - 4;
	smoke->maxYvel = 0;
	size = (GetRandomControl() & 7) + (speed >> 7) + 32;
	smoke->sSize = size >> 2;
	smoke->size = smoke->dSize = size;
}


void TriggerChaffBubbles(PHD_VECTOR* pos, int FlareRoomNumber)
{
	BUBBLE_STRUCT* bubble;

	bubble = &Bubbles[GetFreeBubble()];
	bubble->active = true;
	bubble->size = 0;
	bubble->age = 0;
	bubble->speed = frandMinMax(16, 32);
	bubble->sourceColor = Vector4(0, 0, 0, 0);
	float shade = frandMinMax(0.3, 0.8);
	bubble->destinationColor = Vector4(shade, shade, shade, 0.8);
	bubble->color = bubble->sourceColor;
	bubble->destinationSize = frandMinMax(128, 256);
	bubble->spriteNum = SPR_UNKNOWN1;
	bubble->rotation = frandMinMax(-1, 1) * PI;
	bubble->worldPosition = Vector3(pos->x, pos->y, pos->z);
	float maxAmplitude = 32;
	bubble->amplitude = Vector3(frandMinMax(-maxAmplitude, maxAmplitude), frandMinMax(-maxAmplitude, maxAmplitude), frandMinMax(-maxAmplitude, maxAmplitude));
	bubble->worldPositionCenter = bubble->worldPosition;
	bubble->wavePeriod = Vector3::Zero;
	bubble->waveSpeed = Vector3(1 / frandMinMax(8, 16), 1 / frandMinMax(8, 16), 1 / frandMinMax(8, 16));
	bubble->roomNumber = FlareRoomNumber;
}