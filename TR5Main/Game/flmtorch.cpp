#include "flmtorch.h"
#include "..\Global\global.h"
#include "effect2.h"
#include "laraflar.h"
#include "lara.h"
#include "larafire.h"
#include "collide.h"
#include "laramisc.h"
#include "switch.h"
#include "items.h"

short FireBounds[12] =
{
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xF8E4, 0x071C, 0xEAAC, 0x1554, 0xF8E4, 0x071C
};


void TriggerTorchFlame(char fxObj, char node)
{
	SPARKS* spark = &Sparks[GetFreeSpark()];
	spark->on = 1;
	spark->sR = 255;
	spark->sB = 48;
	spark->sG = (GetRandomControl() & 0x1F) + 48;
	spark->dR = (GetRandomControl() & 0x3F) - 64;
	spark->dB = 32;
	spark->dG = (GetRandomControl() & 0x3F) + -128;
	spark->fadeToBlack = 8;
	spark->colFadeSpeed = (GetRandomControl() & 3) + 12;
	spark->transType = 2;
	spark->life = spark->sLife = (GetRandomControl() & 7) + 24;
	spark->y = 0;
	spark->x = (GetRandomControl() & 0xF) - 8;
	spark->z = (GetRandomControl() & 0xF) - 8;
	spark->xVel = (GetRandomControl() & 0xFF) - 128;
	spark->yVel = -16 - (GetRandomControl() & 0xF);
	spark->friction = 5;
	spark->flags = 4762;
	spark->zVel = (GetRandomControl() & 0xFF) - 128;
	spark->rotAng = GetRandomControl() & 0xFFF;
	if (GetRandomControl() & 1)
		spark->rotAdd = -16 - (GetRandomControl() & 0xF);
	else
		spark->rotAdd = (GetRandomControl() & 0xF) + 16;
	spark->gravity = -16 - (GetRandomControl() & 0x1F);
	spark->nodeNumber = node;
	spark->maxYvel = -16 - (GetRandomControl() & 7);
	spark->fxObj = fxObj;
	spark->scalar = 1;
	spark->sSize = spark->size = (GetRandomControl() & 0x1F) + 80;
	spark->dSize = spark->size >> 3;
}

void DoFlameTorch()
{
	switch (Lara.leftArm.lock)
	{
	case 0:
		if (Lara.requestGunType != Lara.gunType)
		{
			Lara.leftArm.lock = 2;
			Lara.leftArm.frameNumber = 31;
			Lara.leftArm.animNumber = Objects[ID_LARA_TORCH_ANIM].animIndex + 2;
			break;
		}
		
		if (TrInput & IN_DRAW
			&& !(LaraItem->gravityStatus)
			&& !LaraItem->fallspeed
			&& LaraItem->currentAnimState != STATE_LARA_JUMP_PREPARE
			&& LaraItem->currentAnimState != STATE_LARA_JUMP_UP
			&& LaraItem->currentAnimState != STATE_LARA_JUMP_FORWARD
			&& LaraItem->currentAnimState != STATE_LARA_JUMP_BACK
			&& LaraItem->currentAnimState != STATE_LARA_JUMP_LEFT
			&& LaraItem->currentAnimState != STATE_LARA_JUMP_RIGHT
			|| Lara.waterStatus == LW_UNDERWATER)
		{
			Lara.leftArm.lock = 1;
			Lara.leftArm.frameNumber = 1;
			Lara.leftArm.animNumber = Objects[ID_LARA_TORCH_ANIM].animIndex + 1;
			if (Lara.waterStatus == LW_UNDERWATER)
				Lara.litTorch = false;
		}

		break;

	case 1:
		if (Lara.leftArm.frameNumber < 12 && LaraItem->gravityStatus)
		{
			Lara.leftArm.lock = 0;
			Lara.leftArm.frameNumber = 0;
			Lara.leftArm.animNumber = Objects[ID_LARA_TORCH_ANIM].animIndex;
		}
		else
		{
			Lara.leftArm.frameNumber++;
			if (Lara.leftArm.frameNumber == 27)
			{
				Lara.litTorch = false;
				Lara.leftArm.lock = 0;
				Lara.gunType = Lara.lastGunType;
				Lara.requestGunType = WEAPON_NONE;
				Lara.gunStatus = LG_NO_ARMS;
			}
			else if (Lara.leftArm.frameNumber == 12)
			{
				Lara.meshPtrs[LM_LHAND] = Meshes[Objects[ID_LARA].meshIndex + 26];
				CreateFlare(ID_BURNING_TORCH_ITEM, 1);
			}
		}

		break;

	case 2:
		Lara.leftArm.frameNumber++;
		if (Lara.leftArm.frameNumber == 41)
		{
			Lara.litTorch = false;
			Lara.leftArm.lock = 0;
			Lara.lastGunType = WEAPON_NONE;
			Lara.requestGunType = WEAPON_NONE;
			Lara.gunStatus = LG_NO_ARMS;
		}
		else if (Lara.leftArm.frameNumber == 36)
		{
			Lara.meshPtrs[LM_LHAND] = Meshes[Objects[ID_LARA].meshIndex + 26];
			CreateFlare(ID_BURNING_TORCH_ITEM, 0);
		}
		break;
	case 3:
		if (LaraItem->currentAnimState != STATE_LARA_MISC_CONTROL)
		{
			Lara.leftArm.lock = 0;
			Lara.leftArm.frameNumber = 0;
			Lara.flareControlLeft = true;
			Lara.litTorch = LaraItem->itemFlags[3] & 1;
			Lara.leftArm.animNumber = Objects[ID_LARA_TORCH_ANIM].animIndex;
		}
		break;
	default:
		break;
	}

	if (Lara.flareControlLeft)
		Lara.gunStatus = LG_READY;

	Lara.leftArm.frameBase = Anims[Lara.leftArm.animNumber].framePtr;

	if (Lara.litTorch)
	{
		PHD_VECTOR pos;

		pos.x = -32;
		pos.y = 64;
		pos.z = 256;

		GetLaraJointPosition(&pos, LJ_LHAND);

		TriggerDynamicLight(pos.x, pos.y, pos.z, 12 - (GetRandomControl() & 1), (GetRandomControl() & 0x3F) + 192, (GetRandomControl() & 0x1F) + 96, 0);
		
		if (!(Wibble & 7))
			TriggerTorchFlame(LaraItem - Items, 0);
		
		SoundEffect(SFX_LOOP_FOR_SMALL_FIRES, (PHD_3DPOS*)&pos, 0);

		TorchRoom = LaraItem->roomNumber;
	}
}

