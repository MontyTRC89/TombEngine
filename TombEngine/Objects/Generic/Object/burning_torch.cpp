#include "framework.h"
#include "Objects/Generic/Object/burning_torch.h"
#include "Game/Lara/lara_flare.h"
#include "Game/Lara/lara_helpers.h"
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
#include "Renderer/Renderer11Enums.h"

using namespace TEN::Input;
using namespace TEN::Entities::Effects;

namespace TEN::Entities::Generic
{
	void TriggerTorchFlame(char fxObj, char node)
	{
		auto* spark = GetFreeParticle();

		spark->on = true;

		spark->sR = 255;
		spark->sB = 48;
		spark->sG = (GetRandomControl() & 0x1F) + 48;
		spark->dR = (GetRandomControl() & 0x3F) - 64;
		spark->dB = 32;
		spark->dG = (GetRandomControl() & 0x3F) + -128;

		spark->fadeToBlack = 8;
		spark->colFadeSpeed = (GetRandomControl() & 3) + 12;
		spark->blendMode = BLEND_MODES::BLENDMODE_ADDITIVE;
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
		int const holdAnimNumber = Objects[ID_LARA_TORCH_ANIM].animIndex;
		int const throwAnimNumber = Objects[ID_LARA_TORCH_ANIM].animIndex + 1;
		int const dropAnimNumber = Objects[ID_LARA_TORCH_ANIM].animIndex + 2;

		if (Lara.Torch.State == TorchState::Holding)
		{
			if (Lara.Control.Weapon.RequestGunType != Lara.Control.Weapon.GunType)
			{
				Lara.LeftArm.Locked = true;
				Lara.LeftArm.FrameNumber = 31;
				Lara.LeftArm.AnimNumber = dropAnimNumber;
				Lara.Torch.State = TorchState::Dropping;
			}
			else if (TrInput & IN_DRAW &&
				!LaraItem->Animation.IsAirborne &&
				!LaraItem->Animation.VerticalVelocity &&
				LaraItem->Animation.ActiveState != LS_JUMP_PREPARE &&
				LaraItem->Animation.ActiveState != LS_JUMP_UP &&
				LaraItem->Animation.ActiveState != LS_JUMP_FORWARD &&
				LaraItem->Animation.ActiveState != LS_JUMP_BACK &&
				LaraItem->Animation.ActiveState != LS_JUMP_LEFT &&
				LaraItem->Animation.ActiveState != LS_JUMP_RIGHT ||
				Lara.Control.WaterStatus == WaterStatus::Underwater)
			{
				Lara.LeftArm.Locked = true;
				Lara.LeftArm.FrameNumber = 1;
				Lara.LeftArm.AnimNumber = throwAnimNumber;
				Lara.Torch.State = TorchState::Throwing;

				if (Lara.Control.WaterStatus == WaterStatus::Underwater)
					Lara.Torch.IsLit = false;
			}
		}
		else if (Lara.Torch.State == TorchState::Throwing)
		{
			if (Lara.LeftArm.FrameNumber < 12 && LaraItem->Animation.IsAirborne)
			{
				Lara.LeftArm.Locked = false;
				Lara.LeftArm.FrameNumber = 0;
				Lara.LeftArm.AnimNumber = holdAnimNumber;
				Lara.Torch.State = TorchState::Holding;
			}
			else
			{
				Lara.LeftArm.FrameNumber++;
				if (Lara.LeftArm.FrameNumber == 27)
				{
					Lara.Torch.IsLit = false;
					Lara.Flare.ControlLeft = false;
					Lara.LeftArm.Locked = false;
					Lara.Torch.State = TorchState::Holding;
					Lara.Control.Weapon.GunType = Lara.Control.Weapon.LastGunType;
					Lara.Control.Weapon.RequestGunType = LaraWeaponType::None;
					Lara.Control.HandStatus = HandStatus::Free;
				}
				else if (Lara.LeftArm.FrameNumber == 12)
				{
					Lara.MeshPtrs[LM_LHAND] = Objects[ID_LARA_SKIN].meshIndex + LM_LHAND;
					CreateFlare(LaraItem, ID_BURNING_TORCH_ITEM, true);
				}
			}
		}
		else if (Lara.Torch.State == TorchState::Dropping)
		{
			Lara.LeftArm.FrameNumber++;
			if (Lara.LeftArm.FrameNumber == 41)
			{
				Lara.Torch.IsLit = false;
				Lara.Flare.ControlLeft = false;
				Lara.LeftArm.Locked = false;
				Lara.Torch.State = TorchState::Holding;
				Lara.Control.Weapon.LastGunType = LaraWeaponType::None;
				Lara.Control.Weapon.GunType = LaraWeaponType::None;
				Lara.Control.HandStatus = HandStatus::Free;
			}
			else if (Lara.LeftArm.FrameNumber == 36)
			{
				Lara.MeshPtrs[LM_LHAND] = Objects[ID_LARA_SKIN].meshIndex + LM_LHAND;
				CreateFlare(LaraItem, ID_BURNING_TORCH_ITEM, false);
			}
		}
		else if (Lara.Torch.State == TorchState::JustLit)
		{
			if (LaraItem->Animation.ActiveState != LS_MISC_CONTROL)
			{
				Lara.LeftArm.Locked = false;
				Lara.Torch.State = TorchState::Holding;
				Lara.LeftArm.FrameNumber = 0;
				Lara.Flare.ControlLeft = true;
				Lara.Torch.IsLit = LaraItem->ItemFlags[3] & 1;
				Lara.LeftArm.AnimNumber = holdAnimNumber;
			}
		}

		if (Lara.Flare.ControlLeft)
			Lara.Control.HandStatus = HandStatus::WeaponReady;

		Lara.LeftArm.FrameBase = g_Level.Anims[Lara.LeftArm.AnimNumber].framePtr;

		if (Lara.Torch.IsLit)
		{
			auto pos = Vector3Int(-32, 64, 256);
			GetLaraJointPosition(&pos, LM_LHAND);

			TriggerDynamicLight(pos.x, pos.y, pos.z, 12 - (GetRandomControl() & 1), (GetRandomControl() & 0x3F) + 192, (GetRandomControl() & 0x1F) + 96, 0);

			if (!(Wibble & 7))
				TriggerTorchFlame(LaraItem - g_Level.Items.data(), 0);

			SoundEffect(SFX_TR4_LOOP_FOR_SMALL_FIRES, (PHD_3DPOS*)&pos);
		}
	}

