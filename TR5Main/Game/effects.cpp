#include "effects.h"
#include "..\Global\global.h"
#include "Lara.h"
#include "items.h"
#include "lot.h"
#include "tomb4fx.h"
#include "effect2.h"
#include "hair.h"
#include "draw.h"
#include "sphere.h"

int wf = 256;

void(*effect_routines[59])(ITEM_INFO* item) =
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
	void_effect,
	SoundFlipEffect,
	ExplosionFX,
	lara_hands_free,
	void_effect,
	void_effect,
	void_effect,
	shoot_right_gun,
	shoot_left_gun,
	void_effect,
	void_effect,
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
	void_effect /*AddFootprint*/,
	void_effect /*ResetTest*/,
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
	KillActiveBaddies,
	TL_1,
	TL_2,
	TL_3,
	TL_4,
	TL_5,
	TL_6,
	TL_7,
	TL_8,
	TL_9,
	TL_10,
	TL_11,
	TL_12,
};

void TL_1(ITEM_INFO* item)
{
	if (!Savegame.TLCount)
	{
		IsAtmospherePlaying = 0;
		S_CDPlay(9, 0);
		Savegame.TLCount = 1;
	}
}

void TL_2(ITEM_INFO* item)
{
	if (Savegame.TLCount <= 1u)
	{
		IsAtmospherePlaying = 0;
		S_CDPlay(7, 0);
		Savegame.TLCount = 2;
	}
}

void TL_3(ITEM_INFO* item)
{
	if (Savegame.TLCount <= 2u)
	{
		IsAtmospherePlaying = 0;
		S_CDPlay(23, 0);
		Savegame.TLCount = 3;
	}
}

void TL_4(ITEM_INFO* item)
{
	if (Savegame.TLCount <= 3u)
	{
		IsAtmospherePlaying = 0;
		S_CDPlay(39, 0);
		Savegame.TLCount = 4;
	}
}

void TL_5(ITEM_INFO* item)
{
	if (Savegame.TLCount <= 4u)
	{
		IsAtmospherePlaying = 0;
		S_CDPlay(2, 0);
		Savegame.TLCount = 5;
	}
}

void TL_6(ITEM_INFO* item)
{
	if (Savegame.TLCount <= 5u)
	{
		IsAtmospherePlaying = 0;
		S_CDPlay(22, 0);
		Savegame.TLCount = 6;
	}
}

void TL_7(ITEM_INFO* item)
{
	if (Savegame.TLCount <= 6u)
	{
		IsAtmospherePlaying = 0;
		S_CDPlay(51, 0);
		Savegame.TLCount = 7;
	}
}

void TL_8(ITEM_INFO* item)
{
	if (Savegame.TLCount <= 7u)
	{
		IsAtmospherePlaying = 0;
		S_CDPlay(3, 0);
		Savegame.TLCount = 8;
	}
}

void TL_9(ITEM_INFO* item)
{
	if (Savegame.TLCount <= 8u)
	{
		IsAtmospherePlaying = 0;
		S_CDPlay(4, 0);
		Savegame.TLCount = 9;
	}
}

void TL_10(ITEM_INFO* item)
{
	if (Savegame.TLCount == 9)
	{
		IsAtmospherePlaying = 0;
		S_CDPlay(13, 0);
		Savegame.TLCount = 10;
	}
}

void TL_11(ITEM_INFO* item)
{
	if (Savegame.TLCount == 10)
	{
		IsAtmospherePlaying = 0;
		S_CDPlay(0, 0);
		Savegame.TLCount = 11;
	}
}

