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

using namespace TEN::Math::Random;

constexpr DirectX::SimpleMath::Vector3 FlareMainColor = Vector3(1, 0.52947, 0.3921);

void FlareControl(short itemNum)
{
	ITEM_INFO* flareItem = &g_Level.Items[itemNum];

	if (TestLaraSwamp(flareItem))
	{
		KillItem(itemNum);
		return;
	}

	if (flareItem->fallspeed)
	{
		flareItem->pos.xRot += ANGLE(3.0f);
		flareItem->pos.zRot += ANGLE(5.0f);
	}
	else
	{
		flareItem->pos.xRot = 0;
		flareItem->pos.zRot = 0;
	}

	PHD_VECTOR oldPos = { flareItem->pos.xPos , flareItem->pos.yPos , flareItem->pos.zPos };

	int xVel = flareItem->speed * phd_sin(flareItem->pos.yRot);
	int zVel = flareItem->speed * phd_cos(flareItem->pos.yRot);

	flareItem->pos.xPos += xVel;
	flareItem->pos.zPos += zVel;

	if (TestLaraWater(flareItem) || TestLaraSwamp(flareItem)) // TODO: Generic water/swamp test function.
	{
		flareItem->fallspeed += (5 - flareItem->fallspeed) / 2;
		flareItem->speed += (5 - flareItem->speed) / 2;
	}
	else
		flareItem->fallspeed += 6;

	flareItem->pos.yPos += flareItem->fallspeed;

	DoProjectileDynamics(itemNum, oldPos.x, oldPos.y, oldPos.z, xVel, flareItem->fallspeed, zVel);

	short& age = flareItem->data;
	age &= 0x7FFF;
	if (age >= FLARE_AGE)
	{
		if (!flareItem->fallspeed && !flareItem->speed)
		{
			KillItem(itemNum);
			return;
		}
	}
	else
		age++;

	if (DoFlareLight((PHD_VECTOR*)&flareItem->pos, age))
	{
		TriggerChaffEffects(flareItem,age);
		/* Hardcoded code */

		age |= 0x8000;
	}
}

void ReadyFlare(ITEM_INFO* laraItem)
{
	LaraInfo*& laraInfo = laraItem->data;

	laraInfo->gunStatus = LG_NO_ARMS;
	laraInfo->leftArm.xRot = 0;
	laraInfo->leftArm.yRot = 0;
	laraInfo->leftArm.zRot = 0;
	laraInfo->rightArm.xRot = 0;
	laraInfo->rightArm.yRot = 0;
	laraInfo->rightArm.zRot = 0;
	laraInfo->leftArm.lock = false;
	laraInfo->rightArm.lock = false;
	laraInfo->target = NULL;
}

void UndrawFlareMeshes(ITEM_INFO* laraItem)
{
	LaraInfo*& laraInfo = laraItem->data;

	laraInfo->meshPtrs[LM_LHAND] = Objects[ID_LARA_SKIN].meshIndex + LM_LHAND;
}

void DrawFlareMeshes(ITEM_INFO* laraItem)
{
	LaraInfo*& laraInfo = laraItem->data;

	laraInfo->meshPtrs[LM_LHAND] = Objects[ID_LARA_FLARE_ANIM].meshIndex + LM_LHAND;
}

