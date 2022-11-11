#include "framework.h"
#include "LaraObject.h"

#include "lara_fx.h"
#include "lara_helpers.h"
#include "lara_struct.h"
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
	auto lara = GetLaraInfo(m_item);
	if (onFire && lara->BurnType == BurnType::None )
	{
		TEN::Effects::Lara::LaraBurn(m_item);
		lara->BurnType = BurnType::Normal;
	}
	else if(!onFire)
	{
		lara->BurnType = BurnType::None;
	}
}

/// Get whether Lara is on fire or not
// @function LaraObject:GetOnFire
// @treturn bool fire true if lara is on fire, false otherwise
bool LaraObject::GetOnFire() const
{
	auto lara = GetLaraInfo(m_item);
	return lara->BurnType != BurnType::None;
}

void LaraObject::Register(sol::table& parent)
{
	parent.new_usertype<LaraObject>(LUA_CLASS_NAME,
			ScriptReserved_SetOnFire, &LaraObject::SetOnFire,
			ScriptReserved_GetOnFire, &LaraObject::GetOnFire,
			sol::base_classes, sol::bases<Moveable>()
		);
}

