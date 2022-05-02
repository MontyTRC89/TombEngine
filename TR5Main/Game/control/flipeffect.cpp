#include "framework.h"
#include "Game/control/flipeffect.h"

#include "Game/camera.h"
#include "Game/collision/collide_room.h"
#include "Game/control/lot.h"
#include "Game/effects/hair.h"
#include "Game/effects/tomb4fx.h"
#include "Game/effects/weather.h"
#include "Game/effects/footprint.h"
#include "Game/effects/debris.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_fire.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/pickup/pickup.h"
#include "Sound/sound.h"
#include "Specific/level.h"
#include "Specific/setup.h"
#include "Objects/Generic/puzzles_keys.h"
#include "Objects/TR5/Emitter/tr5_spider_emitter.h"
#include "Objects/TR5/Object/tr5_pushableblock.h"

using std::function;
using namespace TEN::Effects::Footprints;
using namespace TEN::Effects::Environment;

int FlipEffect;

function<EffectFunction> effect_routines[NUM_FLIPEFFECTS] =
{
	Turn180,					//0
	FloorShake,					//1
	PoseidonSFX,				//2
	LaraBubbles,				//3
	FinishLevel,				//4
	ActivateCamera,				//5
	ActivateKey,				//6
	RubbleFX,					//7
	SwapCrowbar,				//8
	Pickup,						//9
	PlaySoundEffect,			//10
	ExplosionFX,				//11
	LaraHandsFree,				//12
	Puzzle,						//13
	DrawRightPistol,			//14
	DrawLeftPistol,				//15
	ShootRightGun,				//16
	ShootLeftGun,				//17
	PushLoop,					//18
	PushEnd,					//19
	FlashOrange,				//20
	InvisibilityOn,				//21
	InvisibilityOff,			//22
	VoidEffect,					//23
	VoidEffect,					//24
	VoidEffect,					//25
	ResetHair,					//26
	VoidEffect,					//27
	SetFog,						//28
	VoidEffect,					//29
	LaraLocation,				//30
	ClearSpidersPatch,			//31
	AddLeftFootprint,			//32
	AddRightFootprint,			//33
	VoidEffect,					//34
	VoidEffect,					//35
	VoidEffect,					//36
	VoidEffect,					//37
	VoidEffect,					//38
	VoidEffect,					//39
	VoidEffect,					//40
	VoidEffect,					//41
	VoidEffect,					//42
	MeshSwapToPour,				//43
	MeshSwapFromPour,			//44
	LaraLocationPad,			//45
	KillActiveBaddies			//46
};

void FlashOrange(ITEM_INFO* item) 
{
	FlipEffect = -1;
	Weather.Flash(255, 128, 0, 0.03f);
}

void MeshSwapToPour(ITEM_INFO* item)
{
	auto* lara = GetLaraInfo(item);

	lara->MeshPtrs[LM_LHAND] = Objects[item->ItemFlags[2]].meshIndex + LM_LHAND;
}

void MeshSwapFromPour(ITEM_INFO* item)
{
	auto* lara = GetLaraInfo(item);

	lara->MeshPtrs[LM_LHAND] = Objects[ID_LARA_SKIN].meshIndex + LM_LHAND;
}

void Pickup(ITEM_INFO* item)
{
	DoPickup(item);
}

void Puzzle(ITEM_INFO* item)
{
	DoPuzzle();
}

void AddLeftFootprint(ITEM_INFO* item)
{
	AddFootprint(item, false);
}

void AddRightFootprint(ITEM_INFO* item)
{
	AddFootprint(item, true);
}

void ResetHair(ITEM_INFO* item)
{
	InitialiseHair();
}

void InvisibilityOff(ITEM_INFO* item)
{
	item->Status = ITEM_ACTIVE;
}

void InvisibilityOn(ITEM_INFO* item)
{
	item->Status = ITEM_INVISIBLE;
}

void SetFog(ITEM_INFO* item)
{
	FlipEffect = -1;
}

void DrawLeftPistol(ITEM_INFO* item)
{
	auto* lara = GetLaraInfo(item);

	if (lara->MeshPtrs[LM_LHAND] == Objects[ID_LARA_SKIN].meshIndex + LM_LHAND)
	{
		lara->MeshPtrs[LM_LHAND] = Objects[WeaponObjectMesh(item, LaraWeaponType::Pistol)].meshIndex + LM_LHAND;
		lara->Control.Weapon.HolsterInfo.LeftHolster = HolsterSlot::Empty;
	}
	else
	{
		lara->MeshPtrs[LM_LHAND] = Objects[ID_LARA_SKIN].meshIndex + LM_LHAND;
		lara->Control.Weapon.HolsterInfo.LeftHolster = HolsterSlotForWeapon(LaraWeaponType::Pistol);
	}
}