void TL_12(ITEM_INFO* item)
{
	if (Savegame.TLCount == 11)
	{
		IsAtmospherePlaying = 0;
		S_CDPlay(35, 0);
		Savegame.TLCount = 12;
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

void ClearSpiders()// (F)
{
	/*if (Objects[ID_SPIDER].loaded)
	{
		memset((char*)&Spiders, 0, 64 * sizeof(SPIDER_STRUCT));
		NextSpider = 0;
		FlipEffect = -1;
	}*/
}

void ClearSpidersPatch(ITEM_INFO* item)//39AA4(<), 39FA4(<) (F)
{
	ClearSpiders();
}

/*void SetFog()
{
	FlipEffect = -1;

	
	unsigned __int16 v0; // si
	int v1; // eax
	int v2; // eax
	char v3; // bl
	char v4; // ST14_1
	char v5; // ST18_1

	dword_51CE04 = 0;
	v0 = TriggerTimer;
	v1 = CheckVolumetric();
	if (!v1)
		goto LABEL_5;
	if (v0 == 100)
	{
		dword_51CE04 = 1;
	LABEL_5:
		FlipEffect = -1;
		return v1;
	}
	v2 = FogTable[v0];
	v3 = FogTable[v0] >> 16;
	v4 = BYTE1(v2);
	v5 = FogTable[v0];
	SomeDxFunctionToRemove1(BYTE2(v2), BYTE1(v2), v2);
	LOBYTE(v1) = v4;
	byte_51CE32 = v3;
	byte_51CE31 = v4;
	byte_51CE30 = v5;
	FlipEffect = -1;
	return v1;
}*/

void SetFog(ITEM_INFO* item)//39A44(<), 39F44(<) (F)
{
	FlipEffect = -1;
}

void shoot_left_gun(ITEM_INFO* item)//39A34(<), 39F34(<) (F)
{
	Lara.leftArm.flash_gun = 3;
}

void shoot_right_gun(ITEM_INFO* item)//39A24(<), 39F24(<) (F)
{
	Lara.rightArm.flash_gun = 3;
}

void lara_hands_free(ITEM_INFO* item)//39A18(<), 39F18(<) (F)
{
	Lara.gunStatus = LG_NO_ARMS;
}

void KillActiveBaddies(ITEM_INFO* item)//39938(<), 39E38(<) (F)
{
	if (NextItemActive != NO_ITEM)
	{
		short itemNum = NextItemActive;
		ITEM_INFO* targetItem;

		do
		{
			targetItem = &Items[itemNum];

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

void LaraLocationPad(ITEM_INFO* item)//39710(<), 39C10(<) (F)
{
	FlipEffect = -1;

	Lara.location = TriggerTimer;
	Lara.locationPad = TriggerTimer;
}

void LaraLocation(ITEM_INFO* item)//396D0(<), 39BD0(<) (F)
{
	FlipEffect = -1;

	Lara.location = TriggerTimer;
	if (Lara.highestLocation < TriggerTimer)
		Lara.highestLocation = TriggerTimer;
}

void ExplosionFX(ITEM_INFO* item)//39694(<), 39B94(<) (F)
{
	SoundEffect(SFX_EXPLOSION1, NULL, 0);
	Camera.bounce = -75;
	FlipEffect = -1;
}

void SwapCrowbar(ITEM_INFO* item)//39638(<), 39B38(<) (F)
{
	short* tmp = Meshes[Objects[ID_LARA].meshIndex + 2 * LM_RHAND];

	if (Lara.meshPtrs[LM_RHAND] == tmp)
		Lara.meshPtrs[LM_RHAND] = Meshes[Objects[ID_CROWBAR_ANIM].meshIndex + 2 * LM_RHAND];
	else 
		Lara.meshPtrs[LM_RHAND] = tmp;
}

void ActivateKey(ITEM_INFO* item)//39624(<), 39B24(<) (F)
{
	KeyTriggerActive = 1;
}

void ActivateCamera(ITEM_INFO* item)//39610(<), 39B10(<) (F)
{
	KeyTriggerActive = 2;
}

void PoseidonSFX(ITEM_INFO* item)//395E0(<), 39AE0(<) (F)
{
	SoundEffect(SFX_GRAB_OPEN, NULL, 0);
	FlipEffect = -1;
}

void RubbleFX(ITEM_INFO* item)//39534(<), 39A34(<) (F)
{
	int itemNumber = FindItem(ID_EARTHQUAKE);

	if (itemNumber != NO_ITEM)
	{
		ITEM_INFO* eq = &Items[itemNumber];

		AddActiveItem(itemNumber);
		eq->status = ITEM_ACTIVE;
		eq->flags |= IFLAG_ACTIVATION_MASK;
	}
	else
	{
		Camera.bounce = -150;
	}

	FlipEffect = -1;
}

void SoundFlipEffect(ITEM_INFO* item)//39500(<), 39A00(<) (F)
{
	SoundEffect(TriggerTimer, NULL, 0);
	FlipEffect = -1;
}

void floor_shake_effect(ITEM_INFO* item)//39410, 39910 (F)
{
	int x = abs(item->pos.xPos - Camera.pos.x);
	int y = abs(item->pos.yPos - Camera.pos.y);
	int z = abs(item->pos.zPos - Camera.pos.z);

	if (x < SECTOR(16) && y < SECTOR(16) && z < SECTOR(16))
	{
		Camera.bounce = 66 * ((x * x + y * y + z * z) / 256 - 0x100000) / 0x100000;
	}
}

void turn180_effect(ITEM_INFO* item)//393F4(<), 398F4(<) (F)
{
	item->pos.yRot -= ANGLE(180);
	item->pos.xRot = -item->pos.xRot;
}

void finish_level_effect(ITEM_INFO* item)//393D4(<), 398D4(<) (F)
{
	LevelComplete = CurrentLevel + 1;
}

void void_effect(ITEM_INFO* item)//393CC(<), 398CC(<) (F)
{

}

void ControlWaterfallMist(short itemNumber) // ControlWaterfallMist
{
	ITEM_INFO* item = &Items[itemNumber];
	int x, z;

	if (item->pos.yRot == -ANGLE(180))
	{
		x = item->pos.xPos - (SIN(item->pos.yRot + ANGLE(180)) >> 3) + ((rcossin_tbl[2048] * wf) >> W2V_SHIFT);
		z = item->pos.zPos - (COS(item->pos.yRot + ANGLE(180)) >> 3) + ((rcossin_tbl[2049] * wf) >> W2V_SHIFT);
	}
	else
	{
		//3934C
		x = item->pos.xPos - (SIN(item->pos.yRot + ANGLE(180)) >> 3) + ((SIN(item->pos.yRot + ANGLE(90)) * wf) >> W2V_SHIFT);
		z = item->pos.zPos - (COS(item->pos.yRot + ANGLE(180)) >> 3) + ((COS(item->pos.yRot + ANGLE(90)) * wf) >> W2V_SHIFT);
	}

	//393A0
	TriggerWaterfallMist(x, item->pos.yPos, z, item->pos.yRot + ANGLE(180));
	SoundEffect(SFX_WATERFALL_LOOP, &item->pos, 0);
}

short DoBloodSplat(int x, int y, int z, short a4, short a5, short roomNumber)
{
	GetFloor(x, y, z, &roomNumber);
	if (Rooms[roomNumber].flags & ENV_FLAG_WATER)
		TriggerUnderwaterBlood(x, y, z, a4);
	else
		TriggerBlood(x, y, z, a5 >> 4, a4);

	return 0;
}

int ItemNearLara(PHD_3DPOS* pos, int radius)
{
	int dx = pos->xPos - LaraItem->pos.xPos;
	int dy = pos->yPos - LaraItem->pos.yPos;
	int dz = pos->zPos - LaraItem->pos.zPos;

	if (dx >= -radius
		&& dx <= radius
		&& dz >= -radius
		&& dz <= radius
		&& dy >= -3072
		&& dy <= 3072
		&& SQUARE(dx) + SQUARE(dz) <= SQUARE(radius))
	{
		short* bounds = GetBoundsAccurate(LaraItem);
		if (dy >= bounds[2] && dy <= bounds[3] + 100)
			return 1;
	}

	return 0;
}

void Richochet(PHD_3DPOS* pos)
{
	short angle = mGetAngle(pos->zPos, pos->xPos, LaraItem->pos.zPos, LaraItem->pos.xPos);
	TriggerRicochetSpark((GAME_VECTOR*)pos, angle / 16, 3, 0);
	SoundEffect(SFX_LARA_RICOCHET, pos, 0);
}

void Inject_Effects()
{
	INJECT(0x00432580, ItemNearLara);
	INJECT(0x00478FE0, StopSoundEffect);
	INJECT(0x00432760, DoBloodSplat);
	INJECT(0x00402FB3, ClearSpidersPatch);
	INJECT(0x00432710, Richochet);
}