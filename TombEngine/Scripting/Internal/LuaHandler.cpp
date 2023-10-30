#pragma once
#include "framework.h"

#include <filesystem>
#include "Scripting/Internal/LuaHandler.h"

LuaHandler::LuaHandler(sol::state* lua) : m_lua{ lua }
{
}

void LuaHandler::ResetGlobals()
{
	auto mt = sol::table{ *m_lua, sol::create };
	m_globals = sol::table{ *m_lua, sol::create };
	mt.set(sol::meta_function::new_index, m_globals);
	mt.set(sol::meta_function::index, m_globals);

	m_lua->set(sol::metatable_key, mt);
}

void LuaHandler::ExecuteScript(const std::string& luaFilename, bool isOptional)
{
	if (isOptional && !std::filesystem::is_regular_file(luaFilename))
		return;

	auto result = m_lua->safe_script_file(luaFilename, sol::script_pass_on_error);
	if (!result.valid())
	{
		sol::error error = result;
		throw TENScriptException{ error.what() };
	}
}

void LuaHandler::ExecuteString(const std::string& command)
{
	auto result = m_lua->safe_script(command, sol::environment(m_lua->lua_state(), sol::create, m_lua->globals()), sol::script_pass_on_error);
	if (!result.valid())
	{
		sol::error error = result;
		throw TENScriptException{ error.what() };
	}
}
