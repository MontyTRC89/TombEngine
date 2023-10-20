#include "framework.h"
#include "LaraObject.h"

#include "Game/camera.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_fire.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/Lara/lara_struct.h"
#include "Objects/Generic/Object/burning_torch.h"
#include "Game/effects/item_fx.h"
#include "Specific/level.h"
#include "Scripting/Internal/ReservedScriptNames.h"
#include "Scripting/Internal/TEN/Objects/Lara/AmmoTypes.h"

/***
Class for extra Lara-only functions.
Do not try to create an object of this type; use the built-in *Lara* variable instead.

In addition, LaraObject inherits all the functions of @{Objects.Moveable|Moveable}.

@tenclass Objects.LaraObject
@pragma nostrip
*/

constexpr auto LUA_CLASS_NAME{ ScriptReserved_LaraObject };
using namespace TEN::Entities::Generic;

/// Set Lara poison
// @function LaraObject:SetPoison
// @tparam[opt] int Poison; maximum value is 128 (default 0)
// @usage
// Lara:SetPoison(10)
void LaraObject::SetPoison(sol::optional<int> potency)
{
	auto* lara = GetLaraInfo(m_item);

	if (potency.has_value())
		lara->Status.Poison = std::clamp(potency.value(), 0, (int)LARA_POISON_MAX);
	else
		lara->Status.Poison = 0;
}

/// Get poison potency of Lara
// @function LaraObject:GetPoison
// @treturn int current poison potency
// @usage
// local poisonPotency = Lara:GetPoison()
int LaraObject::GetPoison() const
{
	auto* lara = GetLaraInfo(m_item);
	return lara->Status.Poison;
}

/// Set air value of Lara
// @function LaraObject:SetAir
// @tparam int Air value to give Lara. Maximum value is 1800. 
// @usage
// Lara:SetAir(100)
void LaraObject::SetAir(sol::optional<int> air)
{
	auto* lara = GetLaraInfo(m_item);

	if (air.has_value())
		lara->Status.Air = std::clamp(air.value(), 0, (int)LARA_AIR_MAX);
	else
		lara->Status.Air = LARA_AIR_MAX;
}

/// Get air value of Lara
// @function LaraObject:GetAir
// @treturn int current air value
// @usage
// local currentAir = Lara:GetAir()
int LaraObject::GetAir() const
{
	auto* lara = GetLaraInfo(m_item);
	return lara->Status.Air;
}

/// Set wetness value of Lara (causes dripping)
// @function LaraObject:SetWet
// @tparam int Wetness value. Maximum 255 
// @usage
// Lara:SetWet(100)
void LaraObject::SetWet(sol::optional<int> wetness)
{
	auto* lara = GetLaraInfo(m_item);

	float value = wetness.has_value() ? (float)wetness.value() : PLAYER_DRIP_NODE_MAX;
	for (float& i : lara->Effect.DripNodes)
		i = value;
}

/// Get wetness value of Lara
// @function LaraObject:GetWet
// @treturn int current wetness value
// @usage
// local dripAmount = Lara:GetWet()
int LaraObject::GetWet() const
{
	auto* lara = GetLaraInfo(m_item);
	return lara->Effect.DripNodes[0];
}

/// Set sprint energy value of Lara
// @function LaraObject:SetStamina
// @tparam int stamina to give to Lara; maximum value is 120. 
// @usage
// Lara:SetStamina(120)
void LaraObject::SetStamina(sol::optional<int> value)
{
	auto* lara = GetLaraInfo(m_item);

	if (value.has_value())
		lara->Status.Stamina = std::clamp(value.value(), 0, (int)LARA_STAMINA_MAX);
	else
		lara->Status.Stamina = LARA_STAMINA_MAX;
}

/// Get stamina value of Lara
// @function LaraObject:GetStamina
// @treturn int current sprint value
// @usage
// local sprintEnergy = Lara:GetStamina()
int LaraObject::GetStamina() const
{
	auto* lara = GetLaraInfo(m_item);
	return lara->Status.Stamina;
}

/// Set control lock for Lara.
// @function LaraObject:SetControlLock
// @tparam bool whether set or not set control lock. 
// @usage
// Lara:SetControlLock(true)
void LaraObject::SetControlLock(bool value)
{
	GetLaraInfo(m_item)->Control.Locked = value;
}

/// Get control lock for Lara.
// @function LaraObject:GetControlLock
// @treturn bool current control lock value
// @usage
// local areControlsLocked = Lara:GetControlLock()
bool LaraObject::GetControlLock() const
{
	return GetLaraInfo(m_item)->Control.Locked;
}

/// Get the moveable's airborne status
// @function Moveable:GetAirborne
// @treturn (bool) true if Lara state must react to aerial forces.
bool LaraObject::GetAirborne() const
{
	return m_item->Animation.IsAirborne;
}

/// Set the moveable's airborne status
// @function Moveable:SetAirborne
// @tparam (bool) New airborn status for Lara.
void LaraObject::SetAirborne(bool newAirborne)
{
	m_item->Animation.IsAirborne = newAirborne;
}

/// Lara will undraw her weapon if it is drawn and throw away a flare if she is currently holding one.
// @function LaraObject:UndrawWeapon
// @usage
// Lara:UndrawWeapon()
void LaraObject::UndrawWeapon()
{
	auto* lara = GetLaraInfo(m_item);

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
	auto* lara = GetLaraInfo(m_item);

	if (lara->Control.Weapon.GunType == LaraWeaponType::Torch)
	{
		Lara.Torch.State = TorchState::Dropping;
	}
}

