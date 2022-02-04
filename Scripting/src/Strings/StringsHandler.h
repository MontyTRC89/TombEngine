#pragma once
#include <unordered_map>
#include <optional>
#include "LuaHandler.h"
#include "DisplayString/GameScriptDisplayString.h"
#include "Scripting/Strings/ScriptInterfaceStringsHandler.h"

using DisplayStringMap = std::unordered_map<DisplayStringIDType, UserDisplayString>;

class StringsHandler : public LuaHandler, public ScriptInterfaceStringsHandler {
private:
	DisplayStringMap					m_userDisplayStrings{};
	CallbackDrawString					m_callbackDrawSring;

public:
	StringsHandler(sol::state* lua);
	void								SetCallbackDrawString(CallbackDrawString cb) override;
	void								ProcessDisplayStrings(float dt) override;
	bool								SetDisplayString(DisplayStringIDType id, UserDisplayString const& ds);

	std::optional<std::reference_wrapper<UserDisplayString>>	GetDisplayString(DisplayStringIDType id);
	bool								ScheduleRemoveDisplayString(DisplayStringIDType id);

	void								ShowString(GameScriptDisplayString const&, sol::optional<float> nSeconds);

	void								Register(sol::state* state);
};
