#pragma once

#include "Color/Color.h"

namespace sol {
	class state;
}

struct Fog
{
	bool Enabled{ false };
	byte R{ 0 };
	byte G{ 0 };
	byte B{ 0 };
	short MinDistance{ 0 };
	short MaxDistance{ 0 };

	Fog() = default;
	Fog(ScriptColor const& col, short minDistance, short maxDistance);
	void SetColor(ScriptColor const& col);
	ScriptColor GetColor() const;

	static void Register(sol::table &);
};
