#pragma once

namespace sol { class state; };

namespace Util
{
	void Register(sol::state* lua, sol::table& parent);
};
