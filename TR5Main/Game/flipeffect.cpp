#include "framework.h"
#include "flipeffect.h"
#include "Lara.h"
#include "control/lot.h"
#include "effects\hair.h"
#include "animation.h"
#include "sphere.h"
#include "level.h"
#include "setup.h"
#include "camera.h"
#include "collide.h"
#include "savegame.h"
#include "Sound\sound.h"
#include "tr5_rats_emitter.h"
#include "tr5_bats_emitter.h"
#include "tr5_spider_emitter.h"
#include "tr5_pushableblock.h"
#include "pickup.h"
#include "puzzles_keys.h"
#include "lara_fire.h"
#include "effects\effects.h"
#include "effects\tomb4fx.h"
#include "effects\weather.h"
#include "effects\footprint.h"
#include "effects\groundfx.h"
#include "effects\debris.h"
#include "items.h"

using std::function;
using namespace TEN::Effects::Footprints;
using namespace TEN::Effects::Environment;

short FXType;
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
	AddFootprint,				//32
	VoidEffect,					//33
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
	Lara.meshPtrs[LM_LHAND] = Objects[item->itemFlags[2]].meshIndex + LM_LHAND;
}

void MeshSwapFromPour(ITEM_INFO* item)
{
	Lara.meshPtrs[LM_LHAND] = Objects[ID_LARA_SKIN].meshIndex + LM_LHAND;
}

void Pickup(ITEM_INFO* item)
{
	do_pickup();
}

void Puzzle(ITEM_INFO* item)
{
	do_puzzle();
}

void AddFootprint(ITEM_INFO* item)
{
	if (item != LaraItem)
		return;

	PHD_VECTOR position;
	if (FXType == (int)SOUND_PLAYCONDITION::Land)
		GetLaraJointPosition(&position, LM_LFOOT);
	else
		GetLaraJointPosition(&position, LM_RFOOT);

	auto fx = sound_effects::SFX_TR4_LARA_FEET;
	auto result = GetCollisionResult(position.x, position.y - STEP_SIZE, position.z, item->roomNumber);
	auto floor = result.BottomBlock;

	position.y = result.Position.Floor - 4;

	switch (floor->Material)
	{
	case GroundMaterial::Concrete:
		fx = sound_effects::SFX_TR4_LARA_FEET;
		break;

	case GroundMaterial::Grass:
		fx = sound_effects::SFX_TR4_FOOTSTEPS_SAND__AND__GRASS;
		break;

	case GroundMaterial::Gravel:
		fx = sound_effects::SFX_TR4_FOOTSTEPS_GRAVEL;
		break;

	case GroundMaterial::Ice:
		fx = sound_effects::SFX_TR3_FOOTSTEPS_ICE;
		break;

	case GroundMaterial::Marble:
		fx = sound_effects::SFX_TR4_FOOTSTEPS_MARBLE;
		break;

	case GroundMaterial::Metal:
		fx = sound_effects::SFX_TR4_FOOTSTEPS_METAL;
		break;

	case GroundMaterial::Mud:
		fx = sound_effects::SFX_TR4_FOOTSTEPS_MUD;
		break;

	case GroundMaterial::OldMetal:
		fx = sound_effects::SFX_TR4_FOOTSTEPS_METAL;
		break;

	case GroundMaterial::OldWood:
		fx = sound_effects::SFX_TR4_FOOTSTEPS_WOOD;
		break;

	case GroundMaterial::Sand:
		fx = sound_effects::SFX_TR4_FOOTSTEPS_SAND__AND__GRASS;
		break;

	case GroundMaterial::Snow:
		fx = sound_effects::SFX_TR3_FOOTSTEPS_SNOW;
		break;

	case GroundMaterial::Stone:
		fx = sound_effects::SFX_TR4_LARA_FEET;
		break;

	case GroundMaterial::Water:
		fx = sound_effects::SFX_TR4_LARA_WET_FEET;
		break;

	case GroundMaterial::Wood:
		fx = sound_effects::SFX_TR4_FOOTSTEPS_WOOD;
		break;
	}
	
	// HACK: must be here until reference wad2 is revised
	if (fx != sound_effects::SFX_TR4_LARA_FEET)
		SoundEffect(fx, &item->pos, 0);

	auto plane = floor->FloorCollision.Planes[floor->SectorPlane(position.x, position.z)];

	auto c = phd_cos(item->pos.yRot + ANGLE(180));
	auto s = phd_sin(item->pos.yRot + ANGLE(180));
	auto yRot = TO_RAD(item->pos.yRot);
	auto xRot = plane.x * s + plane.y * c;
	auto zRot = plane.y * s - plane.x * c;

	FOOTPRINT_STRUCT footprint = {};
	auto preciseFootPosition = Vector3();

	footprint.Rotation = Vector3(xRot, yRot, zRot);
	footprint.LifeStartFading = 30 * 10;
	footprint.StartOpacity = 0.25f;
	footprint.Life = 30 * 20;
	footprint.Active = true;

	if (CheckFootOnFloor(*item, LM_LFOOT, preciseFootPosition))
	{
		if (footprints.size() >= MAX_FOOTPRINTS)
			footprints.pop_back();
		
		footprint.Position = preciseFootPosition;
		footprint.RightFoot = false;
		footprints.push_front(footprint);
	}

	if (CheckFootOnFloor(*item, LM_RFOOT, preciseFootPosition))
	{
		if (footprints.size() >= MAX_FOOTPRINTS)
			footprints.pop_back();

		footprint.Position = preciseFootPosition;
		footprint.RightFoot = true;
		footprints.push_front(footprint);
	}
}

