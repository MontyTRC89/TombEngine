#pragma once

namespace sol { class state; };

namespace Sound
{
	void Register(sol::state* lua, sol::table& parent);
};
