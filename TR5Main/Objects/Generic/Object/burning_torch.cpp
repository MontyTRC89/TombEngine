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
#include "Game/collision/collide_room.h"
#include "Game/collision/collide_item.h"
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
		switch (Lara.LeftArm.lock)
		{
		case 0:
			if (Lara.Control.WeaponControl.RequestGunType != Lara.Control.WeaponControl.GunType)
			{
				Lara.LeftArm.lock = true;
				Lara.LeftArm.frameNumber = 31;
				Lara.LeftArm.animNumber = Objects[ID_LARA_TORCH_ANIM].animIndex + 2;
				break;
			}

			if (TrInput & IN_DRAW
				&& !(LaraItem->Airborne)
				&& !LaraItem->VerticalVelocity
				&& LaraItem->ActiveState != LS_JUMP_PREPARE
				&& LaraItem->ActiveState != LS_JUMP_UP
				&& LaraItem->ActiveState != LS_JUMP_FORWARD
				&& LaraItem->ActiveState != LS_JUMP_BACK
				&& LaraItem->ActiveState != LS_JUMP_LEFT
				&& LaraItem->ActiveState != LS_JUMP_RIGHT
				|| Lara.Control.WaterStatus == WaterStatus::Underwater)
			{
				Lara.LeftArm.lock = true;
				Lara.LeftArm.frameNumber = 1;
				Lara.LeftArm.animNumber = Objects[ID_LARA_TORCH_ANIM].animIndex + 1;
				if (Lara.Control.WaterStatus == WaterStatus::Underwater)
					Lara.litTorch = false;
			}

			break;

		case 1:
			if (Lara.LeftArm.frameNumber < 12 && LaraItem->Airborne)
			{
				Lara.LeftArm.lock = false;
				Lara.LeftArm.frameNumber = 0;
				Lara.LeftArm.animNumber = Objects[ID_LARA_TORCH_ANIM].animIndex;
			}
			else
			{
				Lara.LeftArm.frameNumber++;
				if (Lara.LeftArm.frameNumber == 27)
				{
					Lara.litTorch = false;
					Lara.Flare.ControlLeft = false;
					Lara.LeftArm.lock = false;
					Lara.Control.WeaponControl.GunType = Lara.Control.WeaponControl.LastGunType;
					Lara.Control.WeaponControl.RequestGunType = WEAPON_NONE;
					Lara.Control.HandStatus = HandStatus::Free;
				}
				else if (Lara.LeftArm.frameNumber == 12)
				{
					Lara.meshPtrs[LM_LHAND] = Objects[ID_LARA_SKIN].meshIndex + LM_LHAND;
					CreateFlare(LaraItem, ID_BURNING_TORCH_ITEM, true);
				}
			}

			break;

		case 2:
			Lara.LeftArm.frameNumber++;
			if (Lara.LeftArm.frameNumber == 41)
			{
				Lara.litTorch = false;
				Lara.Flare.ControlLeft = false;
				Lara.LeftArm.lock = false;
				Lara.Control.WeaponControl.LastGunType = WEAPON_NONE;
				Lara.Control.WeaponControl.GunType = WEAPON_NONE;
				Lara.Control.HandStatus = HandStatus::Free;
			}
			else if (Lara.LeftArm.frameNumber == 36)
			{
				Lara.meshPtrs[LM_LHAND] = Objects[ID_LARA_SKIN].meshIndex + LM_LHAND;
				CreateFlare(LaraItem, ID_BURNING_TORCH_ITEM, false);
			}
			break;
		case 3:
			if (LaraItem->ActiveState != LS_MISC_CONTROL)
			{
				Lara.LeftArm.lock = false;
				Lara.LeftArm.frameNumber = 0;
				Lara.Flare.ControlLeft = true;
				Lara.litTorch = LaraItem->ItemFlags[3] & 1;
				Lara.LeftArm.animNumber = Objects[ID_LARA_TORCH_ANIM].animIndex;
			}
			break;
		default:
			break;
		}

		if (Lara.Flare.ControlLeft)
			Lara.Control.HandStatus = HandStatus::WeaponReady;

		Lara.LeftArm.frameBase = g_Level.Anims[Lara.LeftArm.animNumber].framePtr;

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
		if (Lara.Control.WeaponControl.GunType == WEAPON_FLARE)
			CreateFlare(LaraItem, ID_FLARE_ITEM, false);

		Lara.Control.WeaponControl.RequestGunType = WEAPON_TORCH;
		Lara.Control.WeaponControl.GunType = WEAPON_TORCH;
		Lara.Flare.ControlLeft = true;
		Lara.LeftArm.animNumber = Objects[ID_LARA_TORCH_ANIM].animIndex;
		Lara.Control.HandStatus = HandStatus::WeaponReady;
		Lara.LeftArm.lock = false;
		Lara.LeftArm.frameNumber = 0;
		Lara.LeftArm.frameBase = g_Level.Anims[Lara.LeftArm.animNumber].framePtr;

		Lara.meshPtrs[LM_LHAND] = Objects[ID_LARA_TORCH_ANIM].meshIndex + LM_LHAND;
	}

	void TorchControl(short itemNumber)
	{
		ITEM_INFO* item = &g_Level.Items[itemNumber];

		int oldX = item->Position.xPos;
		int oldY = item->Position.yPos;
		int oldZ = item->Position.zPos;

		if (item->VerticalVelocity)
			item->Position.zRot += ANGLE(5);
		else if (!item->Velocity)
		{
			item->Position.xRot = 0;
			item->Position.zRot = 0;
		}

		int xv = item->Velocity * phd_sin(item->Position.yRot);
		int zv = item->Velocity * phd_cos(item->Position.yRot);

		item->Position.xPos += xv;
		item->Position.zPos += zv;

		if (g_Level.Rooms[item->RoomNumber].flags & ENV_FLAG_WATER)
		{
			item->VerticalVelocity += (5 - item->VerticalVelocity) / 2;
			item->Velocity += (5 - item->Velocity) / 2;
			if (item->ItemFlags[3] != 0)
				item->ItemFlags[3] = 0;
		}
		else
		{
			item->VerticalVelocity += 6;
		}

		item->Position.yPos += item->VerticalVelocity;

		DoProjectileDynamics(itemNumber, oldX, oldY, oldZ, xv, item->VerticalVelocity, zv);

		if (GetCollidedObjects(item, 0, true, CollidedItems, CollidedMeshes, 0))
		{
			LaraCollision.Setup.EnableObjectPush = true;
			if (CollidedItems)
			{
				if (!Objects[CollidedItems[0]->ObjectNumber].intelligent
					 && CollidedItems[0]->ObjectNumber != ID_LARA)
					ObjectCollision(CollidedItems[0] - g_Level.Items.data(), item, &LaraCollision);
			}
			else
			{
				ItemPushStatic(item, CollidedMeshes[0], &LaraCollision);
			}
			item->Velocity >>= 1;
		}
		if (item->ItemFlags[3])
		{
			TriggerDynamicLight(item->Position.xPos, item->Position.yPos, item->Position.zPos, 12 - (GetRandomControl() & 1), (GetRandomControl() & 0x3F) + 192, (GetRandomControl() & 0x1F) + 96, 0);
			if (!(Wibble & 7))
				TriggerTorchFlame(itemNumber, 1);
			SoundEffect(SFX_TR4_LOOP_FOR_SMALL_FIRES, &item->Position, 0);
		}
	}

	void LaraTorch(PHD_VECTOR* src, PHD_VECTOR* target, int rot, int color)
	{
		GAME_VECTOR pos1;
		pos1.x = src->x;
		pos1.y = src->y;
		pos1.z = src->z;
		pos1.roomNumber = LaraItem->RoomNumber;

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

		if (Lara.Control.WeaponControl.GunType != WEAPON_TORCH
			|| Lara.Control.HandStatus != HandStatus::WeaponReady
			|| Lara.LeftArm.lock
			|| Lara.litTorch == (item->Status == ITEM_ACTIVE)
			|| item->Timer == -1
			|| !(TrInput & IN_ACTION)
			|| l->ActiveState != LS_IDLE
			|| l->AnimNumber != LA_STAND_IDLE
			|| l->Airborne)
		{
			if (item->ObjectNumber == ID_BURNING_ROOTS)
				ObjectCollision(itemNumber, l, coll);
		}
		else
		{
			short rot = item->Position.yRot;

			switch (item->ObjectNumber)
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

			item->Position.yRot = l->Position.yRot;

			if (TestLaraPosition(&FireBounds, item, l))
			{
				if (item->ObjectNumber == ID_BURNING_ROOTS)
				{
					l->AnimNumber = LA_TORCH_LIGHT_5;
				}
				else
				{
					int dy = abs(l->Position.yPos - item->Position.yPos);
					l->ItemFlags[3] = 1;
					l->AnimNumber = (dy >> 8) + LA_TORCH_LIGHT_1;
				}
				l->ActiveState = LS_MISC_CONTROL;
				l->FrameNumber = g_Level.Anims[l->AnimNumber].frameBase;
				Lara.Flare.ControlLeft = false;
				Lara.LeftArm.lock = true;
				Lara.interactedItem = itemNumber;
			}

			item->Position.yRot = rot;
		}
		if (Lara.interactedItem == itemNumber && item->Status != ITEM_ACTIVE && l->ActiveState == LS_MISC_CONTROL)
		{
			if (l->AnimNumber >= LA_TORCH_LIGHT_1 && l->AnimNumber <= LA_TORCH_LIGHT_5)
			{
				if (l->FrameNumber - g_Level.Anims[l->AnimNumber].frameBase == 40)
				{
					TestTriggers(item, true, item->Flags & IFLAG_ACTIVATION_MASK);
					item->Flags |= 0x3E00;
					item->ItemFlags[3] = 0;
					item->Status = ITEM_ACTIVE;
					AddActiveItem(itemNumber);
				}
			}
		}
	}
}