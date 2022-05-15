#include "framework.h"
#include "ScriptUtil.h"

sol::table MakeSpecialTableBase(sol::state * state, std::string const & name)
{
	std::string metaName{ name + "Meta" };
	auto meta = sol::table{ *state, sol::create };
	state->set(metaName, meta);
	meta.set("__metatable", "\"metatable is protected\"");
	state->safe_script("rawset(_G, \"" + name + "\", {})");
	auto tab = (*state)[name];
	tab[sol::metatable_key] = meta;
	state->set(metaName, sol::nil);
	return meta;
}

