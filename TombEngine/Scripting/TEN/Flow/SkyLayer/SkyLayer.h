#pragma once

#include "Color/Color.h"

namespace sol {
	class state;
}

struct SkyLayer
{
	bool Enabled{ false };
	byte R{ 0 };
	byte G{ 0 };
	byte B{ 0 };
	short CloudSpeed{ 0 };

	SkyLayer() = default;
	SkyLayer(ScriptColor const & col, short speed);
	void SetColor(ScriptColor const & col);
	ScriptColor GetColor() const;

	static void Register(sol::table &);
};

