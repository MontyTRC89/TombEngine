#include "framework.h"
#include "lara_two_guns.h"
#include "lara_fire.h"
#include "lara.h"
#include "effects\effects.h"
#include "animation.h"
#include "effects\tomb4fx.h"
#include "level.h"
#include "setup.h"
#include "camera.h"
#include "input.h"
#include "Sound\sound.h"
#include "savegame.h"
#include "Specific\prng.h"
#include "items.h"

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

void AnimatePistols(LARA_WEAPON_TYPE weaponType)
{
	PISTOL_DEF* p = &PistolsTable[static_cast<int>(Lara.gunType)];
	WEAPON_INFO* weapon = &Weapons[weaponType];
	int soundPlayed = false;
	short angleLeft[2], angleRight[2];

	if (LaraItem->meshBits)
	{
		if (SmokeCountL)
		{
			PHD_VECTOR pos;

			switch (static_cast<LARA_WEAPON_TYPE>(SmokeWeapon))
			{
			case LARA_WEAPON_TYPE::WEAPON_PISTOLS:
				pos.x = 4;
				pos.y = 128;
				pos.z = 40;
				break;
			case LARA_WEAPON_TYPE::WEAPON_REVOLVER:
				pos.x = 16;
				pos.y = 160;
				pos.z = 56;
				break;
			case LARA_WEAPON_TYPE::WEAPON_UZI:
				pos.x = 8;
				pos.y = 140;
				pos.z = 48;
				break;
			}

			GetLaraJointPosition(&pos, LM_LHAND);
			TriggerGunSmoke(pos.x, pos.y, pos.z, 0, 0, 0, 0, SmokeWeapon, SmokeCountL);
		}

		if (SmokeCountR)
		{
			PHD_VECTOR pos;

			switch (static_cast<LARA_WEAPON_TYPE>(SmokeWeapon))
			{
			case LARA_WEAPON_TYPE::WEAPON_PISTOLS:
				pos.x = -16;
				pos.y = 128;
				pos.z = 40;
				break;
			case LARA_WEAPON_TYPE::WEAPON_REVOLVER:
				pos.x = -32;
				pos.y = 160;
				pos.z = 56;
				break;
			case LARA_WEAPON_TYPE::WEAPON_UZI:
				pos.x = -16;
				pos.y = 140;
				pos.z = 48;
				break;
			}

			GetLaraJointPosition(&pos, LM_RHAND);
			TriggerGunSmoke(pos.x, pos.y, pos.z, 0, 0, 0, 0, SmokeWeapon, SmokeCountR);
		}
	}

	// Shooting action for right arm.
	short frameRight = Lara.rightArm.frameNumber;  // frame number of DRAW_END?
	if (Lara.rightArm.lock || (TrInput & IN_ACTION && !Lara.target))
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
				if (weaponType != LARA_WEAPON_TYPE::WEAPON_REVOLVER)
				{
					angleRight[0] = Lara.rightArm.yRot + LaraItem->pos.yRot;
					angleRight[1] = Lara.rightArm.xRot;

					if (FireWeapon(weaponType, Lara.target, LaraItem, angleRight) != FireWeaponType::FW_NOAMMO)
					{
						SmokeCountR = 28;
						SmokeWeapon = weaponType;
						TriggerGunShell(1, ID_GUNSHELL, weaponType); // Right Hand

						Lara.rightArm.flash_gun = weapon->flashTime;

						SoundEffect(SFX_TR4_EXPLOSION1, &LaraItem->pos, 0, 0.9f, 0.5f);
						SoundEffect(weapon->sampleNum, &LaraItem->pos, 0);
						soundPlayed = true;

						if (static_cast<LARA_WEAPON_TYPE>(weaponType) == LARA_WEAPON_TYPE::WEAPON_UZI)
							UziRight = true;

						Statistics.Game.AmmoUsed++;
					}
				}

				// go to (3) SHOOT_CONTINUE start frame
				frameRight = p->recoilAnim;
			}
			else if (UziRight)
			{
				SoundEffect(weapon->sampleNum + 1, &LaraItem->pos, 0);
				UziRight = false;
			}
		}
		// at or beyond (3) SHOOT_CONTINUE start frame
		else if (frameRight >= p->recoilAnim)
		{
			if (static_cast<LARA_WEAPON_TYPE>(weaponType) == LARA_WEAPON_TYPE::WEAPON_UZI)
			{
				SoundEffect(weapon->sampleNum, &LaraItem->pos, 0);
				UziRight = true;
			}

			// increment toward (3) SHOOT_CONTINUE end frame (finish recoil before allowing to shoot again)
			frameRight++;

			// at (3) SHOOT_CONTINUE end frame, go to (0) START_SHOOT end frame
			if (frameRight == (p->recoilAnim + weapon->recoilFrame))
				frameRight = p->draw1Anim2;
		}
	}
	// HAS LET GO OF ACTION
	else
	{
		// BUGFIX: rapid-fire no more. -Sezz
		// let (3) SHOOT_CONTINUE finish
		if ((frameRight >= p->recoilAnim) && (frameRight < p->recoilAnim + weapon->recoilFrame))
			frameRight++;
		// at (3) SHOOT_CONTINUE end frame, go to (0) START_SHOOT end frame
		if (frameRight == (p->recoilAnim + weapon->recoilFrame))
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
			SoundEffect(weapon->sampleNum + 1, &LaraItem->pos, 0);
			UziRight = false;
		}
	}
	set_arm_info(&Lara.rightArm, frameRight);

	// Shooting for left arm.
	short frameLeft = Lara.leftArm.frameNumber;
	if (Lara.leftArm.lock || (TrInput & IN_ACTION && !Lara.target))
	{
		if ((frameLeft >= 0) && (frameLeft < p->draw1Anim2))
		{
			frameLeft++;
		}
		else if (frameLeft == p->draw1Anim2)
		{
			if (TrInput & IN_ACTION)
			{
				angleLeft[0] = Lara.leftArm.yRot + LaraItem->pos.yRot;
				angleLeft[1] = Lara.leftArm.xRot;

				if (FireWeapon(weaponType, Lara.target, LaraItem, angleLeft) != FireWeaponType::FW_NOAMMO)
				{
					if (static_cast<LARA_WEAPON_TYPE>(weaponType) == LARA_WEAPON_TYPE::WEAPON_REVOLVER)
					{
						SmokeCountR = 28;
						SmokeWeapon = static_cast<int>(LARA_WEAPON_TYPE::WEAPON_REVOLVER);
						Lara.rightArm.flash_gun = weapon->flashTime;
					}
					else
					{
						SmokeCountL = 28;
						SmokeWeapon = weaponType;
						TriggerGunShell(0, ID_GUNSHELL, weaponType); // left hand
						Lara.leftArm.flash_gun = weapon->flashTime;
					}

					if (!soundPlayed)
					{
						SoundEffect(SFX_TR4_EXPLOSION1, &LaraItem->pos, 0, 0.9f, 0.5f);
						SoundEffect(weapon->sampleNum, &LaraItem->pos, 0);
					}

					if (static_cast<LARA_WEAPON_TYPE>(weaponType) == LARA_WEAPON_TYPE::WEAPON_UZI)
						UziLeft = true;

					Statistics.Game.AmmoUsed++;
				}

				frameLeft = p->recoilAnim;
			}
			else if (UziLeft)
			{
				SoundEffect(weapon->sampleNum + 1, &LaraItem->pos, 0);
				UziLeft = false;
			}
		}
		else if (frameLeft >= p->recoilAnim)
		{
			if (static_cast<LARA_WEAPON_TYPE>(weaponType) == LARA_WEAPON_TYPE::WEAPON_UZI)
			{
				SoundEffect(weapon->sampleNum, &LaraItem->pos, 0);
				UziLeft = true;
			}

			frameLeft++;

			if (frameLeft == (p->recoilAnim + weapon->recoilFrame))
				frameLeft = p->draw1Anim2;
		}
	}
	else       																// Havent GOT a LOCK ON..
	{
		if ((frameLeft >= p->recoilAnim) && (frameLeft < p->recoilAnim + weapon->recoilFrame))
			frameLeft++;
		if (frameLeft == (p->recoilAnim + weapon->recoilFrame))
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
			SoundEffect(weapon->sampleNum + 1, &LaraItem->pos, 0);
			UziLeft = false;
		}
	}
	set_arm_info(&Lara.leftArm, frameLeft);
}

