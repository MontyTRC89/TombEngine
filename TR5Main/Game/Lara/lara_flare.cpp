#include "framework.h"
#include "lara_flare.h"
#include "lara_tests.h"
#include "level.h"
#include "setup.h"
#include "Sound/sound.h"
#include "animation.h"
#include "items.h"
#include "lara_fire.h"
#include "Lara.h"
#include "collide.h"
#include "effects/effects.h"
#include "effects/chaffFX.h"
#include "Specific/prng.h"

constexpr std::array<float, 28> FlareFlickerTable = { 0.7590,0.9880,0.8790,0.920,0.8020,0.7610,0.97878,0.8978,0.9983,0.934763,0.8485,0.762573,0.84642,0.7896,0.817634,0.923424,0.7589,0.81399,0.92834,0.9978,0.7610,0.97878,0.8978,0.9983,0.934763,0.8485,0.762573,0.74642 };
constexpr DirectX::SimpleMath::Vector3 FlareMainColor = Vector3(1,0.52947, 0.3921);
constexpr std::array<float, 28> FlareFlickerTableLow = { 0.7590,0.1880,0.0790,0.920,0.8020,0.07610,0.197878,0.38978,0.09983,0.00934763,0.8485,0.0762573,0.84642,0.7896,0.517634,0.0923424,0.7589,0.081399,0.92834,0.01978,0.17610,0.497878,0.8978,0.69983,0.934763,0.28485,0.1762573,0.374642 };

using namespace TEN::Math::Random;

void FlareControl(short itemNumber)
{
	ITEM_INFO* flare = &g_Level.Items[itemNumber];

	if (TestLaraSwamp(flare))
	{
		KillItem(itemNumber);

		return;
	}

	if (flare->fallspeed)
	{
		flare->pos.xRot += ANGLE(3.0f);
		flare->pos.zRot += ANGLE(5.0f);
	}
	else
	{
		flare->pos.xRot = 0;
		flare->pos.zRot = 0;
	}

	auto oldX = flare->pos.xPos;
	auto oldY = flare->pos.yPos;
	auto oldZ = flare->pos.zPos;

	int xv = flare->speed * phd_sin(flare->pos.yRot);
	int zv = flare->speed * phd_cos(flare->pos.yRot);

	flare->pos.xPos += xv;
	flare->pos.zPos += zv;


	if (TestLaraWater(flare) || TestLaraSwamp(flare)) // TODO: Generic water/swamp test function.
	{
		flare->fallspeed += (5 - flare->fallspeed) / 2;
		flare->speed += (5 - flare->speed) / 2;
	}
	else
		flare->fallspeed += 6;

	flare->pos.yPos += flare->fallspeed;

	DoProjectileDynamics(itemNumber, oldX, oldY, oldZ, xv, flare->fallspeed, zv);

	short& age = flare->data;
	age &= 0x7FFF;
	if (age >= FLARE_AGE)
	{
		if (!flare->fallspeed && !flare->speed)
		{
			KillItem(itemNumber);

			return;
		}
	}
	else
		age++;

	if (DoFlareLight((PHD_VECTOR*)&flare->pos, age))
	{
		TriggerChaffEffects(flare,age);
		/* Hardcoded code */

		age |= 0x8000;
	}
}

void ready_flare(ITEM_INFO* lara)
{
	LaraInfo*& info = lara->data;

	info->gunStatus = LG_HANDS_FREE;
	info->leftArm.zRot = 0;
	info->leftArm.yRot = 0;
	info->leftArm.xRot = 0;
	info->rightArm.zRot = 0;
	info->rightArm.yRot = 0;
	info->rightArm.xRot = 0;
	info->rightArm.lock = false;
	info->leftArm.lock = false;
	info->target = NULL;
}

void undraw_flare_meshes(ITEM_INFO* lara)
{
	LaraInfo*& info = lara->data;

	info->meshPtrs[LM_LHAND] = Objects[ID_LARA_SKIN].meshIndex + LM_LHAND;
}

void draw_flare_meshes(ITEM_INFO * lara)
{
	LaraInfo*& info = lara->data;

	info->meshPtrs[LM_LHAND] = Objects[ID_LARA_FLARE_ANIM].meshIndex + LM_LHAND;
}

