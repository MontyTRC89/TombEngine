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
	void TriggerTorchFlame(char fxObject, char node)
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
		spark->fxObj = fxObject;
		spark->scalar = 1;
		spark->sSize = spark->size = (GetRandomControl() & 0x1F) + 80;
		spark->dSize = spark->size / 8;
	}

	void DoFlameTorch()
	{
		const int holdAnimNumber = Objects[ID_LARA_TORCH_ANIM].animIndex;
		const int throwAnimNumber = Objects[ID_LARA_TORCH_ANIM].animIndex + 1;
		const int dropAnimNumber = Objects[ID_LARA_TORCH_ANIM].animIndex + 2;

		auto* laraItem = LaraItem;
		auto* lara = GetLaraInfo(laraItem);

		if (lara->Torch.State == TorchState::Holding)
		{
			if (lara->Control.Weapon.RequestGunType != lara->Control.Weapon.GunType)
			{
				lara->LeftArm.Locked = true;
				lara->LeftArm.FrameNumber = 31;
				lara->LeftArm.AnimNumber = dropAnimNumber;
				lara->Torch.State = TorchState::Dropping;
			}
			else if (TrInput & IN_DRAW &&
				!laraItem->Animation.IsAirborne &&
				laraItem->Animation.Velocity.y == 0.0f &&
				laraItem->Animation.ActiveState != LS_JUMP_PREPARE &&
				laraItem->Animation.ActiveState != LS_JUMP_UP &&
				laraItem->Animation.ActiveState != LS_JUMP_FORWARD &&
				laraItem->Animation.ActiveState != LS_JUMP_BACK &&
				laraItem->Animation.ActiveState != LS_JUMP_LEFT &&
				laraItem->Animation.ActiveState != LS_JUMP_RIGHT ||
				lara->Control.WaterStatus == WaterStatus::Underwater)
			{
				lara->LeftArm.Locked = true;
				lara->LeftArm.FrameNumber = 1;
				lara->LeftArm.AnimNumber = throwAnimNumber;
				lara->Torch.State = TorchState::Throwing;

				if (lara->Control.WaterStatus == WaterStatus::Underwater)
					lara->Torch.IsLit = false;
			}
		}
		else if (lara->Torch.State == TorchState::Throwing)
		{
			if (lara->LeftArm.FrameNumber < 12 && laraItem->Animation.IsAirborne)
			{
				lara->LeftArm.Locked = false;
				lara->LeftArm.FrameNumber = 0;
				lara->LeftArm.AnimNumber = holdAnimNumber;
				lara->Torch.State = TorchState::Holding;
			}
			else
			{
				lara->LeftArm.FrameNumber++;
				if (lara->LeftArm.FrameNumber == 27)
				{
					lara->Torch.IsLit = false;
					lara->Flare.ControlLeft = false;
					lara->LeftArm.Locked = false;
					lara->Torch.State = TorchState::Holding;
					lara->Control.Weapon.GunType = lara->Control.Weapon.LastGunType;
					lara->Control.Weapon.RequestGunType = LaraWeaponType::None;
					lara->Control.HandStatus = HandStatus::Free;
				}
				else if (lara->LeftArm.FrameNumber == 12)
				{
					lara->MeshPtrs[LM_LHAND] = Objects[ID_LARA_SKIN].meshIndex + LM_LHAND;
					CreateFlare(laraItem, ID_BURNING_TORCH_ITEM, true);
				}
			}
		}
		else if (lara->Torch.State == TorchState::Dropping)
		{
			lara->LeftArm.FrameNumber++;
			if (lara->LeftArm.FrameNumber == 41)
			{
				lara->Torch.IsLit = false;
				lara->Flare.ControlLeft = false;
				lara->LeftArm.Locked = false;
				lara->Torch.State = TorchState::Holding;
				lara->Control.Weapon.LastGunType = LaraWeaponType::None;
				lara->Control.Weapon.GunType = LaraWeaponType::None;
				lara->Control.HandStatus = HandStatus::Free;
			}
			else if (lara->LeftArm.FrameNumber == 36)
			{
				lara->MeshPtrs[LM_LHAND] = Objects[ID_LARA_SKIN].meshIndex + LM_LHAND;
				CreateFlare(laraItem, ID_BURNING_TORCH_ITEM, false);
			}
		}
		else if (lara->Torch.State == TorchState::JustLit)
		{
			if (laraItem->Animation.ActiveState != LS_MISC_CONTROL)
			{
				lara->LeftArm.Locked = false;
				lara->Torch.State = TorchState::Holding;
				lara->LeftArm.FrameNumber = 0;
				lara->Flare.ControlLeft = true;
				lara->Torch.IsLit = laraItem->ItemFlags[3] & 1;
				lara->LeftArm.AnimNumber = holdAnimNumber;
			}
		}

		if (lara->Flare.ControlLeft)
			lara->Control.HandStatus = HandStatus::WeaponReady;

		lara->LeftArm.FrameBase = g_Level.Anims[lara->LeftArm.AnimNumber].FramePtr;

		if (lara->Torch.IsLit)
		{
			auto pos = GetJointPosition(laraItem, LM_LHAND, Vector3i(-32, 64, 256));
			TriggerDynamicLight(pos.x, pos.y, pos.z, 12 - (GetRandomControl() & 1), (GetRandomControl() & 0x3F) + 192, (GetRandomControl() & 0x1F) + 96, 0);

			if (!(Wibble & 7))
				TriggerTorchFlame(laraItem - g_Level.Items.data(), 0);

			SoundEffect(SFX_TR4_LOOP_FOR_SMALL_FIRES, (Pose*)&pos);
		}
	}

	void GetFlameTorch()
	{
		auto* laraItem = LaraItem;
		auto* lara = GetLaraInfo(laraItem);

		if (lara->Control.Weapon.GunType == LaraWeaponType::Flare)
			CreateFlare(laraItem, ID_FLARE_ITEM, false);

		lara->Control.HandStatus = HandStatus::WeaponReady;
		lara->Control.Weapon.RequestGunType = LaraWeaponType::Torch;
		lara->Control.Weapon.GunType = LaraWeaponType::Torch;
		lara->Flare.ControlLeft = true;
		lara->LeftArm.AnimNumber = Objects[ID_LARA_TORCH_ANIM].animIndex;
		lara->LeftArm.Locked = false;
		lara->LeftArm.FrameNumber = 0;
		lara->LeftArm.FrameBase = g_Level.Anims[lara->LeftArm.AnimNumber].FramePtr;

		lara->MeshPtrs[LM_LHAND] = Objects[ID_LARA_TORCH_ANIM].meshIndex + LM_LHAND;
	}

	void TorchControl(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		if (item->Animation.Velocity.y)
		{
			item->Pose.Orientation.x -= ANGLE(5.0f);
			item->Pose.Orientation.z += ANGLE(5.0f);
		}
		else if (!item->Animation.Velocity.z)
		{
			item->Pose.Orientation.x = 0;
			item->Pose.Orientation.z = 0;
		}

		auto velocity = Vector3i(
			item->Animation.Velocity.z * phd_sin(item->Pose.Orientation.y),
			item->Animation.Velocity.y,
			item->Animation.Velocity.z * phd_cos(item->Pose.Orientation.y)
		);

		auto prevPos = item->Pose.Position;
		item->Pose.Position += Vector3i(velocity.x, 0, velocity.z);

		if (TestEnvironment(ENV_FLAG_WATER, item) ||
			TestEnvironment(ENV_FLAG_SWAMP, item))
		{
			item->Animation.Velocity.y += (5 - item->Animation.Velocity.y) / 2;
			item->Animation.Velocity.z += (5 - item->Animation.Velocity.z) / 2;

			if (item->ItemFlags[3] != 0)
				item->ItemFlags[3] = 0;
		}
		else
			item->Animation.Velocity.y += 6;

		item->Pose.Position.y += item->Animation.Velocity.y;
		DoProjectileDynamics(itemNumber, prevPos.x, prevPos.y, prevPos.z, velocity.x, velocity.y, velocity.z);

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
				ItemPushStatic(item, *CollidedMeshes[0], &LaraCollision);
			
			item->Animation.Velocity.z = -int(item->Animation.Velocity.z / 1.5f);
		}

		if (item->ItemFlags[3])
		{
			TriggerDynamicLight(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, 12 - (GetRandomControl() & 1), (GetRandomControl() & 0x3F) + 192, (GetRandomControl() & 0x1F) + 96, 0);
			
			if (!(Wibble & 7))
				TriggerTorchFlame(itemNumber, 1);

			SoundEffect(SFX_TR4_LOOP_FOR_SMALL_FIRES, &item->Pose);
		}
	}

	void LaraTorch(Vector3i* origin, Vector3i* target, int rot, int color)
	{
		auto pos1 = GameVector(*origin, LaraItem->RoomNumber);
		auto pos2 = GameVector(*target);

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
		auto* torchItem = &g_Level.Items[itemNumber];
		auto* lara = GetLaraInfo(laraItem);

		if (!(TrInput & IN_ACTION) ||
			laraItem->Animation.ActiveState != LS_IDLE ||
			laraItem->Animation.AnimNumber != LA_STAND_IDLE ||
			laraItem->Animation.IsAirborne ||
			lara->Control.Weapon.GunType != LaraWeaponType::Torch ||
			lara->Control.HandStatus != HandStatus::WeaponReady ||
			lara->LeftArm.Locked ||
			lara->Torch.IsLit == (torchItem->Status == ITEM_ACTIVE) ||
			torchItem->Timer == -1)
		{
			if (torchItem->ObjectNumber == ID_BURNING_ROOTS)
				ObjectCollision(itemNumber, laraItem, coll);
		}
		else
		{
			short yOrient = torchItem->Pose.Orientation.y;

			switch (torchItem->ObjectNumber)
			{
			case ID_FLAME_EMITTER:
				FireBounds.BoundingBox.X1 = -256;
				FireBounds.BoundingBox.X2 = 256;
				FireBounds.BoundingBox.Y1 = 0;
				FireBounds.BoundingBox.Y2 = 1024;
				FireBounds.BoundingBox.Z1 = -800;
				FireBounds.BoundingBox.Z2 = 800;
				break;

			case ID_FLAME_EMITTER2:
				FireBounds.BoundingBox.X1 = -256;
				FireBounds.BoundingBox.X2 = 256;
				FireBounds.BoundingBox.Y1 = 0;
				FireBounds.BoundingBox.Y2 = 1024;
				FireBounds.BoundingBox.Z1 = -600;
				FireBounds.BoundingBox.Z2 = 600;
				break;

			case ID_BURNING_ROOTS:
				FireBounds.BoundingBox.X1 = -384;
				FireBounds.BoundingBox.X2 = 384;
				FireBounds.BoundingBox.Y1 = 0;
				FireBounds.BoundingBox.Y2 = 2048;
				FireBounds.BoundingBox.Z1 = -384;
				FireBounds.BoundingBox.Z2 = 384;
				break;
			}

			torchItem->Pose.Orientation.y = laraItem->Pose.Orientation.y;

			if (TestPlayerEntityInteract(torchItem, laraItem, FireBounds))
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
				lara->Flare.ControlLeft = false;
				lara->LeftArm.Locked = true;
				lara->InteractedItem = itemNumber;
			}

			torchItem->Pose.Orientation.y = yOrient;
		}
		if (laraItem->Animation.ActiveState == LS_MISC_CONTROL &&
			lara->InteractedItem == itemNumber &&
			torchItem->Status != ITEM_ACTIVE)
		{
			if (laraItem->Animation.AnimNumber >= LA_TORCH_LIGHT_1 &&
				laraItem->Animation.AnimNumber <= LA_TORCH_LIGHT_5)
			{
				if ((laraItem->Animation.FrameNumber - g_Level.Anims[laraItem->Animation.AnimNumber].frameBase) == 40)
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
