#include "framework.h"
#include "Game/Lara/lara_one_gun.h"

#include "Game/animation.h"
#include "Game/camera.h"
#include "Game/collision/collide_item.h"
#include "Game/control/box.h"
#include "Game/control/control.h"
#include "Game/control/los.h"
#include "Game/effects/Bubble.h"
#include "Game/effects/debris.h"
#include "Game/effects/effects.h"
#include "Game/effects/lara_fx.h"
#include "Game/effects/Ripple.h"
#include "Game/effects/tomb4fx.h"
#include "Game/effects/weather.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_fire.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/Lara/lara_two_guns.h"
#include "Game/misc.h"
#include "Game/savegame.h"
#include "Math/Math.h"
#include "Objects/Generic/Object/objects.h"
#include "Objects/Generic/Switches/generic_switch.h"
#include "Sound/sound.h"
#include "Specific/Input/Input.h"
#include "Specific/level.h"
#include "Specific/setup.h"

using namespace TEN::Effects::Bubble;
using namespace TEN::Effects::Environment;
using namespace TEN::Effects::Lara;
using namespace TEN::Effects::Ripple;
using namespace TEN::Entities::Switches;
using namespace TEN::Input;
using namespace TEN::Math;

constexpr auto TRIGGER_TIMEOUT		 = 5;
constexpr auto GRENADE_FRAG_TIMEOUT	 = 16;
constexpr auto GRENADE_FLASH_TIMEOUT = 4;

constexpr auto HARPOON_VELOCITY = CLICK(1);
constexpr auto HARPOON_TIME		= 10 * FPS;
constexpr auto ROCKET_VELOCITY	= CLICK(2);
constexpr auto ROCKET_TIME		= 4.5f * FPS;
constexpr auto GRENADE_VELOCITY = CLICK(0.5f);
constexpr auto GRENADE_TIME		= 4 * FPS;

constexpr auto PROJECTILE_HIT_RADIUS	 = CLICK(0.5f);
constexpr auto PROJECTILE_EXPLODE_RADIUS = BLOCK(1);

constexpr auto HK_BURST_MODE_SHOT_COUNT	   = 5;
constexpr auto HK_BURST_MODE_SHOT_INTERVAL = 12.0f;


void AnimateShotgun(ItemInfo* laraItem, LaraWeaponType weaponType)
{
	auto& lara = *GetLaraInfo(laraItem);
	const auto& weapon = lara.Weapons[(int)LaraWeaponType::HK];

	if (lara.LeftArm.GunSmoke > 0)
	{
		auto pos = Vector3i::Zero;
		if (weaponType == LaraWeaponType::HK)
			pos = Vector3i(0, 228, 96);
		else if (weaponType == LaraWeaponType::Shotgun)
			pos = Vector3i(0, 228, 0);
		else if (weaponType == LaraWeaponType::GrenadeLauncher)
			pos = Vector3i(0, 180, 80);
		else if (weaponType == LaraWeaponType::RocketLauncher)
			pos = Vector3i(0, 84, 72);

		pos = GetJointPosition(laraItem, LM_RHAND, pos);

		if (laraItem->MeshBits.TestAny())
			TriggerGunSmoke(pos.x, pos.y, pos.z, 0, 0, 0, 0, weaponType, lara.LeftArm.GunSmoke);
	}

	auto& item = g_Level.Items[lara.Control.Weapon.WeaponItem];
	bool isRunning = (weaponType == LaraWeaponType::HK && laraItem->Animation.Velocity.z != 0.0f);

	static bool reloadHarpoonGun = false;
	reloadHarpoonGun = (lara.Weapons[(int)weaponType].Ammo->HasInfinite() || weaponType != LaraWeaponType::HarpoonGun) ? false : reloadHarpoonGun;

	if (lara.Control.Weapon.Interval != 0.0f)
	{
		lara.Control.Weapon.Interval -= 1.0f;
		lara.Control.Weapon.Timer = 0.0f;
	}

	switch (item.Animation.ActiveState)
	{
	case WEAPON_STATE_AIM:
		lara.Control.Weapon.NumShotsFired = 0;
		lara.Control.Weapon.Interval = 0.0f;
		lara.Control.Weapon.Timer = 0.0f;

		if (lara.Control.WaterStatus == WaterStatus::Underwater || isRunning)
			item.Animation.TargetState = WEAPON_STATE_UNDERWATER_AIM;
		else if ((!IsHeld(In::Action) || lara.TargetEntity) && lara.LeftArm.Locked == false)
			item.Animation.TargetState = WEAPON_STATE_UNAIM;
		else
			item.Animation.TargetState = WEAPON_STATE_RECOIL;

		if (weaponType == LaraWeaponType::HarpoonGun &&
			!lara.Weapons[(int)weaponType].Ammo->HasInfinite())
		{
			if (reloadHarpoonGun)
			{
				item.Animation.TargetState = WEAPON_STATE_RELOAD;
				reloadHarpoonGun = false;
			}
		}

		break;

	case WEAPON_STATE_UNDERWATER_AIM:
		lara.Control.Weapon.NumShotsFired = 0;
		lara.Control.Weapon.Interval = 0.0f;
		lara.Control.Weapon.Timer = 0.0f;

		if (lara.Control.WaterStatus == WaterStatus::Underwater || isRunning)
		{
			if ((!IsHeld(In::Action) || lara.TargetEntity) && lara.LeftArm.Locked == false)
				item.Animation.TargetState = WEAPON_STATE_UNDERWATER_UNAIM;
			else
				item.Animation.TargetState = WEAPON_STATE_UNDERWATER_RECOIL;
		}
		else
			item.Animation.TargetState = WEAPON_STATE_AIM;

		if (weaponType == LaraWeaponType::HarpoonGun &&
			!lara.Weapons[(int)weaponType].Ammo->HasInfinite())
		{
			if (reloadHarpoonGun)
			{
				item.Animation.TargetState = WEAPON_STATE_RELOAD;
				reloadHarpoonGun = false;
			}
		}

		break;

	case WEAPON_STATE_RECOIL:
		if (item.Animation.FrameNumber == g_Level.Anims[item.Animation.AnimNumber].frameBase)
		{
			item.Animation.TargetState = WEAPON_STATE_UNAIM;

			if (lara.Control.WaterStatus != WaterStatus::Underwater &&
				!isRunning && !reloadHarpoonGun)
			{
				if (IsHeld(In::Action) && (!lara.TargetEntity || lara.LeftArm.Locked))
				{
					if (weaponType == LaraWeaponType::HarpoonGun)
					{
						FireHarpoon(laraItem);

						if (!(lara.Weapons[(int)LaraWeaponType::HarpoonGun].Ammo->GetCount() % 4) &&
							!lara.Weapons[(int)weaponType].Ammo->HasInfinite())
						{
							reloadHarpoonGun = true;
						}
					}
					else if (weaponType == LaraWeaponType::RocketLauncher)
					{
						FireRocket(laraItem);
					}
					else if (weaponType == LaraWeaponType::GrenadeLauncher)
					{
						FireGrenade(laraItem);
					}
					else if (weaponType == LaraWeaponType::Crossbow)
					{
						FireCrossbow(laraItem, nullptr);
					}
					else if (weaponType == LaraWeaponType::HK)
					{
						if ((weapon.WeaponMode == LaraWeaponTypeCarried::WTYPE_AMMO_2 ||
							 weapon.WeaponMode == LaraWeaponTypeCarried::WTYPE_AMMO_3) &&
							lara.Control.Weapon.Interval)
						{
							item.Animation.TargetState = WEAPON_STATE_AIM;
						}
						else
						{
							FireHK(laraItem, 0);
							lara.Control.Weapon.Timer = 1.0f;
							item.Animation.TargetState = WEAPON_STATE_RECOIL;

							SoundEffect(SFX_TR4_EXPLOSION1, &laraItem->Pose, SoundEnvironment::Land, 1.0f, 0.4f);
							SoundEffect(SFX_TR4_HK_FIRE, &laraItem->Pose);
						}
					}
					else
						FireShotgun(laraItem);

					if (weaponType != LaraWeaponType::HK)
						item.Animation.TargetState = WEAPON_STATE_RECOIL;
				}
				else if (lara.LeftArm.Locked)
					item.Animation.TargetState = WEAPON_STATE_AIM;
			}

			if (item.Animation.TargetState != WEAPON_STATE_RECOIL &&
				lara.Control.Weapon.Timer != 0.0f && weaponType == LaraWeaponType::HK)
			{
				StopSoundEffect(SFX_TR4_HK_FIRE);
				SoundEffect(SFX_TR4_HK_STOP, &laraItem->Pose);
				lara.Control.Weapon.Timer = 0.0f;
			}
		}
		else if (lara.Control.Weapon.Timer != 0.0f)
		{
			SoundEffect(SFX_TR4_EXPLOSION1, &laraItem->Pose, SoundEnvironment::Land, 1.0f, 0.4f);
			SoundEffect(SFX_TR4_HK_FIRE, &laraItem->Pose);
		}
		else if (weaponType == LaraWeaponType::Shotgun && !IsHeld(In::Action) && !lara.LeftArm.Locked)
		{
			item.Animation.TargetState = WEAPON_STATE_UNAIM;
		}

		if ((item.Animation.FrameNumber - g_Level.Anims[item.Animation.AnimNumber].frameBase) == 12 &&
			weaponType == LaraWeaponType::Shotgun)
		{
			TriggerGunShell(1, ID_SHOTGUNSHELL, LaraWeaponType::Shotgun);
		}

		break;

	case WEAPON_STATE_UNDERWATER_RECOIL:
		if ((item.Animation.FrameNumber - g_Level.Anims[item.Animation.AnimNumber].frameBase) == 0)
		{
			item.Animation.TargetState = WEAPON_STATE_UNDERWATER_UNAIM;

			if ((lara.Control.WaterStatus == WaterStatus::Underwater || isRunning) && !reloadHarpoonGun)
			{
				if (IsHeld(In::Action) && (!lara.TargetEntity || lara.LeftArm.Locked))
				{
					if (weaponType == LaraWeaponType::HarpoonGun)
					{
						FireHarpoon(laraItem);

						if (!(lara.Weapons[(int)LaraWeaponType::HarpoonGun].Ammo->GetCount() % 4) &&
							!lara.Weapons[(int)weaponType].Ammo->HasInfinite())
						{
							reloadHarpoonGun = true;
						}
					}
					else if (weaponType == LaraWeaponType::HK)
					{
						if ((weapon.WeaponMode == LaraWeaponTypeCarried::WTYPE_AMMO_2 ||
							 weapon.WeaponMode == LaraWeaponTypeCarried::WTYPE_AMMO_3) &&
							lara.Control.Weapon.Interval != 0.0f)
						{
							item.Animation.TargetState = WEAPON_STATE_UNDERWATER_AIM;
						}
						else
						{
							FireHK(laraItem, 1);
							lara.Control.Weapon.Timer = 1.0f;
							item.Animation.TargetState = WEAPON_STATE_UNDERWATER_RECOIL;

							SoundEffect(SFX_TR4_EXPLOSION1, &laraItem->Pose, SoundEnvironment::Land, 1.0f, 0.4f);
							SoundEffect(SFX_TR4_HK_FIRE, &laraItem->Pose);

						}
					}
					else
						item.Animation.TargetState = WEAPON_STATE_UNDERWATER_AIM;

					if (weaponType != LaraWeaponType::HK)
						item.Animation.TargetState = WEAPON_STATE_UNDERWATER_RECOIL;
				}
				else if (lara.LeftArm.Locked)
				{
					item.Animation.TargetState = WEAPON_STATE_UNDERWATER_AIM;
				}
			}

			if (item.Animation.TargetState != WEAPON_STATE_UNDERWATER_RECOIL &&
				lara.Control.Weapon.Timer)
			{
				StopSoundEffect(SFX_TR4_HK_FIRE);
				SoundEffect(SFX_TR4_HK_STOP, &laraItem->Pose);
				lara.Control.Weapon.Timer = 0.0f;
			}
		}
		else if (lara.Control.Weapon.Timer != 0.0f)
		{
			SoundEffect(SFX_TR4_EXPLOSION1, &laraItem->Pose, SoundEnvironment::Land, 1.0f, 0.4f);
			SoundEffect(SFX_TR4_HK_FIRE, &laraItem->Pose);
		}

		break;

	default:
		break;
	}

	AnimateItem(&item);

	lara.LeftArm.FrameBase = lara.RightArm.FrameBase = g_Level.Anims[item.Animation.AnimNumber].FramePtr;
	lara.LeftArm.FrameNumber = lara.RightArm.FrameNumber = item.Animation.FrameNumber - g_Level.Anims[item.Animation.AnimNumber].frameBase;
	lara.LeftArm.AnimNumber = lara.RightArm.AnimNumber = item.Animation.AnimNumber;
}

