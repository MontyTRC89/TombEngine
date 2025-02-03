#pragma once

#include "Scripting/Internal/ScriptAssert.h"

template <typename S> using callbackSetName = std::function<bool(const std::string&, S identifier)>;
using callbackRemoveName = std::function<bool(const std::string&)>;

// Use the "curiously recurring template pattern" to allow classes to inherit static members and functions.
// T is the class that will both derive and instantiate this base class. S is the type used inside GameScriptWhateverInfo
// to actually reference the underlying TombEngine struct.
template <typename T, class S> class NamedBase
{
public:
	static void SetNameCallbacks(callbackSetName<S> cbs, callbackRemoveName cbr)
	{
		_callbackSetName = cbs;
		_callbackRemoveName = cbr;
	}

protected:
	static callbackSetName<S> _callbackSetName;
	static callbackRemoveName _callbackRemoveName;
};

// Default callbacks.
template <typename T, typename S> callbackSetName<S> NamedBase<T, S>::_callbackSetName = [](const std::string& name, S identifier)
{
	auto err = std::string("\"Set Name\" callback is not set.");
	throw TENScriptException(err);
	return false;
};

// This could potentially be called by the GameScriptItemInfo destructor, and thus cannot throw.
template <typename T, typename S> callbackRemoveName NamedBase<T, S>::_callbackRemoveName = [](const std::string& name)
{
	TENLog("\"Remove Name\" callback is not set.", LogLevel::Error);
	std::terminate();
	return false;
};
