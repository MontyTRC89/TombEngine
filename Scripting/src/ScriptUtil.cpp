#include "frameworkandsol.h"
#include "ScriptUtil.h"

sol::table MakeSpecialTableBase(sol::state * state, std::string const & name)
{
	std::string metaName{ name + "Meta" };
	auto meta = sol::table{ *state, sol::create };
	state->set(metaName, meta);
	meta.set("__metatable", "\"metatable is protected\"");
	auto tab = state->create_named_table(name);
	tab[sol::metatable_key] = meta;
	state->set(metaName, sol::nil);
	return meta;
}