void UndrawFlare(ITEM_INFO* laraItem)
{
	LaraInfo*& laraInfo = laraItem->data;
	int flareFrame = laraInfo->flareFrame;
	int armFrame = laraInfo->leftArm.frameNumber;

	laraInfo->flareControlLeft = true;

	if (laraItem->goalAnimState == LS_STOP &&
		laraInfo->Vehicle == NO_ITEM)
	{
		if (laraItem->animNumber == LA_STAND_IDLE)
		{
			laraItem->animNumber = LA_DISCARD_FLARE;
			flareFrame = armFrame + g_Level.Anims[laraItem->animNumber].frameBase;
			laraInfo->flareFrame = flareFrame;
			laraItem->frameNumber = flareFrame;
		}

		if (laraItem->animNumber == LA_DISCARD_FLARE)
		{
			laraInfo->flareControlLeft = false;

			if (flareFrame >= g_Level.Anims[laraItem->animNumber].frameBase + 31) // Last frame.
			{
				laraInfo->requestGunType = laraInfo->lastGunType;
				laraInfo->gunType = laraInfo->lastGunType;
				laraInfo->gunStatus = LG_NO_ARMS;

				InitialiseNewWeapon(laraItem);

				laraInfo->target = NULL;
				laraInfo->rightArm.lock = false;
				laraInfo->leftArm.lock = false;
				SetAnimation(laraItem, LA_STAND_IDLE);
				laraInfo->flareFrame = g_Level.Anims[laraItem->animNumber].frameBase;
				return;
			}

			laraInfo->flareFrame++;
		}
	}
	else if (laraItem->animNumber == LA_DISCARD_FLARE)
		SetAnimation(laraItem, LA_STAND_IDLE);

	if (armFrame >= 33 && armFrame < 72)
	{
		armFrame = 2;
		DoFlareInHand(laraItem, laraInfo->flareAge);
	}
	else if (!armFrame)
	{
		armFrame = 1;
		DoFlareInHand(laraItem, laraInfo->flareAge);
	}
	else if (armFrame >= 72 && armFrame < 95)
	{
		armFrame++;

		if (armFrame == 94)
		{
			armFrame = 1;
			DoFlareInHand(laraItem, laraInfo->flareAge);
		}
	}
	else if (armFrame >= 1 && armFrame < 33)
	{
		armFrame++;

		if (armFrame == 21)
		{
			CreateFlare(laraItem, ID_FLARE_ITEM, true);
			UndrawFlareMeshes(laraItem);
			laraInfo->flareAge = 0;
		}
		else if (armFrame == 33)
		{
			armFrame = 0;
			laraInfo->requestGunType = laraInfo->lastGunType;
			laraInfo->gunType = laraInfo->lastGunType;
			laraInfo->gunStatus = LG_NO_ARMS;

			InitialiseNewWeapon(laraItem);

			laraInfo->flareControlLeft = false;
			laraInfo->target = NULL;
			laraInfo->rightArm.lock = false;
			laraInfo->leftArm.lock = false;
			laraInfo->flareFrame = 0;
		}
		else if (armFrame < 21)
			DoFlareInHand(laraItem, laraInfo->flareAge);
	}
	else if (armFrame >= 95 && armFrame < 110)
	{
		armFrame++;

		if (armFrame == 110)
		{
			armFrame = 1;
			DoFlareInHand(laraItem, laraInfo->flareAge);
		}
	}

	laraInfo->leftArm.frameNumber = armFrame;
	SetFlareArm(laraItem, laraInfo->leftArm.frameNumber);
}

void DrawFlare(ITEM_INFO* laraItem)
{
	LaraInfo*& laraInfo = laraItem->data;

	if (laraItem->currentAnimState == LS_PICKUP_FLARE ||
		laraItem->currentAnimState == LS_PICKUP)
	{
		DoFlareInHand(laraItem, laraInfo->flareAge);
		laraInfo->flareControlLeft = false;
		laraInfo->leftArm.frameNumber = 93;
		SetFlareArm(laraItem, 93);
	}
	else
	{
		int armFrame = laraInfo->leftArm.frameNumber + 1;
		laraInfo->flareControlLeft = true;

		if (armFrame < 33 || armFrame > 94)
			armFrame = 33;
		else if (armFrame == 46)
			DrawFlareMeshes(laraItem);
		else if (armFrame >= 72 && armFrame <= 93)
		{
			if (armFrame == 72)
			{
				SoundEffect(SFX_TR4_OBJ_GEM_SMASH, &laraItem->pos, TestLaraWater(laraItem));
				laraInfo->flareAge = 1;
			}

			DoFlareInHand(laraItem, laraInfo->flareAge);
		}
		else
		{
			if (armFrame == 94)
			{
				ReadyFlare(laraItem);
				armFrame = 0;
				DoFlareInHand(laraItem, laraInfo->flareAge);
			}
		}

		laraInfo->leftArm.frameNumber = armFrame;
		SetFlareArm(laraItem, armFrame);
	}
}

void SetFlareArm(ITEM_INFO* laraItem, int armFrame)
{
	LaraInfo*& laraInfo = laraItem->data;
	int flareAnimNum = Objects[ID_LARA_FLARE_ANIM].animIndex;

	if (armFrame >= 95)
		flareAnimNum += 4;
	else if (armFrame >= 72)
		flareAnimNum += 3;
	else if (armFrame >= 33)
		flareAnimNum += 2;
	else if (armFrame >= 1)
		flareAnimNum += 1;

	laraInfo->leftArm.animNumber = flareAnimNum;
	laraInfo->leftArm.frameBase = g_Level.Anims[flareAnimNum].framePtr;
}

