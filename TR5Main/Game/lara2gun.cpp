#include "lara2gun.h"
#include "..\Global\global.h"
#include "larafire.h"
#include "lara.h"
#include "effect2.h"
#include "draw.h"
#include "tomb4fx.h"
#include "..\Specific\level.h"
#include "..\Specific\setup.h"
#include "camera.h"

PISTOL_DEF PistolsTable[4] =
{
	{ ID_LARA, 0, 0, 0, 0 },
	{ ID_PISTOLS_ANIM, 4, 5, 13, 24 },
	{ ID_REVOLVER_ANIM , 7, 8, 15, 29 },
	{ ID_UZI_ANIM, 4, 5, 13, 24 }
};

bool UziLeft;
bool UziRight;

void AnimatePistols(int weaponType)
{
	PISTOL_DEF* p = &PistolsTable[Lara.gunType];
	WEAPON_INFO* weapon = &Weapons[weaponType];
	int soundPlayed = false;
	short angleLeft[2], angleRight[2];

	if (LaraItem->meshBits)
	{
		if (SmokeCountL)
		{
			PHD_VECTOR pos;

			switch (SmokeWeapon)
			{
			case WEAPON_PISTOLS:
				pos.x = 4;
				pos.y = 128;
				pos.z = 40;
				break;
			case WEAPON_REVOLVER:
				pos.x = 16;
				pos.y = 160;
				pos.z = 56;
				break;
			case WEAPON_UZI:
				pos.x = 8;
				pos.y = 140;
				pos.z = 48;
				break;
			}

			GetLaraJointPosition(&pos, LJ_LHAND);
			TriggerGunSmoke(pos.x, pos.y, pos.z, 0, 0, 0, (byte)0, SmokeWeapon, SmokeCountL);
		}

		if (SmokeCountR)
		{
			PHD_VECTOR pos;

			switch (SmokeWeapon)
			{
			case WEAPON_PISTOLS:
				pos.x = -16;
				pos.y = 128;
				pos.z = 40;
				break;
			case WEAPON_REVOLVER:
				pos.x = -32;
				pos.y = 160;
				pos.z = 56;
				break;
			case WEAPON_UZI:
				pos.x = -16;
				pos.y = 140;
				pos.z = 48;
				break;
			}

			GetLaraJointPosition(&pos, LJ_RHAND);
			TriggerGunSmoke(pos.x, pos.y, pos.z, 0, 0, 0, (byte)0, SmokeWeapon, SmokeCountR);
		}
	}

	short frameRight = Lara.rightArm.frameNumber;
	if (Lara.rightArm.lock || (TrInput & IN_ACTION && !Lara.target))
	{
		if ((frameRight >= 0) && (frameRight < p->draw1Anim2))
		{
			frameRight++;
		}
		else if (frameRight == p->draw1Anim2)
		{
			if (TrInput & IN_ACTION)
			{
				if (weaponType != WEAPON_REVOLVER)
				{
					angleRight[0] = Lara.rightArm.yRot + LaraItem->pos.yRot;
					angleRight[1] = Lara.rightArm.xRot;

					if (FireWeapon(weaponType, Lara.target, LaraItem, angleRight))
					{
						SmokeCountR = 28;
						SmokeWeapon = weaponType;
						TriggerGunShell(1, ID_GUNSHELL, weaponType); // Right Hand

						Lara.rightArm.flash_gun = weapon->flashTime;

						SoundEffect(SFX_EXPLOSION1, &LaraItem->pos, PITCH_SHIFT | 0x2000000);
						SoundEffect(weapon->sampleNum, &LaraItem->pos, 0);
						soundPlayed = true;

						if (weaponType == WEAPON_UZI)
							UziRight = true;

						Savegame.Game.AmmoUsed++;
					}
				}

				frameRight = p->recoilAnim;
			}
			else if (UziRight)
			{
				SoundEffect(weapon->sampleNum + 1, &LaraItem->pos, 0);
				UziRight = false;
			}
		}
		else if (frameRight >= p->recoilAnim)
		{
			if (weaponType == WEAPON_UZI)
			{
				SoundEffect(weapon->sampleNum, &LaraItem->pos, 0);
				UziRight = true;
			}

			frameRight++;

			if (frameRight == (p->recoilAnim + weapon->recoilFrame))
				frameRight = p->draw1Anim2;
		}
	}
	else
	{
		if (frameRight >= p->recoilAnim)
			frameRight = p->draw1Anim2;
		else if ((frameRight > 0) && (frameRight <= p->draw1Anim2))
			frameRight--;

		if (UziRight)
		{
			SoundEffect(weapon->sampleNum + 1, &LaraItem->pos, 0);
			UziRight = false;
		}
	}
	set_arm_info(&Lara.rightArm, frameRight);

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

				if (FireWeapon(weaponType, Lara.target, LaraItem, angleLeft))
				{
					if (weaponType == WEAPON_REVOLVER)
					{
						SmokeCountR = 28;
						SmokeWeapon = WEAPON_REVOLVER;
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
						SoundEffect(SFX_EXPLOSION1, &LaraItem->pos, PITCH_SHIFT | 0x2000000);
						SoundEffect(weapon->sampleNum, &LaraItem->pos, 0);
					}

					if (weaponType == WEAPON_UZI)
						UziLeft = true;

					Savegame.Game.AmmoUsed++;
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
			if (weaponType == WEAPON_UZI)
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
		if (frameLeft >= p->recoilAnim) 									// If Gun is Recoiling Stop it now...
			frameLeft = p->draw1Anim2;
		else if (frameLeft > 0 && frameLeft <= p->draw1Anim2)
			frameLeft--;													// UnLock ARM

		if (UziLeft)
		{
			SoundEffect(weapon->sampleNum + 1, &LaraItem->pos, 0);
			UziLeft = false;
		}
	}
	set_arm_info(&Lara.leftArm, frameLeft);
}