void ReadyShotgun(ItemInfo* laraItem, LaraWeaponType weaponType)
{
	auto* lara = GetLaraInfo(laraItem);

	lara->Control.HandStatus = HandStatus::WeaponReady;
	lara->LeftArm.Orientation = EulerAngles::Zero;
	lara->RightArm.Orientation = EulerAngles::Zero;
	lara->LeftArm.FrameNumber = 0;
	lara->RightArm.FrameNumber = 0;
	lara->LeftArm.Locked = false;
	lara->RightArm.Locked = false;
	lara->LeftArm.FrameBase = Objects[GetWeaponObjectID(weaponType)].frameBase;
	lara->RightArm.FrameBase = Objects[GetWeaponObjectID(weaponType)].frameBase;
	lara->TargetEntity = nullptr;
}

void FireShotgun(ItemInfo* laraItem)
{
	auto* lara = GetLaraInfo(laraItem);

	auto armOrient = EulerAngles(
		lara->LeftArm.Orientation.x,
		lara->LeftArm.Orientation.y + laraItem->Pose.Orientation.y,
		0
	);

	if (!lara->LeftArm.Locked)
	{
		armOrient = EulerAngles(
			lara->ExtraTorsoRot.x + lara->LeftArm.Orientation.x,
			lara->ExtraTorsoRot.y + lara->LeftArm.Orientation.y + laraItem->Pose.Orientation.y,
			0
		);
	}

	int value = (lara->Weapons[(int)LaraWeaponType::Shotgun].SelectedAmmo == WeaponAmmoType::Ammo1 ? 1820 : 5460);
	bool hasFired = false;
	for (int i = 0; i < 6; i++)
	{
		auto wobbledArmOrient = EulerAngles(
			armOrient.x + value * (GetRandomControl() - ANGLE(90.0f)) / 65536,
			armOrient.y + value * (GetRandomControl() - ANGLE(90.0f)) / 65536,
			0
		);

		if (FireWeapon(LaraWeaponType::Shotgun, lara->TargetEntity, laraItem, wobbledArmOrient) != FireWeaponType::NoAmmo)
			hasFired = true;
	}

	if (hasFired)
	{
		auto pos = GetJointPosition(laraItem, LM_RHAND, Vector3i(0, 1508, 32));
		auto pos2 = GetJointPosition(laraItem, LM_RHAND, Vector3i(0, 228, 32));

		lara->LeftArm.GunSmoke = 32;

		if (laraItem->MeshBits != 0)
		{
			for (int i = 0; i < 7; i++)
				TriggerGunSmoke(pos2.x, pos2.y, pos2.z, pos.x - pos2.x, pos.y - pos2.y, pos.z - pos2.z, 1, LaraWeaponType::Shotgun, lara->LeftArm.GunSmoke);
		}

		lara->RightArm.GunFlash = Weapons[(int)LaraWeaponType::Shotgun].FlashTime;

		SoundEffect(SFX_TR4_EXPLOSION1, &laraItem->Pose, TestEnvironment(ENV_FLAG_WATER, laraItem) ? SoundEnvironment::Water : SoundEnvironment::Land);
		SoundEffect(Weapons[(int)LaraWeaponType::Shotgun].SampleNum, &laraItem->Pose);

		Rumble(0.5f, 0.2f);

		Statistics.Game.AmmoUsed++;
	}
}

