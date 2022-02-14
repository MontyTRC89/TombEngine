#include "framework.h"
#include "Game/Lara/lara_flare.h"

#include "Game/animation.h"
#include "Game/collision/collide_item.h"
#include "Game/effects/effects.h"
#include "Game/effects/chaffFX.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_fire.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/Lara/lara_tests.h"
#include "Sound/sound.h"
#include "Specific/level.h"
#include "Specific/setup.h"
#include "Specific/prng.h"

using namespace TEN::Math::Random;

constexpr DirectX::SimpleMath::Vector3 FlareMainColor = Vector3(1, 0.52947, 0.3921);

void FlareControl(short itemNum)
{
	ITEM_INFO* flareItem = &g_Level.Items[itemNum];

	if (TestEnvironment(ENV_FLAG_SWAMP, flareItem))
	{
		KillItem(itemNum);
		return;
	}

	if (flareItem->VerticalVelocity)
	{
		flareItem->Position.xRot += ANGLE(3.0f);
		flareItem->Position.zRot += ANGLE(5.0f);
	}
	else
	{
		flareItem->Position.xRot = 0;
		flareItem->Position.zRot = 0;
	}

	PHD_VECTOR oldPos = { flareItem->Position.xPos , flareItem->Position.yPos , flareItem->Position.zPos };

	int xVel = flareItem->Velocity * phd_sin(flareItem->Position.yRot);
	int zVel = flareItem->Velocity * phd_cos(flareItem->Position.yRot);

	flareItem->Position.xPos += xVel;
	flareItem->Position.zPos += zVel;

	if (TestEnvironment(ENV_FLAG_WATER, flareItem) ||
		TestEnvironment(ENV_FLAG_SWAMP, flareItem))
	{
		flareItem->VerticalVelocity += (5 - flareItem->VerticalVelocity) / 2;
		flareItem->Velocity += (5 - flareItem->Velocity) / 2;
	}
	else
		flareItem->VerticalVelocity += 6;

	flareItem->Position.yPos += flareItem->VerticalVelocity;

	DoProjectileDynamics(itemNum, oldPos.x, oldPos.y, oldPos.z, xVel, flareItem->VerticalVelocity, zVel);

	int& age = flareItem->Data;
	age &= 0x7FFF;
	if (age >= FLARE_LIFE_MAX)
	{
		if (!flareItem->VerticalVelocity && !flareItem->Velocity)
		{
			KillItem(itemNum);
			return;
		}
	}
	else
		age++;

	if (DoFlareLight((PHD_VECTOR*)&flareItem->Position, age))
	{
		TriggerChaffEffects(flareItem, age);
		/* Hardcoded code */

		age |= 0x8000;
	}
}

void ReadyFlare(ITEM_INFO* laraItem)
{
	auto* laraInfo = GetLaraInfo(laraItem);

	laraInfo->Control.HandStatus = HandStatus::Free;
	laraInfo->LeftArm.Rotation.xRot = 0;
	laraInfo->LeftArm.Rotation.yRot = 0;
	laraInfo->LeftArm.Rotation.zRot = 0;
	laraInfo->RightArm.Rotation.xRot = 0;
	laraInfo->RightArm.Rotation.yRot = 0;
	laraInfo->RightArm.Rotation.zRot = 0;
	laraInfo->LeftArm.Locked = false;
	laraInfo->RightArm.Locked = false;
	laraInfo->target = NULL;
}

void UndrawFlareMeshes(ITEM_INFO* laraItem)
{
	auto* laraInfo = GetLaraInfo(laraItem);

	laraInfo->meshPtrs[LM_LHAND] = Objects[ID_LARA_SKIN].meshIndex + LM_LHAND;
}

void DrawFlareMeshes(ITEM_INFO* laraItem)
{
	auto* laraInfo = GetLaraInfo(laraItem);

	laraInfo->meshPtrs[LM_LHAND] = Objects[ID_LARA_FLARE_ANIM].meshIndex + LM_LHAND;
}

