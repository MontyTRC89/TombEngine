#pragma once
#include "framework.h"
#include <sol.hpp>

class LuaHandler {
protected:
	sol::state*	m_lua;
	~LuaHandler() = default;

public:
	LuaHandler(sol::state* lua);
	LuaHandler(LuaHandler const &) = delete;
	LuaHandler& operator=(LuaHandler const &) = delete;

	void ExecuteScript(const std::string & luaFilename);
	void ExecuteString(const std::string & command);

	template <typename T>void MakeReadOnlyTable(std::string const& tableName, T const& container)
	{
		auto mt = tableName + "Meta";
		// Put all the data in the metatable	
		m_lua->set(mt, sol::as_table(container));

		auto mtmt = tableName + "MetaMeta";
		m_lua->create_named_table(mtmt);

		// TODO Make these not raise exceptions by default but just do ScriptAsserts -- squidshire, 29/08/2021
		// Make the metatable's metatable's __index throw an error so that trying to use a variable
		// that doesn't exist just errors.
		m_lua->safe_script(mtmt + ".__index = function(t, key) error('" + tableName +" has no member \"' .. tostring(key) .. '\"')  end");
		m_lua->safe_script("setmetatable(" + mt + ", " + mtmt + ")");

		// Make the metatable's __index refer to itself so that requests
		// to the main table will go through to the metatable (and thus the
		// container's members)
		m_lua->safe_script(mt + ".__index = " + mt);

		// Don't allow the table to have new elements put into it
		m_lua->safe_script(mt + ".__newindex = function() error('" + tableName + " is read-only') end");

		// Protect the metatable
		m_lua->safe_script(mt + ".__metatable = 'metatable is protected'");

		m_lua->create_named_table(tableName);

		m_lua->safe_script("setmetatable(" + tableName + ", " + mt + ")");
		// point the initial metatable variable away from its contents. this is just for cleanliness
		m_lua->safe_script(mt + " = nil");
		m_lua->safe_script(mtmt + " = nil");
	}
};
