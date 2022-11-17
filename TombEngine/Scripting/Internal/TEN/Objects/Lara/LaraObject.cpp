#include "framework.h"
#include "LaraObject.h"

#include "Game/Lara/lara.h"
#include "Game/Lara/lara_fire.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/Lara/lara_struct.h"
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

/// Set Lara poison potency
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

/// Get poison potency of Lara
// @function LaraObject:GetPoison
// @treturn int current poison value
// @usage
// Lara:GetPoison()
int LaraObject::GetPoison() const
{
	auto* lara = GetLaraInfo(m_item);
	return lara->PoisonPotency;
}

/// Set air value of Lara
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

/// Get air value of Lara
// @function LaraObject:GetAir
// @treturn int current air value
// @usage
// Lara:GetAir()
int LaraObject::GetAir() const
{
	auto* lara = GetLaraInfo(m_item);
	return lara->Air;
}

/// Set sprint energy value of Lara
// @function LaraObject:SetSprintEnergy
// @tparam Sprint Value 
// @usage
// Lara:SetSprintEnergy(120)
// Max Value: 120
void LaraObject::SetSprintEnergy(sol::optional<int> value)
{
	auto* lara = GetLaraInfo(m_item);

	if (value.has_value())
		lara->SprintEnergy = value.value();
	else
		lara->SprintEnergy = LARA_SPRINT_ENERGY_MAX;
}

/// Get sprint energy value of Lara
// @function LaraObject:GetSprintEnergy
// @treturn int current sprint value
// @usage
// Lara:GetSprintEnergy()
int LaraObject::GetSprintEnergy() const
{
	auto* lara = GetLaraInfo(m_item);
	return lara->SprintEnergy;
}

/// Set wetness value of Lara (causes dripping)
// @function LaraObject:SetWet
// @tparam Wet Value 
// @usage
// Lara:SetWet(100)
// Max Value: 255
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
// Lara:GetWet()
// Max Value: 255
int LaraObject::GetWet() const
{
	auto* lara = GetLaraInfo(m_item);
	return lara->Wet[0];
}

/// Get current weapon's ammo count
// @function LaraObject:GetAmmoCount
// @treturn int current ammo count (-1 if infinite)
// @usage
// Lara:GetAmmoCount()
int LaraObject::GetAmmoCount() const
{
	auto* lara = GetLaraInfo(m_item);
	auto& ammo = GetAmmo(Lara, Lara.Control.Weapon.GunType);
	return (ammo.HasInfinite()) ? -1 : (int)ammo.GetCount();
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
			ScriptReserved_GetAmmoCount, &LaraObject::GetAmmoCount,
			sol::base_classes, sol::bases<Moveable>()
		);
}