void DrawRightPistol(ITEM_INFO* item)
{
	auto* lara = GetLaraInfo(item);

	if (lara->MeshPtrs[LM_RHAND] == Objects[ID_LARA_SKIN].meshIndex + LM_RHAND)
	{
		lara->MeshPtrs[LM_RHAND] = Objects[WeaponObjectMesh(item, LaraWeaponType::Pistol)].meshIndex + LM_RHAND;
		lara->Control.Weapon.HolsterInfo.RightHolster = HolsterSlot::Empty;
	}
	else
	{
		lara->MeshPtrs[LM_RHAND] = Objects[ID_LARA_SKIN].meshIndex + LM_RHAND;
		lara->Control.Weapon.HolsterInfo.RightHolster = HolsterSlotForWeapon(LaraWeaponType::Pistol);
	}
}

void ShootLeftGun(ITEM_INFO* item)
{
	auto* lara = GetLaraInfo(item);

	lara->LeftArm.FlashGun = 3;
}

void ShootRightGun(ITEM_INFO* item)
{
	auto* lara = GetLaraInfo(item);

	lara->RightArm.FlashGun = 3;
}

void LaraHandsFree(ITEM_INFO* item)
{
	auto* lara = GetLaraInfo(item);

	lara->Control.HandStatus = HandStatus::Free;
}

void KillActiveBaddies(ITEM_INFO* item)
{
	if (NextItemActive != NO_ITEM)
	{
		short itemNumber = NextItemActive;

		do
		{
			auto* targetItem = &g_Level.Items[itemNumber];

			if (Objects[targetItem->ObjectNumber].intelligent)
			{
				targetItem->Status = ITEM_INVISIBLE;

				if (*(int*)&item != 0xABCDEF)
				{
					RemoveActiveItem(itemNumber);
					DisableEntityAI(itemNumber);
					targetItem->Flags |= IFLAG_INVISIBLE;
				}
			}

			itemNumber = targetItem->NextActive;
		} while (itemNumber != NO_ITEM);
	}

	FlipEffect = -1;
}

void LaraLocationPad(ITEM_INFO* item)
{
	auto* lara = GetLaraInfo(item);

	FlipEffect = -1;

	lara->Location = TriggerTimer;
	lara->LocationPad = TriggerTimer;
}

void LaraLocation(ITEM_INFO* item)
{
	auto* lara = GetLaraInfo(item);

	FlipEffect = -1;

	lara->Location = TriggerTimer;
	if (lara->HighestLocation < TriggerTimer)
		lara->HighestLocation = TriggerTimer;
}

void ExplosionFX(ITEM_INFO* item)
{
	SoundEffect(SFX_TR4_EXPLOSION1, NULL, 0);
	Camera.bounce = -75;
	FlipEffect = -1;
}

void SwapCrowbar(ITEM_INFO* item)
{
	auto* lara = GetLaraInfo(item);

	if (lara->MeshPtrs[LM_RHAND] == Objects[ID_LARA_SKIN].meshIndex + LM_RHAND)
		lara->MeshPtrs[LM_RHAND] = Objects[ID_LARA_CROWBAR_ANIM].meshIndex + LM_RHAND;
	else 
		lara->MeshPtrs[LM_RHAND] = Objects[ID_LARA_SKIN].meshIndex + LM_RHAND;
}

void ActivateKey(ITEM_INFO* item)
{
	KeyTriggerActive = 1;
}

void ActivateCamera(ITEM_INFO* item)
{
	KeyTriggerActive = 2;
}

void PoseidonSFX(ITEM_INFO* item)
{
	SoundEffect(SFX_TR4_WATER_FLUSHES, NULL, 0);
	FlipEffect = -1;
}

void RubbleFX(ITEM_INFO* item)
{
	const auto itemList = FindAllItems(ID_EARTHQUAKE);

	if (itemList.size() > 0)
	{
		auto* eq = &g_Level.Items[itemList[0]];

		AddActiveItem(itemList[0]);
		eq->Status = ITEM_ACTIVE;
		eq->Flags |= IFLAG_ACTIVATION_MASK;
	}
	else
		Camera.bounce = -150;

	FlipEffect = -1;
}

void PlaySoundEffect(ITEM_INFO* item)
{
	SoundEffect(TriggerTimer, NULL, 0);
	FlipEffect = -1;
}

void FloorShake(ITEM_INFO* item)
{
	int x = abs(item->Pose.Position.x - Camera.pos.x);
	int y = abs(item->Pose.Position.y - Camera.pos.y);
	int z = abs(item->Pose.Position.z - Camera.pos.z);

	if (x < SECTOR(16) &&
		y < SECTOR(16) &&
		z < SECTOR(16))
	{
		Camera.bounce = 66 * ((pow(x, 2) + pow(y, 2) + pow(z, 2)) / CLICK(1) - pow(SECTOR(1), 2)) / pow(SECTOR(1), 2);
	}
}

void Turn180(ITEM_INFO* item)
{
	item->Pose.Orientation.y -= Angle::DegToRad(180.0f);
	item->Pose.Orientation.x = -item->Pose.Orientation.x;
}

void FinishLevel(ITEM_INFO* item)
{
	LevelComplete = CurrentLevel + 1;
}

void VoidEffect(ITEM_INFO* item)
{

}

void DoFlipEffect(int number, ITEM_INFO* item)
{
	if (number != -1 && number < NUM_FLIPEFFECTS && effect_routines[number] != nullptr)
		effect_routines[number](item);
}