void undraw_flare(ITEM_INFO* lara)
{
	LaraInfo*& info = lara->data;

	info->flareControlLeft = true;

	short frame1 = info->flareFrame;
	short frame2 = info->leftArm.frameNumber;

	if (lara->goalAnimState == LS_IDLE 
		&& info->Vehicle == NO_ITEM)
	{
		if (lara->animNumber == LA_STAND_IDLE)
		{
			lara->animNumber = LA_DISCARD_FLARE;
			frame1 = frame2 + g_Level.Anims[lara->animNumber].frameBase;
			info->flareFrame = frame1;
			lara->frameNumber = frame1;
		}

		if (lara->animNumber == LA_DISCARD_FLARE)
		{
			info->flareControlLeft = false;

			if (frame1 >= g_Level.Anims[lara->animNumber].frameBase + 31)
			{
				info->requestGunType = info->lastGunType;
				info->gunType = info->lastGunType;
				info->gunStatus = LG_HANDS_FREE;

				InitialiseNewWeapon(lara);

				info->target = NULL;
				info->rightArm.lock = false;
				info->leftArm.lock = false;
				SetAnimation(lara, LA_STAND_SOLID);
				info->flareFrame = g_Level.Anims[lara->animNumber].frameBase;

				return;
			}

			info->flareFrame++;
		}
	}
	else if (lara->animNumber == LA_DISCARD_FLARE)
	{
		lara->animNumber = LA_STAND_SOLID;
		lara->frameNumber = g_Level.Anims[lara->animNumber].frameBase;
	}

	if (frame2 >= 33 && frame2 < 72)
	{
		frame2 = 2;
		DoFlareInHand(lara, info->flareAge);
	}
	else if (!frame2)
	{
		frame2 = 1;
		DoFlareInHand(lara, info->flareAge);
	}
	else if (frame2 >= 72 && frame2 < 95)
	{
		frame2++;

		if (frame2 == 94)
		{
			frame2 = 1;
			DoFlareInHand(lara, info->flareAge);
		}
	}
	else if (frame2 >= 1 && frame2 < 33)
	{
		frame2++;

		if (frame2 == 21)
		{
			CreateFlare(lara, ID_FLARE_ITEM, 1);
			undraw_flare_meshes(lara);
			info->flareAge = 0;
		}
		else if (frame2 == 33)
		{
			frame2 = 0;
			info->requestGunType = info->lastGunType;
			info->gunType = info->lastGunType;
			info->gunStatus = LG_HANDS_FREE;

			InitialiseNewWeapon(lara);

			info->flareControlLeft = false;
			info->target = NULL;
			info->rightArm.lock = false;
			info->leftArm.lock = false;
			info->flareFrame = 0;
		}
		else if (frame2 < 21)
			DoFlareInHand(lara, info->flareAge);
	}
	else if (frame2 >= 95 && frame2 < 110)
	{
		frame2++;

		if (frame2 == 110)
		{
			frame2 = 1;
			DoFlareInHand(lara, info->flareAge);
		}
	}

	info->leftArm.frameNumber = frame2;
	set_flare_arm(lara, info->leftArm.frameNumber);
}

void draw_flare(ITEM_INFO* lara)
{
	LaraInfo*& info = lara->data;
	short frame;

	if (lara->currentAnimState == LS_PICKUP_FLARE ||
		lara->currentAnimState == LS_PICKUP)
	{
		DoFlareInHand(lara, info->flareAge);
		info->flareControlLeft = false;
		info->leftArm.frameNumber = 93;
		set_flare_arm(lara, 93);
	}
	else
	{
		frame = info->leftArm.frameNumber + 1;
		info->flareControlLeft = true;

		if (frame < 33 || frame > 94)
			frame = 33;
		else if (frame == 46)
			draw_flare_meshes(lara);
		else if (frame >= 72 && frame <= 93)
		{
			if (frame == 72)
			{
				SoundEffect(SFX_TR4_OBJ_GEM_SMASH, &lara->pos, TestLaraWater(lara));
				info->flareAge = 1;
			}

			DoFlareInHand(lara, info->flareAge);
		}
		else
		{
			if (frame == 94)
			{
				ready_flare(LaraItem);
				frame = 0;
				DoFlareInHand(lara, info->flareAge);
			}
		}

		info->leftArm.frameNumber = frame;
		set_flare_arm(lara, frame);
	}
}

void set_flare_arm(ITEM_INFO* lara, int frame)
{
	LaraInfo*& info = lara->data;
	short anim = Objects[ID_LARA_FLARE_ANIM].animIndex;

	if (frame >= 95)
		anim += 4;
	else if (frame >= 72)
		anim += 3;
	else if (frame >= 33)
		anim += 2;
	else if (frame >= 1)
		anim += 1;

	info->leftArm.animNumber = anim;
	info->leftArm.frameBase = g_Level.Anims[anim].framePtr;
}