void PistolHandler(LARA_WEAPON_TYPE weaponType)
{
	WEAPON_INFO* weapon = &Weapons[weaponType];

	LaraGetNewTarget(LaraItem, weapon);
	if (TrInput & IN_ACTION)
		LaraTargetInfo(LaraItem, weapon);

	AimWeapon(LaraItem, weapon, &Lara.leftArm);
	AimWeapon(LaraItem, weapon, &Lara.rightArm);

	if (Lara.leftArm.lock && !Lara.rightArm.lock)
	{
		Lara.torsoYrot = Lara.leftArm.yRot / 2;
		Lara.torsoXrot = Lara.leftArm.xRot / 2;

		if (Camera.oldType != CAMERA_TYPE::LOOK_CAMERA)
		{
			Lara.headYrot = Lara.torsoYrot;
			Lara.headXrot = Lara.torsoXrot;
		}
	}
	else if (!Lara.leftArm.lock && Lara.rightArm.lock)
	{
		Lara.torsoYrot = Lara.rightArm.yRot / 2;
		Lara.torsoXrot = Lara.rightArm.xRot / 2;

		if (Camera.oldType != CAMERA_TYPE::LOOK_CAMERA)
		{
			Lara.headYrot = Lara.torsoYrot;
			Lara.headXrot = Lara.torsoXrot;
		}
	}
	else if (Lara.leftArm.lock && Lara.rightArm.lock)
	{
		Lara.torsoYrot = (Lara.leftArm.yRot + Lara.rightArm.yRot) / 4;
		Lara.torsoXrot = (Lara.leftArm.xRot + Lara.rightArm.xRot) / 4;

		if (Camera.oldType != CAMERA_TYPE::LOOK_CAMERA)
		{
			Lara.headYrot = Lara.torsoYrot;
			Lara.headXrot = Lara.torsoXrot;
		}
	}

	AnimatePistols(weaponType);
	
	if (Lara.leftArm.flash_gun || Lara.rightArm.flash_gun)
	{
		PHD_VECTOR pos;

		pos.x = (byte)GetRandomControl() - 128;
		pos.y = (GetRandomControl() & 0x7F) - 63;
		pos.z = (byte)GetRandomControl() - 128;

		GetLaraJointPosition(&pos, Lara.leftArm.flash_gun != 0 ? LM_LHAND : LM_RHAND);
			TriggerDynamicLight(pos.x+GenerateFloat(-128,128), pos.y + GenerateFloat(-128, 128), pos.z + GenerateFloat(-128, 128), GenerateFloat(8,11), (GetRandomControl() & 0x3F) + 192, (GetRandomControl() & 0x1F) + 128, GetRandomControl() & 0x3F);
	}
}