void CreateFlare(ITEM_INFO* laraItem, GAME_OBJECT_ID objectNum, bool thrown)
{
	LaraInfo*& laraInfo = laraItem->data;
	auto itemNum = CreateItem();

	if (itemNum != NO_ITEM)
	{
		ITEM_INFO* flareItem = &g_Level.Items[itemNum];
		bool flag = false;
		flareItem->objectNumber = objectNum;
		flareItem->roomNumber = laraItem->roomNumber;

		PHD_VECTOR pos = { -16, 32, 42 };
		GetLaraJointPosition(&pos, LM_LHAND);

		flareItem->pos.xPos = pos.x;
		flareItem->pos.yPos = pos.y;
		flareItem->pos.zPos = pos.z;

		auto probe = GetCollisionResult(pos.x, pos.y, pos.z, laraItem->roomNumber);
		auto collided = GetCollidedObjects(flareItem, 0, true, CollidedItems, CollidedMeshes, true);
		if (probe.Position.Floor < pos.y || collided)
		{
			flag = true;
			flareItem->pos.yRot = laraItem->pos.yRot + ANGLE(180.0f);
			flareItem->pos.xPos = laraItem->pos.xPos + 320 * phd_sin(flareItem->pos.yRot);
			flareItem->pos.zPos = laraItem->pos.zPos + 320 * phd_cos(flareItem->pos.yRot);
			flareItem->roomNumber = laraItem->roomNumber;
		}
		else
		{
			if (thrown)
				flareItem->pos.yRot = laraItem->pos.yRot;
			else
				flareItem->pos.yRot = laraItem->pos.yRot - ANGLE(45.0f);
			flareItem->roomNumber = laraItem->roomNumber;
		}

		InitialiseItem(itemNum);

		flareItem->pos.xRot = 0;
		flareItem->pos.zRot = 0;
		flareItem->shade = -1;

		if (thrown)
		{
			flareItem->speed = laraItem->speed + 50;
			flareItem->fallspeed = laraItem->fallspeed - 50;
		}
		else
		{
			flareItem->speed = laraItem->speed + 10;
			flareItem->fallspeed = laraItem->fallspeed + 50;
		}

		if (flag)
			flareItem->speed >>= 1;

		if (objectNum == ID_FLARE_ITEM)
		{
			flareItem->data = (short)0;
			short& age = flareItem->data;
			if (DoFlareLight((PHD_VECTOR*)&flareItem->pos, laraInfo->flareAge))
				age = (laraInfo->flareAge | 0x8000);
			else
				age = (laraInfo->flareAge & 0x7FFF);
		}
		else
			flareItem->itemFlags[3] = laraInfo->litTorch;

		AddActiveItem(itemNum);
		flareItem->status = ITEM_ACTIVE;
	}
}

void DrawFlareInAir(ITEM_INFO* flareItem)
{
	TENLog("DrawFlareInAir() not implemented!", LogLevel::Warning);
}

void DoFlareInHand(ITEM_INFO* laraItem, int flareAge)
{
	LaraInfo*& ItemInfo = laraItem->data;

	PHD_VECTOR pos = { 11, 32, 41 };
	GetLaraJointPosition(&pos, LM_LHAND);

	if (DoFlareLight(&pos, flareAge))
		TriggerChaffEffects(flareAge);

	/* Hardcoded code */

	if (ItemInfo->flareAge >= FLARE_AGE)
	{
		if (ItemInfo->gunStatus == LG_NO_ARMS)
			ItemInfo->gunStatus = LG_UNDRAW_GUNS;
	}
	else if (ItemInfo->flareAge != 0)
		ItemInfo->flareAge++;
}

int DoFlareLight(PHD_VECTOR* pos, int age)
{
	int r, g, b;
	int falloff;

	if (age >= FLARE_AGE || age == 0)
		return 0;

	auto random = GenerateFloat();

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
		auto multiplier = GenerateFloat(0.75f, 1.0f);
		falloff = 12 * multiplier;

		r = FlareMainColor.x * 255 * multiplier;
		g = FlareMainColor.y * 255 * multiplier;
		b = FlareMainColor.z * 255 * multiplier;
		TriggerDynamicLight(x, y, z, falloff, r, g, b);

		return (random < 0.4f);
	}
	else 
	{
		auto multiplier = GenerateFloat(0.05f, 0.8f);
		falloff = 12 * (1.0f - ((age - (FLARE_AGE - 90)) / (FLARE_AGE - (FLARE_AGE - 90))));

		r = FlareMainColor.x * 255 * multiplier;
		g = FlareMainColor.y * 255 * multiplier;
		b = FlareMainColor.z * 255 * multiplier;
		TriggerDynamicLight(x, y, z, falloff, r, g, b);

		return (random < 0.3f);
	}
}
