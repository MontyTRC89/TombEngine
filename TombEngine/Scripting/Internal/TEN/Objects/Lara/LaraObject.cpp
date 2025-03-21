#include "framework.h"
#include "LaraObject.h"

#include "Game/camera.h"
#include "Game/collision/collide_item.h"
#include "Game/effects/item_fx.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_fire.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/Lara/lara_struct.h"
#include "Objects/Generic/Object/burning_torch.h"
#include "Scripting/Internal/ReservedScriptNames.h"
#include "Scripting/Internal/TEN/Input/ActionIDs.h"
#include "Scripting/Internal/TEN/Objects/Lara/AmmoTypes.h"
#include "Scripting/Internal/TEN/Types/Color/Color.h"
#include "Scripting/Internal/TEN/Types/Rotation/Rotation.h"
#include "Scripting/Internal/TEN/Types/Vec3/Vec3.h"
#include "Specific/Input/Input.h"
#include "Specific/Input/InputAction.h"
#include "Specific/level.h"

using namespace TEN::Input;

/// Class for extra player-only functions.
// Do not try to create an object of this type. Use the built-in *Lara* variable instead.
// LaraObject inherits all the functions of @{Objects.Moveable|Moveable}.
//
// @tenclass Objects.LaraObject
// @pragma nostrip

constexpr auto LUA_CLASS_NAME{ ScriptReserved_LaraObject };
using namespace TEN::Entities::Generic;

/// Set player poison.
// @function LaraObject:SetPoison
// @tparam[opt] int poison Poison strength. Maximum value is 128 (default 0)
// @usage
// Lara:SetPoison(10)
void LaraObject::SetPoison(sol::optional<int> potency)
{
	auto* lara = GetLaraInfo(_moveable);

	if (potency.has_value())
	{
		lara->Status.Poison = std::clamp(potency.value(), 0, (int)LARA_POISON_MAX);
	}
	else
	{
		lara->Status.Poison = 0;
	}
}

/// Get poison potency of Lara.
// @function LaraObject:GetPoison
// @treturn int Current poison potency.
// @usage
// local poisonPotency = Lara:GetPoison()
int LaraObject::GetPoison() const
{
	auto* lara = GetLaraInfo(_moveable);
	return lara->Status.Poison;
}

/// Set air value of Lara.
// @function LaraObject:SetAir
// @tparam int air Air value to give Lara. Maximum value is 1800. 
// @usage
// Lara:SetAir(100)
void LaraObject::SetAir(sol::optional<int> air)
{
	auto* lara = GetLaraInfo(_moveable);

	if (air.has_value())
		lara->Status.Air = std::clamp(air.value(), 0, (int)LARA_AIR_MAX);
	else
		lara->Status.Air = LARA_AIR_MAX;
}

/// Get air value of Lara.
// @function LaraObject:GetAir
// @treturn int Current air value.
// @usage
// local currentAir = Lara:GetAir()
int LaraObject::GetAir() const
{
	auto* lara = GetLaraInfo(_moveable);
	return lara->Status.Air;
}

/// Set wetness value of Lara (causes dripping).
// @function LaraObject:SetWet
// @tparam int wetness Wetness value. Maximum value is 255.
// @usage
// Lara:SetWet(100)
void LaraObject::SetWet(sol::optional<int> wetness)
{
	auto* lara = GetLaraInfo(_moveable);

	float value = wetness.has_value() ? (float)wetness.value() : PLAYER_DRIP_NODE_MAX;
	for (float& i : lara->Effect.DripNodes)
		i = value;
}

/// Get wetness value of Lara.
// @function LaraObject:GetWet
// @treturn int Current wetness value.
// @usage
// local dripAmount = Lara:GetWet()
int LaraObject::GetWet() const
{
	auto* lara = GetLaraInfo(_moveable);
	return lara->Effect.DripNodes[0];
}

/// Set sprint energy value of Lara.
// @function LaraObject:SetStamina
// @tparam int stamina Stamina to give to Lara. Maximum value is 120. 
// @usage
// Lara:SetStamina(120)
void LaraObject::SetStamina(sol::optional<int> value)
{
	auto* lara = GetLaraInfo(_moveable);

	if (value.has_value())
		lara->Status.Stamina = std::clamp(value.value(), 0, (int)LARA_STAMINA_MAX);
	else
		lara->Status.Stamina = LARA_STAMINA_MAX;
}

