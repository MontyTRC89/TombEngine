#include "framework.h"
#include "Objects/Generic/Object/burning_torch.h"
#include "Game/Lara/lara_flare.h"
#include "Game/Lara/lara.h"
#include "Game/animation.h"
#include "Game/items.h"
#include "Specific/level.h"
#include "Specific/input.h"
#include "Sound/sound.h"
#include "Objects/Effects/flame_emitters.h"
#include "Game/effects/effects.h"
#include "Specific/setup.h"
#include "Game/collide.h"
#include "Game/control/los.h"

using namespace TEN::Entities::Effects;

namespace TEN::Entities::Generic
{
	void TriggerTorchFlame(char fxObj, char node)
	{
		SPARKS* spark = &Sparks[GetFreeSpark()];

		spark->on = true;

		spark->sR = 255;
		spark->sB = 48;
		spark->sG = (GetRandomControl() & 0x1F) + 48;
		spark->dR = (GetRandomControl() & 0x3F) - 64;
		spark->dB = 32;
		spark->dG = (GetRandomControl() & 0x3F) + -128;

		spark->fadeToBlack = 8;
		spark->colFadeSpeed = (GetRandomControl() & 3) + 12;
		spark->transType = TransTypeEnum::COLADD;
		spark->life = spark->sLife = (GetRandomControl() & 7) + 24;

		spark->x = (GetRandomControl() & 0xF) - 8;
		spark->y = 0;
		spark->z = (GetRandomControl() & 0xF) - 8;

		spark->xVel = (GetRandomControl() & 0xFF) - 128;
		spark->yVel = -16 - (GetRandomControl() & 0xF);
		spark->zVel = (GetRandomControl() & 0xFF) - 128;

		spark->friction = 5;

		spark->flags = SP_NODEATTACH | SP_EXPDEF | SP_ITEM | SP_ROTATE | SP_DEF | SP_SCALE;

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
		spark->dSize = spark->size / 8;
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
				&& LaraItem->currentAnimState != LS_JUMP_PREPARE
				&& LaraItem->currentAnimState != LS_JUMP_UP
				&& LaraItem->currentAnimState != LS_JUMP_FORWARD
				&& LaraItem->currentAnimState != LS_JUMP_BACK
				&& LaraItem->currentAnimState != LS_JUMP_LEFT
				&& LaraItem->currentAnimState != LS_JUMP_RIGHT
				|| Lara.waterStatus == LW_UNDERWATER)
			{
				Lara.leftArm.lock = true;
				Lara.leftArm.frameNumber = 1;
				Lara.leftArm.animNumber = Objects[ID_LARA_TORCH_ANIM].animIndex + 1;
				if (Lara.waterStatus == LW_UNDERWATER)
					Lara.litTorch = false;
			}

			break;

		case 1:
			if (Lara.leftArm.frameNumber < 12 && LaraItem->gravityStatus)
			{
				Lara.leftArm.lock = false;
				Lara.leftArm.frameNumber = 0;
				Lara.leftArm.animNumber = Objects[ID_LARA_TORCH_ANIM].animIndex;
			}
			else
			{
				Lara.leftArm.frameNumber++;
				if (Lara.leftArm.frameNumber == 27)
				{
					Lara.litTorch = false;
					Lara.flareControlLeft = false;
					Lara.leftArm.lock = false;
					Lara.gunType = Lara.lastGunType;
					Lara.requestGunType = WEAPON_NONE;
					Lara.gunStatus = LG_NO_ARMS;
				}
				else if (Lara.leftArm.frameNumber == 12)
				{
					Lara.meshPtrs[LM_LHAND] = Objects[ID_LARA_SKIN].meshIndex + LM_LHAND;
					CreateFlare(LaraItem, ID_BURNING_TORCH_ITEM, true);
				}
			}

			break;

