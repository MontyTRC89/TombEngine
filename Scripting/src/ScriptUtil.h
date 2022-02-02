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

template <typename funcIndex, typename funcNewindex, typename obj>
static void MakeSpecialTable(sol::state * state, std::string const & name, funcIndex const & fi, funcNewindex const & fni, obj objPtr)
{
	std::string metaName{ name + "Meta" };
	auto meta = sol::table{ *state, sol::create };
	state->set(metaName, meta);
	meta.set("__metatable", "\"metatable is protected\"");
	auto tab = state->create_named_table(name);
	tab[sol::metatable_key] = meta;
	state->set(metaName, sol::nil);
	meta.set_function("__index", fi, objPtr);
	meta.set_function("__newindex", fni, objPtr);
}