void undraw_pistol_mesh_right(LARA_WEAPON_TYPE weaponType)
{
	Lara.meshPtrs[LM_RHAND] = Objects[ID_LARA_SKIN].meshIndex + LM_RHAND;
	
	Lara.holsterInfo.rightHolster = HolsterSlotForWeapon(static_cast<LARA_WEAPON_TYPE>(weaponType));

}

void undraw_pistol_mesh_left(LARA_WEAPON_TYPE weaponType)
{
	if (static_cast<LARA_WEAPON_TYPE>(weaponType) != LARA_WEAPON_TYPE::WEAPON_REVOLVER)
	{
		Lara.meshPtrs[LM_LHAND] = Objects[ID_LARA_SKIN].meshIndex + LM_LHAND;
		
		Lara.holsterInfo.leftHolster = HolsterSlotForWeapon(static_cast<LARA_WEAPON_TYPE>(weaponType));
	}
}

void draw_pistol_meshes(LARA_WEAPON_TYPE weaponType)
{
	if(static_cast<LARA_WEAPON_TYPE>(weaponType) != LARA_WEAPON_TYPE::WEAPON_REVOLVER){
		Lara.holsterInfo.leftHolster = HOLSTER_SLOT::Empty;
	}
	Lara.holsterInfo.rightHolster = HOLSTER_SLOT::Empty;

	Lara.meshPtrs[LM_RHAND] = Objects[WeaponObjectMesh(LaraItem, weaponType)].meshIndex + LM_RHAND;
	if (static_cast<LARA_WEAPON_TYPE>(weaponType) != LARA_WEAPON_TYPE::WEAPON_REVOLVER)
		Lara.meshPtrs[LM_LHAND] = Objects[WeaponObjectMesh(LaraItem, weaponType)].meshIndex + LM_LHAND;
}

void ready_pistols(LARA_WEAPON_TYPE weaponType)
{
	Lara.gunStatus = LG_READY;
	Lara.leftArm.zRot = 0;
	Lara.leftArm.yRot = 0;
	Lara.leftArm.xRot = 0;
	Lara.rightArm.zRot = 0;
	Lara.rightArm.yRot = 0;
	Lara.rightArm.xRot = 0;
	Lara.rightArm.frameNumber = 0;
	Lara.leftArm.frameNumber = 0;
	Lara.target = nullptr;
	Lara.rightArm.lock = false;
	Lara.leftArm.lock = false;
	Lara.rightArm.frameBase = Objects[WeaponObject(weaponType)].frameBase;
	Lara.leftArm.frameBase = Objects[WeaponObject(weaponType)].frameBase;
}

