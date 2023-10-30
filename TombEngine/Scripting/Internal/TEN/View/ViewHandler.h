#pragma once

namespace sol { class state; };

namespace View
{
	void Register(sol::state* lua, sol::table& parent);
};
