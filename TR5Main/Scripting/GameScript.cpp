#include "framework.h"
#include "GameScript.h"

namespace T5M::Script
{
	int lua_exception_handler(lua_State* L, sol::optional<const std::exception&> maybe_exception, sol::string_view description)
	{
		return luaL_error(L, description.data());
	}
}