/// Get stamina value of Lara.
// @function LaraObject:GetStamina
// @treturn int Current sprint value.
// @usage
// local sprintEnergy = Lara:GetStamina()
int LaraObject::GetStamina() const
{
	auto* lara = GetLaraInfo(_moveable);
	return lara->Status.Stamina;
}

/// Get the moveable's airborne status.
// @function Moveable:GetAirborne
// @treturn bool True if Lara state must react to aerial forces.
bool LaraObject::GetAirborne() const
{
	return _moveable->Animation.IsAirborne;
}

/// Set the moveable's airborne status.
// @function Moveable:SetAirborne
// @tparam bool airborne New airborne status for Lara.
void LaraObject::SetAirborne(bool newAirborne)
{
	_moveable->Animation.IsAirborne = newAirborne;
}

/// Lara will undraw her weapon if it is drawn and throw away a flare if she is currently holding one.
// @function LaraObject:UndrawWeapon
// @usage
// Lara:UndrawWeapon()
void LaraObject::UndrawWeapon()
{
	auto* lara = GetLaraInfo(_moveable);

	if (lara->Control.HandStatus != HandStatus::Free ||
		lara->Control.Weapon.GunType == LaraWeaponType::Flare)
	{
		lara->Control.HandStatus = HandStatus::WeaponUndraw;
	}
}

/// Lara will throw away the torch if she currently holds one in her hand.
// @function LaraObject:ThrowAwayTorch
// @usage
// Lara:ThrowAwayTorch()
void LaraObject::ThrowAwayTorch()
{
	auto* lara = GetLaraInfo(_moveable);

	if (lara->Control.Weapon.GunType == LaraWeaponType::Torch)
	{
		Lara.Torch.State = TorchState::Dropping;
	}
}

/// Get actual hand status of Lara.
// @function LaraObject:GetHandStatus
// @usage
// local handStatus = Lara:GetHandStatus()
// @treturn Objects.HandStatus Current hand status.
HandStatus LaraObject::GetHandStatus() const
{
	auto* lara = GetLaraInfo(_moveable);
	return  HandStatus{ lara->Control.HandStatus };
}

/// Get actual weapon type of Lara.
// @function LaraObject:GetWeaponType
// @usage
// local weaponType = Lara:GetWeaponType()
// @treturn Objects.WeaponType Current weapon type.
LaraWeaponType LaraObject::GetWeaponType() const
{
	auto* lara = GetLaraInfo(_moveable);
	return LaraWeaponType{ lara->Control.Weapon.GunType };
}

/// Set Lara weapon type.
// @function LaraObject:SetWeaponType
// @usage
// Lara:SetWeaponType(WeaponType.PISTOLS, false)
// @tparam Objects.WeaponType weaponType New weapon type to set.
// @tparam bool activate If `true`, also draw the weapons or set torch lit. If `false`, keep weapons holstered or leave torch unlit.
void LaraObject::SetWeaponType(LaraWeaponType weaponType, bool activate)
{
	auto* lara = GetLaraInfo(_moveable);

	switch (weaponType)
	{
	case LaraWeaponType::Flare:
		lara->Control.Weapon.RequestGunType = LaraWeaponType::Flare;
		break;

	case LaraWeaponType::Torch:
		GetFlameTorch();
		lara->Torch.IsLit = activate;
		break;

	default:
		if (activate == false)
			lara->Control.Weapon.LastGunType = weaponType;
		else
			lara->Control.Weapon.RequestGunType = weaponType;
		break;
	}
}


