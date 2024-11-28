#pragma once

#include <string>
#include "Scripting/Internal/ScriptAssert.h"
#include "Scripting/Internal/TEN/Color/Color.h"
#include "Specific/clock.h"

namespace sol
{
	class state;
}

namespace TEN::Scripting::Customization
{
	struct FlareCustomization
	{
		ScriptColor Color = ScriptColor(255, 180, 0);
		bool Sparks = true;
		bool Smoke = true;
		bool Lensflare = false;
		bool Flicker = true;
		int  Timeout = 60 * FPS;
		int  Range = 9;
	};

	struct Customizations
	{
		FlareCustomization Flare = {};

		static void Register(sol::table&);
	};
}