void ResetHair(ITEM_INFO* item)
{
	InitialiseHair();
}

void InvisibilityOff(ITEM_INFO* item)
{
	item->status = ITEM_ACTIVE;
}

void InvisibilityOn(ITEM_INFO* item)
{
	item->status = ITEM_INVISIBLE;
}

void SetFog(ITEM_INFO* item)
{
	FlipEffect = -1;
}

void DrawLeftPistol(ITEM_INFO* item)
{
	if (Lara.meshPtrs[LM_LHAND] == Objects[ID_LARA_SKIN].meshIndex + LM_LHAND)
	{
		Lara.meshPtrs[LM_LHAND] = Objects[WeaponObjectMesh(WEAPON_PISTOLS)].meshIndex + LM_LHAND;
		Lara.holsterInfo.leftHolster = HOLSTER_SLOT::Empty;
	}
	else
	{
		Lara.meshPtrs[LM_LHAND] = Objects[ID_LARA_SKIN].meshIndex + LM_LHAND;
		Lara.holsterInfo.leftHolster = HolsterSlotForWeapon(static_cast<LARA_WEAPON_TYPE>(WEAPON_PISTOLS));
	}
}

void DrawRightPistol(ITEM_INFO* item)
{
	if (Lara.meshPtrs[LM_RHAND] == Objects[ID_LARA_SKIN].meshIndex + LM_RHAND)
	{
		Lara.meshPtrs[LM_RHAND] = Objects[WeaponObjectMesh(WEAPON_PISTOLS)].meshIndex + LM_RHAND;
		Lara.holsterInfo.rightHolster = HOLSTER_SLOT::Empty;
	}
	else
	{
		Lara.meshPtrs[LM_RHAND] = Objects[ID_LARA_SKIN].meshIndex + LM_RHAND;
		Lara.holsterInfo.rightHolster = HolsterSlotForWeapon(static_cast<LARA_WEAPON_TYPE>(WEAPON_PISTOLS));
	}
}

void ShootLeftGun(ITEM_INFO* item)
{
	Lara.leftArm.flash_gun = 3;
}

void ShootRightGun(ITEM_INFO* item)
{
	Lara.rightArm.flash_gun = 3;
}

void LaraHandsFree(ITEM_INFO* item)
{
	Lara.gunStatus = LG_NO_ARMS;
}

void KillActiveBaddies(ITEM_INFO* item)
{
	if (NextItemActive != NO_ITEM)
	{
		short itemNum = NextItemActive;
		ITEM_INFO* targetItem;

		do
		{
			targetItem = &g_Level.Items[itemNum];

			if (Objects[targetItem->objectNumber].intelligent)
			{
				targetItem->status = ITEM_INVISIBLE;

				if (*(int*)&item != 0xABCDEF)
				{
					RemoveActiveItem(itemNum);
					DisableBaddieAI(itemNum);
					targetItem->flags |= IFLAG_INVISIBLE;
				}
			}

			itemNum = targetItem->nextActive;
		} while (itemNum != NO_ITEM);
	}

	FlipEffect = -1;
}

void LaraLocationPad(ITEM_INFO* item)
{
	FlipEffect = -1;

	Lara.location = TriggerTimer;
	Lara.locationPad = TriggerTimer;
}

void LaraLocation(ITEM_INFO* item)
{
	FlipEffect = -1;

	Lara.location = TriggerTimer;
	if (Lara.highestLocation < TriggerTimer)
		Lara.highestLocation = TriggerTimer;
}

void ExplosionFX(ITEM_INFO* item)
{
	SoundEffect(SFX_TR4_EXPLOSION1, NULL, 0);
	Camera.bounce = -75;
	FlipEffect = -1;
}

void SwapCrowbar(ITEM_INFO* item)
{
	if (Lara.meshPtrs[LM_RHAND] == Objects[ID_LARA_SKIN].meshIndex + LM_RHAND)
		Lara.meshPtrs[LM_RHAND] = Objects[ID_LARA_CROWBAR_ANIM].meshIndex + LM_RHAND;
	else 
		Lara.meshPtrs[LM_RHAND] = Objects[ID_LARA_SKIN].meshIndex + LM_RHAND;
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
		ITEM_INFO* eq = &g_Level.Items[itemList[0]];

		AddActiveItem(itemList[0]);
		eq->status = ITEM_ACTIVE;
		eq->flags |= IFLAG_ACTIVATION_MASK;
	}
	else
	{
		Camera.bounce = -150;
	}

	FlipEffect = -1;
}

void PlaySoundEffect(ITEM_INFO* item)
{
	SoundEffect(TriggerTimer, NULL, 0);
	FlipEffect = -1;
}

void FloorShake(ITEM_INFO* item)
{
	int x = abs(item->pos.xPos - Camera.pos.x);
	int y = abs(item->pos.yPos - Camera.pos.y);
	int z = abs(item->pos.zPos - Camera.pos.z);

	if (x < SECTOR(16) && y < SECTOR(16) && z < SECTOR(16))
	{
		Camera.bounce = 66 * ((SQUARE(x) + SQUARE(y) + SQUARE(z)) / 256 - SQUARE(1024)) / SQUARE(1024);
	}
}

void Turn180(ITEM_INFO* item)
{
	item->pos.yRot -= ANGLE(180);
	item->pos.xRot = -item->pos.xRot;
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