/// Get player weapon ammo type.
// @function LaraObject:GetAmmoType
// @treturn Objects.AmmoType Player weapon ammo type.
// @usage
// local CurrentAmmoType = Lara:GetAmmoType()
int LaraObject::GetAmmoType() const
{
	const auto& player = GetLaraInfo(*_moveable);

	auto ammoType = std::optional<PlayerAmmoType>(std::nullopt);
	switch (player.Control.Weapon.GunType)
	{
		case::LaraWeaponType::Pistol:
			ammoType = PlayerAmmoType::Pistol;
			break;

		case::LaraWeaponType::Revolver:
			ammoType = PlayerAmmoType::Revolver;
			break;

		case::LaraWeaponType::Uzi:
			ammoType = PlayerAmmoType::Uzi;
			break;

		case::LaraWeaponType::Shotgun:
			if (player.Weapons[(int)LaraWeaponType::Shotgun].SelectedAmmo == WeaponAmmoType::Ammo1)
			{
				ammoType = PlayerAmmoType::ShotgunNormal;
			}
			else
			{
				ammoType = PlayerAmmoType::ShotgunWide;
			}

			break;

		case::LaraWeaponType::HK:
			ammoType = PlayerAmmoType::HK;
			break;

		case::LaraWeaponType::Crossbow:
			if (player.Weapons[(int)LaraWeaponType::Crossbow].SelectedAmmo == WeaponAmmoType::Ammo1)
			{
				ammoType = PlayerAmmoType::CrossbowBoltNormal;
			}
			else if (player.Weapons[(int)LaraWeaponType::Crossbow].SelectedAmmo == WeaponAmmoType::Ammo2)
			{
				ammoType = PlayerAmmoType::CrossbowBoltPoison;
			}
			else
			{
				ammoType = PlayerAmmoType::CrossbowBoltExplosive;
			}

			break;

		case::LaraWeaponType::GrenadeLauncher:
			if (player.Weapons[(int)LaraWeaponType::GrenadeLauncher].SelectedAmmo == WeaponAmmoType::Ammo1)
			{
				ammoType = PlayerAmmoType::GrenadeNormal;
			}
			else if (player.Weapons[(int)LaraWeaponType::GrenadeLauncher].SelectedAmmo == WeaponAmmoType::Ammo2)
			{
				ammoType = PlayerAmmoType::GrenadeFrag;
			}
			else
			{
				ammoType = PlayerAmmoType::GrenadeFlash;
			}

			break;

		case::LaraWeaponType::HarpoonGun:
			ammoType = PlayerAmmoType::Harpoon;
			break;

		case::LaraWeaponType::RocketLauncher:
			ammoType = PlayerAmmoType::Rocket;
			break;

		default:
			break;
	}

	if (!ammoType.has_value())
	{
		TENLog("GetAmmoType() error; no ammo type.", LogLevel::Warning, LogConfig::All);
		ammoType = PlayerAmmoType::None;
	}

	return (int)*ammoType;
}

/// Get current weapon's ammo count.
// @function LaraObject:GetAmmoCount
// @treturn int Current ammo count (-1 if infinite).
// @usage
// local equippedWeaponAmmoLeft = Lara:GetAmmoCount()
int LaraObject::GetAmmoCount() const
{
	auto* lara = GetLaraInfo(_moveable);
	auto& ammo = GetAmmo(Lara, Lara.Control.Weapon.GunType);
	return (ammo.HasInfinite()) ? -1 : (int)ammo.GetCount();
}

/// Get current vehicle, if it exists.
// @function LaraObject:GetVehicle
// @treturn Objects.Moveable current vehicle (nil if no vehicle present).
// @usage
// local vehicle = Lara:GetVehicle()
std::unique_ptr<Moveable> LaraObject::GetVehicle() const
{
	auto* lara = GetLaraInfo(_moveable);

	if (lara->Context.Vehicle == NO_VALUE)
		return nullptr;

	return std::make_unique<Moveable>(lara->Context.Vehicle);
}

/// Get the player's current targeted moveable (if it exists).
// @function LaraObject:GetTarget
// @treturn Objects.Moveable Target moveable (nil if the player is not currently targeting a moveable).
// @usage
// local target = Lara:GetTarget()
std::unique_ptr<Moveable> LaraObject::GetTarget() const
{
	const auto& player = GetLaraInfo(*_moveable);

	if (player.TargetEntity == nullptr)
		return nullptr;

	return std::make_unique<Moveable>(player.TargetEntity->Index);
}

/// Get the player's current interacted moveable (if it exists).
// @function LaraObject:GetInteractedMoveable
// @treturn Objects.Moveable Interacted moveable (nil if the player is not interacting with a moveable).
// @usage
// local interactedMoveable = Lara:GetInteractedMoveable()
std::unique_ptr<Moveable> LaraObject::GetPlayerInteractedMoveable() const
{
	const auto& player = GetLaraInfo(*_moveable);

	if (player.Context.InteractedItem == NO_VALUE)
		return nullptr;

	return std::make_unique<Moveable>(player.Context.InteractedItem);
}

