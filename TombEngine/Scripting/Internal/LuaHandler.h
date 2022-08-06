#pragma once
#include "framework.h"
#include "ScriptAssert.h"

class LuaHandler {
protected:
	sol::state*	m_lua;
	sol::table m_globals;

public:
	LuaHandler(sol::state* lua);
	LuaHandler(LuaHandler const &) = delete;
	LuaHandler& operator=(LuaHandler const &) = delete;
	~LuaHandler() = default;

	void ExecuteScript(const std::string & luaFilename);
	void ExecuteString(const std::string & command);

	void ResetGlobals();

	sol::state* GetState() {
		return m_lua;
	};

	template <typename T>void MakeReadOnlyTable(sol::table & parent, std::string const& tableName, T const& container)
	{
		auto mt = tableName + "Meta";
		// Put all the data in the metatable	
		m_lua->set(mt, sol::as_table(container));

		auto mtmt = tableName + "MetaMeta";
		auto mtmtTable = m_lua->create_named_table(mtmt);

		// Make the metatable's metatable's __index fail an assert so that trying to use a variable
		// that doesn't exist will generate a warning or error.
		auto lam = [tableName](sol::table tab, std::string const& key)
		{
			ScriptAssertF(false, tableName + " has no member \"" + key +"\"");
		};

		mtmtTable[sol::meta_method::index] = lam;
		m_lua->safe_script("setmetatable(" + mt + ", " + mtmt + ")");

		// Make the metatable's __index refer to itself so that requests
		// to the main table will go through to the metatable (and thus the
		// container's members)
		m_lua->safe_script(mt + ".__index = " + mt);
		
		m_lua->safe_script(mt + ".__type = \"readonly\"");

		// Don't allow the table to have new elements put into it
		m_lua->safe_script(mt + ".__newindex = function() error('" + tableName + " is read-only') end");

		// Protect the metatable
		m_lua->safe_script(mt + ".__metatable = 'metatable is protected'");

		auto tab = m_lua->create_named_table(tableName);

		m_lua->safe_script("setmetatable(" + tableName + ", " + mt + ")");

		// point the initial metatable variable away from its contents. this is just for cleanliness
		parent.set(tableName, tab);

		m_lua->safe_script(tableName + " = nil");
		m_lua->safe_script(mt + " = nil");
		m_lua->safe_script(mtmt + " = nil");
	}
};