void undraw_pistols(LARA_WEAPON_TYPE weaponType)
{
	PISTOL_DEF* p = &PistolsTable[static_cast<int>(Lara.gunType)];
	WEAPON_INFO* weapon = &Weapons[weaponType];

	short frameLeft = Lara.leftArm.frameNumber;

	// To go along with the rapid-fire BUGFIX, finish recoil anim before holstering weapon too. -Sezz
	if ((frameLeft >= p->recoilAnim) && (frameLeft < p->recoilAnim + weapon->recoilFrame))
		frameLeft++;
	if (frameLeft == (p->recoilAnim + weapon->recoilFrame))
		frameLeft = p->draw1Anim2;

	// OLD:
	/*if (frameLeft >= p->recoilAnim)
	{
		frameLeft = p->draw1Anim2;
	}*/
	else if (frameLeft > 0 && frameLeft < p->draw1Anim)
	{
		Lara.leftArm.xRot -= Lara.leftArm.xRot / frameLeft;
		Lara.leftArm.yRot -= Lara.leftArm.yRot / frameLeft;
		frameLeft--;
	}
	else if (frameLeft == 0)
	{
		Lara.leftArm.zRot = 0;
		Lara.leftArm.yRot = 0;
		Lara.leftArm.xRot = 0;
		frameLeft = p->recoilAnim - 1;
	}
	else if (frameLeft > p->draw1Anim && (frameLeft < p->recoilAnim))
	{
		frameLeft--;

		if (frameLeft == p->draw2Anim - 1)
		{
			undraw_pistol_mesh_left(weaponType);
			SoundEffect(SFX_TR4_LARA_HOLSTER, &LaraItem->pos, 0);
		}
	}
	set_arm_info(&Lara.leftArm, frameLeft);

	short frameRight = Lara.rightArm.frameNumber;

	if ((frameRight >= p->recoilAnim) && (frameRight < p->recoilAnim + weapon->recoilFrame))
		frameRight++;
	if (frameRight == (p->recoilAnim + weapon->recoilFrame))
		frameRight = p->draw1Anim2;

	// OLD:
	/*if (frameRight >= p->recoilAnim)
	{
		frameRight = p->draw1Anim2;
	}*/
	else if (frameRight > 0 && frameRight < p->draw1Anim)
	{
		Lara.rightArm.xRot -= Lara.rightArm.xRot / frameRight;
		Lara.rightArm.yRot -= Lara.rightArm.yRot / frameRight;
		frameRight--;
	}
	else if (frameRight == 0)
	{
		Lara.rightArm.zRot = 0;
		Lara.rightArm.yRot = 0;
		Lara.rightArm.xRot = 0;
		frameRight = p->recoilAnim - 1;
	}
	else if (frameRight > p->draw1Anim && (frameRight < p->recoilAnim))
	{
		frameRight--;

		if (frameRight == p->draw2Anim - 1)
		{
			undraw_pistol_mesh_right(weaponType);
			SoundEffect(SFX_TR4_LARA_HOLSTER, &LaraItem->pos, 0);
		}
	}
	set_arm_info(&Lara.rightArm, frameRight);

	if (frameLeft == p->draw1Anim && frameRight == p->draw1Anim)
	{
		Lara.gunStatus = LG_NO_ARMS;
		Lara.leftArm.frameNumber = 0;
		Lara.rightArm.frameNumber = 0;
		Lara.target = NULL;
		Lara.rightArm.lock = 0;
		Lara.leftArm.lock = 0;
	}

	if (!(TrInput & IN_LOOK))
	{
		Lara.headYrot = (Lara.leftArm.yRot + Lara.rightArm.yRot) / 4;
		Lara.torsoYrot = (Lara.leftArm.yRot + Lara.rightArm.yRot) / 4;
		Lara.headXrot = (Lara.leftArm.xRot + Lara.rightArm.xRot) / 4;
		Lara.torsoXrot = (Lara.leftArm.xRot + Lara.rightArm.xRot) / 4;
	}
}

void set_arm_info(LARA_ARM* arm, int frame)
{
	PISTOL_DEF* p = &PistolsTable[static_cast<int>(Lara.gunType)];
	short animBase = Objects[p->objectNum].animIndex;
	
	if (frame < p->draw1Anim)
		arm->animNumber = animBase;
	else if (frame < p->draw2Anim)
		arm->animNumber = animBase + 1;
	else if (frame < p->recoilAnim)
		arm->animNumber = animBase + 2;
	else
		arm->animNumber = animBase + 3;

	arm->frameNumber = frame;
	arm->frameBase = g_Level.Anims[arm->animNumber].framePtr;
}

void draw_pistols(LARA_WEAPON_TYPE weaponType)
{
	short frame = Lara.leftArm.frameNumber + 1;
	PISTOL_DEF* p = &PistolsTable[static_cast<int>(Lara.gunType)];

	if (frame < p->draw1Anim || frame > p->recoilAnim - 1)
	{
		frame = p->draw1Anim;
	}
	else if (frame == p->draw2Anim)
	{
		draw_pistol_meshes(weaponType);
		SoundEffect(SFX_TR4_LARA_DRAW, &LaraItem->pos, 0);
	}
	else if (frame == p->recoilAnim - 1)
	{
		ready_pistols(weaponType);
		frame = 0;
	}

	set_arm_info(&Lara.rightArm, frame);
	set_arm_info(&Lara.leftArm, frame);
}

