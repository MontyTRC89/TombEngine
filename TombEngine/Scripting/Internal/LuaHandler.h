#pragma once
#include "framework.h"

#include "Scripting/Internal/ScriptAssert.h"

class LuaHandler
{
protected:
	sol::state*	m_lua;
	sol::table m_globals;

public:
	LuaHandler(sol::state* lua);
	LuaHandler(const LuaHandler&) = delete;
	LuaHandler& operator=(const LuaHandler&) = delete;
	~LuaHandler() = default;

	void ExecuteScript(const std::string& luaFilename, bool isOptional = false);
	void ExecuteString(const std::string& command);

	void ResetGlobals();

	sol::state* GetState()
	{
		return m_lua;
	};

	template <typename T>void MakeReadOnlyTable(sol::table parent, const std::string& tableName, const T& container)
	{
		// Put all data into metatable.
		auto metatable = tableName + "Meta";
		m_lua->set(metatable, sol::as_table(container));

		auto mtmt = tableName + "MetaMeta";
		auto mtmtTable = m_lua->create_named_table(mtmt);

		// Make metatable's metatable's __index fail an assert to generate warning/error when trying to use missing variable.
		auto lam = [tableName](sol::table tab, std::string const& key)
		{
			ScriptAssertF(false, tableName + " has no member \"" + key +"\"");
		};

		mtmtTable[sol::meta_method::index] = lam;
		m_lua->safe_script("setmetatable(" + metatable + ", " + mtmt + ")");

		// Make metatable's __index refer to itself so that requests to main table will go through to metatable
		// (and thus container's members).
		m_lua->safe_script(metatable + ".__index = " + metatable);
		
		m_lua->safe_script(metatable + ".__type = \"readonly\"");

		// Don't allow table to have new elements put into it.
		m_lua->safe_script(metatable + ".__newindex = function() error('" + tableName + " is read-only') end");

		// Protect metatable.
		m_lua->safe_script(metatable + ".__metatable = 'metatable is protected'");

		auto tab = m_lua->create_named_table(tableName);

		m_lua->safe_script("setmetatable(" + tableName + ", " + metatable + ")");

		// Point initial metatable variable away from its contents. This is just for cleanliness.
		parent.set(tableName, tab);

		m_lua->safe_script(tableName + " = nil");
		m_lua->safe_script(metatable + " = nil");
		m_lua->safe_script(mtmt + " = nil");
	}
};
