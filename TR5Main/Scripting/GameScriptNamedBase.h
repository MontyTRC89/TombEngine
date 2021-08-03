#pragma once
#include <functional>
#include <string>

extern bool const WarningsAsErrors;

#define index_error_maker(CPP_TYPE, LUA_CLASS_NAME) [](CPP_TYPE & item, sol::object key) \
{ \
	if (WarningsAsErrors) \
	{ \
		std::string err = "Attempted to read non-existant var \"" + key.as<std::string>() + "\" from " + LUA_CLASS_NAME; \
		throw std::runtime_error(err); \
	} \
}

template <typename S> using callbackSetName = std::function<bool(std::string const&, S identifier)>;
using callbackRemoveName = std::function<bool(std::string const&)>;

// Use the "curiously recurring template pattern" to allow classes to inherit static members and functions.
// T is the class that will both derive and instantiate this base class. S is the type used inside GameScriptWhateverInfo
// to actually reference the underlying TombEngine struct.
template <typename T, class S> class GameScriptNamedBase
{
public:
	static void SetNameCallbacks(callbackSetName<S> cbs, callbackRemoveName cbr)
	{
		s_callbackSetName = cbs;
		s_callbackRemoveName = cbr;
	}

protected:
	static callbackSetName<S> s_callbackSetName;
	static callbackRemoveName s_callbackRemoveName;
};


// default callbacks
template <typename T, typename S> callbackSetName<S> GameScriptNamedBase<T, S>::s_callbackSetName = [](std::string const& n, S identifier) {
		std::string err = "\"Set Name\" callback is not set.";
		if (WarningsAsErrors)
		{
			throw TENScriptException(err);
		}
		return false;
	};

template <typename T, typename S> callbackRemoveName GameScriptNamedBase<T, S>::s_callbackRemoveName = [](std::string const& n) {
		std::string err = "\"Remove Name\" callback is not set.";
		if (WarningsAsErrors)
		{
			throw TENScriptException(err);
		}
		return false;

	};

