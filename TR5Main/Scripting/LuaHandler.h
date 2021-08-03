#pragma once
#include "framework.h"
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
};
