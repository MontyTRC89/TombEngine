#include "framework.h"
#include "effect.h"
#include "effect2.h"
#include "Lara.h"
#include "lot.h"
#include "tomb4fx.h"
#include "hair.h"
#include "draw.h"
#include "sphere.h"
#include "footprint.h"
#include "level.h"
#include "debris.h"
#include "setup.h"
#include "camera.h"
#include "savegame.h"
#include "sound.h"
#include "tr5_rats_emitter.h"
#include "tr5_bats_emitter.h"
#include "tr5_spider_emitter.h"
#include "tr5_pushableblock.h"
#include "pickup.h"
#include "puzzles_keys.h"
#include "lara_fire.h"

using std::function;
constexpr auto ITEM_RADIUS_YMAX = SECTOR(3);
using namespace T5M::Effects::Footprints;

short FXType;
FX_INFO* EffectList;

function<EffectFunction> effect_routines[59] =
{
	turn180_effect,
	floor_shake_effect,
	PoseidonSFX,
	LaraBubbles,
	finish_level_effect,
	ActivateCamera,
	ActivateKey,
	RubbleFX,
	SwapCrowbar,
	pickup,
	SoundFlipEffect,
	ExplosionFX,
	lara_hands_free,
	puzzle,
	draw_right_pistol,
	draw_left_pistol,
	shoot_right_gun,
	shoot_left_gun,
	pushLoop,
	pushEnd,
	void_effect,
	invisibility_on,
	invisibility_off,
	void_effect,
	void_effect,
	void_effect,
	reset_hair,
	void_effect,
	SetFog,
	void_effect,
	LaraLocation,
	ClearSpidersPatch,
	AddFootprint,
	void_effect, // resettest
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	void_effect,
	LaraLocationPad,
	KillActiveBaddies
};

void pickup(ITEM_INFO* item)
{
	do_pickup();
}

void puzzle(ITEM_INFO* item)
{
	do_puzzle();
}

// TODO: here are sound for lara footstep too !
void AddFootprint(ITEM_INFO* item)
{
	if (item != LaraItem)
		return;

	FOOTPRINT_STRUCT footprint;
	PHD_3DPOS footprintPosition;

	if (CheckFootOnFloor(*item, LM_LFOOT, footprintPosition))
	{
		if (footprints.size() >= MAX_FOOTPRINTS)
			footprints.pop_back();
		
		memset(&footprint, 0, sizeof(FOOTPRINT_STRUCT));
		footprint.pos = footprintPosition;
		footprint.lifeStartFading = 30 * 10;
		footprint.startOpacity = 64;
		footprint.life = 30 * 20;
		footprint.active = true;
		footprints.push_front(footprint);
	}

	if (CheckFootOnFloor(*item, LM_RFOOT, footprintPosition))
	{
		if (footprints.size() >= MAX_FOOTPRINTS)
			footprints.pop_back();

		memset(&footprint, 0, sizeof(FOOTPRINT_STRUCT));
		footprint.pos = footprintPosition;
		footprint.lifeStartFading = 30*10;
		footprint.startOpacity = 64;
		footprint.life = 30 * 20;
		footprint.active = true;
		footprints.push_front(footprint);
	}
}

void reset_hair(ITEM_INFO* item)
{
	InitialiseHair();
}

void invisibility_off(ITEM_INFO* item)
{
	item->status = ITEM_ACTIVE;
}

void invisibility_on(ITEM_INFO* item)
{
	item->status = ITEM_INVISIBLE;
}

void SetFog(ITEM_INFO* item)
{
	FlipEffect = -1;
}

void draw_left_pistol(ITEM_INFO* item)
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

void draw_right_pistol(ITEM_INFO* item)
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

void shoot_left_gun(ITEM_INFO* item)
{
	Lara.leftArm.flash_gun = 3;
}

void shoot_right_gun(ITEM_INFO* item)
{
	Lara.rightArm.flash_gun = 3;
}

void lara_hands_free(ITEM_INFO* item)
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
	const auto itemList = FindItem(ID_EARTHQUAKE);

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

void SoundFlipEffect(ITEM_INFO* item)
{
	SoundEffect(TriggerTimer, NULL, 0);
	FlipEffect = -1;
}