//todo make these into enums - Squidshire 18/11/2022

/// Get actual hand status of Lara
// @function LaraObject:GetHandStatus
// @usage
// local handStatus = Lara:GetHandStatus()
// @treturn int hand status 0=HandsFree, 1=Busy(climbing,etc), 2=WeaponDraw, 3=WeaponUndraw, 4=WeaponInHand.
HandStatus LaraObject::GetHandStatus() const
{
	auto* lara = GetLaraInfo(m_item);
	return  HandStatus{ lara->Control.HandStatus };
}

//todo make these into enums - Squidshire 18/11/2022

/// Get actual weapon type of Lara
// @function LaraObject:GetWeaponType
// @usage
// local weaponType = Lara:GetWeaponType()
// @treturn int weapon type 0=None, 1=Pistols, 2=Revolver, 3=Uzi, 4=Shotgun, 5=HK, 6=Crossbow, 7=Flare, 8=Torch, 9=GrenadeLauncher, 10=Harpoon, 11=RocketLauncher.
LaraWeaponType LaraObject::GetWeaponType() const
{
	auto* lara = GetLaraInfo(m_item);
	return LaraWeaponType{ lara->Control.Weapon.GunType };
}

/// Set Lara weapon type
// @function LaraObject:SetWeaponType
// @usage
// Lara:SetWeaponType(LaraWeaponType.PISTOLS, false)
// @tparam LaraWeaponType weaponType
// Must be one of:
//	NONE
//	PISTOLS
//	REVOLVER
//	UZI
//	SHOTGUN
//	HK
//	CROSSBOW
//	FLARE
//	TORCH
//	GRENADELAUNCHER
//	HARPOONGUN
//	ROCKETLAUNCHER
// @tparam bool activate true = let her also draw the weapons, set torch lit. false = let Laras new weapons remain holstered until she draws them, set torch unlit.
void LaraObject::SetWeaponType(LaraWeaponType weaponType, bool activate)
{
	auto* lara = GetLaraInfo(m_item);

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
// @treturn int player weapon ammo type
// @usage
// local CurrentAmmoType = Lara:GetAmmoType()
int LaraObject::GetAmmoType() const
{
	const auto& player = GetLaraInfo(*m_item);

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

/// Get current weapon's ammo count
// @function LaraObject:GetAmmoCount
// @treturn int current ammo count (-1 if infinite)
// @usage
// local equippedWeaponAmmoLeft = Lara:GetAmmoCount()
int LaraObject::GetAmmoCount() const
{
	auto* lara = GetLaraInfo(m_item);
	auto& ammo = GetAmmo(Lara, Lara.Control.Weapon.GunType);
	return (ammo.HasInfinite()) ? -1 : (int)ammo.GetCount();
}

/// Get current vehicle, if it exists
// @function LaraObject:GetVehicle
// @treturn Objects.Moveable current vehicle (nil if no vehicle present)
// @usage
// local vehicle = Lara:GetVehicle()
std::unique_ptr<Moveable> LaraObject::GetVehicle() const
{
	auto* lara = GetLaraInfo(m_item);

	if (lara->Context.Vehicle == NO_ITEM)
		return nullptr;

	return std::make_unique<Moveable>(lara->Context.Vehicle);
}

/// Get current target enemy, if it exists
// @function LaraObject:GetTarget
// @treturn Objects.Moveable current target enemy (nil if no target present)
// @usage
// local target = Lara:GetTarget()
std::unique_ptr<Moveable> LaraObject::GetTarget() const
{
	auto* lara = GetLaraInfo(m_item);

	if (lara->TargetEntity == nullptr)
		return nullptr;

	return std::make_unique<Moveable>(lara->TargetEntity->Index);
}

/// Get current light state of the torch, if it exists
// @function LaraObject:TorchIsLit
// @treturn bool is torch currently lit or not? (false if no torch exists)
// @usage
// local torchIsLit = Lara:TorchIsLit()
bool LaraObject::TorchIsLit() const
{
	auto* lara = GetLaraInfo(m_item);
	return lara->Torch.IsLit;
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
			ScriptReserved_SetControlLock, &LaraObject::SetControlLock,
			ScriptReserved_GetControlLock, &LaraObject::GetControlLock,
			ScriptReserved_UndrawWeapon, &LaraObject::UndrawWeapon,
			ScriptReserved_ThrowAwayTorch, &LaraObject::ThrowAwayTorch,
			ScriptReserved_GetHandStatus, &LaraObject::GetHandStatus,
			ScriptReserved_GetWeaponType, &LaraObject::GetWeaponType,
			ScriptReserved_SetWeaponType, &LaraObject::SetWeaponType,
			ScriptReserved_GetAmmoType, &LaraObject::GetAmmoType,
			ScriptReserved_GetAmmoCount, &LaraObject::GetAmmoCount,
			ScriptReserved_GetVehicle, &LaraObject::GetVehicle,
			ScriptReserved_GetTarget, &LaraObject::GetTarget,
			ScriptReserved_TorchIsLit, &LaraObject::TorchIsLit,
			sol::base_classes, sol::bases<Moveable>()
		);
}

