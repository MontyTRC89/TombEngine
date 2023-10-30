#include "framework.h"

#include "Scripting/Internal/TEN/Strings/DisplayString/DisplayString.h"
#include "Scripting/Internal/ScriptAssert.h"
#include "Scripting/Internal/ReservedScriptNames.h"
#include "Scripting/Internal/ScriptUtil.h"

/*** A string appearing on the screen.
Can be used for subtitles and "2001, somewhere in Egypt"-style messages.

Uses screen-space coordinates, with x values specifying the number of pixels from the left of the window,
and y values specifying the number of pixels from the top of the window.

Since different players will have different resolutions, you should work in terms of percentages where possible,
and use @{Util.ScreenToPercent|ScreenToPercent} and @{Util.PercentToScreen|PercentToScreen}
when you need to use screen-space coordinates.

@tenclass Strings.DisplayString
@pragma nostrip
*/

UserDisplayString::UserDisplayString(const std::string& key, int x, int y, D3DCOLOR color, const FlagArray& flags, bool isTranslated) :
	m_key(key),
	m_x(x),
	m_y(y),
	m_color(color),
	m_flags(flags),
	m_isTranslated(isTranslated)
{
}

DisplayString::DisplayString()
{
	// We don't ever dereference this pointer; it's just
	// a handy way to get a unique key for a hash map.

	m_id = reinterpret_cast<DisplayStringIDType>(this);
}

/*** Create a DisplayString.
For use in @{ Strings.ShowString | ShowString } and @{ Strings.HideString | HideString }.
@function DisplayString
@tparam string string The string to display or key of the translated string.
@tparam int x X component of the string.
@tparam int y Y component of the string.
@tparam Color color The color of the string.
@tparam bool[opt] translated If false or omitted, the input string argument will be displayed.
If true, the string argument will be the key of a translated string specified in strings.lua. __Default: false__.
@tparam table[opt] flags A table of string display options. Can be empty or omitted. The possible values and their effects are:
	TEN.Strings.DisplayStringOption.CENTER: set the horizontal origin point to the center of the string.
	TEN.Strings.DisplayStringOption.RIGHT: set the horizontal origin point to right of the string.
	TEN.Strings.DisplayStringOption.SHADOW: give the string a small shadow.
	TEN.Strings.DisplayStringOption.BLINK: blink the string.
__Default: empty__
@treturn DisplayString A new DisplayString object.
*/
static std::unique_ptr<DisplayString> CreateString(const std::string& key, int x, int y, ScriptColor color, TypeOrNil<bool> maybeTranslated, TypeOrNil<sol::table> flags)
{
	auto ptr = std::make_unique<DisplayString>();
	auto id = ptr->GetID();

	FlagArray f{};
	if (std::holds_alternative<sol::table>(flags))
	{
		auto tab = std::get<sol::table>(flags);
		for (auto& e : tab)
		{
			auto i = e.second.as<size_t>();
			f[i] = true;
		}
	}
	else if (!std::holds_alternative<sol::nil_t>(flags))
	{
		ScriptAssertF(false, "Wrong argument type for {}.new \"flags\" argument; must be a table or nil.", ScriptReserved_DisplayString);
	}

	bool translated = false;
	if (std::holds_alternative<bool>(maybeTranslated))	
	{
		translated = std::get<bool>(maybeTranslated);
	}
	else if (!std::holds_alternative<sol::nil_t>(maybeTranslated))
	{
		ScriptAssertF(false, "Wrong argument type for {}.new \"translated\" argument; must be a bool or nil.", ScriptReserved_DisplayString);
	}

	UserDisplayString ds{ key, x, y, color, f, translated};

	DisplayString::s_setItemCallback(id, ds);
	return ptr;
}

DisplayString::~DisplayString()
{
	s_removeItemCallback(m_id);
}