void floor_shake_effect(ITEM_INFO* item)
{
	int x = abs(item->pos.xPos - Camera.pos.x);
	int y = abs(item->pos.yPos - Camera.pos.y);
	int z = abs(item->pos.zPos - Camera.pos.z);

	if (x < SECTOR(16) && y < SECTOR(16) && z < SECTOR(16))
	{
		Camera.bounce = 66 * ((x * x + y * y + z * z) / 256 - 0x100000) / 0x100000;
	}
}

void turn180_effect(ITEM_INFO* item)
{
	item->pos.yRot -= ANGLE(180);
	item->pos.xRot = -item->pos.xRot;
}

void finish_level_effect(ITEM_INFO* item)
{
	LevelComplete = CurrentLevel + 1;
}

void void_effect(ITEM_INFO* item)
{

}

void ControlWaterfallMist(short itemNumber) // ControlWaterfallMist
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];
	int x, z;

	x = item->pos.xPos - phd_sin(item->pos.yRot + ANGLE(180)) * 512 + phd_sin(item->pos.yRot - ANGLE(90)) * 256;
	z = item->pos.zPos - phd_cos(item->pos.yRot + ANGLE(180)) * 512 + phd_cos(item->pos.yRot - ANGLE(90)) * 256;

	TriggerWaterfallMist(x, item->pos.yPos, z, item->pos.yRot + ANGLE(180));
	SoundEffect(SFX_TR4_WATERFALL_LOOP, &item->pos, 0);
}

short DoBloodSplat(int x, int y, int z, short a4, short a5, short roomNumber)
{
	short roomNum = roomNumber;
	GetFloor(x, y, z, &roomNum);
	if (g_Level.Rooms[roomNum].flags & ENV_FLAG_WATER)
		TriggerUnderwaterBlood(x, y, z, a4);
	else
		TriggerBlood(x, y, z, a5 / 16, a4);
	return 0;
}

static bool ItemCollide(int value, int radius)
{
	return value >= -radius && value <= radius;
}

static bool ItemInRange(int x, int z, int radius)
{
	return (SQUARE(x) + SQUARE(z)) <= SQUARE(radius);
}

bool ItemNearLara(PHD_3DPOS* pos, int radius)
{
	BOUNDING_BOX* bounds;
	GAME_VECTOR target;
	target.x = pos->xPos - LaraItem->pos.xPos;
	target.y = pos->yPos - LaraItem->pos.yPos;
	target.z = pos->zPos - LaraItem->pos.zPos;
	if (!ItemCollide(target.y, ITEM_RADIUS_YMAX))
		return false;
	if (!ItemCollide(target.x, radius) || !ItemCollide(target.z, radius))
		return false;
	if (!ItemInRange(target.x, target.z, radius))
		return false;

	bounds = GetBoundsAccurate(LaraItem);
	if (target.y >= bounds->Y1 && target.y <= (bounds->Y2 + LARA_RAD))
		return true;

	return false;
}

bool ItemNearTarget(PHD_3DPOS* src, ITEM_INFO* target, int radius)
{
	BOUNDING_BOX* bounds;
	PHD_VECTOR pos;
	pos.x = src->xPos - target->pos.xPos;
	pos.y = src->yPos - target->pos.yPos;
	pos.z = src->zPos - target->pos.zPos;
	if (!ItemCollide(pos.y, ITEM_RADIUS_YMAX))
		return false;
	if (!ItemCollide(pos.x, radius) || !ItemCollide(pos.z, radius))
		return false;
	if (!ItemInRange(pos.x, pos.z, radius))
		return false;

	bounds = GetBoundsAccurate(target);
	if (pos.y >= bounds->Y1 && pos.y <= bounds->Y2)
		return true;

	return false;
}

void Richochet(PHD_3DPOS* pos)
{
	short angle = mGetAngle(pos->zPos, pos->xPos, LaraItem->pos.zPos, LaraItem->pos.xPos);
	GAME_VECTOR target;
	target.x = pos->xPos;
	target.y = pos->yPos;
	target.z = pos->zPos;
	TriggerRicochetSpark(&target, angle / 16, 3, 0);
	SoundEffect(SFX_TR4_LARA_RICOCHET, pos, 0);
}

void DoLotsOfBlood(int x, int y, int z, int speed, short direction, short roomNumber, int count)
{
    for (int i = 0; i < count; i++)
    {
        DoBloodSplat(x + 256 - (GetRandomControl() * 512 / 0x8000),
                     y + 256 - (GetRandomControl() * 512 / 0x8000),
                     z + 256 - (GetRandomControl() * 512 / 0x8000),
                     speed, direction, roomNumber);
    }
}