void DrawShotgun(ItemInfo* laraItem, LaraWeaponType weaponType)
{
	auto* lara = GetLaraInfo(laraItem);

	ItemInfo* item = nullptr;

	if (lara->Control.Weapon.WeaponItem == NO_ITEM)
	{
		lara->Control.Weapon.WeaponItem = CreateItem();
		item = &g_Level.Items[lara->Control.Weapon.WeaponItem];
		item->ObjectNumber = GetWeaponObjectID(weaponType);

		if (weaponType == LaraWeaponType::RocketLauncher)
			item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 1;
		else if (weaponType == LaraWeaponType::GrenadeLauncher)
			item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 0;
		else
			item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + 1;

		item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
		item->Animation.TargetState = WEAPON_STATE_DRAW;
		item->Animation.ActiveState = WEAPON_STATE_DRAW;
		item->Status = ITEM_ACTIVE;
		item->RoomNumber = NO_ROOM;
		item->Pose = laraItem->Pose;

		lara->RightArm.FrameBase = Objects[item->ObjectNumber].frameBase;
		lara->LeftArm.FrameBase = lara->RightArm.FrameBase;
	}
	else
		item = &g_Level.Items[lara->Control.Weapon.WeaponItem];

	AnimateItem(item);

	if (item->Animation.ActiveState != WEAPON_STATE_AIM &&
		item->Animation.ActiveState != WEAPON_STATE_UNDERWATER_AIM)
	{
		if (item->Animation.FrameNumber - g_Level.Anims[item->Animation.AnimNumber].frameBase == Weapons[(int)weaponType].DrawFrame)
			DrawShotgunMeshes(laraItem, weaponType);
		else if (lara->Control.WaterStatus == WaterStatus::Underwater)
			item->Animation.TargetState = WEAPON_STATE_UNDERWATER_AIM;
	}
	else
		ReadyShotgun(laraItem, weaponType);

	lara->LeftArm.FrameBase = lara->RightArm.FrameBase = g_Level.Anims[item->Animation.AnimNumber].FramePtr;
	lara->LeftArm.FrameNumber = lara->RightArm.FrameNumber = item->Animation.FrameNumber - g_Level.Anims[item->Animation.AnimNumber].frameBase;
	lara->LeftArm.AnimNumber = lara->RightArm.AnimNumber = item->Animation.AnimNumber;
}

void UndrawShotgun(ItemInfo* laraItem, LaraWeaponType weaponType)
{
	auto* lara = GetLaraInfo(laraItem);

	auto* item = &g_Level.Items[lara->Control.Weapon.WeaponItem];
	item->Animation.TargetState = WEAPON_STATE_UNDRAW;
	item->Pose = laraItem->Pose;

	AnimateItem(item);

	if (item->Status == ITEM_DEACTIVATED)
	{
		lara->Control.HandStatus = HandStatus::Free;
		lara->TargetEntity = nullptr;
		lara->RightArm.Locked = false;
		lara->LeftArm.Locked = false;
		KillItem(lara->Control.Weapon.WeaponItem);
		lara->Control.Weapon.WeaponItem = NO_ITEM;
		lara->RightArm.FrameNumber = 0;
		lara->LeftArm.FrameNumber = 0;
	}
	else if (item->Animation.ActiveState == WEAPON_STATE_UNDRAW)
	{
		if (item->Animation.FrameNumber - g_Level.Anims[item->Animation.AnimNumber].frameBase == 21 ||
			(weaponType == LaraWeaponType::GrenadeLauncher && item->Animation.FrameNumber - g_Level.Anims[item->Animation.AnimNumber].frameBase == 15))
		{
			UndrawShotgunMeshes(laraItem, weaponType);
		}
	}

	lara->RightArm.FrameBase = g_Level.Anims[item->Animation.AnimNumber].FramePtr;
	lara->LeftArm.FrameBase = g_Level.Anims[item->Animation.AnimNumber].FramePtr;
	lara->RightArm.FrameNumber = item->Animation.FrameNumber - g_Level.Anims[item->Animation.AnimNumber].frameBase;
	lara->LeftArm.FrameNumber = item->Animation.FrameNumber - g_Level.Anims[item->Animation.AnimNumber].frameBase;
	lara->RightArm.AnimNumber = item->Animation.AnimNumber;
	lara->LeftArm.AnimNumber = lara->RightArm.AnimNumber;
}

void DrawShotgunMeshes(ItemInfo* laraItem, LaraWeaponType weaponType)
{
	auto* lara = GetLaraInfo(laraItem);

	lara->Control.Weapon.HolsterInfo.BackHolster = HolsterSlot::Empty;
	laraItem->Model.MeshIndex[LM_RHAND] = Objects[GetWeaponObjectMeshID(laraItem, weaponType)].meshIndex + LM_RHAND;
}

void UndrawShotgunMeshes(ItemInfo* laraItem, LaraWeaponType weaponType)
{
	auto* lara = GetLaraInfo(laraItem);

	laraItem->Model.MeshIndex[LM_RHAND] = laraItem->Model.BaseMesh + LM_RHAND;

	if (lara->Weapons[(int)weaponType].Present)
		lara->Control.Weapon.HolsterInfo.BackHolster = GetWeaponHolsterSlot(weaponType);
	else
		lara->Control.Weapon.HolsterInfo.BackHolster = HolsterSlot::Empty;
}

ItemInfo* FireHarpoon(ItemInfo* laraItem)
{
	auto* lara = GetLaraInfo(laraItem);
	auto& ammo = GetAmmo(*lara, LaraWeaponType::HarpoonGun);

	if (!ammo)
		return nullptr;

	lara->Control.Weapon.HasFired = true;

	// Create a new item for harpoon.
	short itemNumber = CreateItem();
	if (itemNumber == NO_ITEM)
		return nullptr;

	if (!ammo.HasInfinite())
		ammo--;

	auto* item = &g_Level.Items[itemNumber];

	item->Color = Vector4(0.5f, 0.5f, 0.5f, 1.0f);
	item->ObjectNumber = ID_HARPOON;
	item->RoomNumber = laraItem->RoomNumber;

	auto jointPos = GetJointPosition(laraItem, LM_RHAND, Vector3i(-2, 373, 77));

	int floorHeight = GetCollision(jointPos.x, jointPos.y, jointPos.z, item->RoomNumber).Position.Floor;
	if (floorHeight >= jointPos.y)
		item->Pose.Position = jointPos;
	else
	{
		item->Pose.Position = Vector3i(laraItem->Pose.Position.x, jointPos.y, laraItem->Pose.Position.z);
		item->RoomNumber = laraItem->RoomNumber;
	}

	InitialiseItem(itemNumber);

	item->Pose.Orientation.x = lara->LeftArm.Orientation.x + laraItem->Pose.Orientation.x;
	item->Pose.Orientation.y = lara->LeftArm.Orientation.y + laraItem->Pose.Orientation.y;
	item->Pose.Orientation.z = 0;

	if (!lara->LeftArm.Locked)
	{
		item->Pose.Orientation.x += lara->ExtraTorsoRot.x;
		item->Pose.Orientation.y += lara->ExtraTorsoRot.y;
	}

	item->Pose.Orientation.z = 0;
	item->Animation.Velocity.z = HARPOON_VELOCITY * phd_cos(item->Pose.Orientation.x);
	item->Animation.Velocity.y = -HARPOON_VELOCITY * phd_sin(item->Pose.Orientation.x);
	item->HitPoints = HARPOON_TIME;

	Rumble(0.2f, 0.1f);
	AddActiveItem(itemNumber);

	Statistics.Level.AmmoUsed++;
	Statistics.Game.AmmoUsed++;

	return item;
}