void PistolHandler(int weaponType)
{
	WEAPON_INFO* weapon = &Weapons[weaponType];

	LaraGetNewTarget(weapon);
	if (TrInput & IN_ACTION)
		LaraTargetInfo(weapon);

	AimWeapon(weapon, &Lara.leftArm);
	AimWeapon(weapon, &Lara.rightArm);

	if (Lara.leftArm.lock && !Lara.rightArm.lock)
	{
		Lara.torsoYrot = Lara.leftArm.yRot / 2;
		Lara.torsoXrot = Lara.leftArm.xRot / 2;

		if (Camera.oldType != LOOK_CAMERA)
		{
			Lara.headYrot = Lara.torsoYrot;
			Lara.headXrot = Lara.torsoXrot;
		}
	}
	else if (!Lara.leftArm.lock && Lara.rightArm.lock)
	{
		Lara.torsoYrot = Lara.rightArm.yRot / 2;
		Lara.torsoXrot = Lara.rightArm.xRot / 2;

		if (Camera.oldType != LOOK_CAMERA)
		{
			Lara.headYrot = Lara.torsoYrot;
			Lara.headXrot = Lara.torsoXrot;
		}
	}
	else if (Lara.leftArm.lock && Lara.rightArm.lock)
	{
		Lara.torsoYrot = (Lara.leftArm.yRot + Lara.rightArm.yRot) / 4;
		Lara.torsoXrot = (Lara.leftArm.xRot + Lara.rightArm.xRot) / 4;

		if (Camera.oldType != LOOK_CAMERA)
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

		GetLaraJointPosition(&pos, Lara.leftArm.flash_gun != 0 ? LJ_LHAND : LJ_RHAND);
		/*if (gfLevelFlags & 0x2000 && LaraItem->roomNumber == gfMirrorRoom)
		{
			v8 = GetRandomControl() & 0x3F;
			v9 = (GetRandomControl() & 0x1F) + 128;
			v10 = GetRandomControl();
			sub_4015A5(v14, v15, v16, 10, (v10 & 0x3F) + 192, v9, v8); // TODO: TriggerDynamicLightMirror !
		}
		else
		{*/
			TriggerDynamicLight(pos.x, pos.y, pos.z, 10, (GetRandomControl() & 0x3F) + 192, (GetRandomControl() & 0x1F) + 128, GetRandomControl() & 0x3F);
		//}
	}
}

