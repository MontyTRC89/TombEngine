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
#include "Game/Lara/lara_struct.h"
#include "Game/savegame.h"
#include "Math/Math.h"
#include "Sound/sound.h"
#include "Specific/Input/Input.h"
#include "Specific/level.h"
#include "Specific/setup.h"

using namespace TEN::Input;
using namespace TEN::Math;

struct WeaponDef
{
	int ObjectNumber = 0;
	int Draw1Anim2	 = 0;
	int Draw1Anim	 = 0;
	int Draw2Anim	 = 0;
	int RecoilAnim	 = 0;
};

const WeaponDef WeaponDefs[4] =
{
	{ ID_LARA, 0, 0, 0, 0 },
	{ ID_PISTOLS_ANIM, 4, 5, 13, 24 },
	{ ID_REVOLVER_ANIM , 7, 8, 15, 29 },
	{ ID_UZI_ANIM, 4, 5, 13, 24 }
};

static Vector3i GetWeaponSmokeRelOffset(LaraWeaponType weaponType, bool isRightWeapon)
{
	switch (weaponType)
	{
	case LaraWeaponType::Pistol:
		return Vector3i(isRightWeapon ? -16 : 4, 128, 40);

	case LaraWeaponType::Revolver:
		return Vector3i(isRightWeapon ? -32 : 16, 160, 56);

	case LaraWeaponType::Uzi:
		return Vector3i(isRightWeapon ? -16 : 8, 140, 48);

	default:
		return Vector3i::Zero;
	}
}

static void SetArmInfo(ItemInfo& laraItem, ArmInfo& arm, int frame)
{
	auto& player = *GetLaraInfo(&laraItem);
	const auto& weaponDef = WeaponDefs[(int)player.Control.Weapon.GunType];

	int animBase = Objects[(int)weaponDef.ObjectNumber].animIndex;

	if (frame < weaponDef.Draw1Anim)
	{
		arm.AnimNumber = animBase;
	}
	else if (frame < weaponDef.Draw2Anim)
	{
		arm.AnimNumber = animBase + 1;
	}
	else if (frame < weaponDef.RecoilAnim)
	{
		arm.AnimNumber = animBase + 2;
	}
	else
	{
		arm.AnimNumber = animBase + 3;
	}

	arm.FrameNumber = frame;
	arm.FrameBase = g_Level.Anims[arm.AnimNumber].FramePtr;
}

static void ReadyPistols(ItemInfo& laraItem, LaraWeaponType weaponType)
{
	auto& player = *GetLaraInfo(&laraItem);

	player.Control.HandStatus = HandStatus::WeaponReady;
	player.TargetEntity = nullptr;
	player.LeftArm.FrameBase =
	player.RightArm.FrameBase = Objects[GetWeaponObjectID(weaponType)].frameBase;
	player.LeftArm.FrameNumber =
	player.RightArm.FrameNumber = 0;
	player.LeftArm.Orientation =
	player.RightArm.Orientation = EulerAngles::Zero;
	player.LeftArm.Locked =
	player.RightArm.Locked = false;
}