/// Get current light state of the torch, if it exists
// @function LaraObject:TorchIsLit
// @treturn bool is torch currently lit or not? (false if no torch exists)
// @usage
// local torchIsLit = Lara:TorchIsLit()
bool LaraObject::TorchIsLit() const
{
	auto* lara = GetLaraInfo(_moveable);
	return lara->Torch.IsLit;
}

/// Align player to a moveable object with optional animation, bounds, rotation, and offset parameters.
// @function LaraObject:AlignToMoveable
// @tparam Moveable mov The moveable object to align player with..
// @tparam[opt] int animNumber The animation to play after alignment is complete. __default: BUTTON_PUSH__
// @tparam[opt] Vec3 bound1 The lower boundary for alignment, defining the minimum allowed position. __default: Vec3(-256,-512,0)__
// @tparam[opt] Vec3 bound2 The upper boundary for alignment, defining the maximum allowed position. __default: Vec3(256,0,512)__
// @tparam[opt] Rotation rot1 The minimum rotation bounds for alignment. __default: Rotation(-10,-30,-10)__
// @tparam[opt] Rotation rot2 The maximum rotation bounds for alignment. __default: Rotation(10, 30, 10)__
// @tparam[opt] Vec3 offset The positional offset applied during alignment. __default: Vec3(0,0,312)__
// @tparam[opt] Input.ActionID actionID The action ID that triggers the alignment. __default: Input.ActionID.ACTION__
// @usage
// local Lara:AlignToMoveable(moveable, 197, Vec3(-256,-512,-256), Vec3(256,0,512), Rotation(-10,-30,-10), Rotation(10, 30, 10), Vec3(0,0,312), TEN.Input.ActionID.ACTION)
void LaraObject::AlignToMoveable(Moveable& mov, TypeOrNil<int> animNumber, TypeOrNil<Vec3> bound1, TypeOrNil<Vec3> bound2, TypeOrNil<Rotation> rot1, TypeOrNil<Rotation> rot2, TypeOrNil <Vec3> offset, TypeOrNil<In> actionID) const
{
	auto b1 = ValueOr<Vec3>(bound1, Vec3(-BLOCK(1 / 4), -BLOCK(1 / 2), 0));
	auto b2 = ValueOr<Vec3>(bound2, Vec3(BLOCK(1 / 4), 0, BLOCK(1 / 2)));
	auto r1 = ValueOr<Rotation>(rot1, Rotation(-10, -30, -10));
	auto r2 = ValueOr<Rotation>(rot2, Rotation(10, 30, 10));
	auto position = ValueOr<Vec3>(offset, Vec3(0, 0, 312));
	auto anim = ValueOr<int>(animNumber, LaraAnim::LA_BUTTON_SMALL_PUSH);
	auto key = (InputActionID)ValueOr<In>(actionID, In::Action);

	ObjectCollisionBounds bounds =
	{
	GameBoundingBox(
			b1.x, b2.x,
			b1.y, b2.y,
			b1.z, b2.z
		),
		std::pair(
			EulerAngles(ANGLE(r1.x), ANGLE(r1.y), ANGLE(r1.z)),
			EulerAngles(ANGLE(r2.x), ANGLE(r2.y), ANGLE(r2.z))
		)
	};

	Vector3i pos = position.ToVector3i();

	auto* player = GetLaraInfo(_moveable);
	auto itemIndex = mov.GetIndex();
	auto* item = &g_Level.Items[itemIndex];

	bool isUnderwater = (player->Control.WaterStatus == WaterStatus::Underwater);

	bool isActionActive = player->Control.IsMoving && player->Context.InteractedItem == itemIndex;
	bool isActionReady = IsHeld(key);
	bool isPlayerAvailable = player->Control.HandStatus == HandStatus::Free;

	bool isPlayerIdle = (!isUnderwater && _moveable->Animation.ActiveState == LS_IDLE && _moveable->Animation.AnimNumber == LA_STAND_IDLE) ||
		(isUnderwater && _moveable->Animation.ActiveState == LS_UNDERWATER_IDLE && _moveable->Animation.AnimNumber == LA_UNDERWATER_IDLE);

	if (isActionActive || (isActionReady && isPlayerAvailable && isPlayerIdle))
	{
		if (TestLaraPosition(bounds, item, _moveable))
		{
			if (MoveLaraPosition(pos, item, _moveable))
			{
				ResetPlayerFlex(_moveable);
				SetAnimation(_moveable, anim);
				_moveable->Animation.FrameNumber = GetAnimData(_moveable).frameBase;
				player->Control.IsMoving = false;
				player->Control.HandStatus = HandStatus::Busy;
			}
			else
			{
				player->Context.InteractedItem = itemIndex;
			}

		}

	}
}

