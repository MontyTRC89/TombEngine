#pragma once

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
	void								ClearDisplayStrings() override;
	bool								SetDisplayString(DisplayStringIDType id, UserDisplayString const& ds);

	std::optional<std::reference_wrapper<UserDisplayString>>	GetDisplayString(DisplayStringIDType id);
	bool								ScheduleRemoveDisplayString(DisplayStringIDType id);

	void								ShowString(DisplayString const&, sol::optional<float> nSeconds);

	bool								IsStringDisplaying(DisplayString const& str);

	void								Register(sol::state* state);
};