		case 2:
			Lara.leftArm.frameNumber++;
			if (Lara.leftArm.frameNumber == 41)
			{
				Lara.litTorch = false;
				Lara.flareControlLeft = false;
				Lara.leftArm.lock = false;
				Lara.lastGunType = WEAPON_NONE;
				Lara.gunType = WEAPON_NONE;
				Lara.gunStatus = LG_NO_ARMS;
			}
			else if (Lara.leftArm.frameNumber == 36)
			{
				Lara.meshPtrs[LM_LHAND] = Objects[ID_LARA_SKIN].meshIndex + LM_LHAND;
				CreateFlare(LaraItem, ID_BURNING_TORCH_ITEM, false);
			}
			break;
		case 3:
			if (LaraItem->currentAnimState != LS_MISC_CONTROL)
			{
				Lara.leftArm.lock = false;
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

		Lara.leftArm.frameBase = g_Level.Anims[Lara.leftArm.animNumber].framePtr;

		if (Lara.litTorch)
		{
			PHD_VECTOR pos;

			pos.x = -32;
			pos.y = 64;
			pos.z = 256;

			GetLaraJointPosition(&pos, LM_LHAND);

			TriggerDynamicLight(pos.x, pos.y, pos.z, 12 - (GetRandomControl() & 1), (GetRandomControl() & 0x3F) + 192, (GetRandomControl() & 0x1F) + 96, 0);

			if (!(Wibble & 7))
				TriggerTorchFlame(LaraItem - g_Level.Items.data(), 0);

			SoundEffect(SFX_TR4_LOOP_FOR_SMALL_FIRES, (PHD_3DPOS*)&pos, 0);
		}
	}

	void GetFlameTorch()
	{
		if (Lara.gunType == WEAPON_FLARE)
			CreateFlare(LaraItem, ID_FLARE_ITEM, false);

		Lara.requestGunType = WEAPON_TORCH;
		Lara.gunType = WEAPON_TORCH;
		Lara.flareControlLeft = true;
		Lara.leftArm.animNumber = Objects[ID_LARA_TORCH_ANIM].animIndex;
		Lara.gunStatus = LG_READY;
		Lara.leftArm.lock = false;
		Lara.leftArm.frameNumber = 0;
		Lara.leftArm.frameBase = g_Level.Anims[Lara.leftArm.animNumber].framePtr;

		Lara.meshPtrs[LM_LHAND] = Objects[ID_LARA_TORCH_ANIM].meshIndex + LM_LHAND;
	}

	void TorchControl(short itemNumber)
	{
		ITEM_INFO* item = &g_Level.Items[itemNumber];

		int oldX = item->pos.xPos;
		int oldY = item->pos.yPos;
		int oldZ = item->pos.zPos;

		if (item->fallspeed)
			item->pos.zRot += ANGLE(5);
		else if (!item->speed)
		{
			item->pos.xRot = 0;
			item->pos.zRot = 0;
		}

		int xv = item->speed * phd_sin(item->pos.yRot);
		int zv = item->speed * phd_cos(item->pos.yRot);

		item->pos.xPos += xv;
		item->pos.zPos += zv;

		if (g_Level.Rooms[item->roomNumber].flags & ENV_FLAG_WATER)
		{
			item->fallspeed += (5 - item->fallspeed) / 2;
			item->speed += (5 - item->speed) / 2;
			if (item->itemFlags[3] != 0)
				item->itemFlags[3] = 0;
		}
		else
		{
			item->fallspeed += 6;
		}

		item->pos.yPos += item->fallspeed;

		DoProjectileDynamics(itemNumber, oldX, oldY, oldZ, xv, item->fallspeed, zv);

		if (GetCollidedObjects(item, 0, 1, CollidedItems, CollidedMeshes, 0))
		{
			LaraCollision.Setup.EnableObjectPush = true;
			if (CollidedItems)
			{
				if (!Objects[CollidedItems[0]->objectNumber].intelligent
					 && CollidedItems[0]->objectNumber != ID_LARA)
					ObjectCollision(CollidedItems[0] - g_Level.Items.data(), item, &LaraCollision);
			}
			else
			{
				ItemPushStatic(item, CollidedMeshes[0], &LaraCollision);
			}
			item->speed >>= 1;
		}
		if (item->itemFlags[3])
		{
			TriggerDynamicLight(item->pos.xPos, item->pos.yPos, item->pos.zPos, 12 - (GetRandomControl() & 1), (GetRandomControl() & 0x3F) + 192, (GetRandomControl() & 0x1F) + 96, 0);
			if (!(Wibble & 7))
				TriggerTorchFlame(itemNumber, 1);
			SoundEffect(SFX_TR4_LOOP_FOR_SMALL_FIRES, &item->pos, 0);
		}
	}

