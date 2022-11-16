#include "framework.h"
#include "LaraObject.h"

#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/Lara/lara_struct.h"
#include "Objects/Generic/Object/burning_torch.h"
#include "Game/effects/lara_fx.h"
#include "ReservedScriptNames.h"

/***
Class for extra Lara-only functions.
Do not try to create an object of this type; use the built-in *Lara* variable instead.

In addition, LaraObject inherits all the functions of @{Objects.Moveable|Moveable}.

@tenclass Objects.LaraObject
@pragma nostrip
*/

constexpr auto LUA_CLASS_NAME{ ScriptReserved_LaraObject };

/// Set Lara on fire
// @function LaraObject:SetOnFire
// @bool fire true to set lara on fire, false to extinguish her
void LaraObject::SetOnFire(bool onFire)
{
	//todo add support for other BurnTypes -squidshire 11/11/2022
	auto* lara = GetLaraInfo(m_item);
	if (onFire && lara->BurnType == BurnType::None)
	{
		TEN::Effects::Lara::LaraBurn(m_item);
		lara->BurnType = BurnType::Normal;
	}
	else if (!onFire)
	{
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

/// Set Poison with potency of poision
// @function LaraObject:SetPoison
// @tparam Potency Value (optional, resets to 0 if not provided)
// @usage
// Lara:SetPoison(10)
// Max Value: 64
void LaraObject::SetPoison(sol::optional<int> potency)
{
	auto* lara = GetLaraInfo(m_item);

	if (potency.has_value())
		lara->PoisonPotency = potency.value();
	else
		lara->PoisonPotency = 0;
}

/// Get Poison potency of Lara
// @function LaraObject:GetPoison
// @usage
// Lara:GetPoison()
int LaraObject::GetPoison() const
{
	auto* lara = GetLaraInfo(m_item);
	return lara->PoisonPotency;
}

/// Set Air of Lara
// @function LaraObject:SetAir
// @tparam Air Value 
// @usage
// Lara:SetAir(100)
// Max Value: 1800
void LaraObject::SetAir(sol::optional<int> air)
{
	auto* lara = GetLaraInfo(m_item);

	if (air.has_value())
		lara->Air = air.value();
	else
		lara->Air = LARA_AIR_MAX;
}

/// Get Air value of Lara
// @function LaraObject:GetAir
// @usage
// Lara:GetAir()
int LaraObject::GetAir() const
{
	auto* lara = GetLaraInfo(m_item);
	return lara->Air;
}

//Lara will undraw her weapons if they are drawn, throw away flare if she helds one in her hand.
// @function LaraObject:UndrawWeapons
void LaraObject::UndrawWeapons()
{
	auto* lara = GetLaraInfo(m_item);

	if (lara->Control.HandStatus != HandStatus::Free ||
		lara->Control.Weapon.GunType == LaraWeaponType::Flare)
	{
		lara->Control.HandStatus = HandStatus::WeaponUndraw;
	}
}

//Lara will throw away the torch if she helds one in her hand.
// @function LaraObject:ThrowAwayTorch
void LaraObject::ThrowAwayTorch()
{
	auto* lara = GetLaraInfo(m_item);

	if (lara->Control.Weapon.GunType == LaraWeaponType::Torch)
	{
		Lara.Torch.State = TorchState::Dropping;
	}
}

// @function LaraObject:GetLaraHandStatus
// @treturn 0=HandsFree, 1=Busy(climbing,etc), 2=WeaponDraw, 3=WeaponUndraw, 4=WeaponInHand.
HandStatus LaraObject::GetLaraHandStatus() const
{
	auto* lara = GetLaraInfo(m_item);
	return  HandStatus{ lara->Control.HandStatus };
}

// @function LaraObject:GetLaraWeaponType
// @treturn 0=None, 1=Pistols, 2=Revolver, 3=Uzi, 4=Shotgun, 5=HK, 6=Crossbow, 7=Flare, 8=Torch, 9=GrenadeLauncher, 10=Harpoon, 11=RocketLauncher.
LaraWeaponType LaraObject::GetLaraWeaponType() const
{
	auto* lara = GetLaraInfo(m_item);
	return LaraWeaponType{ lara->Control.Weapon.GunType };
}

//Set Lara Weapon type
// @function LaraObject:SetLaraWeaponType
// 0=None, 1=Pistols, 2=Revolver, 3=Uzi, 4=Shotgun, 5=HK, 6=Crossbow, 7=Flare, 8=Torch, 9=GrenadeLauncher, 10=Harpoon, 11=RocketLauncher.
void LaraObject::SetLaraWeaponType(LaraWeaponType weaponType, bool activate)
{
	auto* lara = GetLaraInfo(m_item);

	switch (weaponType)
	{
	case LaraWeaponType::Flare:
		lara->Control.Weapon.RequestGunType = LaraWeaponType::Flare;
		break;
	case LaraWeaponType::Torch:
		using namespace TEN::Entities::Generic;
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
			ScriptReserved_UndrawWeapons, &LaraObject::UndrawWeapons,
			ScriptReserved_ThrowAwayTorch, &LaraObject::ThrowAwayTorch,
			ScriptReserved_GetLaraHandStatus, &LaraObject::GetLaraHandStatus,
			ScriptReserved_GetLaraWeaponType, &LaraObject::GetLaraWeaponType,
			ScriptReserved_SetLaraWeaponType, &LaraObject::SetLaraWeaponType,
			sol::base_classes, sol::bases<Moveable>()
		);
}

