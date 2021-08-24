#pragma once
#define index_error_maker(CPP_TYPE, LUA_CLASS_NAME) [](CPP_TYPE & item, sol::object key) \
{ \
	std::string err = "Attempted to read non-existant var \"" + key.as<std::string>() + "\" from " + LUA_CLASS_NAME; \
	ScriptAssert(false, err);\
}

#define newindex_error_maker(CPP_TYPE, LUA_CLASS_NAME) [](CPP_TYPE & item, sol::object key) \
{ \
	std::string err = "Attempted to set non-existant var \"" + key.as<std::string>() + "\" of " + LUA_CLASS_NAME; \
	ScriptAssert(false, err);\
}