static void AnimateWeapon(ItemInfo& laraItem, LaraWeaponType weaponType, bool& hasFired, bool isRightWeapon)
{
	auto& player = *GetLaraInfo(&laraItem);
	auto& arm = isRightWeapon ? player.RightArm : player.LeftArm;
	auto& uziBool = isRightWeapon ? player.Control.Weapon.UziRight : player.Control.Weapon.UziLeft;
	const auto& weapon = Weapons[(int)weaponType];
	const auto& weaponDef = WeaponDefs[(int)player.Control.Weapon.GunType];

	// Spawn weapon smoke.
	if (laraItem.MeshBits.TestAny() && arm.GunSmoke)
	{
		auto relOffset = GetWeaponSmokeRelOffset(weaponType, isRightWeapon);
		auto pos = GetJointPosition(&laraItem, isRightWeapon ? LM_RHAND : LM_LHAND, relOffset);
		TriggerGunSmoke(pos.x, pos.y, pos.z, 0, 0, 0, 0, weaponType, arm.GunSmoke);
	}

	int frame = arm.FrameNumber;

	// Wind animation forward.
	if ((IsHeld(In::Action) && !player.TargetEntity) || arm.Locked)
	{
		// At or beyond DRAW_END (2) start frame AND before SHOOT_START (0) end frame; increment toward SHOOT_START (0) end frame.
		if (frame >= 0 && frame < weaponDef.Draw1Anim2)
		{
			frame++;
		}
		// At SHOOT_START (0) end frame.
		else if (frame == weaponDef.Draw1Anim2)
		{
			// Shoot weapon.
			if (IsHeld(In::Action))
			{
				// HACK: Special case for revolver.
				bool canShoot = (weaponType == LaraWeaponType::Revolver) ? isRightWeapon : true;

				if (canShoot)
				{
					auto armOrient = EulerAngles(
						arm.Orientation.x,
						arm.Orientation.y + laraItem.Pose.Orientation.y,
						0);

					if (FireWeapon(weaponType, *player.TargetEntity, laraItem, armOrient) != FireWeaponType::NoAmmo)
					{
						arm.GunSmoke = 28;
						TriggerGunShell(isRightWeapon ? true : false, ID_GUNSHELL, weaponType);
						arm.GunFlash = weapon.FlashTime;

						if (weaponType == LaraWeaponType::Uzi)
							uziBool = true;

						if (!hasFired)
						{
							SoundEffect(SFX_TR4_EXPLOSION1, &laraItem.Pose, SoundEnvironment::Land, 0.9f, 0.3f);
							SoundEffect(weapon.SampleNum, &laraItem.Pose);
							hasFired = true;
						}

						Statistics.Game.AmmoUsed++;
					}
				}

				// Go to SHOOT_CONTINUE (3) start frame.
				frame = weaponDef.RecoilAnim;
			}
			else
			{
				if (uziBool)
				{
					SoundEffect(weapon.SampleNum + 1, &laraItem.Pose);
					uziBool = false;
				}
			}
		}
		// At or beyond SHOOT_CONTINUE (3) start frame; increment toward SHOOT_CONTINUE (3) end frame to finish recoil before allowing to shoot again.
		else if (frame >= weaponDef.RecoilAnim)
		{
			if (weaponType == LaraWeaponType::Uzi)
			{
				SoundEffect(weapon.SampleNum, &laraItem.Pose);
				uziBool = true;
			}

			frame++;

			// At SHOOT_CONTINUE (3) end frame; go to START_SHOOT (0) end frame.
			if (frame == (weaponDef.RecoilAnim + weapon.RecoilFrame))
				frame = weaponDef.Draw1Anim2;
		}
	}
	// Wind animation backward.
	else
	{
		// Let SHOOT_CONTINUE (3) finish.
		if (frame >= weaponDef.RecoilAnim && frame < (weaponDef.RecoilAnim + weapon.RecoilFrame))
			frame++;

		// At SHOOT_CONTINUE (3) end frame; go to START_SHOOT (0) end frame.
		if (frame == (weaponDef.RecoilAnim + weapon.RecoilFrame))
		{
			frame = weaponDef.Draw1Anim2;
		}
		// Go back to "ready" stance.
		else if ((frame > 0) && (frame <= weaponDef.Draw1Anim2))
		{
			frame--;
		}

		if (uziBool)
		{
			SoundEffect(weapon.SampleNum + 1, &laraItem.Pose);
			uziBool = false;
		}
	}

	SetArmInfo(laraItem, arm, frame);
}

