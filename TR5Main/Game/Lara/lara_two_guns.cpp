#include "framework.h"
#include "Game/Lara/lara_two_guns.h"

#include "Game/animation.h"
#include "Game/camera.h"
#include "Game/effects/effects.h"
#include "Game/effects/tomb4fx.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_fire.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/savegame.h"
#include "Sound/sound.h"
#include "Specific/prng.h"
#include "Specific/level.h"
#include "Specific/setup.h"
#include "Specific/input.h"

using namespace TEN::Math::Random;

struct PISTOL_DEF
{
	short objectNum;
	char draw1Anim2;
	char draw1Anim;
	char draw2Anim;
	char recoilAnim;
};

PISTOL_DEF PistolsTable[4] =
{
	{ ID_LARA, 0, 0, 0, 0 },
	{ ID_PISTOLS_ANIM, 4, 5, 13, 24 },
	{ ID_REVOLVER_ANIM , 7, 8, 15, 29 },
	{ ID_UZI_ANIM, 4, 5, 13, 24 }
};

bool UziLeft;
bool UziRight;

void AnimatePistols(ITEM_INFO* laraItem, LaraWeaponType weaponType)
{
	auto* laraInfo = GetLaraInfo(laraItem);
	auto* p = &PistolsTable[(int)laraInfo->Control.WeaponControl.GunType];
	auto* weapon = &Weapons[(int)weaponType];

	int soundPlayed = false;
	short angleLeft[2], angleRight[2];

	if (laraItem->MeshBits)
	{
		if (SmokeCountL)
		{
			PHD_VECTOR pos;

			switch ((LaraWeaponType)SmokeWeapon)
			{
			case LaraWeaponType::WEAPON_PISTOLS:
				pos = { 4, 128, 40 };
				break;

			case LaraWeaponType::WEAPON_REVOLVER:
				pos = { 16, 160, 56 };
				break;

			case LaraWeaponType::WEAPON_UZI:
				pos = { 8, 140, 48 };
				break;
			}

			GetLaraJointPosition(&pos, LM_LHAND);
			TriggerGunSmoke(pos.x, pos.y, pos.z, 0, 0, 0, 0, SmokeWeapon, SmokeCountL);
		}

		if (SmokeCountR)
		{
			PHD_VECTOR pos;

			switch ((LaraWeaponType)SmokeWeapon)
			{
			case LaraWeaponType::WEAPON_PISTOLS:
				pos = { -16, 128, 40 };
				break;

			case LaraWeaponType::WEAPON_REVOLVER:
				pos = { -32, 160, 56 };
				break;

			case LaraWeaponType::WEAPON_UZI:
				pos = { -16, 140, 48 };
				break;
			}

			GetLaraJointPosition(&pos, LM_RHAND);
			TriggerGunSmoke(pos.x, pos.y, pos.z, 0, 0, 0, 0, SmokeWeapon, SmokeCountR);
		}
	}

	// Shooting action for right arm.
	int frameRight = laraInfo->RightArm.FrameNumber;  // frame number of DRAW_END?
	if (laraInfo->RightArm.Locked || (TrInput & IN_ACTION && !laraInfo->target))
	{
		// POINT ARMS FORWARD
		// at or beyond (2) DRAW_END start frame AND before (0) SHOOT_START end frame...
		if ((frameRight >= 0) && (frameRight < p->draw1Anim2))
		{
			// ...increment toward (0) SHOOT_START end frame
			frameRight++;
		}
		// at (0) SHOOT_START end frame
		else if (frameRight == p->draw1Anim2)
		{
			// actually shoot, bang bang
			if (TrInput & IN_ACTION)
			{
				if (weaponType != LaraWeaponType::WEAPON_REVOLVER)
				{
					angleRight[0] = laraInfo->RightArm.Rotation.yRot + laraItem->Position.yRot;
					angleRight[1] = laraInfo->RightArm.Rotation.xRot;

					if (FireWeapon(weaponType, laraInfo->target, laraItem, angleRight) != FireWeaponType::NoAmmo)
					{
						SmokeCountR = 28;
						SmokeWeapon = (int)weaponType;
						TriggerGunShell(1, ID_GUNSHELL, weaponType); // Right Hand

						laraInfo->RightArm.FlashGun = weapon->FlashTime;

						SoundEffect(SFX_TR4_EXPLOSION1, &laraItem->Position, 0, 0.9f, 0.5f);
						SoundEffect(weapon->SampleNum, &laraItem->Position, 0);
						soundPlayed = true;

						if (weaponType == LaraWeaponType::WEAPON_UZI)
							UziRight = true;

						Statistics.Game.AmmoUsed++;
					}
				}

				// go to (3) SHOOT_CONTINUE start frame
				frameRight = p->recoilAnim;
			}
			else if (UziRight)
			{
				SoundEffect(weapon->SampleNum + 1, &laraItem->Position, 0);
				UziRight = false;
			}
		}
		// at or beyond (3) SHOOT_CONTINUE start frame
		else if (frameRight >= p->recoilAnim)
		{
			if (weaponType == LaraWeaponType::WEAPON_UZI)
			{
				SoundEffect(weapon->SampleNum, &laraItem->Position, 0);
				UziRight = true;
			}

			// increment toward (3) SHOOT_CONTINUE end frame (finish recoil before allowing to shoot again)
			frameRight++;

			// at (3) SHOOT_CONTINUE end frame, go to (0) START_SHOOT end frame
			if (frameRight == (p->recoilAnim + weapon->RecoilFrame))
				frameRight = p->draw1Anim2;
		}
	}
	// HAS LET GO OF ACTION
	else
	{
		// BUGFIX: rapid-fire no more. -Sezz
		// let (3) SHOOT_CONTINUE finish
		if ((frameRight >= p->recoilAnim) && (frameRight < p->recoilAnim + weapon->RecoilFrame))
			frameRight++;
		// at (3) SHOOT_CONTINUE end frame, go to (0) START_SHOOT end frame
		if (frameRight == (p->recoilAnim + weapon->RecoilFrame))
			frameRight = p->draw1Anim2;
		// go back to ready stance
		else if ((frameRight > 0) && (frameRight <= p->draw1Anim2))
			frameRight--;

		// OLD:
		//if (frameRight >= p->recoilAnim)
		//	frameRight = p->draw1Anim2;
		//else if ((frameRight > 0) && (frameRight <= p->draw1Anim2))
		//	frameRight--;

		if (UziRight)
		{
			SoundEffect(weapon->SampleNum + 1, &laraItem->Position, 0);
			UziRight = false;
		}
	}

	SetArmInfo(laraItem, &laraInfo->RightArm, frameRight);

	// Shooting for left arm.
	int frameLeft = laraInfo->LeftArm.FrameNumber;
	if (laraInfo->LeftArm.Locked || (TrInput & IN_ACTION && !laraInfo->target))
	{
		if ((frameLeft >= 0) && (frameLeft < p->draw1Anim2))
			frameLeft++;
		else if (frameLeft == p->draw1Anim2)
		{
			if (TrInput & IN_ACTION)
			{
				angleLeft[0] = laraInfo->LeftArm.Rotation.yRot + laraItem->Position.yRot;
				angleLeft[1] = laraInfo->LeftArm.Rotation.xRot;

				if (FireWeapon(weaponType, laraInfo->target, laraItem, angleLeft) != FireWeaponType::NoAmmo)
				{
					if (weaponType == LaraWeaponType::WEAPON_REVOLVER)
					{
						SmokeCountR = 28;
						SmokeWeapon = (int)LaraWeaponType::WEAPON_REVOLVER;
						laraInfo->RightArm.FlashGun = weapon->FlashTime;
					}
					else
					{
						SmokeCountL = 28;
						SmokeWeapon = (int)weaponType;
						TriggerGunShell(0, ID_GUNSHELL, weaponType); // left hand
						laraInfo->LeftArm.FlashGun = weapon->FlashTime;
					}

					if (!soundPlayed)
					{
						SoundEffect(SFX_TR4_EXPLOSION1, &laraItem->Position, 0, 0.9f, 0.5f);
						SoundEffect(weapon->SampleNum, &laraItem->Position, 0);
					}

					if (weaponType == LaraWeaponType::WEAPON_UZI)
						UziLeft = true;

					Statistics.Game.AmmoUsed++;
				}

				frameLeft = p->recoilAnim;
			}
			else if (UziLeft)
			{
				SoundEffect(weapon->SampleNum + 1, &laraItem->Position, 0);
				UziLeft = false;
			}
		}
		else if (frameLeft >= p->recoilAnim)
		{
			if (weaponType == LaraWeaponType::WEAPON_UZI)
			{
				SoundEffect(weapon->SampleNum, &laraItem->Position, 0);
				UziLeft = true;
			}

			frameLeft++;

			if (frameLeft == (p->recoilAnim + weapon->RecoilFrame))
				frameLeft = p->draw1Anim2;
		}
	}
	else       																// Havent GOT a LOCK ON..
	{
		if ((frameLeft >= p->recoilAnim) && (frameLeft < p->recoilAnim + weapon->RecoilFrame))
			frameLeft++;
		if (frameLeft == (p->recoilAnim + weapon->RecoilFrame))
			frameLeft = p->draw1Anim2;
		else if ((frameLeft > 0) && (frameLeft <= p->draw1Anim2))
			frameLeft--;

		// OLD:
		//if (frameLeft >= p->recoilAnim) 									// If Gun is Recoiling Stop it now...
		//	frameLeft = p->draw1Anim2;
		//else if (frameLeft > 0 && frameLeft <= p->draw1Anim2)
		//	frameLeft--;													// UnLock ARM

		if (UziLeft)
		{
			SoundEffect(weapon->SampleNum + 1, &laraItem->Position, 0);
			UziLeft = false;
		}
	}

	SetArmInfo(laraItem, &laraInfo->LeftArm, frameLeft);
}

