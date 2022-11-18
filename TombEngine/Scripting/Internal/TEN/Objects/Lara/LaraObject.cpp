#include "framework.h"
#include "LaraObject.h"

#include "Game/camera.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_fire.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/Lara/lara_struct.h"
#include "Objects/Generic/Object/burning_torch.h"
#include "Game/effects/lara_fx.h"
#include "Specific/level.h"
#include "ReservedScriptNames.h"

/***
Class for extra Lara-only functions.
Do not try to create an object of this type; use the built-in *Lara* variable instead.

In addition, LaraObject inherits all the functions of @{Objects.Moveable|Moveable}.

@tenclass Objects.LaraObject
@pragma nostrip
*/

constexpr auto LUA_CLASS_NAME{ ScriptReserved_LaraObject };
using namespace TEN::Entities::Generic;

/// Set Lara on fire
// @function LaraObject:SetOnFire
// @bool fire true to set lara on fire, false to extinguish her
void LaraObject::SetOnFire(bool onFire)
{
	//todo add support for other BurnTypes -squidshire 11/11/2022
	auto* lara = GetLaraInfo(m_item);
	if (onFire && !lara->Burn)
	{
		TEN::Effects::Lara::LaraBurn(m_item);
		lara->Burn = true;
		lara->BurnType = BurnType::Normal;
	}
	else if (!onFire)
	{
		lara->Burn = false;
		lara->BurnType = BurnType::None;
	}
}

/// Get whether Lara is on fire or not
// @function LaraObject:GetOnFire
// @treturn bool fire true if lara is on fire, false otherwise
bool LaraObject::GetOnFire() const
{
	auto* lara = GetLaraInfo(m_item);
	return lara->BurnType != BurnType::None;
}

/// Set Lara poison potency
// @function LaraObject:SetPoison
// @tparam[opt] int Potency; maximum value is 64 (default 0)
// @usage
// Lara:SetPoison(10)
void LaraObject::SetPoison(sol::optional<int> potency)
{
	auto* lara = GetLaraInfo(m_item);

	if (potency.has_value())
		lara->PoisonPotency = std::clamp(potency.value(), 0, (int)LARA_POISON_POTENCY_MAX);
	else
		lara->PoisonPotency = 0;
}

/// Get poison potency of Lara
// @function LaraObject:GetPoison
// @treturn int current poison potency
// @usage
// local poisonPotency = Lara:GetPoison()
int LaraObject::GetPoison() const
{
	auto* lara = GetLaraInfo(m_item);
	return lara->PoisonPotency;
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
		lara->Air = std::clamp(air.value(), 0, (int)LARA_AIR_MAX);
	else
		lara->Air = LARA_AIR_MAX;
}

/// Get air value of Lara
// @function LaraObject:GetAir
// @treturn int current air value
// @usage
// local currentAir = Lara:GetAir()
int LaraObject::GetAir() const
{
	auto* lara = GetLaraInfo(m_item);
	return lara->Air;
}

/// Set sprint energy value of Lara
// @function LaraObject:SetSprintEnergy
// @tparam int sprint energy to give to Lara; maximum value is 120. 
// @usage
// Lara:SetSprintEnergy(120)
void LaraObject::SetSprintEnergy(sol::optional<int> value)
{
	auto* lara = GetLaraInfo(m_item);

	if (value.has_value())
		lara->SprintEnergy = std::clamp(value.value(), 0, (int)LARA_SPRINT_ENERGY_MAX);
	else
		lara->SprintEnergy = LARA_SPRINT_ENERGY_MAX;
}

/// Get sprint energy value of Lara
// @function LaraObject:GetSprintEnergy
// @treturn int current sprint value
// @usage
// local sprintEnergy = Lara:GetSprintEnergy()
int LaraObject::GetSprintEnergy() const
{
	auto* lara = GetLaraInfo(m_item);
	return lara->SprintEnergy;
}

/// Set wetness value of Lara (causes dripping)
// @function LaraObject:SetWet
// @tparam int Wetness value. Maximum 255 
// @usage
// Lara:SetWet(100)
void LaraObject::SetWet(sol::optional<int> wetness)
{
	auto* lara = GetLaraInfo(m_item);

	unsigned char value = wetness.has_value() ? (unsigned char)wetness.value() : UCHAR_MAX;
	for (unsigned char& i : lara->Wet)
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
	return lara->Wet[0];
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
// @treturn 0=HandsFree, 1=Busy(climbing,etc), 2=WeaponDraw, 3=WeaponUndraw, 4=WeaponInHand.
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
// @treturn 0=None, 1=Pistols, 2=Revolver, 3=Uzi, 4=Shotgun, 5=HK, 6=Crossbow, 7=Flare, 8=Torch, 9=GrenadeLauncher, 10=Harpoon, 11=RocketLauncher.
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
		if (activate == false)
		{
			GetFlameTorch();
			lara->Torch.IsLit = false;
		}
		else
		{
			GetFlameTorch();
			lara->Torch.IsLit = true;
		}
		break;
	default:
		if (activate == false)
			lara->Control.Weapon.LastGunType = weaponType;
		else
			lara->Control.Weapon.RequestGunType = weaponType;
		break;
	}
}

void LaraObject::Register(sol::table& parent)
{
	parent.new_usertype<LaraObject>(LUA_CLASS_NAME,
			ScriptReserved_SetOnFire, &LaraObject::SetOnFire,
			ScriptReserved_GetOnFire, &LaraObject::GetOnFire,
			ScriptReserved_SetPoison, &LaraObject::SetPoison,
			ScriptReserved_GetPoison, &LaraObject::GetPoison,
			ScriptReserved_SetAir, &LaraObject::SetAir,
			ScriptReserved_GetAir, &LaraObject::GetAir,
			ScriptReserved_SetWet, &LaraObject::SetWet,
			ScriptReserved_GetWet, &LaraObject::GetWet,
			ScriptReserved_SetSprintEnergy, &LaraObject::SetSprintEnergy,
			ScriptReserved_GetSprintEnergy, &LaraObject::GetSprintEnergy,
			ScriptReserved_UndrawWeapon, &LaraObject::UndrawWeapon,
			ScriptReserved_ThrowAwayTorch, &LaraObject::ThrowAwayTorch,
			ScriptReserved_GetHandStatus, &LaraObject::GetHandStatus,
			ScriptReserved_GetWeaponType, &LaraObject::GetWeaponType,
			ScriptReserved_SetWeaponType, &LaraObject::SetWeaponType,
			ScriptReserved_GetAmmoCount, &LaraObject::GetAmmoCount,
			sol::base_classes, sol::bases<Moveable>()
		);
}