static int AnimateWeaponUndraw(ItemInfo& laraItem, LaraWeaponType weaponType, bool isRightWeapon)
{
	auto& lara = *GetLaraInfo(&laraItem);
	auto& arm = isRightWeapon ? lara.RightArm : lara.LeftArm;
	const auto& weapon = Weapons[(int)weaponType];
	const auto& weaponDef = WeaponDefs[(int)lara.Control.Weapon.GunType];

	int frame = arm.FrameNumber;

	// Finish recoil anim before reholstering weapon.
	if (frame >= weaponDef.RecoilAnim && frame < (weaponDef.RecoilAnim + weapon.RecoilFrame))
		frame++;

	if (frame == (weaponDef.RecoilAnim + weapon.RecoilFrame))
	{
		frame = weaponDef.Draw1Anim2;
	}
	else if (frame > 0 && frame < weaponDef.Draw1Anim)
	{
		arm.Orientation -= arm.Orientation / frame;
		frame--;
	}
	else if (frame == 0)
	{
		arm.Orientation = EulerAngles::Zero;
		frame = weaponDef.RecoilAnim - 1;
	}
	else if (frame > weaponDef.Draw1Anim && frame < weaponDef.RecoilAnim)
	{
		frame--;

		if (frame == (weaponDef.Draw2Anim - 1))
		{
			UndrawPistolMesh(laraItem, weaponType, isRightWeapon);
			SoundEffect(SFX_TR4_LARA_HOLSTER, &laraItem.Pose);
		}
	}

	SetArmInfo(laraItem, arm, frame);
	return frame;
}

void HandlePistols(ItemInfo& laraItem, LaraWeaponType weaponType)
{
	auto& lara = *GetLaraInfo(&laraItem);
	auto& weapon = Weapons[(int)weaponType];

	FindNewTarget(laraItem, weapon);
	if (IsHeld(In::Action))
		LaraTargetInfo(laraItem, weapon);

	AimWeapon(laraItem, lara.LeftArm, weapon);
	AimWeapon(laraItem, lara.RightArm, weapon);

	if (lara.LeftArm.Locked && !lara.RightArm.Locked)
	{
		lara.ExtraTorsoRot = lara.LeftArm.Orientation / 2;

		if (Camera.oldType != CameraType::Look)
			lara.ExtraHeadRot = lara.ExtraTorsoRot;
	}
	else if (!lara.LeftArm.Locked && lara.RightArm.Locked)
	{
		lara.ExtraTorsoRot = lara.RightArm.Orientation / 2;

		if (Camera.oldType != CameraType::Look)
			lara.ExtraHeadRot = lara.ExtraTorsoRot;
	}
	else if (lara.LeftArm.Locked && lara.RightArm.Locked)
	{
		lara.ExtraTorsoRot = (lara.LeftArm.Orientation + lara.RightArm.Orientation) / 4;

		if (Camera.oldType != CameraType::Look)
			lara.ExtraHeadRot = lara.ExtraTorsoRot;
	}

	AnimatePistols(laraItem, weaponType);

	if (lara.LeftArm.GunFlash || lara.RightArm.GunFlash)
	{
		auto basePos = GetJointPosition(&laraItem, (lara.LeftArm.GunFlash != 0) ? LM_LHAND : LM_RHAND).ToVector3();
		auto sphere = BoundingSphere(basePos, BLOCK(1 / 8.0f));
		auto lightPos = Random::GeneratePointInSphere(sphere);

		TriggerDynamicLight(
			lightPos.x, lightPos.y, lightPos.z,
			Random::GenerateFloat(8.0f, 11.0f),
			(GetRandomControl() & 0x3F) + 192, (GetRandomControl() & 0x1F) + 128, GetRandomControl() & 0x3F);
	}
}

void AnimatePistols(ItemInfo& laraItem, LaraWeaponType weaponType)
{
	bool hasFired = false;
	AnimateWeapon(laraItem, weaponType, hasFired, true);
	AnimateWeapon(laraItem, weaponType, hasFired, false);

	// If either weapon has fired, rumble gamepad.
	if (hasFired)
	{
		float power = (weaponType == LaraWeaponType::Uzi) ? Random::GenerateFloat(0.1f, 0.3f) : 1.0f;
		Rumble(power, 0.1f);
	}
}

