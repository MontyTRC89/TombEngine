#pragma once

#include "Scripting/Internal/TEN/Types/Color/Color.h"

namespace sol { class state; }

namespace TEN::Scripting::Types { class ScriptColor; }

using namespace TEN::Scripting::Types;

struct Fog
{
	byte R{ 0 };
	byte G{ 0 };
	byte B{ 0 };
	short MinDistance{ 0 };
	short MaxDistance{ 0 };

	Fog() = default;
	Fog(ScriptColor const& color, short minDistance, short maxDistance);
	void SetColor(ScriptColor const& color);
	ScriptColor GetColor() const;

	static void Register(sol::table&);
};