	void LaraTorch(PHD_VECTOR* src, PHD_VECTOR* target, int rot, int color)
	{
		GAME_VECTOR pos1;
		pos1.x = src->x;
		pos1.y = src->y;
		pos1.z = src->z;
		pos1.roomNumber = LaraItem->roomNumber;

		GAME_VECTOR pos2;
		pos2.x = target->x;
		pos2.y = target->y;
		pos2.z = target->z;

		TriggerDynamicLight(pos1.x, pos1.y, pos1.z, 12, color, color, color >> 1);

		if (!LOS(&pos1, &pos2))
		{
			int l = sqrt(SQUARE(pos1.x - pos2.x) + SQUARE(pos1.y - pos2.y) + SQUARE(pos1.z - pos2.z)) * STEP_SIZE;

			if (l + 8 > 31)
				l = 31;

			if (color - l >= 0)
				TriggerDynamicLight(pos2.x, pos2.y, pos2.z, l + 8, color - l, color - l, (color - l) * 2);
		}
	}

	void FireCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* coll)
	{
		ITEM_INFO* item = &g_Level.Items[itemNumber];

		if (Lara.gunType != WEAPON_TORCH
			|| Lara.gunStatus != LG_READY
			|| Lara.leftArm.lock
			|| Lara.litTorch == (item->status == ITEM_ACTIVE)
			|| item->timer == -1
			|| !(TrInput & IN_ACTION)
			|| l->currentAnimState != LS_STOP
			|| l->animNumber != LA_STAND_IDLE
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
				FireBounds.boundingBox.X1 = -256;
				FireBounds.boundingBox.X2 = 256;
				FireBounds.boundingBox.Y1 = 0;
				FireBounds.boundingBox.Y2 = 1024;
				FireBounds.boundingBox.Z1 = -800;
				FireBounds.boundingBox.Z2 = 800;
				break;
			case ID_FLAME_EMITTER2:
				FireBounds.boundingBox.X1 = -256;
				FireBounds.boundingBox.X2 = 256;
				FireBounds.boundingBox.Y1 = 0;
				FireBounds.boundingBox.Y2 = 1024;
				FireBounds.boundingBox.Z1 = -600;
				FireBounds.boundingBox.Z2 = 600;
				break;
			case ID_BURNING_ROOTS:
				FireBounds.boundingBox.X1 = -384;
				FireBounds.boundingBox.X2 = 384;
				FireBounds.boundingBox.Y1 = 0;
				FireBounds.boundingBox.Y2 = 2048;
				FireBounds.boundingBox.Z1 = -384;
				FireBounds.boundingBox.Z2 = 384;
				break;
			}

			item->pos.yRot = l->pos.yRot;

			if (TestLaraPosition(&FireBounds, item, l))
			{
				if (item->objectNumber == ID_BURNING_ROOTS)
				{
					l->animNumber = LA_TORCH_LIGHT_5;
				}
				else
				{
					int dy = abs(l->pos.yPos - item->pos.yPos);
					l->itemFlags[3] = 1;
					l->animNumber = (dy >> 8) + LA_TORCH_LIGHT_1;
				}
				l->currentAnimState = LS_MISC_CONTROL;
				l->frameNumber = g_Level.Anims[l->animNumber].frameBase;
				Lara.flareControlLeft = false;
				Lara.leftArm.lock = 3;
				Lara.interactedItem = itemNumber;
			}

			item->pos.yRot = rot;
		}
		if (Lara.interactedItem == itemNumber && item->status != ITEM_ACTIVE && l->currentAnimState == LS_MISC_CONTROL)
		{
			if (l->animNumber >= LA_TORCH_LIGHT_1 && l->animNumber <= LA_TORCH_LIGHT_5)
			{
				if (l->frameNumber - g_Level.Anims[l->animNumber].frameBase == 40)
				{
					TestTriggers(item, true, item->flags & IFLAG_ACTIVATION_MASK);
					item->flags |= 0x3E00;
					item->itemFlags[3] = 0;
					item->status = ITEM_ACTIVE;
					AddActiveItem(itemNumber);
				}
			}
		}
	}
}