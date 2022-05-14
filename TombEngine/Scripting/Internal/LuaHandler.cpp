#pragma once
#include "framework.h"
#include "LuaHandler.h"

LuaHandler::LuaHandler(sol::state* lua) : m_lua{ lua }
{
	ResetGlobals();
}

void LuaHandler::ResetGlobals()
{
	sol::table mt = sol::table{ *m_lua, sol::create };
	m_globals = sol::table{ *m_lua, sol::create };
	mt.set(sol::meta_function::new_index, m_globals);
	mt.set(sol::meta_function::index, m_globals);

	m_lua->set(sol::metatable_key, mt);
}

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
