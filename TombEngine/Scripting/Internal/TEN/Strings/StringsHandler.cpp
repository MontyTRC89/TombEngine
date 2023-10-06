#include "framework.h"
#include "StringsHandler.h"

#include "Scripting/Internal/ScriptAssert.h"
#include "Scripting/Internal/TEN/Flow/FlowHandler.h"
#include "Renderer/Renderer11Enums.h"
#include "Scripting/Internal/ReservedScriptNames.h"

/***
On-screen strings.
@tentable Strings 
@pragma nostrip
*/

StringsHandler::StringsHandler(sol::state* lua, sol::table& parent) :
	LuaHandler{ lua }
{
	sol::table table_strings{ m_lua->lua_state(), sol::create };
	parent.set(ScriptReserved_Strings, table_strings);

/***
Show some text on-screen.
@tparam DisplayString str the string object to draw
@function ShowString
@tparam float time the time in seconds for which to show the string.
If not given, the string will have an "infinite" life, and will show
until @{HideString} is called or until the level is finished.
Default: nil (i.e. infinite)
*/
	table_strings.set_function(ScriptReserved_ShowString, &StringsHandler::ShowString, this);

/***
Hide some on-screen text.
@function HideString
@tparam DisplayString str the string object to hide. Must previously have been shown
with a call to @{ShowString}, or this function will have no effect.
*/
	table_strings.set_function(ScriptReserved_HideString, [this](DisplayString const& s) {ShowString(s, 0.0f); });

/***
Checks if the string is shown
@function IsStringDisplaying
@tparam DisplayString str the string object to be checked
@treturn bool true if it is shown, false if it is hidden
*/
	table_strings.set_function(ScriptReserved_IsStringDisplaying, &StringsHandler::IsStringDisplaying, this);

	DisplayString::Register(table_strings);
	DisplayString::SetCallbacks(
		[this](auto && ... param) { return SetDisplayString(std::forward<decltype(param)>(param)...); },
		[this](auto && ... param) { return ScheduleRemoveDisplayString(std::forward<decltype(param)>(param)...); },
		[this](auto && ... param) { return GetDisplayString(std::forward<decltype(param)>(param)...); });
	
	MakeReadOnlyTable(table_strings, ScriptReserved_DisplayStringOption, DISPLAY_STRING_OPTION_NAMES);
}

std::optional<std::reference_wrapper<UserDisplayString>> StringsHandler::GetDisplayString(DisplayStringIDType id)
{
	auto it = m_userDisplayStrings.find(id);
	if (std::cend(m_userDisplayStrings) == it)
		return std::nullopt;

	return std::ref(m_userDisplayStrings.at(id));
}

bool StringsHandler::ScheduleRemoveDisplayString(DisplayStringIDType id)
{
	auto it = m_userDisplayStrings.find(id);
	if (std::cend(m_userDisplayStrings) == it)
		return false;

	it->second.m_deleteWhenZero = true;
	return true;
}

void StringsHandler::SetCallbackDrawString(CallbackDrawString cb)
{
	m_callbackDrawSring = cb;
}

bool StringsHandler::SetDisplayString(DisplayStringIDType id, const UserDisplayString& displayString)
{
	return m_userDisplayStrings.insert_or_assign(id, displayString).second;
}

void StringsHandler::ShowString(const DisplayString& str, sol::optional<float> numSeconds)
{
	auto it = m_userDisplayStrings.find(str.GetID());
	it->second.m_timeRemaining = numSeconds.value_or(0.0f);
	it->second.m_isInfinite = !numSeconds.has_value();
}

bool StringsHandler::IsStringDisplaying(const DisplayString& displayString)
{
	auto it = m_userDisplayStrings.find(displayString.GetID());
	bool isAtEndOfLife = (0.0f >= it->second.m_timeRemaining);
	return (it->second.m_isInfinite ? isAtEndOfLife : !isAtEndOfLife);
}

void StringsHandler::ProcessDisplayStrings(float deltaTime)
{
	auto it = std::begin(m_userDisplayStrings);
	while (it != std::end(m_userDisplayStrings))
	{
		auto& str = it->second;
		bool endOfLife = 0.0f >= str.m_timeRemaining;
		if (str.m_deleteWhenZero && endOfLife)
		{
			ScriptAssertF(!str.m_isInfinite, "The infinite string {} (key \"{}\") went out of scope without being hidden.", it->first, str.m_key);
			it = m_userDisplayStrings.erase(it);
		}
		else
		{
			if (!endOfLife || str.m_isInfinite)
			{
				auto cstr = str.m_isTranslated ? g_GameFlow->GetString(str.m_key.c_str()) : str.m_key.c_str();
				int flags = 0;

				if (str.m_flags[(size_t)DisplayStringOptions::Center])
					flags |= PRINTSTRING_CENTER;

				if (str.m_flags[(size_t)DisplayStringOptions::Right])
					flags |= PRINTSTRING_RIGHT;

				if (str.m_flags[(size_t)DisplayStringOptions::Outline])
					flags |= PRINTSTRING_OUTLINE;

				if (str.m_flags[(size_t)DisplayStringOptions::Blink])
					flags |= PRINTSTRING_BLINK;

				m_callbackDrawSring(cstr, str.m_color, str.m_x, str.m_y, flags);

				str.m_timeRemaining -= deltaTime;
			}

			++it;
		}
	}
}

void StringsHandler::ClearDisplayStrings()
{
	m_userDisplayStrings.clear();
}