void HarpoonBoltControl(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	if (item->HitPoints < HARPOON_TIME)
	{
		if (item->HitPoints > 0)
			item->HitPoints--;
		else
		{
			ExplodeItemNode(item, 0, 0, BODY_EXPLODE);
			KillItem(itemNumber);
		}

		return;
	}

	item->Pose.Orientation.z += ANGLE(35.0f);

	if (!TestEnvironment(ENV_FLAG_WATER, item->RoomNumber))
	{
		item->Pose.Orientation.x -= ANGLE(1.0f);

		if (item->Pose.Orientation.x < -ANGLE(90.0f))
			item->Pose.Orientation.x = -ANGLE(90.0f);

		item->Animation.Velocity.y = -HARPOON_VELOCITY * phd_sin(item->Pose.Orientation.x);
		item->Animation.Velocity.z = HARPOON_VELOCITY * phd_cos(item->Pose.Orientation.x);
	}
	else
	{
		// Create bubbles.
		if (Wibble & 4)
			SpawnBubble(item->Pose.Position.ToVector3(), item->RoomNumber, 0, 0, BubbleFlags::Clump | BubbleFlags::HighAmplitude, 0, 0, 0);
			
		item->Animation.Velocity.y = -HARPOON_VELOCITY * phd_sin(item->Pose.Orientation.x) / 2;
		item->Animation.Velocity.z = HARPOON_VELOCITY * phd_cos(item->Pose.Orientation.x) / 2;
	}

	TranslateItem(item, item->Pose.Orientation, item->Animation.Velocity.z);
	HandleProjectile(*item, *LaraItem, item->Pose.Position, ProjectileType::Harpoon, Weapons[(int)LaraWeaponType::HarpoonGun].Damage);
}

void FireGrenade(ItemInfo* laraItem)
{
	auto* lara = GetLaraInfo(laraItem);
	auto& ammo = GetAmmo(*lara, LaraWeaponType::GrenadeLauncher);

	if (!ammo)
		return;

	lara->Control.Weapon.HasFired = true;

	short itemNumber = CreateItem();
	if (itemNumber == NO_ITEM)
		return;

	auto* item = &g_Level.Items[itemNumber];
		
	item->Color = Vector4(0.5f, 0.5f, 0.5f, 1.0f);
	item->ObjectNumber = ID_GRENADE;
	item->RoomNumber = laraItem->RoomNumber;

	auto jointPos = GetJointPosition(laraItem, LM_RHAND, Vector3i(0, 276, 80));
	item->Pose.Position = jointPos;
	auto smokePos = jointPos;

	int floorHeight = GetCollision(jointPos.x, jointPos.y, jointPos.z, item->RoomNumber).Position.Floor;
	if (floorHeight < jointPos.y)
	{
		item->Pose.Position.x = laraItem->Pose.Position.x;
		item->Pose.Position.y = jointPos.y;
		item->Pose.Position.z = laraItem->Pose.Position.z;
		item->RoomNumber = laraItem->RoomNumber;
	}

	jointPos = GetJointPosition(laraItem, LM_RHAND, Vector3i(0, 1204, 5));

	lara->LeftArm.GunSmoke = 32;

	if (laraItem->MeshBits.TestAny())
	{
		for (int i = 0; i < 5; i++)
			TriggerGunSmoke(smokePos.x, smokePos.y, smokePos.z, jointPos.x - smokePos.x, jointPos.y - smokePos.y, jointPos.z - smokePos.z, 1, LaraWeaponType::GrenadeLauncher, lara->LeftArm.GunSmoke);
	}

	InitialiseItem(itemNumber);

	item->Pose.Orientation.x = laraItem->Pose.Orientation.x + lara->LeftArm.Orientation.x;
	item->Pose.Orientation.y = laraItem->Pose.Orientation.y + lara->LeftArm.Orientation.y;
	item->Pose.Orientation.z = 0;

	if (!lara->LeftArm.Locked)
	{
		item->Pose.Orientation.x += lara->ExtraTorsoRot.x;
		item->Pose.Orientation.y += lara->ExtraTorsoRot.y;
	}

	item->Animation.Velocity.z = GRENADE_VELOCITY;
	item->Animation.Velocity.y = -CLICK(2) * phd_sin(item->Pose.Orientation.x);
	item->Animation.ActiveState = item->Pose.Orientation.x;
	item->Animation.TargetState = item->Pose.Orientation.y;
	item->Animation.RequiredState = 0;
	item->HitPoints = GRENADE_TIME;
	item->ItemFlags[0] = (int)WeaponAmmoType::Ammo2;

	Rumble(0.4f, 0.25f);

	AddActiveItem(itemNumber);

	if (!ammo.HasInfinite())
		ammo--;

	switch (lara->Weapons[(int)LaraWeaponType::GrenadeLauncher].SelectedAmmo)
	{
	case WeaponAmmoType::Ammo1:
		item->ItemFlags[0] = (int)ProjectileType::Grenade;
		break;

	case WeaponAmmoType::Ammo2:
		item->ItemFlags[0] = (int)ProjectileType::FragGrenade;
		break;

	case WeaponAmmoType::Ammo3:
		item->ItemFlags[0] = (int)ProjectileType::FlashGrenade;
		break;
	}

	Statistics.Level.AmmoUsed++;
	Statistics.Game.AmmoUsed++;
}

void GrenadeControl(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	item->Color = Vector4(0.5f, 0.5f, 0.5f, 1.0f);

	// Check if above water and update Y and Z velocities.
	bool aboveWater = false;
	if (TestEnvironment(ENV_FLAG_WATER, item->RoomNumber) ||
		TestEnvironment(ENV_FLAG_SWAMP, item->RoomNumber))
	{
		item->Animation.Velocity.y += (5.0f - item->Animation.Velocity.y) / 2;
		item->Animation.Velocity.z -= item->Animation.Velocity.z / 4;

		if (item->Animation.Velocity.z)
		{
			item->Pose.Orientation.z += (short((item->Animation.Velocity.z / 16) + 3.0f) * ANGLE(1.0f));
			if (item->Animation.RequiredState)
				item->Pose.Orientation.y += (short((item->Animation.Velocity.z / 4) + 3.0f) * ANGLE(1.0f));
			else
				item->Pose.Orientation.x += (short((item->Animation.Velocity.z / 4) + 3.0f) * ANGLE(1.0f));
		}
	}
	else
	{
		aboveWater = true;
		item->Animation.Velocity.y += 3.0f;

		if (item->Animation.Velocity.z)
		{
			item->Pose.Orientation.z += (short((item->Animation.Velocity.z / 4) + 7.0f) * ANGLE(1.0f));
			if (item->Animation.RequiredState)
				item->Pose.Orientation.y += (short((item->Animation.Velocity.z / 2) + 7.0f) * ANGLE(1.0f));
			else
				item->Pose.Orientation.x += (short((item->Animation.Velocity.z / 2) + 7.0f) * ANGLE(1.0f));
		}
	}

	// Trigger fire and smoke sparks in the direction of motion.
	if (item->Animation.Velocity.z && aboveWater)
	{
		auto world = Matrix::CreateFromYawPitchRoll(
			TO_RAD(item->Pose.Orientation.y - ANGLE(180.0f)),
			TO_RAD(item->Pose.Orientation.x),
			TO_RAD(item->Pose.Orientation.z)) *
			Matrix::CreateTranslation(0, 0, -64);

		int wx = world.Translation().x;
		int wy = world.Translation().y;
		int wz = world.Translation().z;

		TriggerRocketSmoke(wx + item->Pose.Position.x, wy + item->Pose.Position.y, wz + item->Pose.Position.z, -1);
		TriggerRocketFire(wx + item->Pose.Position.x, wy + item->Pose.Position.y, wz + item->Pose.Position.z);
	}

	auto velocity = Vector3i(
		item->Animation.Velocity.z * phd_sin(item->Animation.TargetState),
		item->Animation.Velocity.y,
		item->Animation.Velocity.z * phd_cos(item->Animation.TargetState)
	);

	// Update grenade position.
	auto prevPos = item->Pose.Position;
	item->Pose.Position += velocity;

	// Do dynamics only for first-order grenades (not fragments, which have ProjectileType set to Explosive)
	if (item->ItemFlags[0] != (int)ProjectileType::Explosive)
	{
		// Do grenade physics.
		short sYOrient = item->Pose.Orientation.y;
		item->Pose.Orientation.y = item->Animation.TargetState;

		DoProjectileDynamics(itemNumber, prevPos.x, prevPos.y, prevPos.z, velocity.x, velocity.y, velocity.z);

		item->Animation.TargetState = item->Pose.Orientation.y;
		item->Pose.Orientation.y = sYOrient;
	}

	HandleProjectile(*item, *LaraItem, prevPos, (ProjectileType)item->ItemFlags[0], Weapons[(int)LaraWeaponType::GrenadeLauncher].ExplosiveDamage);
}

