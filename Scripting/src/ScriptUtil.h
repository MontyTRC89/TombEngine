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

// Helper type to allow us to more easily specify "give a value of type X or just give nil" parameters.
// Sol doesn't (at the time of writing) have any mechanisms to do this kind of optional argument without
// drawbacks, or at least no mechanisms that I could find.
//
// sol::optional doesn't distinguish between nil values and values of the wrong type
// (so we can't provide the user with an error message to tell them they messed up).
//
// std::variant works better, providing an error if the user passes in an arg of the wrong type, but
// the error isn't too helpful and exposes a lot of C++ code which will not help them fix the error.
//
// sol::object lets us check that the user has given the right type, but takes valuable type information
// away from the function's C++ signature, giving us things like void func(sol::object, sol::object, sol::object),
// even if the function's actual expected parameter types are (for example) float, sol::table, SomeUserType.
// 
// This alias is an effort to avoid the above problems.
template <typename ... Ts> using TypeOrNil = std::variant<Ts..., sol::nil_t, sol::object>;


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