void PistolHandler(ITEM_INFO* laraItem, LaraWeaponType weaponType)
{
	auto* laraInfo = GetLaraInfo(laraItem);
	auto* weapon = &Weapons[weaponType];

	LaraGetNewTarget(laraItem, weapon);
	if (TrInput & IN_ACTION)
		LaraTargetInfo(laraItem, weapon);

	AimWeapon(laraItem, weapon, &laraInfo->LeftArm);
	AimWeapon(laraItem, weapon, &laraInfo->RightArm);

	if (laraInfo->LeftArm.Locked && !laraInfo->RightArm.Locked)
	{
		laraInfo->ExtraTorsoRot.xRot = laraInfo->LeftArm.Rotation.xRot / 2;
		laraInfo->ExtraTorsoRot.yRot = laraInfo->LeftArm.Rotation.yRot / 2;

		if (Camera.oldType != CameraType::Look)
			laraInfo->ExtraHeadRot = laraInfo->ExtraTorsoRot;
	}
	else if (!laraInfo->LeftArm.Locked && laraInfo->RightArm.Locked)
	{
		laraInfo->ExtraTorsoRot.xRot = laraInfo->RightArm.Rotation.xRot / 2;
		laraInfo->ExtraTorsoRot.yRot = laraInfo->RightArm.Rotation.yRot / 2;

		if (Camera.oldType != CameraType::Look)
			laraInfo->ExtraHeadRot = laraInfo->ExtraTorsoRot;
	}
	else if (laraInfo->LeftArm.Locked && laraInfo->RightArm.Locked)
	{
		laraInfo->ExtraTorsoRot.xRot = (laraInfo->LeftArm.Rotation.xRot + laraInfo->RightArm.Rotation.xRot) / 4;
		laraInfo->ExtraTorsoRot.yRot = (laraInfo->LeftArm.Rotation.yRot + laraInfo->RightArm.Rotation.yRot) / 4;

		if (Camera.oldType != CameraType::Look)
			laraInfo->ExtraHeadRot = laraInfo->ExtraTorsoRot;
	}

	AnimatePistols(laraItem, weaponType);
	
	if (laraInfo->LeftArm.FlashGun || laraInfo->RightArm.FlashGun)
	{
		PHD_VECTOR pos;
		pos.x = (byte)GetRandomControl() - 128;
		pos.y = (GetRandomControl() & 0x7F) - 63;
		pos.z = (byte)GetRandomControl() - 128;
		GetLaraJointPosition(&pos, laraInfo->LeftArm.FlashGun != 0 ? LM_LHAND : LM_RHAND);

		TriggerDynamicLight(pos.x+GenerateFloat(-128,128), pos.y + GenerateFloat(-128, 128), pos.z + GenerateFloat(-128, 128), GenerateFloat(8,11), (GetRandomControl() & 0x3F) + 192, (GetRandomControl() & 0x1F) + 128, GetRandomControl() & 0x3F);
	}
}