void FireRocket(ItemInfo* laraItem)
{
	auto* lara = GetLaraInfo(laraItem);
	auto& ammo = GetAmmo(*lara, LaraWeaponType::RocketLauncher);

	if (!ammo)
		return;

	lara->Control.Weapon.HasFired = true;

	short itemNumber = CreateItem();
	if (itemNumber == NO_ITEM)
		return;

	auto* item = &g_Level.Items[itemNumber];
	item->ObjectNumber = ID_ROCKET;
	item->RoomNumber = laraItem->RoomNumber;

	if (!ammo.HasInfinite())
		ammo--;

	auto jointPos = GetJointPosition(laraItem, LM_RHAND, Vector3i(0, 180, 72));

	int x, y, z;
	item->Pose.Position.x = x = jointPos.x;
	item->Pose.Position.y = y = jointPos.y;
	item->Pose.Position.z = z = jointPos.z;

	jointPos = GetJointPosition(laraItem, LM_RHAND, Vector3i(0, 2004, 72));

	lara->LeftArm.GunSmoke = 32;

	for (int i = 0; i < 5; i++)
		TriggerGunSmoke(x, y, z, jointPos.x - x, jointPos.y - y, jointPos.z - z, 1, LaraWeaponType::RocketLauncher, lara->LeftArm.GunSmoke);

	jointPos = GetJointPosition(laraItem, LM_RHAND, Vector3i(0, -CLICK(1), 0));

	for (int i = 0; i < 10; i++)
		TriggerGunSmoke(jointPos.x, jointPos.y, jointPos.z, jointPos.x - x, jointPos.y - y, jointPos.z - z, 2, LaraWeaponType::RocketLauncher, 32);

	InitialiseItem(itemNumber);

	item->Pose.Orientation.x = laraItem->Pose.Orientation.x + lara->LeftArm.Orientation.x;
	item->Pose.Orientation.y = laraItem->Pose.Orientation.y + lara->LeftArm.Orientation.y;
	item->Pose.Orientation.z = 0;
	item->HitPoints = ROCKET_TIME;

	if (!lara->LeftArm.Locked)
	{
		item->Pose.Orientation.x += lara->ExtraTorsoRot.x;
		item->Pose.Orientation.y += lara->ExtraTorsoRot.y;
	}

	item->Animation.Velocity.z = 512.0f / 32.0f;
	item->ItemFlags[0] = 0;

	AddActiveItem(itemNumber);

	Rumble(0.4f, 0.3f);
	SoundEffect(SFX_TR4_EXPLOSION1, &laraItem->Pose);

	Statistics.Level.AmmoUsed++;
	Statistics.Game.AmmoUsed++;
}

void RocketControl(short itemNumber)
{
	auto& item = g_Level.Items[itemNumber];

	// Update velocity and orientation.

	if (TestEnvironment(ENV_FLAG_WATER, item.RoomNumber))
	{
		if (item.Animation.Velocity.z > (ROCKET_VELOCITY / 4))
			item.Animation.Velocity.z -= item.Animation.Velocity.z / 4;
		else
		{
			item.Animation.Velocity.z += (item.Animation.Velocity.z / 4) + 4.0f;

			if (item.Animation.Velocity.z > (ROCKET_VELOCITY / 4))
				item.Animation.Velocity.z = ROCKET_VELOCITY / 4;
		}

		item.Pose.Orientation.z += short((item.Animation.Velocity.z / 8) + 3.0f) * ANGLE(1.0f);
	}
	else
	{
		if (item.Animation.Velocity.z < ROCKET_VELOCITY)
			item.Animation.Velocity.z += (item.Animation.Velocity.z / 4) + 4.0f;

		item.Pose.Orientation.z += short((item.Animation.Velocity.z / 4) + 7.0f) * ANGLE(1.0f);
	}

	item.Color = Vector4(0.5f, 0.5f, 0.5f, 1.0f);

	// Calculate offset in rocket direction for fire and smoke sparks.
	auto world = Matrix::CreateFromYawPitchRoll(
		TO_RAD(item.Pose.Orientation.y - ANGLE(180.0f)),
		TO_RAD(item.Pose.Orientation.x),
		TO_RAD(item.Pose.Orientation.z)) *
		Matrix::CreateTranslation(0, 0, -64);

	int wx = world.Translation().x;
	int wy = world.Translation().y;
	int wz = world.Translation().z;

	// Trigger fire, smoke, and light.
	TriggerRocketSmoke(wx + item.Pose.Position.x, wy + item.Pose.Position.y, wz + item.Pose.Position.z, -1);
	TriggerRocketFire(wx + item.Pose.Position.x, wy + item.Pose.Position.y, wz + item.Pose.Position.z);
	TriggerDynamicLight(
		wx + item.Pose.Position.x + (GetRandomControl() & 15) - 8, 
		wy + item.Pose.Position.y + (GetRandomControl() & 15) - 8, 
		wz + item.Pose.Position.z + (GetRandomControl() & 15) - 8, 
		14, 28 + (GetRandomControl() & 3), 16 + (GetRandomControl() & 7), (GetRandomControl() & 7));

	// If underwater, generate bubbles.
	if (TestEnvironment(ENV_FLAG_WATER, item.RoomNumber))
	{
		auto pos = item.Pose.Position.ToVector3() + Vector3(wx, wy, wz);
		SpawnBubble(pos, item.RoomNumber, 4, 8, 0, 0, 0, 0);
	}

	// Update rocket's position.
	auto prevPos = item.Pose.Position;
	TranslateItem(&item, item.Pose.Orientation, item.Animation.Velocity.z);

	HandleProjectile(item, *LaraItem, prevPos, ProjectileType::Explosive, Weapons[(int)LaraWeaponType::RocketLauncher].ExplosiveDamage);
}

void FireCrossbow(ItemInfo* laraItem, Pose* pos)
{
	auto* lara = GetLaraInfo(laraItem);
	auto& ammo = GetAmmo(*lara, LaraWeaponType::Crossbow);

	if (!ammo)
		return;

	lara->Control.Weapon.HasFired = true;

	short itemNumber = CreateItem();
	if (itemNumber == NO_ITEM)
		return;

	auto* item = &g_Level.Items[itemNumber];
	item->ObjectNumber = ID_CROSSBOW_BOLT;
	item->Color = Vector4(0.5f, 0.5f, 0.5f, 1.0f);

	if (!ammo.HasInfinite())
		ammo--;

	if (pos)
	{
		item->Pose.Position = pos->Position;
		item->RoomNumber = laraItem->RoomNumber;

		InitialiseItem(itemNumber);

		item->Pose.Orientation = pos->Orientation;
	}
	else
	{
		auto jointPos = GetJointPosition(laraItem, LM_RHAND, Vector3i(0, 228, 32));

		item->RoomNumber = laraItem->RoomNumber;

		int floorHeight = GetCollision(jointPos.x, jointPos.y, jointPos.z, item->RoomNumber).Position.Floor;
		if (floorHeight >= jointPos.y)
			item->Pose.Position = jointPos;
		else
		{
			item->Pose.Position = Vector3i(laraItem->Pose.Position.x, jointPos.y, laraItem->Pose.Position.z);
			item->RoomNumber = laraItem->RoomNumber;
		}

		InitialiseItem(itemNumber);

		item->Pose.Orientation.x = lara->LeftArm.Orientation.x + laraItem->Pose.Orientation.x;
		item->Pose.Orientation.z = 0;
		item->Pose.Orientation.y = lara->LeftArm.Orientation.y + laraItem->Pose.Orientation.y;

		if (!lara->LeftArm.Locked)
		{
			item->Pose.Orientation.x += lara->ExtraTorsoRot.x;
			item->Pose.Orientation.y += lara->ExtraTorsoRot.y;
		}
	}

	item->Animation.Velocity.z = 512.0f;
	item->HitPoints = HARPOON_TIME;

	AddActiveItem(itemNumber);

	switch (lara->Weapons[(int)LaraWeaponType::Crossbow].SelectedAmmo)
	{
	case WeaponAmmoType::Ammo1:
		item->ItemFlags[0] = (int)ProjectileType::Normal;
		break;

	case WeaponAmmoType::Ammo2:
		item->ItemFlags[0] = (int)ProjectileType::Poison;
		break;

	case WeaponAmmoType::Ammo3:
		item->ItemFlags[0] = (int)ProjectileType::Explosive;
		break;
	}

	Rumble(0.2f, 0.1f);
	SoundEffect(SFX_TR4_CROSSBOW_FIRE, &laraItem->Pose);

	Statistics.Level.AmmoUsed++;
	Statistics.Game.AmmoUsed++;
}