void DrawPistols(ItemInfo& laraItem, LaraWeaponType weaponType)
{
	auto& lara = *GetLaraInfo(&laraItem);
	const auto& weaponDef = WeaponDefs[(int)lara.Control.Weapon.GunType];

	int frame = lara.LeftArm.FrameNumber + 1;

	if (frame < weaponDef.Draw1Anim || frame > (weaponDef.RecoilAnim - 1))
	{
		frame = weaponDef.Draw1Anim;
	}
	else if (frame == weaponDef.Draw2Anim)
	{
		DrawPistolMeshes(laraItem, weaponType);
		SoundEffect(SFX_TR4_LARA_DRAW, &laraItem.Pose);
	}
	else if (frame == (weaponDef.RecoilAnim - 1))
	{
		ReadyPistols(laraItem, weaponType);
		frame = 0;
	}

	SetArmInfo(laraItem, lara.RightArm, frame);
	SetArmInfo(laraItem, lara.LeftArm, frame);
}

void UndrawPistols(ItemInfo& laraItem, LaraWeaponType weaponType)
{
	auto& lara = *GetLaraInfo(&laraItem);
	const auto& weaponDef = WeaponDefs[(int)lara.Control.Weapon.GunType];

	int frameLeft = AnimateWeaponUndraw(laraItem, weaponType, false);
	int frameRight = AnimateWeaponUndraw(laraItem, weaponType, true);

	if (frameLeft == weaponDef.Draw1Anim &&
		frameRight == weaponDef.Draw1Anim)
	{
		lara.Control.HandStatus = HandStatus::Free;
		lara.TargetEntity = nullptr;
		lara.LeftArm.FrameNumber =
		lara.RightArm.FrameNumber = 0;
		lara.LeftArm.Locked =
		lara.RightArm.Locked = false;
	}

	if (!IsHeld(In::Look))
	{
		lara.ExtraHeadRot = (lara.LeftArm.Orientation + lara.RightArm.Orientation) / 4;
		lara.ExtraTorsoRot = (lara.LeftArm.Orientation + lara.RightArm.Orientation) / 4;
	}
}

void DrawPistolMeshes(ItemInfo& laraItem, LaraWeaponType weaponType)
{
	auto& lara = *GetLaraInfo(&laraItem);

	if (weaponType != LaraWeaponType::Revolver)
		lara.Control.Weapon.HolsterInfo.LeftHolster = HolsterSlot::Empty;

	lara.Control.Weapon.HolsterInfo.RightHolster = HolsterSlot::Empty;

	laraItem.Model.MeshIndex[LM_RHAND] = Objects[GetWeaponObjectMeshID(laraItem, weaponType)].meshIndex + LM_RHAND;
	if (weaponType != LaraWeaponType::Revolver)
		laraItem.Model.MeshIndex[LM_LHAND] = Objects[GetWeaponObjectMeshID(laraItem, weaponType)].meshIndex + LM_LHAND;
}

void UndrawPistolMesh(ItemInfo& laraItem, LaraWeaponType weaponType, bool isRightWeapon)
{
	auto& player = *GetLaraInfo(&laraItem);
	auto& holster = isRightWeapon ? player.Control.Weapon.HolsterInfo.RightHolster : player.Control.Weapon.HolsterInfo.LeftHolster;

	// HACK: Special case for revolver.
	if (!isRightWeapon)
	{
		if (weaponType == LaraWeaponType::Revolver)
			return;
	}

	int jointIndex = isRightWeapon ? LM_RHAND : LM_LHAND;
	laraItem.Model.MeshIndex[jointIndex] = laraItem.Model.BaseMesh + jointIndex;

	if (player.Weapons[(int)weaponType].Present)
		holster = GetWeaponHolsterSlot(weaponType);
	else
		holster = HolsterSlot::Empty;
}
