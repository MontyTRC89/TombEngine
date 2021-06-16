#pragma once
#include "framework.h"
#include "LuaHandler.h"

LuaHandler::LuaHandler(sol::state* lua) {
	m_lua = lua;
}

bool	LuaHandler::ExecuteScript(std::string const& luaFilename, std::string & message) {
	auto result = m_lua->safe_script_file(luaFilename, sol::environment(m_lua->lua_state(), sol::create, m_lua->globals()), sol::script_pass_on_error);
	if (!result.valid())
	{
		sol::error error = result;
		message = error.what();
		return false;
	}
	return true;

}

bool	LuaHandler::ExecuteString(std::string const & command, std::string& message) {
	auto result = m_lua->safe_script(command, sol::environment(m_lua->lua_state(), sol::create, m_lua->globals()), sol::script_pass_on_error);
	if (!result.valid())
	{
		sol::error error = result;
		message = error.what();
		return false;
	}
	return true;

}