void UndrawFlare(ITEM_INFO* laraItem)
{
	auto* laraInfo = GetLaraInfo(laraItem);
	int flareFrame = laraInfo->Flare.Frame;
	int armFrame = laraInfo->LeftArm.FrameNumber;

	laraInfo->Flare.ControlLeft = true;

	if (laraItem->TargetState == LS_IDLE &&
		laraInfo->Vehicle == NO_ITEM)
	{
		if (laraItem->AnimNumber == LA_STAND_IDLE)
		{
			laraItem->AnimNumber = LA_DISCARD_FLARE;
			flareFrame = armFrame + g_Level.Anims[laraItem->AnimNumber].frameBase;
			laraInfo->Flare.Frame = flareFrame;
			laraItem->FrameNumber = flareFrame;
		}

		if (laraItem->AnimNumber == LA_DISCARD_FLARE)
		{
			laraInfo->Flare.ControlLeft = false;

			if (flareFrame >= g_Level.Anims[laraItem->AnimNumber].frameBase + 31) // Last frame.
			{
				laraInfo->Control.WeaponControl.RequestGunType = laraInfo->Control.WeaponControl.LastGunType;
				laraInfo->Control.WeaponControl.GunType = laraInfo->Control.WeaponControl.LastGunType;
				laraInfo->Control.HandStatus = HandStatus::Free;

				InitialiseNewWeapon(laraItem);

				laraInfo->target = NULL;
				laraInfo->RightArm.Locked = false;
				laraInfo->LeftArm.Locked = false;
				SetAnimation(laraItem, LA_STAND_IDLE);
				laraInfo->Flare.Frame = g_Level.Anims[laraItem->AnimNumber].frameBase;
				return;
			}

			laraInfo->Flare.Frame++;
		}
	}
	else if (laraItem->AnimNumber == LA_DISCARD_FLARE)
		SetAnimation(laraItem, LA_STAND_IDLE);

	if (armFrame >= 33 && armFrame < 72)
	{
		armFrame = 2;
		DoFlareInHand(laraItem, laraInfo->Flare.Life);
	}
	else if (!armFrame)
	{
		armFrame = 1;
		DoFlareInHand(laraItem, laraInfo->Flare.Life);
	}
	else if (armFrame >= 72 && armFrame < 95)
	{
		armFrame++;

		if (armFrame == 94)
		{
			armFrame = 1;
			DoFlareInHand(laraItem, laraInfo->Flare.Life);
		}
	}
	else if (armFrame >= 1 && armFrame < 33)
	{
		armFrame++;

		if (armFrame == 21)
		{
			CreateFlare(laraItem, ID_FLARE_ITEM, true);
			UndrawFlareMeshes(laraItem);
			laraInfo->Flare.Life = 0;
		}
		else if (armFrame == 33)
		{
			armFrame = 0;
			laraInfo->Control.WeaponControl.RequestGunType = laraInfo->Control.WeaponControl.LastGunType;
			laraInfo->Control.WeaponControl.GunType = laraInfo->Control.WeaponControl.LastGunType;
			laraInfo->Control.HandStatus = HandStatus::Free;

			InitialiseNewWeapon(laraItem);

			laraInfo->Flare.ControlLeft = false;
			laraInfo->target = NULL;
			laraInfo->RightArm.Locked = false;
			laraInfo->LeftArm.Locked = false;
			laraInfo->Flare.Frame = 0;
		}
		else if (armFrame < 21)
			DoFlareInHand(laraItem, laraInfo->Flare.Life);
	}
	else if (armFrame >= 95 && armFrame < 110)
	{
		armFrame++;

		if (armFrame == 110)
		{
			armFrame = 1;
			DoFlareInHand(laraItem, laraInfo->Flare.Life);
		}
	}

	laraInfo->LeftArm.FrameNumber = armFrame;
	SetFlareArm(laraItem, laraInfo->LeftArm.FrameNumber);
}

void DrawFlare(ITEM_INFO* laraItem)
{
	auto* laraInfo = GetLaraInfo(laraItem);

	if (laraItem->ActiveState == LS_PICKUP_FLARE ||
		laraItem->ActiveState == LS_PICKUP)
	{
		DoFlareInHand(laraItem, laraInfo->Flare.Life);
		laraInfo->Flare.ControlLeft = false;
		laraInfo->LeftArm.FrameNumber = 93;
		SetFlareArm(laraItem, 93);
	}
	else
	{
		int armFrame = laraInfo->LeftArm.FrameNumber + 1;
		laraInfo->Flare.ControlLeft = true;

		if (armFrame < 33 || armFrame > 94)
			armFrame = 33;
		else if (armFrame == 46)
			DrawFlareMeshes(laraItem);
		else if (armFrame >= 72 && armFrame <= 93)
		{
			if (armFrame == 72)
			{
				SoundEffect(SFX_TR4_OBJ_GEM_SMASH, &laraItem->Position, TestEnvironment(ENV_FLAG_WATER, laraItem));
				laraInfo->Flare.Life = 1;
			}

			DoFlareInHand(laraItem, laraInfo->Flare.Life);
		}
		else
		{
			if (armFrame == 94)
			{
				ReadyFlare(laraItem);
				armFrame = 0;
				DoFlareInHand(laraItem, laraInfo->Flare.Life);
			}
		}

		laraInfo->LeftArm.FrameNumber = armFrame;
		SetFlareArm(laraItem, armFrame);
	}
}