void DisplayString::Register(sol::table& parent)
{
	parent.new_usertype<DisplayString>(
		ScriptReserved_DisplayString,
		sol::call_constructor, &CreateString,

		/// Get the display string's color
		// @function DisplayString:GetColor
		// @treturn Color a copy of the display string's color
		ScriptReserved_GetColor, &DisplayString::GetColor,

		/// Set the display string's color 
		// @function DisplayString:SetColor
		// @tparam Color color the new color of the display string 
		ScriptReserved_SetColor, &DisplayString::SetColor,

		/// Get the string key to use. If `isTranslated` is true when @{ DisplayString }
		// is called, this will be the string key for the translation that will be displayed.
		// If false or omitted, this will be the string that's displayed.
		// @function DisplayString:GetKey
		// @treturn string a string
		ScriptReserved_GetKey, &DisplayString::GetKey, 

		/// Set the string key to use. If `isTranslated` is true when @{ DisplayString }
		// is called, this will be the string key for the translation that will be displayed.
		// If false or omitted, this will be the string that's displayed.
		// @function DisplayString:SetKey
		// @tparam string string the new key for the display string 
		ScriptReserved_SetKey, &DisplayString::SetKey, 


		/// Set the position of the string.
		// Screen-space coordinates are expected.
		// @function DisplayString:SetPosition
		// @tparam int x X component.
		// @tparam int y Y component.
		ScriptReserved_SetPosition, &DisplayString::SetPos,

		/// Get the position of the string.
		// Screen-space coordinates are returned.
		// @function DisplayString:GetPosition
		// @treturn int x X component.
		// @treturn int y Y component.
		ScriptReserved_GetPosition, &DisplayString::GetPos,

		/// Set the display string's flags 
		// @function DisplayString:SetFlags
		// @tparam table table the new table with display flags options
		// @usage
		// local varDisplayString = DisplayString('example string', 0, 0, Color(255, 255, 255), false)
		// possible values:
		// varDisplayString:SetFlags({})
		// varDisplayString:SetFlags({ TEN.Strings.DisplayStringOption.SHADOW })
		// varDisplayString:SetFlags({ TEN.Strings.DisplayStringOption.CENTER })
		// varDisplayString:SetFlags({ TEN.Strings.DisplayStringOption.SHADOW, TEN.Strings.DisplayStringOption.CENTER })
		// -- When passing a table to a function, you can omit the parentheses
		// varDisplayString:SetFlags{ TEN.Strings.DisplayStringOption.CENTER }
		ScriptReserved_SetFlags, &DisplayString::SetFlags,

		/// Set translated parameter of the string
		// @function DisplayString:SetTranslated
		// @tparam bool shouldTranslate if true, the string's key will be used as the key for the translation that will be displayed.
		// If false, the key itself will be displayed
		ScriptReserved_SetTranslated, &DisplayString::SetTranslated);
}

DisplayStringIDType DisplayString::GetID() const
{
	return m_id;
}

void DisplayString::SetPos(int x, int y)
{
	UserDisplayString& displayString = s_getItemCallback(m_id).value();
	displayString.m_x = x;
	displayString.m_y = y;
}

std::tuple<int, int> DisplayString::GetPos() const
{	
	UserDisplayString& displayString = s_getItemCallback(m_id).value();
	return std::make_tuple(displayString.m_x, displayString.m_y);
}
	
void DisplayString::SetColor(const ScriptColor& color)
{
	UserDisplayString& displayString = s_getItemCallback(m_id).value();
	displayString.m_color = color;
	//todo maybe change getItemCallback to return a ref instead? or move its
	//todo UserDisplayString object? and then move back?
	//s_addItemCallback(m_id, s);
}

ScriptColor DisplayString::GetColor() 
{
	UserDisplayString& displayString = s_getItemCallback(m_id).value();
	return displayString.m_color;
}

void DisplayString::SetKey(const std::string& key)
{
	UserDisplayString& displayString = s_getItemCallback(m_id).value();
	displayString.m_key = key;
}

std::string DisplayString::GetKey() const
{
	UserDisplayString& displayString = s_getItemCallback(m_id).value();
	return displayString.m_key;
}

void DisplayString::SetFlags(const sol::table& flags) 
{
	UserDisplayString& displayString = s_getItemCallback(m_id).value();

	auto flagArray = FlagArray {};
	for (const auto& val : flags)
	{
		auto i = val.second.as<size_t>();
		flagArray[i] = true;
	}

	displayString.m_flags = flagArray;
}

void DisplayString::SetTranslated(bool isTranslated)
{
	UserDisplayString& displayString = s_getItemCallback(m_id).value();
	TENLog(isTranslated ? "Translated string " : "Untranslated string " + std::to_string(isTranslated), LogLevel::Info);
	displayString.m_isTranslated = isTranslated;
}

SetItemCallback DisplayString::s_setItemCallback = [](DisplayStringIDType, UserDisplayString)
{
	std::string err = "\"Set string\" callback is not set.";
	throw TENScriptException(err);
	return false;
};

// This is called by a destructor (or will be if we forget to assign it during a refactor)
// and destructors "must never throw", so we terminate instead.
RemoveItemCallback DisplayString::s_removeItemCallback = [](DisplayStringIDType)
{
	TENLog("\"Remove string\" callback is not set.", LogLevel::Error);
	std::terminate();
	return false;
};

GetItemCallback DisplayString::s_getItemCallback = [](DisplayStringIDType)
{
	std::string err = "\"Get string\" callback is not set.";
	throw TENScriptException(err);
	return std::nullopt;
};
