#pragma once
#include "frameworkandsol.h"
#include "LuaHandler.h"

LuaHandler::LuaHandler(sol::state* lua) : m_lua{ lua }
{}

void LuaHandler::ExecuteScript(std::string const& luaFilename) {
	auto result = m_lua->safe_script_file(luaFilename, sol::script_pass_on_error);
	if (!result.valid())
	{
		sol::error error = result;
		throw TENScriptException{ error.what() };
	}
}

void LuaHandler::ExecuteString(std::string const & command) {
	auto result = m_lua->safe_script(command, sol::environment(m_lua->lua_state(), sol::create, m_lua->globals()), sol::script_pass_on_error);
	if (!result.valid())
	{
		sol::error error = result;
		throw TENScriptException{ error.what() };
	}
}
