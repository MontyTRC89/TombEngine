#include "framework.h"

#include "DisplayString.h"
#include "ScriptAssert.h"
#include "ReservedScriptNames.h"
#include "ScriptUtil.h"

/*** A string appearing on the screen.
Can be used for subtitles and "2001, somewhere in Egypt"-style messages.

Uses screen-space coordinates, with x values specifying the number of pixels from the left of the window,
and y values specifying the number of pixels from the top of the window.

Since different players will have different resolutions, you should work in terms of percentages where possible,
and use @{Misc.ScreenToPercent|ScreenToPercent} and @{Misc.PercentToScreen|PercentToScreen}
when you need to use screen-space coordinates.

@tenclass Strings.DisplayString
@pragma nostrip
*/

UserDisplayString::UserDisplayString(std::string const& key, int x, int y, D3DCOLOR col, FlagArray const & flags, bool translated) :
	m_key{ key },
	m_x{ x },
	m_y{ y },
	m_color{ col },
	m_flags{ flags },
	m_isTranslated{ translated }
{
}

DisplayString::DisplayString()
{
	// We don't ever dereference this pointer; it's just
	// a handy way to get a unique key for a hash map.

	m_id = reinterpret_cast<DisplayStringIDType>(this);
}

/*** Create a DisplayString.
For use in @{Strings.ShowString|ShowString} and @{Strings.HideString|HideString}.
@function DisplayString
@tparam string str string to print or key of translated string
@tparam int x x-coordinate of top-left of string (or the center if DisplayStringOption.CENTER is given)
@tparam int y y-coordinate of top-left of string (or the center if DisplayStringOption.CENTER is given)
@tparam Color color the color of the text
@tparam bool translated if false or omitted, the str argument will be printed.
If true, the str argument will be the key of a translated string specified in
strings.lua. __Default: false__.
@tparam table flags a table of display options. Can be empty or omitted. The possible values and their effects are...
	TEN.Strings.DisplayStringOption.CENTER -- see x and y parameters
	TEN.Strings.DisplayStringOption.SHADOW -- will give the text a small shadow
__Default: empty__
@return A new DisplayString object.
*/
static std::unique_ptr<DisplayString> CreateString(std::string const & key, int x, int y, ScriptColor col, TypeOrNil<bool> maybeTranslated, TypeOrNil<sol::table> flags)
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
		translated = std::get<bool>(maybeTranslated);
	else if (!std::holds_alternative<sol::nil_t>(maybeTranslated))
		ScriptAssertF(false, "Wrong argument type for {}.new \"translated\" argument; must be a bool or nil.", ScriptReserved_DisplayString);

	UserDisplayString ds{ key, x, y, col, f, translated};

	DisplayString::s_setItemCallback(id, ds);
	return ptr;
}

DisplayString::~DisplayString()
{
	s_removeItemCallback(m_id);
}

void DisplayString::Register(sol::table & parent)
{
	parent.new_usertype<DisplayString>(
		ScriptReserved_DisplayString,
		ScriptReserved_New, &CreateString,
		sol::call_constructor, &CreateString,

		/// Get the display string's color
		// @function GetColor
		// @treturn Color a copy of the display string's color
		ScriptReserved_GetColor, &DisplayString::GetCol,

		/// Set the display string's color 
		// @function SetColor
		// @tparam Color color the new color of the display string 
		ScriptReserved_SetColor, &DisplayString::SetCol,

		/// Get the string key to use. If `translated` is true when @{DisplayString}
		// is called, this will be the string key for the translation that will be displayed.
		// If false or omitted, this will be the string that's displayed.
		// @treturn String a string
		ScriptReserved_GetKey, &DisplayString::GetKey, 

		/// Set the string key to use. If `translated` is true when @{DisplayString}
		// is called, this will be the string key for the translation that will be displayed.
		// If false or omitted, this will be the string that's displayed.
		// @function SetKey
		// @tparam String string the new key for the display string 
		ScriptReserved_SetKey, &DisplayString::SetKey, 


		/// Set the position of the string.
		// Screen-space coordinates are expected.
		// @function DisplayString:SetPosition
		// @tparam int x x-coordinate of the string
		// @tparam int y y-coordinate of the string
		ScriptReserved_SetPosition, &DisplayString::SetPos,

		/// Get the position of the string.
		// Screen-space coordinates are returned.
		// @function DisplayString:GetPosition
		// @treturn int x x-coordinate of the string
		// @treturn int y y-coordinate of the string
		ScriptReserved_GetPosition, &DisplayString::GetPos
	);
}

DisplayStringIDType DisplayString::GetID() const
{
	return m_id;
}

void DisplayString::SetPos(int x, int y)
{
	UserDisplayString& s = s_getItemCallback(m_id).value();
	s.m_x = x;
	s.m_y = y;
}

std::tuple<int, int> DisplayString::GetPos() const
{	
	UserDisplayString& s = s_getItemCallback(m_id).value();
	return std::make_tuple(s.m_x, s.m_y);
}
	
void DisplayString::SetCol(ScriptColor const & col)
{
	UserDisplayString& s = s_getItemCallback(m_id).value();
	s.m_color = col;
	//todo maybe change getItemCallback to return a ref instead? or move its
	//todo UserDisplayString object? and then move back?
	//s_addItemCallback(m_id, s);
}

ScriptColor DisplayString::GetCol() 
{
	UserDisplayString& s = s_getItemCallback(m_id).value();
	return s.m_color;
}

void DisplayString::SetKey(std::string const & key)
{
	UserDisplayString& s = s_getItemCallback(m_id).value();
	s.m_key = key;
}

std::string DisplayString::GetKey() const
{
	UserDisplayString& s = s_getItemCallback(m_id).value();
	return s.m_key;
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