void FireCrossBowFromLaserSight(ItemInfo* laraItem, GameVector* origin, GameVector* target)
{
	auto orient = Geometry::GetOrientToPoint(origin->ToVector3(), target->ToVector3());
	auto boltPose = Pose(origin->x, origin->y, origin->z, orient);
	FireCrossbow(laraItem, &boltPose);
}

void CrossbowBoltControl(short itemNumber)
{
	auto* lara = GetLaraInfo(LaraItem);
	auto* item = &g_Level.Items[itemNumber];

	if (TestEnvironment(ENV_FLAG_WATER, item))
	{
		if (item->Animation.Velocity.z > 64.0f)
			item->Animation.Velocity.z -= item->Animation.Velocity.z / 16;

		if (GlobalCounter & 1)
			SpawnBubble(item->Pose.Position.ToVector3(), item->RoomNumber, 4, 7, 0, 0, 0, 0);
	}

	auto prevPos = item->Pose.Position;
	TranslateItem(item, item->Pose.Orientation, item->Animation.Velocity.z);

	int damage = (item->ItemFlags[0] == (int)ProjectileType::Explosive) ?
		Weapons[(int)LaraWeaponType::Crossbow].ExplosiveDamage : Weapons[(int)LaraWeaponType::Crossbow].Damage;

	HandleProjectile(*item, *LaraItem, prevPos, (ProjectileType)item->ItemFlags[0], damage);
}

void FireHK(ItemInfo* laraItem, int mode)
{
	auto& lara = GetLaraInfo(laraItem);
	const auto& weapon = lara->Weapons[(int)LaraWeaponType::HK];

	auto angles = EulerAngles(
		lara->LeftArm.Orientation.x,
		lara->LeftArm.Orientation.y + laraItem->Pose.Orientation.y,
		0
	);

	if (weapon.WeaponMode == LaraWeaponTypeCarried::WTYPE_AMMO_3)
	{
		lara->Control.Weapon.Interval = HK_BURST_MODE_SHOT_INTERVAL;
	}
	else if (weapon.WeaponMode == LaraWeaponTypeCarried::WTYPE_AMMO_2)
	{
		lara->Control.Weapon.NumShotsFired++;
		if (lara->Control.Weapon.NumShotsFired == HK_BURST_MODE_SHOT_COUNT)
		{
			lara->Control.Weapon.NumShotsFired = 0;
			lara->Control.Weapon.Interval = HK_BURST_MODE_SHOT_INTERVAL;
		}
	}

	if (!lara->LeftArm.Locked)
	{
		angles = EulerAngles(
			lara->ExtraTorsoRot.x + lara->LeftArm.Orientation.x,
			lara->ExtraTorsoRot.y + lara->LeftArm.Orientation.y + laraItem->Pose.Orientation.y,
			0
		);
	}

	if (mode)
	{
		Weapons[(int)LaraWeaponType::HK].ShotAccuracy = ANGLE(12.0f);
		Weapons[(int)LaraWeaponType::HK].Damage = 1;
	}
	else
	{
		Weapons[(int)LaraWeaponType::HK].ShotAccuracy = ANGLE(4.0f);
		Weapons[(int)LaraWeaponType::HK].Damage = 3;
	}

	if (FireWeapon(LaraWeaponType::HK, lara->TargetEntity, laraItem, angles) != FireWeaponType::NoAmmo)
	{
		lara->LeftArm.GunSmoke = 12;

		TriggerGunShell(1, ID_GUNSHELL, LaraWeaponType::HK);
		lara->RightArm.GunFlash = Weapons[(int)LaraWeaponType::HK].FlashTime;

		Rumble(0.2f, 0.1f);
	}
}

void LasersightWeaponHandler(ItemInfo* item, LaraWeaponType weaponType)
{
	auto& lara = *GetLaraInfo(item);
	auto& ammo = GetAmmo(lara, lara.Control.Weapon.GunType);
	const auto& weapon = lara.Weapons[(int)weaponType];

	if (!LaserSight || (!weapon.HasLasersight))
		return;

	bool isFiring = false;

	if (lara.Control.Weapon.Interval != 0.0f)
		lara.Control.Weapon.Interval -= 1.0f;

	if (lara.Control.Weapon.Timer != 0.0f)
		lara.Control.Weapon.Timer -= 1.0f;

	if (!IsHeld(In::Action) || lara.Control.Weapon.Interval != 0.0f || !ammo)
	{
		if (!IsHeld(In::Action))
		{
			lara.Control.Weapon.NumShotsFired = 0;
			Camera.bounce = 0;
		}
	}
	else
	{
		if (lara.Control.Weapon.GunType == LaraWeaponType::Revolver)
		{
			lara.Control.Weapon.Interval = 16.0f;
			Statistics.Game.AmmoUsed++;
			isFiring = true;

			if (!ammo.HasInfinite())
				(ammo)--;

			Camera.bounce = -16 - (GetRandomControl() & 0x1F);
		}
		else if (lara.Control.Weapon.GunType == LaraWeaponType::Crossbow)
		{
			lara.Control.Weapon.Interval = 32.0f;
			isFiring = true;
		}
		else
		{
			if (lara.Control.Weapon.GunType == LaraWeaponType::HK &&
				weapon.WeaponMode == LaraWeaponTypeCarried::WTYPE_AMMO_3 &&
				weapon.SelectedAmmo == WeaponAmmoType::Ammo1)
			{
				lara.Control.Weapon.Interval = HK_BURST_MODE_SHOT_INTERVAL;
				isFiring = true;

				SoundEffect(SFX_TR4_EXPLOSION1, nullptr, SoundEnvironment::Land, 1.0f, 0.4f);
				SoundEffect(SFX_TR4_HK_FIRE, nullptr);
				Camera.bounce = -16 - (GetRandomControl() & 0x1F);
			}
			else if (lara.Control.Weapon.GunType == LaraWeaponType::HK &&
				weapon.WeaponMode == LaraWeaponTypeCarried::WTYPE_AMMO_2 &&
				weapon.SelectedAmmo == WeaponAmmoType::Ammo1)
			{
				if (!lara.Control.Weapon.Timer)
				{
					if (++lara.Control.Weapon.NumShotsFired == HK_BURST_MODE_SHOT_COUNT)
					{
						lara.Control.Weapon.NumShotsFired = 0;
						lara.Control.Weapon.Interval = HK_BURST_MODE_SHOT_INTERVAL;
					}

					lara.Control.Weapon.Timer = 4.0f;
					isFiring = true;

					SoundEffect(SFX_TR4_EXPLOSION1, nullptr, SoundEnvironment::Land, 1.0f, 0.4f);
					SoundEffect(SFX_TR4_HK_FIRE, nullptr);
					Camera.bounce = -16 - (GetRandomControl() & 0x1F);
				}
				else
				{
					SoundEffect(SFX_TR4_EXPLOSION1, nullptr, SoundEnvironment::Land, 1.0f, 0.4f);
					SoundEffect(SFX_TR4_HK_FIRE, nullptr);
					Camera.bounce = -16 - (GetRandomControl() & 0x1F);
				}
			}
			else
			{
				if (lara.Control.Weapon.Timer)
				{
					SoundEffect(SFX_TR4_EXPLOSION1, nullptr, SoundEnvironment::Land, 1.0f, 0.4f);
					SoundEffect(SFX_TR4_HK_FIRE, nullptr);
				}
				else
				{
					lara.Control.Weapon.Timer = 4.0f;
					isFiring = true;

					SoundEffect(SFX_TR4_EXPLOSION1, nullptr, SoundEnvironment::Land, 1.0f, 0.4f);
					SoundEffect(SFX_TR4_HK_FIRE, nullptr);
					Camera.bounce = -16 - (GetRandomControl() & 0x1F);
				}
			}

			if (!ammo.HasInfinite())
				ammo--;
		}
	}

	GetTargetOnLOS(&Camera.pos, &Camera.target, true, isFiring);
}