void ReadyPistols(ITEM_INFO* laraItem, LaraWeaponType weaponType)
{
	auto* laraInfo = GetLaraInfo(laraItem);

	laraInfo->Control.HandStatus = HandStatus::WeaponReady;
	laraInfo->LeftArm.Rotation = PHD_3DPOS();
	laraInfo->RightArm.Rotation = PHD_3DPOS();
	laraInfo->LeftArm.FrameNumber = 0;
	laraInfo->RightArm.FrameNumber = 0;
	laraInfo->target = nullptr;
	laraInfo->LeftArm.Locked = false;
	laraInfo->RightArm.Locked = false;
	laraInfo->LeftArm.FrameBase = Objects[WeaponObject(weaponType)].frameBase;
	laraInfo->RightArm.FrameBase = Objects[WeaponObject(weaponType)].frameBase;
}

void DrawPistols(ITEM_INFO* laraItem, LaraWeaponType weaponType)
{
	auto* laraInfo = GetLaraInfo(laraItem);
	auto* p = &PistolsTable[(int)laraInfo->Control.WeaponControl.GunType];
	int frame = laraInfo->LeftArm.FrameNumber + 1;

	if (frame < p->draw1Anim || frame > p->recoilAnim - 1)
		frame = p->draw1Anim;
	else if (frame == p->draw2Anim)
	{
		DrawPistolMeshes(laraItem, weaponType);
		SoundEffect(SFX_TR4_LARA_DRAW, &laraItem->Position, 0);
	}
	else if (frame == p->recoilAnim - 1)
	{
		ReadyPistols(laraItem, weaponType);
		frame = 0;
	}

	SetArmInfo(laraItem, &laraInfo->RightArm, frame);
	SetArmInfo(laraItem, &laraInfo->LeftArm, frame);
}