void SetFlareArm(ITEM_INFO* laraItem, int armFrame)
{
	auto* laraInfo = GetLaraInfo(laraItem);
	int flareAnimNum = Objects[ID_LARA_FLARE_ANIM].animIndex;

	if (armFrame >= 95)
		flareAnimNum += 4;
	else if (armFrame >= 72)
		flareAnimNum += 3;
	else if (armFrame >= 33)
		flareAnimNum += 2;
	else if (armFrame >= 1)
		flareAnimNum += 1;

	laraInfo->LeftArm.AnimNumber = flareAnimNum;
	laraInfo->LeftArm.FrameBase = g_Level.Anims[flareAnimNum].framePtr;
}

void CreateFlare(ITEM_INFO* laraItem, GAME_OBJECT_ID objectNum, bool thrown)
{
	auto* laraInfo = GetLaraInfo(laraItem);
	auto itemNum = CreateItem();

	if (itemNum != NO_ITEM)
	{
		ITEM_INFO* flareItem = &g_Level.Items[itemNum];
		bool flag = false;
		flareItem->ObjectNumber = objectNum;
		flareItem->RoomNumber = laraItem->RoomNumber;

		PHD_VECTOR pos = { -16, 32, 42 };
		GetLaraJointPosition(&pos, LM_LHAND);

		flareItem->Position.xPos = pos.x;
		flareItem->Position.yPos = pos.y;
		flareItem->Position.zPos = pos.z;

		auto probe = GetCollisionResult(pos.x, pos.y, pos.z, laraItem->RoomNumber);
		auto collided = GetCollidedObjects(flareItem, 0, true, CollidedItems, CollidedMeshes, true);
		if (probe.Position.Floor < pos.y || collided)
		{
			flag = true;
			flareItem->Position.yRot = laraItem->Position.yRot + ANGLE(180.0f);
			flareItem->Position.xPos = laraItem->Position.xPos + 320 * phd_sin(flareItem->Position.yRot);
			flareItem->Position.zPos = laraItem->Position.zPos + 320 * phd_cos(flareItem->Position.yRot);
			flareItem->RoomNumber = laraItem->RoomNumber;
		}
		else
		{
			if (thrown)
				flareItem->Position.yRot = laraItem->Position.yRot;
			else
				flareItem->Position.yRot = laraItem->Position.yRot - ANGLE(45.0f);
			flareItem->RoomNumber = laraItem->RoomNumber;
		}

		InitialiseItem(itemNum);

		flareItem->Position.xRot = 0;
		flareItem->Position.zRot = 0;
		flareItem->Shade = -1;

		if (thrown)
		{
			flareItem->Velocity = laraItem->Velocity + 50;
			flareItem->VerticalVelocity = laraItem->VerticalVelocity - 50;
		}
		else
		{
			flareItem->Velocity = laraItem->Velocity + 10;
			flareItem->VerticalVelocity = laraItem->VerticalVelocity + 50;
		}

		if (flag)
			flareItem->Velocity /= 2;

		if (objectNum == ID_FLARE_ITEM)
		{
			flareItem->Data = (int)0;
			int& age = flareItem->Data;
			if (DoFlareLight((PHD_VECTOR*)&flareItem->Position, laraInfo->Flare.Life))
				age = laraInfo->Flare.Life | 0x8000;
			else
				age = laraInfo->Flare.Life & 0x7FFF;
		}
		else
			flareItem->ItemFlags[3] = laraInfo->LitTorch;

		AddActiveItem(itemNum);
		flareItem->Status = ITEM_ACTIVE;
	}
}

void DrawFlareInAir(ITEM_INFO* flareItem)
{
	TENLog("DrawFlareInAir() not implemented!", LogLevel::Warning);
}

void DoFlareInHand(ITEM_INFO* laraItem, int flareAge)
{
	LaraInfo*& ItemInfo = laraItem->Data;

	PHD_VECTOR pos = { 11, 32, 41 };
	GetLaraJointPosition(&pos, LM_LHAND);

	if (DoFlareLight(&pos, flareAge))
		TriggerChaffEffects(flareAge);

	/* Hardcoded code */

	if (ItemInfo->Flare.Life >= FLARE_LIFE_MAX)
	{
		if (ItemInfo->Control.HandStatus == HandStatus::Free)
			ItemInfo->Control.HandStatus = HandStatus::UndrawWeapon;
	}
	else if (ItemInfo->Flare.Life != 0)
		ItemInfo->Flare.Life++;
}

int DoFlareLight(PHD_VECTOR* pos, int age)
{
	int r, g, b;
	int falloff;

	if (age >= FLARE_LIFE_MAX || age == 0)
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
	else if (age < (FLARE_LIFE_MAX - 90))
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
		falloff = 12 * (1.0f - ((age - (FLARE_LIFE_MAX - 90)) / (FLARE_LIFE_MAX - (FLARE_LIFE_MAX - 90))));

		r = FlareMainColor.x * 255 * multiplier;
		g = FlareMainColor.y * 255 * multiplier;
		b = FlareMainColor.z * 255 * multiplier;
		TriggerDynamicLight(x, y, z, falloff, r, g, b);

		return (random < 0.3f);
	}
}