void RifleHandler(ItemInfo* laraItem, LaraWeaponType weaponType)
{
	auto& lara = *GetLaraInfo(laraItem);
	const auto& weapon = Weapons[(int)weaponType];

	if (BinocularRange || LaserSight)
		return; // Never handle weapons while in binocular mode.
	{
		FindNewTarget(laraItem, weapon);

		if (TrInput & IN_ACTION)
			LaraTargetInfo(laraItem, weapon);

		AimWeapon(laraItem, lara.LeftArm, weapon);

		if (lara.LeftArm.Locked)
		{
			lara.ExtraTorsoRot.x = lara.LeftArm.Orientation.x;
			lara.ExtraTorsoRot.y = lara.LeftArm.Orientation.y;

			if (Camera.oldType != CameraType::Look && !BinocularRange)
				lara.ExtraHeadRot = EulerAngles::Zero;
		}
	}

	if (weaponType == LaraWeaponType::Revolver)
		AnimatePistols(laraItem, LaraWeaponType::Revolver);
	else
		AnimateShotgun(laraItem, weaponType);

	if (lara.RightArm.GunFlash)
	{
		if (weaponType == LaraWeaponType::Shotgun || weaponType == LaraWeaponType::HK)
		{
			auto pos = GetJointPosition(laraItem, LM_RHAND, Vector3i(0, -64, 0));
			TriggerDynamicLight(
				pos.x, pos.y, pos.z,
				12,
				(GetRandomControl() & 0x3F) + 192,
				(GetRandomControl() & 0x1F) + 128,
				GetRandomControl() & 0x3F
			);
		}
		else if (weaponType == LaraWeaponType::Revolver)
		{
			auto pos = GetJointPosition(laraItem, LM_RHAND, Vector3i(0, -32, 0));
			TriggerDynamicLight(pos.x, pos.y, pos.z, 12, (GetRandomControl() & 0x3F) + 192, (GetRandomControl() & 0x1F) + 128, (GetRandomControl() & 0x3F));
		}
	}
}

void DoExplosiveDamage(ItemInfo& emitter, ItemInfo& target, ItemInfo& projectile, int damage)
{
	if (target.Flags & IFLAG_KILLED)
		return;

	if (target.HitPoints == NOT_TARGETABLE)
		return;

	if (&target != &emitter && !target.IsLara() && target.HitPoints > 0)
	{
		if (emitter.ItemFlags[2])
			return;

		emitter.HitStatus = true;

		HitTarget(&emitter, &target, nullptr, damage, 1);
					
		if (&target != &emitter)
		{
			Statistics.Game.AmmoHits++;
			if (target.HitPoints <= 0)
			{
				Statistics.Level.Kills++;
				CreatureDie(target.Index, true);
			}
		}
	}
	else
	{
		DoDamage(&emitter, damage * 5);
		if (!TestEnvironment(ENV_FLAG_WATER, target.RoomNumber) && emitter.HitPoints <= 0)
			LaraBurn(&emitter);
	}
}

bool EmitFromProjectile(ItemInfo& projectile, ProjectileType type)
{
	if (!projectile.ItemFlags[1])
		return false;

	projectile.ItemFlags[1]--;

	if (!projectile.ItemFlags[1])
	{
		KillItem(projectile.Index);
		return true;
	}

	if (type == ProjectileType::FlashGrenade)
	{
		// Flash grenades.
		int r, g, b;
		if (projectile.ItemFlags[1] == 1)
		{
			FlashGrenadeAftershockTimer = 120;
			r = 255;
			g = 255;
			b = 255;
		}
		else
		{
			r = (GetRandomControl() & 0x1F) + 224;
			g = b = r - GetRandomControl() & 0x1F;
		}

		Weather.Flash(r, g, b, 0.03f);
		TriggerFlashSmoke(projectile.Pose.Position.x, projectile.Pose.Position.y, projectile.Pose.Position.z, projectile.RoomNumber);
	}
	else if (type == ProjectileType::FragGrenade)
	{
		// Trigger a new fragment in the case of GRENADE_SUPER until itemFlags[1] is > 0.
		short newGrenadeItemNumber = CreateItem();
		if (newGrenadeItemNumber == NO_ITEM)
			return true;

		auto& newGrenade = g_Level.Items[newGrenadeItemNumber];

		newGrenade.Color = Vector4(0.5f, 0.5f, 0.5f, 1.0f);
		newGrenade.ObjectNumber = ID_GRENADE;
		newGrenade.RoomNumber = projectile.RoomNumber;
		newGrenade.Pose.Position.x = Random::GenerateInt(0, 512) + projectile.Pose.Position.x - CLICK(1);
		newGrenade.Pose.Position.y = projectile.Pose.Position.y - CLICK(1);
		newGrenade.Pose.Position.z = Random::GenerateInt(0, 512) + projectile.Pose.Position.z - CLICK(1);

		InitialiseItem(newGrenadeItemNumber);

		newGrenade.Pose.Orientation.x = Random::GenerateAngle(0, ANGLE(90.0f)) + ANGLE(45.0f);
		newGrenade.Pose.Orientation.y = Random::GenerateAngle(0, ANGLE(359.0f));
		newGrenade.Pose.Orientation.z = 0;
		newGrenade.Animation.Velocity.z = 64.0f;
		newGrenade.Animation.Velocity.y = -64.0f * phd_sin(newGrenade.Pose.Orientation.x);
		newGrenade.Animation.ActiveState = newGrenade.Pose.Orientation.x;
		newGrenade.Animation.TargetState = newGrenade.Pose.Orientation.y;
		newGrenade.Animation.RequiredState = 0;

		AddActiveItem(newGrenadeItemNumber);

		newGrenade.Status = ITEM_INVISIBLE;
		newGrenade.ItemFlags[3] = 1;
		newGrenade.HitPoints = 3000;
		newGrenade.ItemFlags[0] = (short)ProjectileType::Explosive;

		if (TestEnvironment(ENV_FLAG_WATER, newGrenade.RoomNumber))
			newGrenade.HitPoints = Random::GenerateInt(5, 15);
	}

	return true;
}

bool TestProjectileNewRoom(ItemInfo& item, const CollisionResult& coll)
{
	// Has projectile changed room?
	if (item.RoomNumber == coll.RoomNumber)
		return false;

	// If currently in water and previously on land, add a ripple.
	if (TestEnvironment(ENV_FLAG_WATER, item.RoomNumber) != TestEnvironment(ENV_FLAG_WATER, coll.RoomNumber))
	{
		int floorDiff = abs(coll.Position.Floor - item.Pose.Position.y);
		int ceilingDiff = abs(coll.Position.Ceiling - item.Pose.Position.y);
		int yPoint = (floorDiff > ceilingDiff) ? coll.Position.Ceiling : coll.Position.Floor;

		SpawnRipple(Vector3(item.Pose.Position.x, yPoint, item.Pose.Position.z), Random::GenerateFloat(8.0f, 16.0f));
	}

	ItemNewRoom(item.Index, coll.RoomNumber);
	return true;
}