	void GetFlameTorch()
	{
		if (Lara.Control.Weapon.GunType == LaraWeaponType::Flare)
			CreateFlare(LaraItem, ID_FLARE_ITEM, false);

		Lara.Control.HandStatus = HandStatus::WeaponReady;
		Lara.Control.Weapon.RequestGunType = LaraWeaponType::Torch;
		Lara.Control.Weapon.GunType = LaraWeaponType::Torch;
		Lara.Flare.ControlLeft = true;
		Lara.LeftArm.AnimNumber = Objects[ID_LARA_TORCH_ANIM].animIndex;
		Lara.LeftArm.Locked = false;
		Lara.LeftArm.FrameNumber = 0;
		Lara.LeftArm.FrameBase = g_Level.Anims[Lara.LeftArm.AnimNumber].framePtr;

		Lara.MeshPtrs[LM_LHAND] = Objects[ID_LARA_TORCH_ANIM].meshIndex + LM_LHAND;
	}

	void TorchControl(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		if (item->Animation.VerticalVelocity)
		{
			item->Pose.Orientation.x -= ANGLE(5.0f);
			item->Pose.Orientation.z += ANGLE(5.0f);
		}
		else if (!item->Animation.Velocity)
		{
			item->Pose.Orientation.x = 0;
			item->Pose.Orientation.z = 0;
		}

		auto velocity = Vector3Int(
			item->Animation.Velocity * phd_sin(item->Pose.Orientation.y),
			item->Animation.VerticalVelocity,
			item->Animation.Velocity * phd_cos(item->Pose.Orientation.y)
		);

		auto oldPos = item->Pose.Position;
		item->Pose.Position += Vector3Int(velocity.x, 0, velocity.z);

		if (TestEnvironment(ENV_FLAG_WATER, item) ||
			TestEnvironment(ENV_FLAG_SWAMP, item))
		{
			item->Animation.VerticalVelocity += (5 - item->Animation.VerticalVelocity) / 2;
			item->Animation.Velocity += (5 - item->Animation.Velocity) / 2;

			if (item->ItemFlags[3] != 0)
				item->ItemFlags[3] = 0;
		}
		else
			item->Animation.VerticalVelocity += 6;

		item->Pose.Position.y += item->Animation.VerticalVelocity;
		DoProjectileDynamics(itemNumber, oldPos.x, oldPos.y, oldPos.z, velocity.x, velocity.y, velocity.z);

		// Collide with entities.
		if (GetCollidedObjects(item, 0, true, CollidedItems, CollidedMeshes, true))
		{
			LaraCollision.Setup.EnableObjectPush = true;
			if (CollidedItems[0])
			{
				if (!Objects[CollidedItems[0]->ObjectNumber].intelligent &&
					CollidedItems[0]->ObjectNumber != ID_LARA)
				{
					ObjectCollision(CollidedItems[0] - g_Level.Items.data(), item, &LaraCollision);
				}
			}
			else if (CollidedMeshes[0])
			{
				ItemPushStatic(item, CollidedMeshes[0], &LaraCollision);
			}
			
			item->Animation.Velocity = -int(item->Animation.Velocity / 1.5f);
		}

		if (item->ItemFlags[3])
		{
			TriggerDynamicLight(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, 12 - (GetRandomControl() & 1), (GetRandomControl() & 0x3F) + 192, (GetRandomControl() & 0x1F) + 96, 0);
			
			if (!(Wibble & 7))
				TriggerTorchFlame(itemNumber, 1);

			SoundEffect(SFX_TR4_LOOP_FOR_SMALL_FIRES, &item->Pose);
		}
	}