void UndrawPistols(ITEM_INFO* laraItem, LaraWeaponType weaponType)
{
	auto* laraInfo = GetLaraInfo(laraItem);
	auto* p = &PistolsTable[(int)laraInfo->Control.WeaponControl.GunType];
	auto* weapon = &Weapons[(int)weaponType];

	int frameLeft = laraInfo->LeftArm.FrameNumber;

	// Finish recoil anim before reholstering weapon.
	if ((frameLeft >= p->recoilAnim) && (frameLeft < p->recoilAnim + weapon->RecoilFrame))
		frameLeft++;
	if (frameLeft == (p->recoilAnim + weapon->RecoilFrame))
		frameLeft = p->draw1Anim2;

	// OLD:
	/*if (frameLeft >= p->recoilAnim)
	{
		frameLeft = p->draw1Anim2;
	}*/
	else if (frameLeft > 0 && frameLeft < p->draw1Anim)
	{
		laraInfo->LeftArm.Rotation.xRot -= laraInfo->LeftArm.Rotation.xRot / frameLeft;
		laraInfo->LeftArm.Rotation.yRot -= laraInfo->LeftArm.Rotation.yRot / frameLeft;
		frameLeft--;
	}
	else if (frameLeft == 0)
	{
		laraInfo->LeftArm.Rotation.yRot = 0;
		laraInfo->LeftArm.Rotation.xRot = 0;
		laraInfo->LeftArm.Rotation.zRot = 0;
		frameLeft = p->recoilAnim - 1;
	}
	else if (frameLeft > p->draw1Anim && (frameLeft < p->recoilAnim))
	{
		frameLeft--;

		if (frameLeft == p->draw2Anim - 1)
		{
			UndrawPistolMeshLeft(laraItem, weaponType);
			SoundEffect(SFX_TR4_LARA_HOLSTER, &laraItem->Position, 0);
		}
	}

	SetArmInfo(laraItem, &laraInfo->LeftArm, frameLeft);

	int frameRight = laraInfo->RightArm.FrameNumber;

	if ((frameRight >= p->recoilAnim) && (frameRight < p->recoilAnim + weapon->RecoilFrame))
		frameRight++;
	if (frameRight == (p->recoilAnim + weapon->RecoilFrame))
		frameRight = p->draw1Anim2;

	// OLD:
	/*if (frameRight >= p->recoilAnim)
	{
		frameRight = p->draw1Anim2;
	}*/
	else if (frameRight > 0 && frameRight < p->draw1Anim)
	{
		laraInfo->RightArm.Rotation.xRot -= laraInfo->RightArm.Rotation.xRot / frameRight;
		laraInfo->RightArm.Rotation.yRot -= laraInfo->RightArm.Rotation.yRot / frameRight;
		frameRight--;
	}
	else if (frameRight == 0)
	{
		laraInfo->RightArm.Rotation.yRot = 0;
		laraInfo->RightArm.Rotation.xRot = 0;
		laraInfo->RightArm.Rotation.zRot = 0;
		frameRight = p->recoilAnim - 1;
	}
	else if (frameRight > p->draw1Anim && (frameRight < p->recoilAnim))
	{
		frameRight--;

		if (frameRight == p->draw2Anim - 1)
		{
			UndrawPistolMeshRight(laraItem, weaponType);
			SoundEffect(SFX_TR4_LARA_HOLSTER, &laraItem->Position, 0);
		}
	}

	SetArmInfo(laraItem, &laraInfo->RightArm, frameRight);

	if (frameLeft == p->draw1Anim && frameRight == p->draw1Anim)
	{
		laraInfo->Control.HandStatus = HandStatus::Free;
		laraInfo->LeftArm.FrameNumber = 0;
		laraInfo->RightArm.FrameNumber = 0;
		laraInfo->target = NULL;
		laraInfo->RightArm.Locked = false;
		laraInfo->LeftArm.Locked = false;
	}

	if (!(TrInput & IN_LOOK))
	{
		laraInfo->ExtraHeadRot.xRot = (laraInfo->LeftArm.Rotation.xRot + laraInfo->RightArm.Rotation.xRot) / 4;
		laraInfo->ExtraTorsoRot.xRot = (laraInfo->LeftArm.Rotation.xRot + laraInfo->RightArm.Rotation.xRot) / 4;
		laraInfo->ExtraHeadRot.yRot = (laraInfo->LeftArm.Rotation.yRot + laraInfo->RightArm.Rotation.yRot) / 4;
		laraInfo->ExtraTorsoRot.yRot = (laraInfo->LeftArm.Rotation.yRot + laraInfo->RightArm.Rotation.yRot) / 4;
	}
}