void undraw_pistol_mesh_right(int weaponType)
{
	LARA_MESHES(ID_LARA, LM_RHAND);
	
	switch (weaponType)
	{
		case WEAPON_PISTOLS:
			Lara.holster = ID_LARA_HOLSTERS_PISTOLS;
			break;
		case WEAPON_UZI:
			Lara.holster = ID_LARA_HOLSTERS_UZIS;
			break;
		case WEAPON_REVOLVER:
			Lara.holster = ID_LARA_HOLSTERS_REVOLVER;
			break;
	}

}

void undraw_pistol_mesh_left(int weaponType)
{
	if (weaponType != WEAPON_REVOLVER)
	{
		LARA_MESHES(ID_LARA, LM_LHAND);
		
		switch (weaponType)
		{
			case WEAPON_PISTOLS:
				Lara.holster = ID_LARA_HOLSTERS_PISTOLS;
				break;
			case WEAPON_UZI:
				Lara.holster = ID_LARA_HOLSTERS_UZIS;
				break;
		}
	}
}

void draw_pistol_meshes(int weaponType)
{
	Lara.holster = ID_LARA_HOLSTERS;

	LARA_MESHES(WeaponObjectMesh(weaponType), LM_RHAND);
	if (weaponType != WEAPON_REVOLVER)
		LARA_MESHES(WeaponObjectMesh(weaponType), LM_LHAND);
}

void ready_pistols(int weaponType)
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
	Lara.target = NULL;
	Lara.rightArm.lock = 0;
	Lara.leftArm.lock = 0;
	Lara.rightArm.frameBase = Objects[WeaponObject(weaponType)].frameBase;
	Lara.leftArm.frameBase = Objects[WeaponObject(weaponType)].frameBase;
}

void undraw_pistols(int weaponType)
{
	PISTOL_DEF* p = &PistolsTable[Lara.gunType];

	short frameLeft = Lara.leftArm.frameNumber;
	if (frameLeft >= p->recoilAnim)
	{
		frameLeft = p->draw1Anim2;
	}
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
			SoundEffect(SFX_LARA_HOLSTER_AWAY, &LaraItem->pos, 0);
		}
	}
	set_arm_info(&Lara.leftArm, frameLeft);

	short frameRight = Lara.rightArm.frameNumber;
	if (frameRight >= p->recoilAnim)
	{
		frameRight = p->draw1Anim2;
	}
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
			SoundEffect(SFX_LARA_HOLSTER_AWAY, &LaraItem->pos, 0);
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
	PISTOL_DEF* p = &PistolsTable[Lara.gunType];
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
	arm->frameBase = Anims[arm->animNumber].framePtr;
}

void draw_pistols(int weaponType)
{
	short frame = Lara.leftArm.frameNumber + 1;
	PISTOL_DEF* p = &PistolsTable[Lara.gunType];

	if (frame < p->draw1Anim || frame > p->recoilAnim - 1)
	{
		frame = p->draw1Anim;
	}
	else if (frame == p->draw2Anim)
	{
		draw_pistol_meshes(weaponType);
		SoundEffect(SFX_LARA_HOLSTER_DRAW, &LaraItem->pos, 0);
	}
	else if (frame == p->recoilAnim - 1)
	{
		ready_pistols(weaponType);
		frame = 0;
	}

	set_arm_info(&Lara.rightArm, frame);
	set_arm_info(&Lara.leftArm, frame);
}

