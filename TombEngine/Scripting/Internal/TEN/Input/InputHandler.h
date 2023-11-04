#pragma once

namespace sol { class state; };

namespace Input
{
	void Register(sol::state* lua, sol::table& parent);
};