void CreateFlare(ITEM_INFO* lara, GAME_OBJECT_ID objectNum, int thrown)
{
	LaraInfo*& info = lara->data;
	short itemNum = CreateItem();

	if (itemNum != NO_ITEM)
	{
		bool flag = false;
		ITEM_INFO* item = &g_Level.Items[itemNum];
		item->objectNumber = objectNum;
		item->roomNumber = lara->roomNumber;

		PHD_VECTOR pos;
		pos.x = -16;
		pos.y = 32;
		pos.z = 42;

		GetLaraJointPosition(&pos, LM_LHAND);

		item->pos.xPos = pos.x;
		item->pos.yPos = pos.y;
		item->pos.zPos = pos.z;

		auto probe = GetCollisionResult(pos.x, pos.y, pos.z, lara->roomNumber);
		auto floorHeight = probe.Position.Floor;
		auto collided = GetCollidedObjects(item, 0, 1, CollidedItems, CollidedMeshes, true);

		if (collided || floorHeight < pos.y)
		{
			flag = true;
			item->pos.yRot = lara->pos.yRot + ANGLE(180.0f);
			item->pos.xPos = lara->pos.xPos + 320 * phd_sin(item->pos.yRot);
			item->pos.zPos = lara->pos.zPos + 320 * phd_cos(item->pos.yRot);
			item->roomNumber = lara->roomNumber;
		}
		else
		{
			if (thrown)
				item->pos.yRot = lara->pos.yRot;
			else
				item->pos.yRot = lara->pos.yRot - ANGLE(45.0f);
			item->roomNumber = lara->roomNumber;
		}

		InitialiseItem(itemNum);

		item->pos.zRot = 0;
		item->pos.xRot = 0;
		item->shade = -1;

		if (thrown)
		{
			item->speed = lara->speed + 50;
			item->fallspeed = lara->fallspeed - 50;
		}
		else
		{
			item->speed = lara->speed + 10;
			item->fallspeed = lara->fallspeed + 50;
		}

		if (flag)
			item->speed >>= 1;

		if (objectNum == ID_FLARE_ITEM)
		{
			item->data = (short)0;
			short& age = item->data;
			if (DoFlareLight((PHD_VECTOR*)&item->pos, info->flareAge))
				age = (info->flareAge | 0x8000);
			else
				age = (info->flareAge & 0x7FFF);
		}
		else
			item->itemFlags[3] = info->litTorch;

		AddActiveItem(itemNum);
		item->status = ITEM_ACTIVE;
	}
}

void DrawFlareInAir(ITEM_INFO* item)
{
	TENLog("DrawFlareInAir() not implemented!", LogLevel::Warning);
}

void DoFlareInHand(ITEM_INFO* lara, int flare_age)
{
	LaraInfo*& info = lara->data;
	PHD_VECTOR pos = { 11, 32, 41 };

	GetLaraJointPosition(&pos, LM_LHAND);
	if (DoFlareLight(&pos, flare_age))
		TriggerChaffEffects(flare_age);

	/* Hardcoded code */

	if (info->flareAge >= FLARE_AGE)
	{
		if (info->gunStatus == LG_HANDS_FREE)
			info->gunStatus = LG_UNDRAW_GUNS;
	}
	else if (info->flareAge != 0)
		info->flareAge++;
}

int DoFlareLight(PHD_VECTOR* pos, int age)
{
	int r, g, b;
	float random;
	int falloff;

	if (age >= FLARE_AGE || age == 0)
		return 0;

	random = GenerateFloat();

	int x = pos->x + (random * 120);
	int y = pos->y + (random * 120) - 256;
	int z = pos->z + (random * 120);

	if (age < 4)
	{
		falloff = 12 + ((1 - (age / 4.0f)) * 16);
		
		r = FlareMainColor.x * 255;
		g = FlareMainColor.y * 255;
		b = FlareMainColor.z * 255;

		TriggerDynamicLight(x, y, z, falloff, r, g, b);

		return (random < 0.9f);
	}
	else if (age < (FLARE_AGE - 90))
	{
		auto multiplier = FlareFlickerTable[age % FlareFlickerTable.size()];
		falloff = 12 * multiplier;

		r = FlareMainColor.x * 255 * multiplier;
		g = FlareMainColor.y * 255 * multiplier;
		b = FlareMainColor.z * 255 * multiplier;
		TriggerDynamicLight(x, y, z, falloff, r, g, b);

		return (random < 0.4f);
	}
	else 
	{
		auto multiplier = FlareFlickerTableLow[age % FlareFlickerTableLow.size()];
		falloff = 12 * (1.0f - ((age - (FLARE_AGE - 90)) / (FLARE_AGE - (FLARE_AGE - 90))));

		r = FlareMainColor.x * 255 * multiplier;
		g = FlareMainColor.y * 255 * multiplier;
		b = FlareMainColor.z * 255 * multiplier;
		TriggerDynamicLight(x, y, z, falloff, r, g, b);

		return (random < .3f);
	}
}
