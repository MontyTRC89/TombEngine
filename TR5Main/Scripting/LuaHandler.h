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

	bool		ExecuteScript(const std::string & luaFilename, std::string & message);
	bool		ExecuteString(const std::string& command, std::string& message);
};