void SetArmInfo(ITEM_INFO* laraItem, ArmInfo* arm, int frame)
{
	auto* laraInfo = GetLaraInfo(laraItem);
	auto* p = &PistolsTable[(int)laraInfo->Control.WeaponControl.GunType];
	int animBase = Objects[(int)p->objectNum].animIndex;

	if (frame < p->draw1Anim)
		arm->AnimNumber = animBase;
	else if (frame < p->draw2Anim)
		arm->AnimNumber = animBase + 1;
	else if (frame < p->recoilAnim)
		arm->AnimNumber = animBase + 2;
	else
		arm->AnimNumber = animBase + 3;

	arm->FrameNumber = frame;
	arm->FrameBase = g_Level.Anims[arm->AnimNumber].framePtr;
}

void DrawPistolMeshes(ITEM_INFO* laraItem, LaraWeaponType weaponType)
{
	auto* laraInfo = GetLaraInfo(laraItem);

	if (weaponType != LaraWeaponType::WEAPON_REVOLVER)
		laraInfo->Control.WeaponControl.HolsterInfo.LeftHolster = HolsterSlot::Empty;

	laraInfo->Control.WeaponControl.HolsterInfo.RightHolster = HolsterSlot::Empty;

	laraInfo->meshPtrs[LM_RHAND] = Objects[WeaponObjectMesh(laraItem, weaponType)].meshIndex + LM_RHAND;
	if (weaponType != LaraWeaponType::WEAPON_REVOLVER)
		laraInfo->meshPtrs[LM_LHAND] = Objects[WeaponObjectMesh(laraItem, weaponType)].meshIndex + LM_LHAND;
}

void UndrawPistolMeshRight(ITEM_INFO* laraItem, LaraWeaponType weaponType)
{
	auto* laraInfo = GetLaraInfo(laraItem);

	laraInfo->meshPtrs[LM_RHAND] = Objects[ID_LARA_SKIN].meshIndex + LM_RHAND;
	laraInfo->Control.WeaponControl.HolsterInfo.RightHolster = HolsterSlotForWeapon(weaponType);
}

void UndrawPistolMeshLeft(ITEM_INFO* laraItem, LaraWeaponType weaponType)
{
	auto* laraInfo = GetLaraInfo(laraItem);

	if (weaponType != LaraWeaponType::WEAPON_REVOLVER)
	{
		laraInfo->meshPtrs[LM_LHAND] = Objects[ID_LARA_SKIN].meshIndex + LM_LHAND;
		laraInfo->Control.WeaponControl.HolsterInfo.LeftHolster = HolsterSlotForWeapon(weaponType);
	}
}