/// Tests player to a moveable object with bounds and rotation parameters.
// @function LaraObject:TestPosition
// @tparam Moveable mov The moveable object to align player with..
// @tparam[opt] Vec3 bound1 The lower boundary for alignment, defining the minimum allowed position. __default: Vec3(-256,-512,0)__
// @tparam[opt] Vec3 bound2 The upper boundary for alignment, defining the maximum allowed position. __default: Vec3(256,0,512)__
// @tparam[opt] Rotation rot1 The minimum rotation bounds for test. __default: Rotation(-10,-30,-10)__
// @tparam[opt] Rotation rot2 The maximum rotation bounds for test. __default: Rotation(10, 30, 10)__
bool LaraObject::TestPosition(Moveable& mov, TypeOrNil<Vec3> bound1, TypeOrNil<Vec3> bound2, TypeOrNil<Rotation> rot1, TypeOrNil<Rotation> rot2) const
{

	auto b1 = ValueOr<Vec3>(bound1, Vec3(-BLOCK(1 / 4), -BLOCK(1 / 2), 0));
	auto b2 = ValueOr<Vec3>(bound2, Vec3(BLOCK(1 / 4), 0, BLOCK(1 / 2)));
	auto r1 = ValueOr<Rotation>(rot1, Rotation(-10, -30, -10));
	auto r2 = ValueOr<Rotation>(rot2, Rotation(10, 30, 10));

	ObjectCollisionBounds bounds =
	{
	GameBoundingBox(
			b1.x, b2.x,
			b1.y, b2.y,
			b1.z, b2.z
		),
		std::pair(
			EulerAngles(ANGLE(r1.x), ANGLE(r1.y), ANGLE(r1.z)),
			EulerAngles(ANGLE(r2.x), ANGLE(r2.y), ANGLE(r2.z))
		)
	};

	auto* player = GetLaraInfo(_moveable);
	auto itemIndex = mov.GetIndex();
	auto* item = &g_Level.Items[mov.GetIndex()];

	if (TestLaraPosition(bounds, item, _moveable))
		return true;

	return false;
}


void LaraObject::Register(sol::table& parent)
{
	parent.new_usertype<LaraObject>(LUA_CLASS_NAME,
			ScriptReserved_SetPoison, &LaraObject::SetPoison,
			ScriptReserved_GetPoison, &LaraObject::GetPoison,
			ScriptReserved_SetAir, &LaraObject::SetAir,
			ScriptReserved_GetAir, &LaraObject::GetAir,
			ScriptReserved_SetWet, &LaraObject::SetWet,
			ScriptReserved_GetWet, &LaraObject::GetWet,
			ScriptReserved_SetStamina, &LaraObject::SetStamina,
			ScriptReserved_GetStamina, &LaraObject::GetStamina,
			ScriptReserved_GetAirborne, &LaraObject::GetAirborne,
			ScriptReserved_SetAirborne, &LaraObject::SetAirborne,
			ScriptReserved_UndrawWeapon, &LaraObject::UndrawWeapon,
			ScriptReserved_ThrowAwayTorch, &LaraObject::ThrowAwayTorch,
			ScriptReserved_GetHandStatus, &LaraObject::GetHandStatus,
			ScriptReserved_GetWeaponType, &LaraObject::GetWeaponType,
			ScriptReserved_SetWeaponType, &LaraObject::SetWeaponType,
			ScriptReserved_GetAmmoType, &LaraObject::GetAmmoType,
			ScriptReserved_GetAmmoCount, &LaraObject::GetAmmoCount,
			ScriptReserved_GetVehicle, &LaraObject::GetVehicle,
			ScriptReserved_GetTarget, &LaraObject::GetTarget,
			ScriptReserved_GetPlayerInteractedMoveable, &LaraObject::GetPlayerInteractedMoveable,
			ScriptReserved_TorchIsLit, &LaraObject::TorchIsLit,
			ScriptReserved_AlignToMoveable, &LaraObject::AlignToMoveable,
			ScriptReserved_TestPosition, &LaraObject::TestPosition,
			sol::base_classes, sol::bases<Moveable>()
		);
}