void GetFlameTorch()
{
	if (Lara.gunType == WEAPON_FLARE)
		CreateFlare(ID_FLARE_ITEM, 0);

	Lara.requestGunType = WEAPON_TORCH;
	Lara.gunType = WEAPON_TORCH;
	Lara.flareControlLeft = true;
	Lara.leftArm.animNumber = Objects[ID_LARA_TORCH_ANIM].animIndex;
	Lara.gunStatus = LG_READY;
	Lara.leftArm.lock = 0;
	Lara.leftArm.frameNumber = 0;
	Lara.leftArm.frameBase = Anims[Lara.leftArm.animNumber].framePtr;
	Lara.meshPtrs[LM_LHAND] = Meshes[Objects[ID_LARA].meshIndex + 26];
}

void TorchControl(short itemNumber)
{
	ITEM_INFO* item = &Items[itemNumber];
	
	int oldX = item->pos.xPos;
	int oldY = item->pos.yPos;
	int oldZ = item->pos.zPos;

	if (item->fallspeed)
		item->pos.zRot += ANGLE(1);
	else if (!item->speed)
	{
		item->pos.xRot = 0;
		item->pos.zRot = 0;
	}

	item->pos.xPos += item->speed * SIN(item->pos.yRot) >> W2V_SHIFT;
	item->pos.zPos += item->speed * COS(item->pos.yRot) >> W2V_SHIFT;

	if (Rooms[item->roomNumber].flags & ENV_FLAG_WATER)
	{
		item->fallspeed += (5 - item->fallspeed) / 2;
		item->speed += (5 - item->speed) >> 1;
		if (item->itemFlags[3] != 0)
			item->itemFlags[3] = 0;
	}
	else
	{
		item->fallspeed += 6;
	}

	item->pos.yPos += item->fallspeed;

	DoProperDetection(itemNumber, oldX, oldY, oldZ, SIN(item->pos.yRot) >> W2V_SHIFT, item->fallspeed);
	if (GetCollidedObjects(item, 0, 1, CollidedItems, CollidedMeshes, 0))
	{
		coll.enableBaddiePush = true;
		if (CollidedItems)
		{
			if (!Objects[CollidedItems[0]->objectNumber].intelligent)
				ObjectCollision((CollidedItems[0] - Items) / sizeof(ITEM_INFO), item, &coll);
		}
		else
		{
			STATIC_INFO* sobj = &StaticObjects[CollidedMeshes[0]->staticNumber];
			PHD_3DPOS pos;
			pos.xPos = CollidedMeshes[0]->x;
			pos.yPos = CollidedMeshes[0]->y;
			pos.zPos = CollidedMeshes[0]->z;
			pos.yRot = CollidedMeshes[0]->yRot;
			ItemPushLaraStatic(item, &sobj->xMinc, &pos, &coll);
		}
		item->speed >>= 1;
	}
	if (item->itemFlags[3])
	{
		TriggerDynamicLight(item->pos.xPos, item->pos.yPos, item->pos.zPos, 12 - (GetRandomControl() & 1), (GetRandomControl() & 0x3F) + 192, (GetRandomControl() & 0x1F) + 96, 0);
		if (!(Wibble & 7))
			TriggerTorchFlame(itemNumber, 1);
		TorchRoom = item->roomNumber;
		SoundEffect(SFX_LOOP_FOR_SMALL_FIRES, &item->pos, 0);
	}
}

void FireCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* coll)
{
	ITEM_INFO* item = &Items[itemNumber];

	if (Lara.gunType != WEAPON_TORCH
		|| Lara.gunStatus != LG_READY
		|| Lara.leftArm.lock
		|| Lara.litTorch == (item->status == ITEM_ACTIVE)
		|| item->timer == -1
		|| !(TrInput & IN_ACTION)
		|| l->currentAnimState != STATE_LARA_STOP
		|| l->animNumber != ANIMATION_LARA_STAY_IDLE
		|| l->gravityStatus)
	{
		if (item->objectNumber == ID_BURNING_ROOTS)
			ObjectCollision(itemNumber, l, coll);
	}
	else
	{
		short rot = item->pos.yRot;

		switch (item->objectNumber)
		{
		case ID_FLAME_EMITTER:
			FireBounds[0] = -256;
			FireBounds[1] = 256;
			FireBounds[2] = 0;
			FireBounds[3] = 1024;
			FireBounds[4] = -800;
			FireBounds[5] = 800;
			break;
		case ID_FLAME_EMITTER2:
			FireBounds[1] = -256;
			FireBounds[2] = 256;
			FireBounds[3] = 0;
			FireBounds[4] = 1024;
			FireBounds[5] = -600;
			FireBounds[6] = 600;
			break;
		case ID_BURNING_ROOTS:
			FireBounds[0] = -384;
			FireBounds[1] = 384;
			FireBounds[2] = 0;
			FireBounds[3] = 2048;
			FireBounds[4] = -384;
			FireBounds[5] = 384;
			break;
		}

		item->pos.yRot = l->pos.yRot;

		if (TestLaraPosition(FireBounds, item, l))
		{
			if (item->objectNumber == ID_BURNING_ROOTS)
			{
				l->animNumber = ANIMATION_LARA_TORCH_LIGHT_5;
			}
			else
			{
				int dy = abs(l->pos.yPos - item->pos.yPos);
				l->itemFlags[3] = 1;
				l->animNumber = (dy >> 8) + ANIMATION_LARA_TORCH_LIGHT_1;
			}
			l->currentAnimState = STATE_LARA_MISC_CONTROL;
			l->frameNumber = Anims[l->animNumber].frameBase;
			Lara.flareControlLeft = false;
			Lara.leftArm.lock = 3;
			Lara.generalPtr = (void*)itemNumber;
		}
		
		item->pos.yRot = rot;
	}
	if ((short)Lara.generalPtr == itemNumber && item->status != ITEM_ACTIVE && l->currentAnimState == STATE_LARA_MISC_CONTROL)
	{
		if (l->animNumber >= ANIMATION_LARA_TORCH_LIGHT_1 && l->animNumber <= ANIMATION_LARA_TORCH_LIGHT_5)
		{
			if (l->frameNumber - Anims[l->animNumber].frameBase == 40)
			{
				TestTriggersAtXYZ(item->pos.xPos, item->pos.yPos, item->pos.zPos, item->roomNumber, 1, item->flags & 0x3E00);
				item->flags |= 0x3E00;
				item->itemFlags[3] = 0;
				item->status = ITEM_ACTIVE;
				AddActiveItem(itemNumber);
			}
		}
	}
}