void ExplodeProjectile(ItemInfo& item, const Vector3i& prevPos)
{
	if (TestEnvironment(ENV_FLAG_WATER, item.RoomNumber))
	{
		TriggerUnderwaterExplosion(&item, 0);
	}
	else
	{
		TriggerShockwave(&item.Pose, 48, 304, 96, 0, 96, 128, 24, 0, 0);
		item.Pose.Position.y += CLICK(1.0f / 2);
		TriggerExplosionSparks(prevPos.x, prevPos.y, prevPos.z, 3, -2, 0, item.RoomNumber);

		for (int j = 0; j < 2; j++)
			TriggerExplosionSparks(prevPos.x, prevPos.y, prevPos.z, 3, -1, 0, item.RoomNumber);
	}

	AlertNearbyGuards(&item);

	SoundEffect(SFX_TR4_EXPLOSION1, &item.Pose, SoundEnvironment::Land, 0.7f, 0.5f);
	SoundEffect(SFX_TR4_EXPLOSION2, &item.Pose);
}

void HandleProjectile(ItemInfo& item, ItemInfo& emitter, const Vector3i& prevPos, ProjectileType type, int damage)
{
	auto pointColl = GetCollision(&item);

	bool hasHit = false;
	bool hasHitNotByEmitter = false;
	bool isExplosive = type >= ProjectileType::Explosive;
	bool isShatterable = type != ProjectileType::Harpoon;

	if (type < ProjectileType::Grenade)
	{
		// For non-grenade projectiles, check for room collision.

		if (pointColl.Position.Floor < item.Pose.Position.y ||
			pointColl.Position.Ceiling > item.Pose.Position.y)
		{
			item.Pose.Position = prevPos;
			hasHit = hasHitNotByEmitter = true;
		}
	}
	else if (EmitFromProjectile(item, type))
	{
		// If projectile is timed grenade, try to emit from it, according to flags.
		return;
	}

	if (type == ProjectileType::Explosive && item.ItemFlags[3])
	{
		// Fire trail and water collision for grenade fragments
		TriggerFireFlame(item.Pose.Position.x, item.Pose.Position.y, item.Pose.Position.z, -1, 1);
		if (TestEnvironment(ENV_FLAG_WATER, item.RoomNumber))
			hasHit = true;
	}

	TestProjectileNewRoom(item, pointColl);

	// Decrease launch timer (only if projectile is not harpoon or harpoon has hit wall).
	if (type != ProjectileType::Harpoon || hasHit)
		item.HitPoints--;

	// Increase trigger timeout (to prevent emitter from exploding).
	item.ItemFlags[2]++;

	bool doExplosion = false;
	bool doShatter = false;

	// Item has reached EOL; mark it to destroy.
	if (item.HitPoints <= 0)
	{
		doExplosion = isExplosive;
		doShatter = hasHit = true;
	}

	// Store list of objects which were already affected by this explosion.
	auto affectedObjects = std::vector<int>();

	// Step 0: Check for specific collision in small radius.
	// Step 1: If exploding, try smashing all objects in blast radius.

	for (int n = 0; n < 2; n++)
	{
		bool isFirstPass = (n == 0);

		// Projectile is already hit because of room collision, bypass 1st pass.
		if (isFirstPass && hasHit)
		{
			doExplosion = isExplosive;
			doShatter = isShatterable;
			isFirstPass = false;
		}

		// Use bigger radius for second pass, or break if projectile is not explosive.
		int radius = PROJECTILE_HIT_RADIUS;
		if (!isExplosive && !isFirstPass)
			break;
		else if (!isFirstPass)
			radius = PROJECTILE_EXPLODE_RADIUS;

		// Found possible collided items and statics.
		GetCollidedObjects(&item, radius, true, &CollidedItems[0], &CollidedMeshes[0], false);

		// If no collided items and meshes are found, exit the loop.
		if (!CollidedItems[0] && !CollidedMeshes[0])
			break;

		for (int i = 0; i < MAX_COLLIDED_OBJECTS; i++)
		{
			auto* currentMesh = CollidedMeshes[i];
			if (!currentMesh)
				break;

			hasHit = hasHitNotByEmitter = doShatter = true;
			doExplosion = isExplosive;

			if (StaticObjects[currentMesh->staticNumber].shatterType == SHT_NONE)
				continue;

			currentMesh->HitPoints -= damage;
			if (currentMesh->HitPoints <= 0)
				ShatterObject(nullptr, currentMesh, -128, item.RoomNumber, 0);

			if (!isExplosive)
				continue;

			TriggerExplosionSparks(currentMesh->pos.Position.x, currentMesh->pos.Position.y, currentMesh->pos.Position.z, 3, -2, 0, item.RoomNumber);
			auto pose = Pose(currentMesh->pos.Position.x, currentMesh->pos.Position.y - 128, currentMesh->pos.Position.z, 0, currentMesh->pos.Orientation.y, 0);
			TriggerShockwave(&pose, 40, 176, 64, 0, 96, 128, 16, 0, 0);
		}

		for (int i = 0; i < MAX_COLLIDED_OBJECTS; i++)
		{
			auto* currentItem = CollidedItems[i];
			if (!currentItem)
				break;
#
			// Object was already affected by collision, skip it.
			if (std::find(affectedObjects.begin(), affectedObjects.end(), currentItem->Index) != affectedObjects.end())
				continue;

			const auto& currentObject = Objects[currentItem->ObjectNumber];

			if ((currentObject.intelligent && currentObject.collision && currentItem->Status == ITEM_ACTIVE) ||
				currentItem->IsLara() ||
				(currentItem->Flags & 0x40 && Objects[currentItem->ObjectNumber].explodableMeshbits))
			{
				// If we collide with emitter, don't process further in early launch stages.
				if (!hasHitNotByEmitter && currentItem == &emitter)
				{
					// Non-grenade projectiles require larger timeout
					int timeout = type >= ProjectileType::Grenade ? TRIGGER_TIMEOUT : TRIGGER_TIMEOUT * 2;
					if (item.ItemFlags[2] < timeout)
						continue;
				}
				else
					hasHitNotByEmitter = true;

				hasHit = true;
				doShatter = !currentObject.intelligent && !currentItem->IsLara();

				affectedObjects.push_back(currentItem->Index);

				if (isExplosive)
				{
					doExplosion = isExplosive;
					if (type != ProjectileType::FlashGrenade)
						DoExplosiveDamage(emitter, *currentItem, item, damage);
				}
				else if (type == ProjectileType::Poison)
				{
					if (currentItem->IsCreature())
						GetCreatureInfo(currentItem)->Poisoned = true;

					if (currentItem->IsLara())
						GetLaraInfo(currentItem)->PoisonPotency += 5;
				}
				else
				{
					DoDamage(currentItem, damage);
				}

			}
			else if (currentItem->ObjectNumber >= ID_SMASH_OBJECT1 &&
					 currentItem->ObjectNumber <= ID_SMASH_OBJECT8)
			{
				doShatter = hasHit = true;

				// Smash objects are legacy objects from TRC. Let's make them explode in the legacy way.
				ExplodeItemNode(currentItem, 0, 0, 128);
				short currentItemNumber = (currentItem - CollidedItems[0]);
				SmashObject(currentItemNumber);
				KillItem(currentItemNumber);
			}
		}

		if (isFirstPass && !hasHit)
			return;

		if (!isFirstPass)
			break;
	}

	if (!doShatter && !doExplosion && !hasHit)
		return;

	if (doExplosion && isExplosive)
		ExplodeProjectile(item, prevPos);
	else if (doShatter)
		ExplodeItemNode(&item, 0, 0, BODY_EXPLODE);

	switch (type)
	{
	case ProjectileType::FlashGrenade:
		hasHit = false;
		item.ItemFlags[1] = GRENADE_FLASH_TIMEOUT;
		break;

	case ProjectileType::FragGrenade:
		hasHit = false;
		item.ItemFlags[1] = GRENADE_FRAG_TIMEOUT;
		return;
	}

	if (type == ProjectileType::Harpoon)
		return;

	if (hasHit)
		KillItem(item.Index);
}
