#include "framework.h"
#include "Scripting/Internal/TEN/Flow/Customization/Customization.h"
#include "Scripting/Internal/ReservedScriptNames.h"


namespace TEN::Scripting::Customization
{
	void Customizations::Register(sol::table& parent)
	{
		parent.new_usertype<FlareCustomization>(ScriptReserved_CustomFlare,
			sol::constructors<FlareCustomization()>(),

			"color", &FlareCustomization::Color,
			"range", &FlareCustomization::Range,
			"hasSparks", &FlareCustomization::HasSparks,
			"hasSmoke", &FlareCustomization::HasSmoke,
			"timeout", &FlareCustomization::Timeout
		);

		parent.new_usertype<Customizations>(ScriptReserved_Custom,
			sol::constructors<Customizations()>(),
			ScriptReserved_CustomFlare, &Customizations::Flare);
	}
}