	void LaraTorch(Vector3Int* src, Vector3Int* target, int rot, int color)
	{
		auto pos1 = GameVector(
			src->x,
			src->y,
			src->z,
			LaraItem->RoomNumber
		);

		auto pos2 = GameVector(
			target->x,
			target->y,
			target->z
		);

		TriggerDynamicLight(pos1.x, pos1.y, pos1.z, 12, color, color, color >> 1);

		if (!LOS(&pos1, &pos2))
		{
			int l = sqrt(pow(pos1.x - pos2.x, 2) + pow(pos1.y - pos2.y, 2) + pow(pos1.z - pos2.z, 2)) * CLICK(1);

			if (l + 8 > 31)
				l = 31;

			if (color - l >= 0)
				TriggerDynamicLight(pos2.x, pos2.y, pos2.z, l + 8, color - l, color - l, (color - l) * 2);
		}
	}

	void FireCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
	{
		auto* laraInfo = GetLaraInfo(laraItem);
		auto* torchItem = &g_Level.Items[itemNumber];

		if (!(TrInput & IN_ACTION) ||
			laraItem->Animation.ActiveState != LS_IDLE ||
			laraItem->Animation.AnimNumber != LA_STAND_IDLE ||
			laraItem->Animation.IsAirborne ||
			laraInfo->Control.Weapon.GunType != LaraWeaponType::Torch ||
			laraInfo->Control.HandStatus != HandStatus::WeaponReady ||
			laraInfo->LeftArm.Locked ||
			laraInfo->Torch.IsLit == (torchItem->Status == ITEM_ACTIVE) ||
			torchItem->Timer == -1)
		{
			if (torchItem->ObjectNumber == ID_BURNING_ROOTS)
				ObjectCollision(itemNumber, laraItem, coll);
		}
		else
		{
			short rot = torchItem->Pose.Orientation.y;

			switch (torchItem->ObjectNumber)
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

			torchItem->Pose.Orientation.y = laraItem->Pose.Orientation.y;

			if (TestLaraPosition(&FireBounds, torchItem, laraItem))
			{
				if (torchItem->ObjectNumber == ID_BURNING_ROOTS)
					laraItem->Animation.AnimNumber = LA_TORCH_LIGHT_5;
				else
				{
					int dy = abs(laraItem->Pose.Position.y - torchItem->Pose.Position.y);
					laraItem->ItemFlags[3] = 1;
					laraItem->Animation.AnimNumber = (dy >> 8) + LA_TORCH_LIGHT_1;
				}

				laraItem->Animation.ActiveState = LS_MISC_CONTROL;
				laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;
				laraInfo->Flare.ControlLeft = false;
				laraInfo->LeftArm.Locked = true;
				laraInfo->InteractedItem = itemNumber;
			}

			torchItem->Pose.Orientation.y = rot;
		}
		if (laraItem->Animation.ActiveState == LS_MISC_CONTROL &&
			laraInfo->InteractedItem == itemNumber &&
			torchItem->Status != ITEM_ACTIVE)
		{
			if (laraItem->Animation.AnimNumber >= LA_TORCH_LIGHT_1 &&
				laraItem->Animation.AnimNumber <= LA_TORCH_LIGHT_5)
			{
				if (laraItem->Animation.FrameNumber - g_Level.Anims[laraItem->Animation.AnimNumber].frameBase == 40)
				{
					TestTriggers(torchItem, true, torchItem->Flags & IFLAG_ACTIVATION_MASK);
					torchItem->Flags |= 0x3E00;
					torchItem->ItemFlags[3] = 0;
					torchItem->Status = ITEM_ACTIVE;
					AddActiveItem(itemNumber);
				}
			}
		}
	}
}
