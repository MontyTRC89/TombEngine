#pragma once
#include <unordered_map>
#include <optional>
#include "LuaHandler.h"
#include "DisplayString/DisplayString.h"
#include "Strings/ScriptInterfaceStringsHandler.h"

using DisplayStringMap = std::unordered_map<DisplayStringIDType, UserDisplayString>;

class StringsHandler : public LuaHandler, public ScriptInterfaceStringsHandler {
private:
	DisplayStringMap					m_userDisplayStrings{};
	CallbackDrawString					m_callbackDrawSring;

public:
	StringsHandler(sol::state* lua, sol::table & parent);
	void								SetCallbackDrawString(CallbackDrawString cb) override;
	void								ProcessDisplayStrings(float dt) override;
	bool								SetDisplayString(DisplayStringIDType id, UserDisplayString const& ds);

	std::optional<std::reference_wrapper<UserDisplayString>>	GetDisplayString(DisplayStringIDType id);
	bool								ScheduleRemoveDisplayString(DisplayStringIDType id);

	void								ShowString(DisplayString const&, sol::optional<float> nSeconds);

	void								Register(sol::state* state);
};
